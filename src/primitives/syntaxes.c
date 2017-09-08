#include "../include/cfrtil.h"

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
    Ovt_AutoVarOff ( ) ;
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
    Lexer_SetBasicTokenDelimiters ( cntx->Lexer0, ( byte* ) " \n\r\t", COMPILER_TEMP ) ;
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
        CfrTil_InitSourceCode ( _CfrTil_ ) ;
        Compiler_Init ( compiler, 0 ) ;
    }
    else
    {
        compiler->LHS_Word = 0 ;
        if ( compiler->C_BackgroundNamespace ) _CfrTil_Namespace_InNamespaceSet ( compiler->C_BackgroundNamespace ) ;
        //List_Init ( compiler->WordList ) ;
    }
}

#if 0

void
CfrTil_End_C_Block ( )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    int64 numberOfLocals = compiler->NumberOfLocals, numberOfArgs = compiler->NumberOfArgs ; //CfrTil_EndBlock calls Compiler_Init ?? better way to do this ??
    CfrTil_EndBlock ( ) ; // NB. CfrTil_EndBlock changes cntx->Compiler0->BlockLevel
    if ( ! cntx->Compiler0->BlockLevel )
    {
        ///CfrTil_SemiColon ( ) ;
        block b = ( block ) _DataStack_Pop ( ) ;
        Word * word = ( Word* ) _DataStack_Pop ( ) ;
        _Word_InitFinal ( word, ( byte* ) b ) ;
        word->W_NumberOfArgs = numberOfArgs ;
        word->W_NumberOfLocals = numberOfLocals ;
        _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
    }
}
#else

void
CfrTil_End_C_Block ( )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    int64 numberOfLocals = compiler->NumberOfLocals, numberOfArgs = compiler->NumberOfArgs ; //CfrTil_EndBlock calls Compiler_Init ?? better way to do this ??
    CfrTil_EndBlock ( ) ; // NB. CfrTil_EndBlock changes cntx->Compiler0->BlockLevel
    if ( ! cntx->Compiler0->BlockLevel ) _CfrTil_SemiColon ( ) ;
    else
    {
        Word * word = cntx->CurrentlyRunningWord ;
        word->W_NumberOfArgs = numberOfArgs ;
        word->W_NumberOfLocals = numberOfLocals ;
    }
    _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
}
#endif

void
CfrTil_Begin_C_Block ( )
{
    if ( Compiling && GetState ( _Context_, C_SYNTAX ) )
    {
        if ( GetState ( _Compiler_, C_COMBINATOR_PARSING ) ) CfrTil_BeginBlock ( ) ;
        else
        {
            _Interpret_Until_Token ( _Interpreter_, "}", 0 ) ;
        }
    }
}

Namespace *
CfrTil_C_Class_New ( void )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;

    return _DataObject_New ( C_CLASS, 0, name, 0, 0, 0, 0, 0 ) ;
}

void
CfrTil_C_Infix_Equal ( )
{
    _CfrTil_C_Infix_EqualOp ( 0 ) ;
}

void
CfrTil_If_C_Combinator ( )
{
    Word * currentWord0 = Compiler_WordList ( 0 ) ;
    currentWord0->W_SC_ScratchPadIndex = _CfrTil_->SC_ScratchPadIndex ;
    CfrTil_InterpretNBlocks ( 2, 1 ) ;
    if ( ! _Context_StrCmpNextToken ( _Context_, ( byte* ) "else" ) )
    {
        _CfrTil_GetTokenFromTokenList ( _Context_->Lexer0 ) ; // drop the "else" token
        CfrTil_InterpretNBlocks ( 1, 0 ) ;
        _Context_->SC_CurrentCombinator = currentWord0 ;
        CfrTil_TrueFalseCombinator3 ( ) ;
    }
    else
    {
        _Context_->SC_CurrentCombinator = currentWord0 ;
        CfrTil_If2Combinator ( ) ;
    }
}

void
CfrTil_DoWhile_C_Combinator ( )
{
    Word * currentWord0 = Compiler_WordList ( 0 ) ;
    currentWord0->W_SC_ScratchPadIndex = _CfrTil_->SC_ScratchPadIndex ;
    byte * start = Here ;
    CfrTil_InterpretNBlocks ( 1, 0 ) ;
    // just assume 'while' is there 
    Lexer_ReadToken ( _Context_->Lexer0 ) ; // drop the "while" token
    CfrTil_InterpretNBlocks ( 1, 1 ) ;
    _Context_->SC_CurrentCombinator = currentWord0 ;
    if ( ! CfrTil_DoWhileCombinator ( ) )
    {
        SetHere ( start ) ;
    }
}

void
CfrTil_For_C_Combinator ( )
{
    Word * currentWord0 = Compiler_WordList ( 0 ) ;
    currentWord0->W_SC_ScratchPadIndex = _CfrTil_->SC_ScratchPadIndex ;
    CfrTil_InterpretNBlocks ( 4, 1 ) ;
    _Context_->SC_CurrentCombinator = currentWord0 ;
    CfrTil_ForCombinator ( ) ;
}

void
CfrTil_Loop_C_Combinator ( )
{
    Word * currentWord0 = Compiler_WordList ( 0 ) ;
    currentWord0->W_SC_ScratchPadIndex = _CfrTil_->SC_ScratchPadIndex ;
    CfrTil_InterpretNBlocks ( 1, 0 ) ;
    _Context_->SC_CurrentCombinator = currentWord0 ;
    CfrTil_LoopCombinator ( ) ;
}

void
CfrTil_While_C_Combinator ( )
{
    Word * currentWord0 = Compiler_WordList ( 0 ) ;
    currentWord0->W_SC_ScratchPadIndex = _CfrTil_->SC_ScratchPadIndex ;
    byte * start = Here ;
    CfrTil_InterpretNBlocks ( 2, 1 ) ;
    _Context_->SC_CurrentCombinator = currentWord0 ;
    if ( ! CfrTil_WhileCombinator ( ) ) // TODO : has this idea been fully applied to the rest of the code?
    {
        SetHere ( start ) ;
    }
}

void
CfrTil_TypedefStructBegin ( void )
{
    _CfrTil_Parse_ClassStructure ( 0 ) ;
}

void
CfrTil_TypedefStructEnd ( void )
{
    Namespace_SetAsNotUsing ( ( byte* ) "C_Typedef" ) ;
    _CfrTil_Namespace_InNamespaceSet ( _Context_->Compiler0->C_BackgroundNamespace ) ;
}

// infix equal is unique in 'C' because the right hand side of '=' runs to the ';'

// type : typedef

void
_Type_Create ( )
{
    Context * cntx = _Context_ ;
    Lexer * lexer = cntx->Lexer0 ;
    byte * token = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0, 1 ) ;
    //byte c = Lexer_NextNonDelimiterChar ( lexer ) ;
    if ( token [ 0 ] == '{' )
    {

        Lexer_ReadToken ( lexer ) ;
        CfrTil_TypedefStructBegin ( ) ; //Namespace_ActivateAsPrimary ( ( byte* ) "C_Typedef" ) ;
    }
    _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
}

void
_CfrTil_TypeDef ( )
{
    Context * cntx = _Context_ ;
    Namespace * ns = CfrTil_Property_New ( ) ;
    Lexer * lexer = cntx->Lexer0 ;
    Lexer_SetTokenDelimiters ( lexer, ( byte* ) " ,\n\r\t", COMPILER_TEMP ) ;
    do
    {
        byte * token = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0, 1 ) ;
        //byte c = Lexer_NextNonDelimiterChar ( lexer ) ;
        if ( token [ 0 ] == ';' ) break ;
        token = Lexer_ReadToken ( cntx->Lexer0 ) ; //, ( byte* ) " ,\n\r\t" ) ;
        Word * alias = _CfrTil_Alias ( ns, token ) ;
        alias->Lo_List = ns->Lo_List ;
        alias->CProperty |= IMMEDIATE ;
    }

    while ( 1 ) ;
}

