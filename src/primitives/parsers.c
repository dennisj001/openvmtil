
#include "../include/cfrtil.h"

void
CfrTil_Token ( )
{
    _DataStack_Push ( ( int64 ) Lexer_ReadToken ( _Context_->Lexer0 ) ) ;
}

void
_CfrTil_SingleQuote ( int64 findWordFlag )
{
    if ( _ReadLine_PeekIndexedChar ( _ReadLiner_, 1 ) == '\'' ) // parse a char type, eg. 'c' 
    {
        byte c = _ReadLine_GetNextChar ( _ReadLiner_ ) ;
        _ReadLine_GetNextChar ( _ReadLiner_ ) ;
        char buffer [4] ; buffer[0]= '\'' ; buffer[1]= c ; buffer[2]= '\'' ; buffer[3]= 0 ;
        CfrTil_WordLists_PopWord ( ) ; // pop the "'" token
        Word * word = _Interpreter_TokenToWord ( _Interpreter_, buffer ) ;
        _Interpreter_DoWord ( _Interpreter_, word, _Lexer_->TokenStart_ReadLineIndex ) ;
    }
    else
    {
        CfrTil_Token ( ) ;
        _Tick ( _Context_, findWordFlag ) ;
    }
}

void
CfrTil_SingleQuote ( )
{
    _CfrTil_SingleQuote ( 0 ) ;
}

void
CfrTil_Tick ( )
{
    _CfrTil_SingleQuote ( 1 ) ;
}


#if 0

void
Parse_SkipUntil_EitherToken ( byte * end1, byte* end2 )
{
    while ( 1 )
    {
        byte * token ;
        int64 inChar = ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) ;
        if ( ( inChar == - 1 ) || ( inChar == eof ) ) break ;

        if ( ( token = ( char* ) Lexer_ReadToken ( _Context_->Lexer0 ) ) )
        {
            if ( String_Equal ( token, end1 ) ) break ;
            if ( String_Equal ( token, end2 ) ) break ;
        }
    }
}
#endif

void
Parse_SkipUntil_Token ( byte * end )
{
    while ( 1 )
    {
        byte * token ;
        int64 inChar = ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) ;
        if ( ( inChar == 0 ) || ( inChar == - 1 ) || ( inChar == eof ) ) break ;

        if ( ( token = Lexer_ReadToken ( _Context_->Lexer0 ) ) )
        {
            if ( String_Equal ( token, end ) ) break ;
        }
    }
}

void
CfrTil_Parse ( )
{
    Lexer * lexer = _Context_->Lexer0 ;
    byte * token = ( byte* ) _DataStack_Pop ( ) ;
    Lexer_ParseObject ( lexer, token ) ;
    _DataStack_Push ( lexer->Literal ) ;
}

