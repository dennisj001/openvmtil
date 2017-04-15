
#include "../../include/cfrtil.h"

int32
_Interpret_CheckEqualBeforeSemi_LValue ( byte * nc )
{
    //if ( GetState ( _Context_, ADDRESS_OF_MODE ) ) return true ;
    while ( *nc )
    {
        if ( *nc == '=' )
        {
            if ( * ( nc + 1 ) == '=' ) return false ;
            else return true ; // we have an lvalue
        }
        else if ( *nc == ';' ) return false ; // we have an rvalue
            //else if ( *nc == '"' ) return false ; // we have an rvalue
        else if ( *nc == ')' ) return false ; // we have an rvalue
        else if ( *nc == '(' ) return false ; // we have an rvalue
        else if ( *nc == '{' ) return false ; // we have an rvalue
        else if ( *nc == '}' ) return false ; // we have an rvalue
        nc ++ ;
    }
    return false ;
}

int32
Interpret_CheckEqualBeforeSemi_LValue ( Word * word )
{
    int32 tokenStartReadLineIndex = ( ( int32 ) word == - 1 ) ? _Context_->Lexer0->TokenStart_ReadLineIndex : word->W_StartCharRlIndex ;
    return _Interpret_CheckEqualBeforeSemi_LValue ( & _Context_->ReadLiner0->InputLine [ tokenStartReadLineIndex ] ) ; //word->W_StartCharRlIndex ] ) ;
}

void
Interpret_DoParenthesizedRValue ( )
{
    Context * cntx = _Context_ ;
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
        Interpreter_InterpretAToken ( cntx->Interpreter0, token, - 1 ) ;
    }
    SetState ( compiler, COMPILE_MODE, svcm ) ;
}

int32
_Interpret_Do_CombinatorLeftParen ( )
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    int32 svcm = GetState ( compiler, COMPILE_MODE ), svclps = GetState ( compiler, C_COMBINATOR_LPAREN ) ;
    int32 blocksParsed = 0, semiFlag = 0 ;
    byte * token ;

    SetState ( compiler, C_COMBINATOR_LPAREN, true ) ;
    SetState ( compiler, PREFIX_PARSING, true ) ;
    while ( 1 )
    {
        token = _Lexer_ReadToken ( cntx->Lexer0, 0 ) ;
        while ( ( semiFlag ) && String_Equal ( ( char* ) token, ";" ) )
        {
            token = _Lexer_ReadToken ( cntx->Lexer0, 0 ) ;
        } // essential don't do two semi ';' in a row if they were on the list for some reason
        semiFlag = 0 ;

        if ( String_Equal ( ( char* ) token, ";" ) )
        {
            CfrTil_EndBlock ( ) ;
            CfrTil_BeginBlock ( ) ;
            blocksParsed ++ ;
            semiFlag = 1 ;
            continue ;
        }
        else if ( String_Equal ( ( char* ) token, ")" ) )
        {
            byte * token1 = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0, 1 ) ;
            if ( String_Equal ( token1, "{" ) )
            {
                _Lexer_ReadToken ( cntx->Lexer0, 0 ) ; // drop the "{" token
                goto doLeftBracket ;
            }
            else
            {
                CfrTil_ClearTokenList ( ) ;
                token = token1 ;
            }
        }
        else if ( String_Equal ( token, "{" ) )
        {
doLeftBracket:
            CfrTil_EndBlock ( ) ;
            CfrTil_BeginBlock ( ) ;
            blocksParsed ++ ;
            break ;
        }
        Interpreter_InterpretAToken ( cntx->Interpreter0, token, - 1 ) ;
        if ( ( blocksParsed == 0 ) && ( cntx->CurrentlyRunningWord->CProperty & LITERAL ) && ( ! Is_LValue ( cntx->Interpreter0->w_Word ) ) ) //GetState ( cntx, C_LHS ) )
        {
            // setup for optimization if this literal constant is the loop conditional
            BlockInfo * bi = ( BlockInfo* ) _Stack_Top ( compiler->BlockStack ) ;
            bi->LiteralWord = cntx->CurrentlyRunningWord ;
        }
    }
    SetState ( compiler, COMPILE_MODE, svcm ) ;
    SetState ( compiler, C_COMBINATOR_LPAREN, svclps ) ;
    //SetState ( compiler, PREFIX_ARG_PARSING, false ) ;
    SetState ( compiler, PREFIX_PARSING, false ) ;

    return blocksParsed ;
}

void
CfrTil_C_LeftParen ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    if ( ( ( ! CompileMode ) && ( ! GetState ( _Context_->Interpreter0, PREPROCESSOR_MODE ) ) ) ||
        ( ( CompileMode && ( ! GetState ( compiler, VARIABLE_FRAME ) ) ) || ( ReadLine_PeekNextNonWhitespaceChar ( _Context_->Lexer0->ReadLiner0 ) == '|' ) ) ) //( ! GetState ( _Context_, INFIX_MODE ) ) )
    {
        CfrTil_LocalsAndStackVariablesBegin ( ) ;
    }
    else Interpret_DoParenthesizedRValue ( ) ;
}

void
CfrTil_InterpretNBlocks ( int blocks, int takesLParenAsBlockFlag )
{
    Context * cntx = _Context_ ;
    Interpreter * interp = cntx->Interpreter0 ;
    Word * word ;
    int32 blocksParsed = 0, gotLpf = 0 ; // got leftParen Flag
    byte * token ;
    SetState ( _Compiler_, C_COMBINATOR_PARSING, true ) ;
    for ( blocksParsed = 0 ; blocksParsed < blocks ; )
    {
        token = Lexer_ReadToken ( cntx->Lexer0 ) ;

        if ( String_Equal ( ( char* ) token, "(" ) && takesLParenAsBlockFlag && ( ! gotLpf ) )
        {
            CfrTil_BeginBlock ( ) ;
            blocksParsed += _Interpret_Do_CombinatorLeftParen ( ) ;
            SetState ( _Compiler_, C_COMBINATOR_PARSING, false ) ;
            gotLpf = 1 ;
            continue ;
        }
        word = Interpreter_InterpretAToken ( interp, token, - 1 ) ;
        if ( word && word->Definition == ( block ) CfrTil_EndBlock ) blocksParsed ++ ;
        else if ( word && word->Definition == CfrTil_End_C_Block ) blocksParsed ++ ;
        else if ( String_Equal ( ( char* ) token, "{" ) )
        {
            SetState ( _Compiler_, C_COMBINATOR_PARSING, false ) ;
        }
    }
    SetState ( _Compiler_, C_COMBINATOR_PARSING, true ) ;
}
