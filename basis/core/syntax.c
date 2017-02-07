
#include "../../include/cfrtil.h"

int32
__Interpret_CheckEqualBeforeSemi_LValue ( byte * nc )
{
    //if ( GetState ( _Context_, ADDRESS_OF_MODE ) ) return true ;
    while ( *nc )
    {
        if ( *nc == '=' ) return true ; // we have an lvalue
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
_Interpret_CheckEqualBeforeSemi_LValue ( )
{
    return __Interpret_CheckEqualBeforeSemi_LValue ( _ReadLine_pb_NextChar ( _Context_->ReadLiner0 ) ) ;
}

int32
Interpret_CheckEqualBeforeSemi_LValue ( Word * word )
{
    int32 tokenStartReadLineIndex = ( ( int32 ) word == - 1 ) ? _Context_->Lexer0->TokenStart_ReadLineIndex : word->W_StartCharRlIndex ;
    return __Interpret_CheckEqualBeforeSemi_LValue ( & _Context_->ReadLiner0->InputLine [ tokenStartReadLineIndex ] ) ; //word->W_StartCharRlIndex ] ) ;
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
            byte * token1 = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0 ) ;
            if ( String_Equal ( token1, "{" ) )
            {
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
            token = _Lexer_ReadToken ( cntx->Lexer0, 0 ) ;
            //CfrTil_ClearTokenList ( ) ;
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
    if ( ( CompileMode && ( ! GetState ( compiler, VARIABLE_FRAME ) ) ) ||
        ( ReadLine_PeekNextNonWhitespaceChar ( _Context_->Lexer0->ReadLiner0 ) == '|' ) ) //( ! GetState ( _Context_, INFIX_MODE ) ) )
    {
        CfrTil_LocalsAndStackVariablesBegin ( ) ;
    }
    else Interpret_DoParenthesizedRValue ( ) ;
}