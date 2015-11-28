
#include "../includes/cfrtil.h"

void
Interpret_DoParenthesizedRValue ( )
{
    Context * cntx = _Q_->OVT_Context ;
    Compiler * compiler = cntx->Compiler0 ;
    int32 svcm = GetState ( compiler, COMPILE_MODE ) ; 
    int32 svs_c_rhs = GetState ( cntx, C_RHS ) ;
    int32 svs_c_lhs = GetState ( cntx, C_LHS ) ;
    SetState ( cntx, C_RHS, true ) ;
    SetState ( cntx, C_LHS, false ) ;
    byte * token ;

    while ( 1 )
    {
        token = _Lexer_ReadToken ( cntx->Lexer0, 0 ) ;
        if ( String_Equal ( ( char* ) token, ")" ) )
        {
            break ;
        }
        _Interpreter_InterpretAToken ( cntx->Interpreter0, token ) ;
    }
    SetState ( cntx, C_RHS, svs_c_rhs ) ;
    SetState ( cntx, C_LHS, svs_c_lhs ) ;
    SetState ( compiler, COMPILE_MODE, svcm ) ;
}

int32
_Interpret_Do_CombinatorLeftParen ( )
{
    Context * cntx = _Q_->OVT_Context ;
    Compiler * compiler = cntx->Compiler0 ;
    int32 svcm = GetState ( compiler, COMPILE_MODE ), svclps = GetState ( compiler, C_COMBINATOR_LPAREN ) ;
    int32 svs_c_rhs = GetState ( cntx, C_RHS ) ;
    int32 svs_c_lhs = GetState ( cntx, C_LHS ) ;
    int32 blocksParsed = 0 ;
    byte * token ;

    SetState ( compiler, C_COMBINATOR_LPAREN, true ) ;
    while ( 1 )
    {
        token = _Lexer_ReadToken ( cntx->Lexer0, 0 ) ;
        if ( String_Equal ( ( char* ) token, ";" ) )
        {
            CfrTil_EndBlock ( ) ;
            CfrTil_BeginBlock ( ) ;
            blocksParsed ++ ;
            continue ;
        }
        if ( String_Equal ( ( char* ) token, ")" ) )
        {
            byte * token1 = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0 ) ;
            if ( String_Equal ( token1, "{" ) )
            {
                CfrTil_EndBlock ( ) ;
                blocksParsed ++ ;
                break ;
            }
            else
            {
                token = _Lexer_ReadToken ( cntx->Lexer0, 0 ) ; // drop the ")" ?!? must be a better way than this ?!?
                continue ;
            }
        }
        if ( ( blocksParsed == 0 ) && ( cntx->Interpreter0->w_Word->CType & LITERAL ) && GetState ( cntx, C_LHS ) )
        {
            // setup for optimization if this literal constant is the loop conditional
            BlockInfo * bi = ( BlockInfo* ) _Stack_Top ( compiler->BlockStack ) ;
            bi->LiteralWord = cntx->Interpreter0->w_Word ;
        }
        _Interpreter_InterpretAToken ( cntx->Interpreter0, token ) ;
    }
    SetState ( compiler, COMPILE_MODE, svcm ) ;
    SetState ( compiler, C_COMBINATOR_LPAREN, svclps ) ;
    SetState ( cntx, C_RHS, svs_c_rhs ) ;
    SetState ( cntx, C_LHS, svs_c_lhs ) ;
    return blocksParsed ;
}

void
CfrTil_InterpretNBlocks ( int blocks, int takesLParenFlag )
{
    Context * cntx = _Q_->OVT_Context ;
    Interpreter * interp = cntx->Interpreter0 ;
    int32 blocksParsed = 0, lpf = 0 ;
    byte * token ;
    for ( blocksParsed = 0 ; blocksParsed < blocks ; )
    {
        token = Lexer_ReadToken ( cntx->Lexer0 ) ;

        if ( String_Equal ( ( char* ) token, "(" ) && takesLParenFlag && ( ! lpf ) )
        {
            CfrTil_BeginBlock ( ) ;
            SetState ( cntx, C_LHS, true ) ;
            SetState ( cntx, C_RHS, false ) ;
            blocksParsed += _Interpret_Do_CombinatorLeftParen ( ) ;
            lpf = 1 ;
            continue ;
        }
        _Interpreter_InterpretAToken ( interp, token ) ;
        if ( interp->w_Word->Definition == ( block ) CfrTil_EndBlock ) blocksParsed ++ ;
        else if ( interp->w_Word->Definition == CfrTil_End_C_Block ) blocksParsed ++ ;
    }
    SetState ( cntx, C_RHS, false ) ;
    SetState ( cntx, C_LHS, true ) ;
}
