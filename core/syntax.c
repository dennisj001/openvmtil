
#include "../includes/cfrtil.h"

// this is just a prelim prototype idea

void
_Namespace_Do_C_Type ( Namespace * ns )
{
    Context * cntx = _Q_->OVT_Context ;
    Lexer * lexer = cntx->Lexer0 ;
    byte * token1, *token2 ;
    if ( ( ! Compiling ) && ! GetState ( cntx->Compiler0, LC_ARG_PARSING ) )
    {
        _CfrTil_InitSourceCode_WithName ( ns->Name ) ;
    }
    if ( ( ! _Q_->OVT_LC ) && GetState ( cntx, C_SYNTAX ) && ( cntx->System0->IncludeFileStackNumber ) ) //&& ( strlen ( cntx->ReadLiner0->InputLine ) != strlen ( ns->Name ) ) )
    {
        // ?? this could be screwing up other things and adds an unnecessary level of complexity
        // for parsing C functions 
        token1 = _Lexer_NextNonDebugTokenWord ( lexer ) ;
        if ( token1 [0] != '"' )
        {
            token2 = Lexer_PeekNextNonDebugTokenWord ( lexer ) ;
            if ( token2 [0] == '(' )
            {
                _DataStack_Push ( ( int32 ) token1 ) ; // token1 is the function name 
                goto done ; //skip DoNamespace
            }
        }
        _CfrTil_AddTokenToHeadOfPeekTokenList ( token1 ) ; // add ahead of token2 :: ?? this could be screwing up other things and adds an unnecessary level of complexity
    }
    _Namespace_DoNamespace ( ns ) ;

done:
    Finder_SetQualifyingNamespace ( cntx->Finder0, ns ) ; // _Lexer_NextNonDebugTokenWord clears QualifyingNamespace
}

void
Interpret_DoParenthesizedRValue ( )
{
    Context * cntx = _Q_->OVT_Context ;
    byte * token ;
    Compiler * compiler = cntx->Compiler0 ;
    int32 svcm = GetState ( compiler, COMPILE_MODE ), svclps = GetState ( compiler, C_COMBINATOR_LPAREN ) ;
    int32 svs_c_rhs = GetState ( cntx, C_RHS ) ;
    int32 svs_c_lhs = GetState ( cntx, C_LHS ) ;
    SetState ( cntx, C_RHS, true ) ;

    if ( svclps )
    {
        SetState ( compiler, C_COMBINATOR_LPAREN, false ) ;
        CfrTil_BeginBlock ( ) ;
    }
    else
    {
        while ( 1 )
        {
            token = _Lexer_ReadToken ( cntx->Lexer0, 0 ) ;
            if ( ! strcmp ( ( char* ) token, ";" ) )
            {
                SetState ( cntx, C_LHS, true ) ;
                SetState ( cntx, C_RHS, false ) ;
                break ;
            }
            if ( ! strcmp ( ( char* ) token, "word" ) ) break ;
            if ( ! strcmp ( ( char* ) token, ")" ) )
            {
                if ( svclps )
                {
                    CfrTil_EndBlock ( ) ;
                    SetState ( compiler, C_COMBINATOR_LPAREN, svclps ) ;
                    SetState ( cntx, C_RHS, svs_c_rhs ) ;
                    SetState ( cntx, C_LHS, svs_c_lhs ) ;
                }
                break ;
            }
            _Interpreter_InterpretAToken ( cntx->Interpreter0, token ) ;
        }
    }
    SetState ( compiler, COMPILE_MODE, svcm ) ;
}

void
CfrTil_InterpretNBlocks ( int blocks, int takesLParenFlag )
{
    Context * cntx = _Q_->OVT_Context ;
    Compiler * compiler = cntx->Compiler0 ;
    Interpreter * interp = cntx->Interpreter0 ;
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
        byte * token = Lexer_ReadToken ( cntx->Lexer0 ) ;
        _Interpreter_InterpretAToken ( cntx->Interpreter0, token ) ;
        if ( ( i == 0 ) && ( cntx->Interpreter0->w_Word->CType & LITERAL ) && GetState ( cntx, C_LHS ) )
        {
            // setup for optimization if this literal constant is the loop conditional
            BlockInfo * bi = ( BlockInfo* ) _Stack_Top ( compiler->BlockStack ) ;
            bi->LiteralWord = cntx->Interpreter0->w_Word ;
        }
        nowDepth = Stack_Depth ( compiler->BlockStack ) ;
        if ( ( ( interp->w_Word->Definition == CfrTil_EndBlock ) ) && ( depth == nowDepth ) ) i ++ ;
        else if ( c_syntaxFlag && ( interp->w_Word->Definition == CfrTil_End_C_Block ) && ( depth == nowDepth ) ) i ++ ;
    }
    SetState ( compiler, C_COMBINATOR_LPAREN, saveState ) ;
    SetState ( cntx, C_RHS, false ) ;
    SetState ( cntx, C_LHS, true ) ;
}

