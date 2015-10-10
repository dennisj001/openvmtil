
#include "../includes/cfrtil.h"

// this is just a prelim prototype idea

void
_Namespace_Do_C_Type ( Namespace * ns )
{
    Lexer * lexer = _Q_->OVT_Context->Lexer0 ;
    _Namespace_DoNamespace ( ns ) ;
    if ( ! Compiling ) _CfrTil_InitSourceCode_WithName ( ns->Name ) ;
    if ( ( _Q_->OVT_Context->System0->IncludeFileStackNumber ) && ( strlen ( _Q_->OVT_Context->ReadLiner0->InputLine ) != strlen ( ns->Name ) ) )
    {
        byte * token1, *token2 ;
        token1 = _Lexer_NextNonDebugTokenWord ( lexer ) ;
        if ( token1 [0] != '"' )
        {
            token2 = Lexer_PeekNextNonDebugTokenWord ( lexer ) ;
            if ( token2 [0] == '(' )
            {
                _DataStack_Push ( ( int32 ) token1 ) ; // token1 is the function name 
                return ;
            }
        }
        _CfrTil_AddTokenToHeadOfPeekTokenList ( token1 ) ; // add ahead of token2
    }
}

void
Interpret_DoParenthesizedRValue ( )
{
    byte * token ;
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    int32 svcm = GetState ( compiler, COMPILE_MODE ), svclps = GetState ( compiler, C_COMBINATOR_LPAREN ) ;
    int32 svs_c_rhs = GetState ( _Q_->OVT_Context, C_RHS ) ;
    int32 svs_c_lhs = GetState ( _Q_->OVT_Context, C_LHS ) ;
    SetState ( _Q_->OVT_Context, C_RHS, true ) ;

    if ( svclps )
    {
        SetState ( compiler, C_COMBINATOR_LPAREN, false ) ;
        CfrTil_BeginBlock ( ) ;
    }
    else
    {
        while ( 1 )
        {
            token = _Lexer_ReadToken ( _Q_->OVT_Context->Lexer0, 0 ) ;
            if ( ! strcmp ( ( char* ) token, ";" ) )
            {
                SetState ( _Q_->OVT_Context, C_LHS, true ) ;
                SetState ( _Q_->OVT_Context, C_RHS, false ) ;
                break ;
            }
            if ( ! strcmp ( ( char* ) token, "word" ) ) break ;
            if ( ! strcmp ( ( char* ) token, ")" ) )
            {
                if ( svclps )
                {
                    CfrTil_EndBlock ( ) ;
                    SetState ( compiler, C_COMBINATOR_LPAREN, svclps ) ;
                    SetState ( _Q_->OVT_Context, C_RHS, svs_c_rhs ) ;
                    SetState ( _Q_->OVT_Context, C_LHS, svs_c_lhs ) ;
                }
                break ;
            }
            _Interpreter_InterpretAToken ( _Q_->OVT_Context->Interpreter0, token ) ;
        }
    }
    SetState ( compiler, COMPILE_MODE, svcm ) ;
}

void
CfrTil_InterpretNBlocks ( int blocks, int takesLParenFlag )
{
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    Interpreter * interp = _Q_->OVT_Context->Interpreter0 ;
    int32 i, c_syntaxFlag = 0, nowDepth ;
    int32 depth = Stack_Depth ( compiler->BlockStack ), saveState = GetState ( compiler, C_COMBINATOR_LPAREN ) ;
    if ( Namespace_IsUsing ( "C_Syntax" ) ) //word = Finder_Word_FindUsing ( interp->Finder, "}" ) )
    {
        c_syntaxFlag = 1 ;
    }
    if ( takesLParenFlag )
    {
        SetState ( compiler, C_COMBINATOR_LPAREN, true ) ; // this state could be reset in this loop by a nested 'if' for example
    }
    for ( i = 0 ; i < blocks ; )
    {
        byte * token = Lexer_ReadToken ( _Q_->OVT_Context->Lexer0 ) ;
        _Interpreter_InterpretAToken ( _Q_->OVT_Context->Interpreter0, token ) ;
        if ( ( i == 0 ) && ( _Q_->OVT_Context->Interpreter0->w_Word->CType & LITERAL ) && GetState ( _Q_->OVT_Context, C_LHS ) )
        {
            // setup for optimization if this literal constant is the loop conditional
            BlockInfo * bi = ( BlockInfo* ) _Stack_Top ( compiler->BlockStack ) ;
            bi->LiteralWord = _Q_->OVT_Context->Interpreter0->w_Word ;
        }
        nowDepth = Stack_Depth ( compiler->BlockStack ) ;
        if ( ( ( interp->w_Word->Definition == CfrTil_EndBlock ) ) && ( depth == nowDepth ) ) i ++ ;
        else if ( c_syntaxFlag && ( interp->w_Word->Definition == CfrTil_End_C_Block ) && ( depth == nowDepth ) ) i ++ ;
    }
    SetState ( compiler, C_COMBINATOR_LPAREN, saveState ) ;
    SetState ( _Q_->OVT_Context, C_RHS, false ) ;
    SetState ( _Q_->OVT_Context, C_LHS, true ) ;
}

