
#include "../includes/cfrtil.h"

void
_Compile_C_Call_1_Arg ( byte* function, int32 arg )
{
    _Compile_Esp_Push ( arg ) ;
    _Compile_Call ( function ) ;
    _Compile_Rsp_Drop ( ) ;
}

void
_Compile_CallFunctionWithArg ( byte * function, int32 arg )
{
    _Compile_C_Call_1_Arg ( function, arg ) ;
}

void
_CfrTil_Do_ClassField ( Word * word )
{
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    _Q_->OVT_Context->Interpreter0->ObjectField = word ;
    compiler->ArrayEnds = 0 ;
    uint32 accumulatedAddress ;

    if ( Lexer_IsTokenForwardDotted ( _Q_->OVT_Context->Lexer0 ) ) Finder_SetQualifyingNamespace ( _Q_->OVT_Context->Finder0, word->ClassFieldNamespace ) ;
    if ( CompileMode )
    {
        if ( word->Offset )
        {
            IncrementCurrentAccumulatedOffset ( word->Offset ) ;
        }
    }
    else
    {
        accumulatedAddress = _DataStack_Pop ( ) ;
        accumulatedAddress += word->Offset ;
        if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) && GetState ( _Q_->OVT_Context, C_RHS ) && ( Lexer_NextNonDelimiterChar ( _Q_->OVT_Context->Lexer0 ) != '.' ) ) _Push ( * ( int32* ) accumulatedAddress ) ;
        else _Push ( accumulatedAddress ) ;
    }
    Stack_Pop ( _Q_->OVT_Context->Compiler0->WordStack ) ;
}

// nb. 'word' is the previous word to the '.' (dot) cf. CfrTil_Dot so it can be recompiled, a little different maybe, as an object

void
_CfrTil_Do_Object ( Word * word )
{
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    _Q_->OVT_Context->Interpreter0->BaseObject = word ;
    _Q_->OVT_Context->Interpreter0->ObjectField = TypeNamespace_Get ( word ) ;

    if ( ( ! _Q_->OVT_Context->Finder0->QualifyingNamespace ) && Lexer_IsTokenForwardDotted ( _Q_->OVT_Context->Lexer0 ) ) Finder_SetQualifyingNamespace ( _Q_->OVT_Context->Finder0, word->ContainingNamespace ) ;
    if ( CompileMode )
    {
        byte * saveHere = word->Coding ; // Here : instead overwrite the previous word's Code as Here since 'word' is the word before the '.'
        SetHere ( word->Coding ) ; // we don't need the word's code if compiling -- this is an optimization though
        _Compile_VarConstOrLit_RValue_To_Reg ( word, EAX ) ;
        compiler->AddOffsetCodePointer = ( int32* ) Here ;
        Compile_ADDI ( REG, EAX, 0, 0, CELL ) ;
        // !! this block could (and should) be moved to overwrite the above ADDI if the AccumulatedOffset remains 0 !!
        compiler->AccumulatedOffsetPointer = ( int32* ) ( Here - CELL ) ; // offset will be calculated as we go along by ClassFields and Array accesses
        if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) && GetState ( _Q_->OVT_Context, C_RHS ) )
        {
            _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;
        }
        _Word_CompileAndRecord_PushEAX ( word ) ;
        if ( GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) )
        {
            // we sometimes refer to more than one field of the same object, eg. 'this' in a block
            // each reference may be to a different labeled field each with a different offset so we must 
            // create copies of the multiply referenced word to hold the referenced offsets for the optimizer
            // 'word' is the 'baseObject' word. If it is already on the Object word Stack certain optimizations can be made.
            int32 i, stackSize = Stack_Depth ( compiler->ObjectStack ) ;
            for ( i = 0 ; i < stackSize ; i ++ )
            {
                if ( word == ( Word* ) ( Stack_OffsetValue ( compiler->ObjectStack, - i ) ) )
                {
                    word = Word_Copy ( word, SESSION ) ; // especially for "this" so we can use a different Code & AccumulatedOffsetPointer not the existing 
                    //word = wordCopy ; // this line is only needed for debugging
                    break ;
                }
            }
            _Stack_Push ( compiler->ObjectStack, ( int32 ) word ) ; // push the copy, so both are now on the stack
            //d1 ( int sd = Stack_Depth ( compiler->ObjectStack ) ) ;
            word->ObjectCode = saveHere ; // the new word's code pointer
        }
        word->AccumulatedOffset = 0 ;
        compiler->AccumulatedOptimizeOffsetPointer = & word->AccumulatedOffset ;
    }
    else
    {
        compiler->AccumulatedOffsetPointer = 0 ; // ?? used as a flag for non compile mode ??
        _Push ( ( int32 ) word->W_Value ) ;
    }
}

void
CfrTil_Dot ( ) // .
{
    if ( ! _Q_->OVT_Context->Interpreter0->BaseObject )
    {
        SetState ( _Q_->OVT_Context, CONTEXT_PARSING_QUALIFIED_ID, true ) ;
        Word * word = Compiler_PreviousNonDebugWord ( ) ;
        if ( word->CType & ( NAMESPACE | CLASS | DOBJECT ) )
        {
            Finder_SetQualifyingNamespace ( _Q_->OVT_Context->Finder0, word ) ;
        }
        else _CfrTil_Do_Object ( word ) ;
    }
        // for the optimizer ... this can't be optimized
    else Stack_Pop ( _Q_->OVT_Context->Compiler0->WordStack ) ;
}

// rvalue - rhs value - right hand side of '=' - the actual value, used on the right hand side of C statements

_Word_CompileAndRecord_PushEAX ( Word * word )
{
    word->StackPushRegisterCode = Here ; // nb. used! by the rewriting optimizer
    Compile_Stack_PushEAX ( DSP ) ;
}

void
_Do_Literal ( int32 value )
{
    if ( CompileMode )
    {
        //_Compile_MoveImm_To_Reg ( EAX, value, CELL ) ;
        //_Word_CompileAndRecord_PushEAX ( word ) ;
        _Compile_Stack_Push ( DSP, value ) ;
    }

    else _Push ( value ) ;
}

void
Do_LiteralOrConstant ( Word * word )
{
    //if ( word->LType & ( LITERAL | T_LISP_SYMBOL ) ) _Do_Literal ( ( int32 ) word->Lo_Value ) ;
    //else 
    _Do_Literal ( ( int32 ) word->W_Value ) ;
}

void
Do_Variable ( Word * word )
{
    int32 value ;
    if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) && GetState ( _Q_->OVT_Context, C_LHS ) )
    {
        SetState ( _Q_->OVT_Context, C_LHS, false ) ;
        _Q_->OVT_Context->Compiler0->LHS_Word = word ;
    }
    else
    {
        if ( word->CType & VARIABLE )
        {
            if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) && GetState ( _Q_->OVT_Context, C_RHS ) )
            {
                value = ( int32 ) word->W_Value ;
            }
            else value = ( int32 ) & word->W_Value ;
        }
        else value = ( int32 ) word->W_Value ;
        _Do_Literal ( value ) ;
    }
}

void
_Namespace_DoNamespace ( Namespace * ns )
{
    if ( CompileMode && ( Lexer_NextNonDelimiterChar ( _Q_->OVT_Context->Lexer0 ) != '.' ) ) // ( ! GetState ( _Q_->OVT_Context, CONTEXT_PARSING_QUALIFIED_ID ) ) ) //&& ( ! GetState ( _Q_->OVT_Context, C_SYNTAX ) ) )
    {
        _Compile_C_Call_1_Arg ( ( byte* ) _Namespace_DoNamespace, ( int32 ) ns ) ;
    }
    if ( ! Lexer_IsTokenForwardDotted ( _Q_->OVT_Context->Lexer0 ) )
    {
        _Namespace_ActivateAsPrimary ( ns ) ;
    }
    else
    {
        Finder_SetQualifyingNamespace ( _Q_->OVT_Context->Finder0, ns ) ;
    }
}

void
_CfrTil_Do_DObject ( DObject * dobject )
{
    Lexer * lexer = _Q_->OVT_Context->Lexer0 ;
    DObject * ndobject ;
    byte * token ;
    while ( Lexer_IsTokenForwardDotted ( lexer ) )
    {
        Lexer_ReadToken ( lexer ) ; // the '.'
        token = Lexer_ReadToken ( lexer ) ;
        if ( String_Equal ( "prototype", ( char* ) token ) )
        {
            dobject = dobject->ContainingNamespace ;
            continue ;
        }
        if ( ! ( ndobject = _DObject_FindSlot_BottomUp ( dobject, token ) ) )
        {
            _Q_->OVT_CfrTil->InNamespace = dobject ; // put the new dynamic object in the namespace of it's mother object
            dobject = _DObject_NewSlot ( dobject, token, 0 ) ;
        }
        else dobject = ndobject ;
    }
    _Q_->OVT_Context->Interpreter0->ObjectField = TypeNamespace_Get ( dobject ) ;
    if ( CompileMode )
    {
        _Compile_DataStack_Push ( ( int32 ) dobject ) ;
    }
    else
    {
        _Push ( ( int32 ) dobject ) ;
    }
}

void
Do_LocalOrStackVariable ( Word * word )
{
    if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) )
    {
        if ( GetState ( _Q_->OVT_Context, C_RHS ) )
        {
            _Compile_VarConstOrLit_RValue_To_Reg ( word, EAX ) ;
            _Word_CompileAndRecord_PushEAX ( word ) ;
        }
        else if GetState ( _Q_->OVT_Context, C_LHS )
        {
            SetState ( _Q_->OVT_Context, C_LHS, false ) ;
            _Q_->OVT_Context->Compiler0->LHS_Word = word ;
        }
    }
    else
    {
        _Compile_VarConstOrLit_LValue_To_Reg ( word, EAX ) ;
        _Word_CompileAndRecord_PushEAX ( word ) ;
    }
}

// 'Run' :: this is part of runtime in the interpreter/compiler for data objects
// they are compiled to much more optimized native code

void
DataObject_Run ( Word * word )
{
    Debugger * debugger = _Q_->OVT_CfrTil->Debugger0 ;
    int32 dm = GetState ( _Q_->OVT_CfrTil, DEBUG_MODE ) && ( ! GetState ( debugger, DBG_STEPPING ) ) ;
    if ( dm ) _Debugger_PreSetup ( debugger, 0, word ) ;
    if ( ! ( dm && GetState ( debugger, DBG_DONE ) ) )
    {
        if ( word->CType & T_LISP_SYMBOL )
        {
            _Compile_Move_StackN_To_Reg ( EAX, FP, StackVarOffset ( word ) ) ; 
            _Word_CompileAndRecord_PushEAX ( word ) ;
        }
        else if ( word->CType & DOBJECT )
        {
            _CfrTil_Do_DObject ( word ) ;
        }
        else if ( word->CType & OBJECT )
        {
            _CfrTil_Do_Object ( word ) ;
        }
        else if ( word->CType & OBJECT_FIELD )
        {
            _CfrTil_Do_ClassField ( word ) ;
        }
        else if ( word->CType & ( C_TYPE | C_CLASS ) )
        {
            _Namespace_Do_C_Type ( word ) ;
        }
        else if ( word->CType & ( NAMESPACE | CLASS | CLASS_CLONE ) )
        {
            _Namespace_DoNamespace ( word ) ;
        }
        else if ( word->CType & ( LOCAL_VARIABLE | STACK_VARIABLE ) )
        {
            Do_LocalOrStackVariable ( word ) ;
        }
        else if ( word->CType & VARIABLE )
        {
            Do_Variable ( word ) ;
        }
        else if ( word->CType & ( LITERAL | CONSTANT ) )
        {
            Do_LiteralOrConstant ( word ) ;
        }
    }
    if ( dm ) _Debugger_PostShow ( debugger, 0, _Q_->OVT_Context->Interpreter0->w_Word ) ; // a literal could have been created and shown by _Word_Run
}