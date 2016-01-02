
#include "../../includes/cfrtil.h"

int32
_CheckArrayDimensionForVariables_And_UpdateCompilerState ( )
{
    if ( _Readline_CheckArrayDimensionForVariables ( _Q_->OVT_Context->ReadLiner0 ) ) return true ;
    else return false ;
}

// ?!?
// offset is calculated using this formula :
// d1 + d2*(D1) + d3*(D2*D1) + d4*(D3*D2*D1) ...
// where d1, d2, d3, ... are the dimension variables and D1, D2, D3, ... are the Dimension sizes
// ?!? this formula needs a correctness proof but it has been working ?!?
/*
 * This is pretty compilicated so comments are necessary ...
 * What must be dealt with in ArrayBegin :
 * CompileMode or not; Variables in array dimensions or not => 4 combinations
 *      - each dimension produces an offset which is added to any previous AccumulatedOffset (in EAX) which is finally added to the object reference pointer
 * so ...
 *  varaibleFlag = _CheckArrayDimensionForVariables_And_UpdateCompilerState ( )
 * so ... 
 *      generally if a dimension has variables it must be compiled
 *          if no variables then calculate the dimension's offset and increment AccumulatedOffset
 *  if ( variableFlag )
 *  {
 *      Compile the 
 *      ( if ( ! variableFlag ) SetHere at exit to startHere and use AccumulatedOffset mechanism ( AccumulatedOffsetFlag ? ) )
 *  }
 *  else
 *  {
 *      // just interpret it with :
 *      // assume accumulated offset is TOS and keep it there
 *      _DataStack_SetTop ( _DataStack_GetTop ( ) + increment ) ; 
 *  }
 *  set compileMode to necessary state ( saveCompileMode || variableFlag )
 *  rem if we are incoming CompileMode and no variables then we want to interpret - set compileMode false
 *      - so no matter what if ( ! variableFlag ) set compile mode off
 *  Interpret ( token ) 
 *  if ( ! variableFlag ) reset compileMode to incoming state
 *  if ( CompileMode && ( ! variableFlag ) )
 *  {
 *      SetHere ( start ) ;
 *      SetCurrentAccumulatedOffset ( totalIncrement ) ;
 *  }
 */

void
Compile_ArrayDimensionOffset ( Word * word, int32 dimSize, int32 objSize )
{
    if ( *word->W_PtrToValue ) // if ! zero else 
    {
        // assume arrayIndex has just been pushed to TOS
        if ( word->StackPushRegisterCode )
        {
            SetHere ( word->StackPushRegisterCode ) ;
            //_Compile_IMULI ( int32 mod, int32 reg, int32 rm, int32 sib, int32 disp, int32 imm, int32 size )
            _Compile_IMULI ( REG, EAX, EAX, 0, 0, dimSize * objSize, 0 ) ;
            //Compile_ADD( toRegOrMem, mod, reg, rm, sib, disp, isize ) 
            Compile_ADD ( MEM, MEM, EAX, DSP, 0, 0, CELL ) ;
        }
        else
        {
            //_Compile_IMULI ( int32 mod, int32 reg, int32 rm, int32 sib, int32 disp, int32 imm, int32 size )
            _Compile_IMULI ( MEM, EAX, DSP, 0, 0, dimSize * objSize, 0 ) ;
            _Compile_Stack_DropN ( DSP, 1 ) ; // drop the array index
            //Compile_ADD( toRegOrMem, mod, reg, rm, sib, disp, isize ) 
            Compile_ADD ( MEM, MEM, EAX, DSP, 0, 0, CELL ) ;
        }
    }
    else SetHere ( word->Coding ) ; // is 0 don't compile anything for that word
}

void
CfrTil_ArrayBegin ( void )
{
    Interpreter * interpreter = _Q_->OVT_Context->Interpreter0 ;
    Word * baseObject = interpreter->BaseObject ;
    if ( baseObject )
    {
        Namespace * ns = 0 ;
        Word * word = 0 ; // word is used in DEBUG_*
        Compiler *compiler = _Q_->OVT_Context->Compiler0 ;
        Lexer * lexer = _Q_->OVT_Context->Lexer0 ;
        byte * token = lexer->OriginalToken ;
        int32 objSize = 0, arrayIndex, increment = 0, variableFlag = false ;
        int32 saveCompileMode = Compiler_GetState ( compiler, COMPILE_MODE ), *saveWordStackPointer ;

        DEBUG_INIT ;

        if ( interpreter->ObjectNamespace ) ns = TypeNamespace_Get ( interpreter->ObjectNamespace ) ;
        if ( ns && ( ! ns->ArrayDimensions ) ) ns = TypeNamespace_Get ( baseObject ) ;
        if ( ns && ( ! ns->ArrayDimensions ) ) CfrTil_Exception ( ARRAY_DIMENSION_ERROR, QUIT ) ;
        if ( interpreter->ObjectNamespace ) objSize = interpreter->ObjectNamespace->Size ; //_CfrTil_VariableValueGet ( _Q_->OVT_Context->Interpreter0->CurrentClassField, ( byte* ) "size" ) ; 
        if ( ! objSize )
        {
            CfrTil_Exception ( OBJECT_SIZE_ERROR, QUIT ) ;
        }
        //if ( baseObject->StackPushRegisterCode ) SetHere ( baseObject->StackPushRegisterCode ) ;
        //d1 ( if ( DebugOn ) Compiler_ShowWordStack ( "\nArrayBegin : entering : " ) ) ;
        variableFlag = _CheckArrayDimensionForVariables_And_UpdateCompilerState ( ) ;
        Stack_Pop ( _Q_->OVT_Context->Compiler0->WordStack ) ; // pop the initial '['
        saveWordStackPointer = CompilerWordStack->StackPointer ;
        do
        {
            token = Lexer_ReadToken ( lexer ) ;
            word = Finder_Word_FindUsing ( _Q_->OVT_Context->Finder0, token, 0 ) ;
            // only two tokens are interpreted without the system ( _Interpreter_InterpretAToken ) here '[' and ']'
            if ( token [0] == '[' ) // '[' == an "array begin"
            {
                variableFlag = _CheckArrayDimensionForVariables_And_UpdateCompilerState ( ) ;
                if ( variableFlag ) saveWordStackPointer = CompilerWordStack->StackPointer ;
                continue ;
            }
            else if ( token [0] == ']' ) // ']' == an "array end"
            {
                int32 dimNumber = compiler->ArrayEnds, dimSize = 1 ;
                while ( -- dimNumber >= 0 ) // -- : zero based ns->ArrayDimensions
                {
                    dimSize *= ns->ArrayDimensions [ dimNumber ] ; // the parser created and populated this array in _CfrTil_Parse_ClassStructure 
                }
                compiler->ArrayEnds ++ ;
                DEBUG_PRE ;
                if ( variableFlag ) Compile_ArrayDimensionOffset ( _Q_->OVT_Context->CurrentRunWord, dimSize, objSize ) ;
                else
                {
                    // 'little endian' arrays (to maybe coin a term) : first index refers to lowest addresses
                    // d1 + d2*(D1) + d3*(D2*D1) + d4*(D3*D2*D1) ...
                    arrayIndex = _DataStack_Pop ( ) ;
                    increment = arrayIndex * dimSize * objSize ; // keep a running total of 
                    IncrementCurrentAccumulatedOffset ( increment ) ;
                    if ( ! CompileMode ) _DataStack_SetTop ( _DataStack_GetTop ( ) + increment ) ; // after each dimension : in the end we have one lvalue remaining on the stack
                }
                if ( variableFlag ) CompilerWordStack->StackPointer = saveWordStackPointer ; // rem we don't pop this stuff in compile mode for the optimizer so clean up now
                if ( _Context_StrCmpNextToken ( _Q_->OVT_Context, (byte*) "[" ) )
                {
                    break ;
                }
                if ( DebugOn ) Word_PrintOffset ( word, increment, baseObject->AccumulatedOffset ) ;
                DEBUG_SHOW ;
                continue ;
            }
            if ( variableFlag ) Compiler_SetState ( compiler, COMPILE_MODE, true ) ;
            else Compiler_SetState ( compiler, COMPILE_MODE, false ) ;
            _Interpreter_InterpretAToken ( interpreter, token ) ;
            if ( ! CompileMode ) Stack_Pop ( _Q_->OVT_Context->Compiler0->WordStack ) ; // pop all tokens interpreted between '[' and ']'
            Compiler_SetState ( compiler, COMPILE_MODE, saveCompileMode ) ;
            DEBUG_SHOW ;
            d0 ( if ( DebugOn ) Compiler_ShowWordStack ( "\nArrayBegin : after Interpret :" ) ) ;
        }
        while ( 1 ) ;
        if ( DebugOn ) Word_PrintOffset ( word, increment, baseObject->AccumulatedOffset ) ;
        compiler->ArrayEnds = 0 ; // reset for next array word in the current word being compiled
        //interpreter->BaseObject = baseObject ; // nb. : _Q_->OVT_Context->Interpreter0->baseObject is reset by the interpreter by the types of words between array brackets
        if ( CompileMode )
        {
            if ( ! variableFlag ) //Do_ObjectOffset ( baseObject, EAX, 0 ) ;
            {
                SetHere ( baseObject->Coding ) ;
                _Compile_VarLitObj_LValue_To_Reg ( baseObject, EAX ) ;
                _Word_CompileAndRecord_PushEAX ( baseObject ) ;
            }
            else SetState ( baseObject, OPTIMIZE_OFF, true ) ;
        }
        DEBUG_SHOW ;
        Compiler_SetState ( compiler, COMPILE_MODE, saveCompileMode ) ;
    }
}

void
CfrTil_ArrayEnd ( void )
{
    int dbg = 1 ; // for the debugger
}
