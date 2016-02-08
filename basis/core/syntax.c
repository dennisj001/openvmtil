
#include "../../includes/cfrtil.h"

int32
__Interpret_CheckEqualBeforeSemi_LValue ( byte * nc )
{
    //if ( GetState ( _Q_->OVT_Context, ADDRESS_OF_MODE ) ) return true ;
    while ( *nc++ )
    {
        if ( *nc == '=' ) return true ; // we have an lvalue
        else if ( *nc == ';' ) return false ; // we have an rvalue
        //else if ( *nc == '"' ) return false ; // we have an rvalue
        else if ( *nc == ')' ) return false ; // we have an rvalue
        else if ( *nc == '(' ) return false ; // we have an rvalue
        else if ( *nc == '{' ) return false ; // we have an rvalue
        else if ( *nc == '}' ) return false ; // we have an rvalue
    }
    return false ;
}

void
_Interpret_CheckToken ( byte * token )
{
    if ( GetState ( _Q_->OVT_Context->Compiler0, C_COMBINATOR_LPAREN ) && ( String_Equal ( token, ";" ) ) )
    {
        _CfrTil_AddTokenToHeadOfTokenList ( token ) ; 
    }
}

int32
_Interpret_CheckEqualBeforeSemi_LValue ( )
{
    return __Interpret_CheckEqualBeforeSemi_LValue ( _ReadLine_pb_NextChar ( _Q_->OVT_Context->ReadLiner0 )  ) ;
}

int32
Interpret_CheckEqualBeforeSemi_LValue ( Word * word )
{
    return __Interpret_CheckEqualBeforeSemi_LValue ( & _Q_->OVT_Context->ReadLiner0->InputLine [ _Q_->OVT_Context->Lexer0->TokenStart_ReadLineIndex ] ) ; //word->W_StartCharRlIndex ] ) ;
}

void
Interpret_DoParenthesizedRValue ( )
{
    Context * cntx = _Q_->OVT_Context ;
    Compiler * compiler = cntx->Compiler0 ;
    int32 svcm = GetState ( compiler, COMPILE_MODE ) ;
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
    SetState ( compiler, COMPILE_MODE, svcm ) ;
}

int32
_Interpret_Do_CombinatorLeftParen ( )
{
    Context * cntx = _Q_->OVT_Context ;
    Compiler * compiler = cntx->Compiler0 ;
    int32 svcm = GetState ( compiler, COMPILE_MODE ), svclps = GetState ( compiler, C_COMBINATOR_LPAREN ) ;
    int32 blocksParsed = 0 ;
    byte * token ;

    SetState ( compiler, C_COMBINATOR_LPAREN, true ) ;
    SetState ( compiler, PREFIX_PARSING, true ) ;
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
                //_Lexer_NextNonDebugTokenWord ( cntx->Lexer0 ) ; // actually get token1 = "{"
                CfrTil_EndBlock ( ) ;
                //CfrTil_BeginBlock ( ) ; // callee handles this
                blocksParsed ++ ;
                break ;
            }
            else
            {
                token = _Lexer_ReadToken ( cntx->Lexer0, 0 ) ; // drop the ")" ?!? must be a better way than this ?!?
                continue ;
            }
        }
        _Interpreter_InterpretAToken ( cntx->Interpreter0, token ) ;
        if ( ( blocksParsed == 0 ) && ( cntx->CurrentRunWord->CType & LITERAL ) && (! IsLValue ( cntx->Interpreter0->w_Word ) ) ) //GetState ( cntx, C_LHS ) )
        {
            // setup for optimization if this literal constant is the loop conditional
            BlockInfo * bi = ( BlockInfo* ) _Stack_Top ( compiler->BlockStack ) ;
            bi->LiteralWord = cntx->CurrentRunWord ;
        }
    }
    SetState ( compiler, COMPILE_MODE, svcm ) ;
    SetState ( compiler, C_COMBINATOR_LPAREN, svclps ) ;
    //SetState ( compiler, PREFIX_ARG_PARSING, false ) ;
    SetState ( compiler, PREFIX_PARSING, false ) ;
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
            blocksParsed += _Interpret_Do_CombinatorLeftParen ( ) ;
            lpf = 1 ;
            continue ;
        }
        _Interpreter_InterpretAToken ( interp, token ) ;
        if ( interp->w_Word->Definition == ( block ) CfrTil_EndBlock ) blocksParsed ++ ;
        else if ( interp->w_Word->Definition == CfrTil_End_C_Block ) blocksParsed ++ ;
    }
}

void
CfrTil_C_LeftParen ( )
{
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    if ( ( CompileMode && ( ! GetState ( compiler, VARIABLE_FRAME ) ) ) || 
        ( ReadLine_PeekNextNonWhitespaceChar ( _Q_->OVT_Context->Lexer0->ReadLiner0 ) == '|') )  //( ! GetState ( _Q_->OVT_Context, INFIX_MODE ) ) )
    {
        CfrTil_LocalsAndStackVariablesBegin ( ) ;
    }
    else Interpret_DoParenthesizedRValue ( ) ;
}