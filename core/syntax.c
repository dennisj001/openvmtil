
#include "../includes/cfrtil.h"

// this is just a prelim prototype idea

#if 0 // new version

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
        //SetState ( compiler, C_COMBINATOR_LPAREN, false ) ;
        //CfrTil_BeginBlock ( ) ;
        _Interpret_Do_CombinatorLeftParen ( ) ;
    }
    else
    {
        while ( 1 )
        {
            token = _Lexer_ReadToken ( cntx->Lexer0, 0 ) ;
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

int32
_Interpret_Do_CombinatorLeftParen ( )
{
    Context * cntx = _Q_->OVT_Context ;
    byte * token ;
    Compiler * compiler = cntx->Compiler0 ;
    int32 svcm = GetState ( compiler, COMPILE_MODE ), svclps = GetState ( compiler, C_COMBINATOR_LPAREN ) ;
    int32 svs_c_rhs = GetState ( cntx, C_RHS ) ;
    int32 svs_c_lhs = GetState ( cntx, C_LHS ) ;
    SetState ( cntx, C_RHS, true ) ;
    int32 blocksParsed = 0 ;

    while ( 1 )
    {
        token = _Lexer_ReadToken ( cntx->Lexer0, 0 ) ;
        if ( String_Equal ( ( char* ) token, ";" ) )
        {
            CfrTil_EndBlock ( ) ;
            CfrTil_BeginBlock ( ) ;
            SetState ( cntx, C_LHS, true ) ;
            SetState ( cntx, C_RHS, false ) ;
            blocksParsed ++ ;
            continue ;
        }
        //if ( ! strcmp ( ( char* ) token, "word" ) ) break ;
        if ( ! strcmp ( ( char* ) token, ")" ) )
        {
            CfrTil_EndBlock ( ) ;
            SetState ( compiler, C_COMBINATOR_LPAREN, svclps ) ;
            SetState ( cntx, C_RHS, svs_c_rhs ) ;
            SetState ( cntx, C_LHS, svs_c_lhs ) ;
            blocksParsed ++ ;
            break ;
        }
        _Interpreter_InterpretAToken ( cntx->Interpreter0, token ) ;
    }
    SetState ( compiler, COMPILE_MODE, svcm ) ;
    return blocksParsed ;
}

void
CfrTil_InterpretNBlocks ( int blocks, int takesLParenFlag )
{
    Context * cntx = _Q_->OVT_Context ;
    Compiler * compiler = cntx->Compiler0 ;
    Interpreter * interp = cntx->Interpreter0 ;
    int32 blocksParsed = 0, c_syntaxFlag = 0, lpf = 0, nowDepth ;
    int32 depth = Stack_Depth ( compiler->BlockStack ) ; //, saveState = GetState ( compiler, C_COMBINATOR_LPAREN ) ;
    if ( Namespace_IsUsing ( "C_Syntax" ) ) //word = Finder_Word_FindUsing ( interp->Finder, "}" ) )
    {
        c_syntaxFlag = 1 ;
    }
    if ( takesLParenFlag )
    {
        SetState ( compiler, C_COMBINATOR_LPAREN, true ) ; // this state could be reset in this loop by a nested 'if' for example
    }
    for ( blocksParsed = 0 ; blocksParsed < blocks ; )
    {
        byte * token = Lexer_ReadToken ( cntx->Lexer0 ) ;

        if ( String_Equal ( ( char* ) token, "(" ) && takesLParenFlag && ( ! lpf ) )
        {
            blocksParsed += _Interpret_Do_CombinatorLeftParen ( ) ;
            lpf = 1 ;
            continue ;
        }
        _Interpreter_InterpretAToken ( cntx->Interpreter0, token ) ;
        if ( ( blocksParsed == 0 ) && ( cntx->Interpreter0->w_Word->CType & LITERAL ) && GetState ( cntx, C_LHS ) )
        {
            // setup for optimization if this literal constant is the loop conditional
            BlockInfo * bi = ( BlockInfo* ) _Stack_Top ( compiler->BlockStack ) ;
            bi->LiteralWord = cntx->Interpreter0->w_Word ;
        }
        //nowDepth = Stack_Depth ( compiler->BlockStack ) ;
        //if ( ( ( interp->w_Word->Definition == ( block ) CfrTil_EndBlock ) ) && ( depth == nowDepth ) ) i ++ ;
        //else if ( c_syntaxFlag && ( interp->w_Word->Definition == CfrTil_End_C_Block ) && ( depth == nowDepth ) ) i ++ ;
        if ( ( ( interp->w_Word->Definition == ( block ) CfrTil_EndBlock ) ) ) blocksParsed ++ ;
        else if ( c_syntaxFlag && ( interp->w_Word->Definition == CfrTil_End_C_Block ) ) blocksParsed ++ ;
    }
    //SetState ( compiler, C_COMBINATOR_LPAREN, saveState ) ;
    SetState ( cntx, C_RHS, false ) ;
    SetState ( cntx, C_LHS, true ) ;
}
#else // old partially working version

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
        if ( ( ( interp->w_Word->Definition == ( block ) CfrTil_EndBlock ) ) && ( depth == nowDepth ) ) i ++ ;
        else if ( c_syntaxFlag && ( interp->w_Word->Definition == CfrTil_End_C_Block ) && ( depth == nowDepth ) ) i ++ ;
    }
    SetState ( compiler, C_COMBINATOR_LPAREN, saveState ) ;
    SetState ( cntx, C_RHS, false ) ;
    SetState ( cntx, C_LHS, true ) ;
}

#endif
