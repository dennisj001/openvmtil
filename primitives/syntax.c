#include "../includes/cfrtil.h"

// examples of the use of C_Combinators are in interpreter.cft

void
CfrTil_InfixModeOff ( )
{
    SetState ( _Q_->OVT_Context, INFIX_MODE, false ) ;
}

void
CfrTil_InfixModeOn ( )
{
    SetState ( _Q_->OVT_Context, INFIX_MODE, true ) ;
    //Namespace_DoNamespace ( "Infix" ) ;
}

void
CfrTil_PrefixModeOff ( )
{
    SetState ( _Q_->OVT_Context, PREFIX_MODE, false ) ;
}

void
CfrTil_PrefixModeOn ( )
{
    SetState ( _Q_->OVT_Context, PREFIX_MODE, true ) ;
}

void
CfrTil_C_Syntax_Off ( )
{
    Context * cntx = _Q_->OVT_Context ;
    SetState ( cntx, C_SYNTAX | PREFIX_MODE | INFIX_MODE, false ) ;
    //Namespace_SetAsNotUsing ( "C" ) ;
    Namespace_SetAsNotUsing_MoveToTail ( "C_Combinators" ) ;
    Namespace_SetAsNotUsing_MoveToTail ( "Infix" ) ;
    Namespace_SetAsNotUsing_MoveToTail ( "C_Syntax" ) ;
    //Namespace_DoNamespace ( "Bits" ) ; // TODO : must be a better way
    if ( cntx->Compiler0->C_BackgroundNamespace ) _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
}

void
CfrTil_C_Syntax_On ( )
{
    Context * cntx = _Q_->OVT_Context ;
    cntx->Compiler0->C_BackgroundNamespace = _Namespace_FirstOnUsingList ( ) ;
    SetState ( cntx, C_SYNTAX | PREFIX_MODE | INFIX_MODE, true ) ;
    Namespace_DoNamespace ( "C" ) ;
    Namespace_DoNamespace ( "C_Combinators" ) ;
    Namespace_DoNamespace ( "Infix" ) ;
    Namespace_DoNamespace ( "C_Syntax" ) ;
    _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
}

void
CfrTil_AddressOf ( )
{
    SetState ( _Q_->OVT_Context, ADDRESS_OF_MODE, true ) ; // turned off after one object
}

void
CfrTil_C_Semi ( )
{
    Context * cntx = _Q_->OVT_Context ;
    SetState ( cntx, C_LHS, true ) ;
    SetState ( cntx, C_RHS, false ) ;
    if ( ! Compiling )
    {
        _CfrTil_InitSourceCode ( ) ;
        Compiler_Init ( cntx->Compiler0, 0 ) ;
    }
    else
    {
        cntx->Compiler0->LHS_Word = 0 ;
        if ( cntx->Compiler0->C_BackgroundNamespace ) _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
    }
}

void
CfrTil_End_C_Block ( )
{
    Context * cntx = _Q_->OVT_Context ;
    CfrTil_EndBlock ( ) ;
    if ( ! cntx->Compiler0->BlockLevel )
    {
        _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
        block b = ( block ) _DataStack_Pop ( ) ;
        byte * name = ( byte* ) _DataStack_Pop ( ) ;
        Word * word = _Word_Create ( name ) ;
        _Word ( word, ( byte* ) b ) ;
        SetState ( cntx, C_RHS, false ) ;
        SetState ( cntx, C_LHS, true ) ;
        Set_CompileMode ( false ) ;
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
    Namespace_SetAsNotUsing ( "C_Typedef" ) ;
    _CfrTil_Namespace_InNamespaceSet ( _Q_->OVT_Context->Compiler0->C_BackgroundNamespace ) ;
}

void
CfrTil_C_Infix_Equal ( )
{
    Word * word ;
    Context * cntx = _Q_->OVT_Context ;
    Interpreter * interp = cntx->Interpreter0 ;
    Compiler *compiler = cntx->Compiler0 ;
    SetState ( cntx, C_LHS, false ) ;
    SetState ( cntx, C_RHS, true ) ;
    d1 ( if ( DebugOn ) Compiler_ShowWordStack ( "\nCfrTil_C_Infix_Equal : before interpret until ';' :" ) ) ;
    _Interpret_PrefixFunction_Until_Token ( interp, 0, ";", ( byte* ) " ,\n\r\t" ) ; // TODO : a "," could also delimit in c
    d1 ( if ( DebugOn ) Compiler_ShowWordStack ( "\nCfrTil_C_Infix_Equal : after interpret until ';' :" ) ) ;
    if ( compiler->LHS_Word )// also needs to account for qid
    {
        // this block is an optimization; LHS_Word has should have been already been set up by the compiler
        SetState ( cntx, C_SYNTAX, false ) ; //bypass C_SYNTAX order and interpret the lhs word here in a straight rpn fashion
        //word = compiler->LHS_Word ;
        //word = Compiler_CheckAndCopyDuplicates ( compiler, word, compiler->WordStack ) ;
        _Interpreter_Do_MorphismWord ( interp, compiler->LHS_Word ) ;
        //word = _Q_->OVT_CfrTil->StoreWord ;
        SetState ( cntx, C_SYNTAX, true ) ; // don't forget to turn C_SYNTAX back on

        d1 ( if ( DebugOn ) Compiler_ShowWordStack ( "\nCfrTil_C_Infix_Equal : before 'store' :" ) ) ;

        //word = Compiler_CheckAndCopyDuplicates ( compiler, word, compiler->WordStack ) ;
        _Interpreter_Do_MorphismWord ( interp, _Q_->OVT_CfrTil->StoreWord ) ;
    }
    else
    {
        //word = _Q_->OVT_CfrTil->PokeWord ;
        //word = Compiler_CheckAndCopyDuplicates ( compiler, word, compiler->WordStack ) ;
        _Interpreter_Do_MorphismWord ( interp, _Q_->OVT_CfrTil->PokeWord ) ; // we have an object already set up
    }
    compiler->LHS_Word = 0 ;
    SetState ( cntx, C_LHS, true ) ;
    SetState ( cntx, C_RHS, false ) ;
    if ( ! Compiling ) _CfrTil_InitSourceCode ( ) ;
}


// type : typedef

void
_Type_Create ( )
{
    Context * cntx = _Q_->OVT_Context ;
    Lexer * lexer = cntx->Lexer0 ;
    byte * token = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0 ) ;
    //byte c = Lexer_NextNonDelimiterChar ( lexer ) ;
    if ( token [ 0 ] == '{' )
    {
        Lexer_ReadToken ( lexer ) ;
        CfrTil_TypedefStructBegin ( ) ; //Namespace_ActivateAsPrimary ( ( byte* ) "C_Typedef" ) ;
    }
    _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
}

void
_CfrTil_Typedef ( )
{
    Context * cntx = _Q_->OVT_Context ;
    Namespace * ns = CfrTil_Type_New ( ) ;
    //Namespace * ns = CfrTil_Type_New ( ) ;
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
        alias->CType |= IMMEDIATE ;
    }
    while ( 1 ) ;
}

Namespace *
CfrTil_C_Class_New ( void )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    //return _Class_New ( name, C_TYPE, 0, ( byte* ) _Namespace_Do_C_Type ) ;
    return _DataObject_New ( C_CLASS, name, 0, 0, 0, 0 ) ;
}

void
CfrTil_If_C_Combinator ( )
{
    CfrTil_InterpretNBlocks ( 2, 1 ) ;
    if ( ! _Context_StrCmpNextToken ( _Q_->OVT_Context, "else" ) )
    {
        _CfrTil_GetTokenFromPeekedTokenList ( ) ; // drop the "else" token
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
    Lexer_ReadToken ( _Q_->OVT_Context->Lexer0 ) ; // drop the "while" token
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
    CfrTil_InterpretNBlocks ( 2, 1 ) ;
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

