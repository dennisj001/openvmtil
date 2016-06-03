#include "../includes/cfrtil.h"

// examples of the use of C_Combinators are in interpreter.cft

void
CfrTil_InfixModeOff ( )
{
    SetState ( _Context_, INFIX_MODE, false ) ;
}

void
CfrTil_InfixModeOn ( )
{
    SetState ( _Context_, INFIX_MODE, true ) ;
    //Namespace_DoNamespace ( "Infix" ) ;
}

void
CfrTil_PrefixModeOff ( )
{
    SetState ( _Context_, PREFIX_MODE, false ) ;
}

void
CfrTil_PrefixModeOn ( )
{
    SetState ( _Context_, PREFIX_MODE, true ) ;
}

void
CfrTil_C_Syntax_Off ( )
{
    Context * cntx = _Context_ ;
    SetState ( cntx, C_SYNTAX | PREFIX_MODE | INFIX_MODE, false ) ;
    //Namespace_SetAsNotUsing ( "C" ) ;
    Namespace_SetAsNotUsing_MoveToTail ( ( byte* ) "C_Combinators" ) ;
    Namespace_SetAsNotUsing_MoveToTail ( ( byte* ) "Infix" ) ;
    Namespace_SetAsNotUsing_MoveToTail ( ( byte* ) "C_Syntax" ) ;
    //Namespace_DoNamespace ( "Bits" ) ; // TODO : must be a better way
    if ( cntx->Compiler0->C_BackgroundNamespace ) _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
}

void
CfrTil_C_Syntax_On ( )
{
    Context * cntx = _Context_ ;
    cntx->Compiler0->C_BackgroundNamespace = _Namespace_FirstOnUsingList ( ) ;
    SetState ( cntx, C_SYNTAX | PREFIX_MODE | INFIX_MODE, true ) ;
    Namespace_DoNamespace ( ( byte* ) "C" ) ;
    Namespace_DoNamespace ( ( byte* ) "C_Combinators" ) ;
    Namespace_DoNamespace ( ( byte* ) "Infix" ) ;
    Namespace_DoNamespace ( ( byte* ) "C_Syntax" ) ;
    _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
    Lexer_SetBasicTokenDelimiters ( cntx->Lexer0, ( byte* ) " \n\r\t", CONTEXT ) ;
}

void
CfrTil_AddressOf ( )
{
    SetState ( _Context_, ADDRESS_OF_MODE, true ) ; // turned off after one object
}

void
CfrTil_C_Semi ( )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    if ( ! Compiling )
    {
        _CfrTil_InitSourceCode ( ) ;
        Compiler_Init ( compiler, 0 ) ;
    }
    else
    {
        compiler->LHS_Word = 0 ;
        if ( compiler->C_BackgroundNamespace ) _CfrTil_Namespace_InNamespaceSet ( compiler->C_BackgroundNamespace ) ;
        List_Init ( compiler->WordList ) ;
    }
}

void
CfrTil_End_C_Block ( )
{
    Context * cntx = _Context_ ;
    CfrTil_EndBlock ( ) ;
    if ( ! cntx->Compiler0->BlockLevel )
    {
        //CfrTil_SemiColon ( ) ;
        block b = ( block ) _DataStack_Pop ( ) ;
        Word * word = ( Word* ) _DataStack_Pop ( ) ;
        _Word ( word, ( byte* ) b ) ;
        _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
    }
}

void
CfrTil_PropertydefStructBegin ( void )
{
    _CfrTil_Parse_ClassStructure ( 0 ) ;
}

void
CfrTil_PropertydefStructEnd ( void )
{
    Namespace_SetAsNotUsing ( ( byte* ) "C_Propertydef" ) ;
    _CfrTil_Namespace_InNamespaceSet ( _Context_->Compiler0->C_BackgroundNamespace ) ;
}

// infix equal is unique in 'C' because the right hand side of '=' runs to the ';'

void
CfrTil_C_Infix_Equal ( )
#if 0
{
    Context * cntx = _Context_ ;
    Interpreter * interp = cntx->Interpreter0 ;
    Compiler *compiler = cntx->Compiler0 ;
    Word * word = ( Word* ) List_Pop ( compiler->WordList ) ; // adjust for rearranged syntax
    //DebugOn ;
    d0 ( if ( Is_DebugOn ) Compiler_Show_WordList ( "\nCfrTil_C_Infix_Equal : before interpret until ',' or ';' :" ) ) ;
    byte * token = _Interpret_Until_EitherToken ( interp, ( byte* ) ";", ( byte* ) ",", ( byte* ) " \n\r\t" ) ; // TODO : a "," could also delimit in c
    _CfrTil_AddTokenToHeadOfTokenList ( token ) ; // so the callee can check/use or use
    d0 ( if ( Is_DebugOn ) Compiler_Show_WordList ( "\nCfrTil_C_Infix_Equal : after interpret until ';' :" ) ) ;
    //if ( List_Depth ( compiler->WordList ) > 7 )
    {
        if ( compiler->LHS_Word ) // also needs to account for qid
        {
            _DEBUG_SETUP ( compiler->LHS_Word ) ;
            if ( ( word = ( Word* ) Compiler_WordList ( 0 ) ) && word->StackPushRegisterCode ) SetHere ( word->StackPushRegisterCode ) ;
            if ( ! ( compiler->LHS_Word->CProperty & REGISTER_VARIABLE ) )
            {
                if ( word->StackPushRegisterCode ) SetHere ( word->StackPushRegisterCode ) ;
                if ( GetState ( cntx->Compiler0, DOING_C_TYPE ) )
                {
                    int32 value = ( int32 ) compiler->LHS_Word->W_PtrToValue ;
                    _Compile_Move_Literal_Immediate_To_Reg ( ECX, ( int32 ) value ) ;
                }
                else _Compile_GetVarLitObj_LValue_To_Reg ( compiler->LHS_Word, ECX ) ;
                // this block is an optimization; LHS_Word has should have been already been set up by the compiler
                _Compile_Move_Reg_To_Rm ( ECX, EAX, 0 ) ;
            }
            else
            {
                if ( compiler->LHS_Word->RegToUse != EAX ) _Compile_Move_Reg_To_Rm ( compiler->LHS_Word->RegToUse, EAX, 0 ) ;
            }
        }
        else
        {
            word = _Q_->OVT_CfrTil->PokeWord ;
            _DEBUG_SETUP ( word ) ;
            SetState ( _Debugger_, DEBUG_SHTL_OFF, true ) ;
            _Interpreter_Do_MorphismWord ( interp, word, - 1 ) ; // we have an object already set up
        }
    }
#if 0    
    else
    {
        _DEBUG_SETUP ( word ) ;
        _Compiler_WordList_PushWord ( _Context_->Compiler0, word ) ;
        //SetState ( _Debugger_, DEBUG_SHTL_OFF, true ) ;
        CfrTil_Poke ( ) ;
    }
#endif    
    List_Init ( compiler->WordList ) ;
    DEBUG_SHOW ;
    compiler->LHS_Word = 0 ;
    if ( ! Compiling ) _CfrTil_InitSourceCode ( ) ;
    SetState ( _Debugger_, DEBUG_SHTL_OFF, false ) ;
}
#elif 0
{
    Context * cntx = _Context_ ;
    Interpreter * interp = cntx->Interpreter0 ;
    Compiler *compiler = cntx->Compiler0 ;
    Word * word = ( Word* ) List_Pop ( compiler->WordList ) ; // adjust for rearranged syntax
    //DebugOn ;
    d0 ( if ( Is_DebugOn ) Compiler_Show_WordList ( "\nCfrTil_C_Infix_Equal : before interpret until ',' or ';' :" ) ) ;
    byte * token = _Interpret_Until_EitherToken ( interp, ( byte* ) ";", ( byte* ) ",", ( byte* ) " \n\r\t" ) ; // TODO : a "," could also delimit in c
    _CfrTil_AddTokenToHeadOfTokenList ( token ) ; // so the callee can check/use or use
    d0 ( if ( Is_DebugOn ) Compiler_Show_WordList ( "\nCfrTil_C_Infix_Equal : after interpret until ';' :" ) ) ;
    //if ( List_Depth ( compiler->WordList ) > 7 )
    {
        if ( compiler->LHS_Word ) // also needs to account for qid
        {
            _DEBUG_SETUP ( compiler->LHS_Word ) ;
            if ( ( word = ( Word* ) Compiler_WordList ( 0 ) ) && word->StackPushRegisterCode ) SetHere ( word->StackPushRegisterCode ) ;
            if ( ! ( compiler->LHS_Word->CProperty & REGISTER_VARIABLE ) )
            {
                if ( word->StackPushRegisterCode ) SetHere ( word->StackPushRegisterCode ) ;
                if ( GetState ( cntx->Compiler0, DOING_C_TYPE ) )
                {
                    int32 value = ( int32 ) compiler->LHS_Word->W_PtrToValue ;
                    _Compile_Move_Literal_Immediate_To_Reg ( ECX, ( int32 ) value ) ;
                }
                else _Compile_GetVarLitObj_LValue_To_Reg ( compiler->LHS_Word, ECX ) ;
                // this block is an optimization; LHS_Word has should have been already been set up by the compiler
                _Compile_Move_Reg_To_Rm ( ECX, EAX, 0 ) ;
            }
            else
            {
                if ( compiler->LHS_Word->RegToUse != EAX ) _Compile_Move_Reg_To_Rm ( compiler->LHS_Word->RegToUse, EAX, 0 ) ;
            }
        }
        else
        {
            word = _Q_->OVT_CfrTil->PokeWord ;
            _DEBUG_SETUP ( word ) ;
            SetState ( _Debugger_, DEBUG_SHTL_OFF, true ) ;
            _Interpreter_Do_MorphismWord ( interp, word, - 1 ) ; // we have an object already set up
        }
    }
#if 0    
    else
    {
        _DEBUG_SETUP ( word ) ;
        _Compiler_WordList_PushWord ( _Context_->Compiler0, word ) ;
        //SetState ( _Debugger_, DEBUG_SHTL_OFF, true ) ;
        CfrTil_Poke ( ) ;
    }
#endif    
    List_Init ( compiler->WordList ) ;
    DEBUG_SHOW ;
    compiler->LHS_Word = 0 ;
    if ( ! Compiling ) _CfrTil_InitSourceCode ( ) ;
    SetState ( _Debugger_, DEBUG_SHTL_OFF, false ) ;
}

#else
{
    Context * cntx = _Context_ ;
    Interpreter * interp = cntx->Interpreter0 ;
    Compiler *compiler = cntx->Compiler0 ;
    Word * word = ( Word* ) List_Pop ( compiler->WordList ), *lhsWord = compiler->LHS_Word ; // adjust for rearranged syntax
    int32 disp ;
    //DebugOn ;
    d0 ( if ( Is_DebugOn ) Compiler_Show_WordList ( "\nCfrTil_C_Infix_Equal : before interpret until ',' or ';' :" ) ) ;
    byte * token = _Interpret_Until_EitherToken ( interp, ( byte* ) ";", ( byte* ) ",", ( byte* ) " \n\r\t" ) ; // TODO : a "," could also delimit in c
    _CfrTil_AddTokenToHeadOfTokenList ( token ) ; // so the callee can check/use or use
    d0 ( if ( Is_DebugOn ) Compiler_Show_WordList ( "\nCfrTil_C_Infix_Equal : after interpret until ';' :" ) ) ;
    //if ( List_Depth ( compiler->WordList ) > 7 )
    {
        if ( lhsWord ) // also needs to account for qid
        {
            _DEBUG_SETUP ( lhsWord ) ;
            if ( ( word = ( Word* ) Compiler_WordList ( 0 ) ) && word->StackPushRegisterCode ) SetHere ( word->StackPushRegisterCode ) ;
            if ( ! ( lhsWord->CProperty & REGISTER_VARIABLE ) )
            {
                if ( word->StackPushRegisterCode ) SetHere ( word->StackPushRegisterCode ) ;
                if ( GetState ( cntx->Compiler0, DOING_C_TYPE ) )
                {
                    int32 value = ( int32 ) lhsWord->W_PtrToValue ;
                    _Compile_Move_Literal_Immediate_To_Reg ( ECX, ( int32 ) value ) ;
                    _Compile_Move_Reg_To_Rm ( ECX, EAX, 0 ) ;
                    goto done ;
                }
                else if ( lhsWord->CProperty & ( NAMESPACE_VARIABLE ) )
                {
                    _Compile_Move_Literal_Immediate_To_Reg ( ECX, ( int32 ) lhsWord->W_PtrToValue ) ;
                    _Compile_Move_Reg_To_Rm ( ECX, EAX, 0 ) ;
                    goto done ;
                }
                else if ( lhsWord->CProperty & LOCAL_VARIABLE ) disp = LocalVarIndex_Disp ( LocalVarOffset ( lhsWord ) ) ;
                else if ( lhsWord->CProperty & PARAMETER_VARIABLE ) disp = LocalVarIndex_Disp ( ParameterVarOffset ( lhsWord ) ) ;
                _Compile_Move ( MEM, EAX, FP, 0, disp ) ; //LocalVarIndex_Disp ( LocalVarOffset ( word ) ) ;
            }
            else
            {
                if ( lhsWord->RegToUse != EAX ) _Compile_Move_Reg_To_Rm ( lhsWord->RegToUse, EAX, 0 ) ;
            }
        }
        else
        {
            word = _Q_->OVT_CfrTil->PokeWord ;
            _DEBUG_SETUP ( word ) ;
            SetState ( _Debugger_, DEBUG_SHTL_OFF, true ) ;
            _Interpreter_Do_MorphismWord ( interp, word, - 1 ) ; // we have an object already set up
        }
    }
#if 0    
    else
    {
        _DEBUG_SETUP ( word ) ;
        _Compiler_WordList_PushWord ( _Context_->Compiler0, word ) ;
        //SetState ( _Debugger_, DEBUG_SHTL_OFF, true ) ;
        CfrTil_Poke ( ) ;
    }
#endif    
    done :
    List_Init ( compiler->WordList ) ;
    DEBUG_SHOW ;
    compiler->LHS_Word = 0 ;
    if ( ! Compiling ) _CfrTil_InitSourceCode ( ) ;
    SetState ( _Debugger_, DEBUG_SHTL_OFF, false ) ;
}
#endif

// type : typedef

void
_Property_Create ( )
{
    Context * cntx = _Context_ ;
    Lexer * lexer = cntx->Lexer0 ;
    byte * token = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0 ) ;
    //byte c = Lexer_NextNonDelimiterChar ( lexer ) ;
    if ( token [ 0 ] == '{' )
    {
        Lexer_ReadToken ( lexer ) ;
        CfrTil_PropertydefStructBegin ( ) ; //Namespace_ActivateAsPrimary ( ( byte* ) "C_Propertydef" ) ;
    }
    _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
}

void
_CfrTil_Propertydef ( )
{
    Context * cntx = _Context_ ;
    Namespace * ns = CfrTil_Property_New ( ) ;
    Lexer * lexer = cntx->Lexer0 ;
    Lexer_SetTokenDelimiters ( lexer, ( byte* ) " ,\n\r\t", SESSION ) ;
    do
    {
        byte * token = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0 ) ;
        //byte c = Lexer_NextNonDelimiterChar ( lexer ) ;
        if ( token [ 0 ] == ';' ) break ;
        token = Lexer_ReadToken ( cntx->Lexer0 ) ; //, ( byte* ) " ,\n\r\t" ) ;
        Word * alias = _CfrTil_Alias ( ns, token ) ;
        alias->Lo_List = ns->Lo_List ;
        alias->CProperty |= IMMEDIATE ;
    }
    while ( 1 ) ;
}

Namespace *
CfrTil_C_Class_New ( void )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    return _DataObject_New ( C_CLASS, 0, name, 0, 0, 0, 0, 0 ) ;
}

void
CfrTil_If_C_Combinator ( )
{
    CfrTil_InterpretNBlocks ( 2, 1 ) ;
    if ( ! _Context_StrCmpNextToken ( _Context_, ( byte* ) "else" ) )
    {
        _CfrTil_GetTokenFromTokenList ( _Context_->Lexer0 ) ; // drop the "else" token
        CfrTil_InterpretNBlocks ( 1, 0 ) ;
        CfrTil_TrueFalseCombinator3 ( ) ;
    }
    else CfrTil_If2Combinator ( ) ;
}

void
CfrTil_DoWhile_C_Combinator ( )
{
    byte * start = Here ;
    CfrTil_InterpretNBlocks ( 1, 0 ) ;
    // just assume 'while' is there 
    Lexer_ReadToken ( _Context_->Lexer0 ) ; // drop the "while" token
    CfrTil_InterpretNBlocks ( 1, 1 ) ;
    //CfrTil_DoWhileCombinator ( ) ;
    if ( ! CfrTil_DoWhileCombinator ( ) )
    {
        SetHere ( start ) ;
    }
}

void
CfrTil_For_C_Combinator ( )
{
    CfrTil_InterpretNBlocks ( 4, 1 ) ;
    CfrTil_ForCombinator ( ) ;
}

void
CfrTil_Loop_C_Combinator ( )
{
    CfrTil_InterpretNBlocks ( 1, 0 ) ;
    CfrTil_LoopCombinator ( ) ;
}

void
CfrTil_While_C_Combinator ( )
{
    byte * start = Here ;
    CfrTil_InterpretNBlocks ( 2, 1 ) ;
    if ( ! CfrTil_WhileCombinator ( ) ) // TODO : has this idea been fully applied to the rest of the code?
    {
        SetHere ( start ) ;
    }
}

