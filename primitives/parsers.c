
#include "../include/cfrtil.h"

void
_CfrTil_Tick ( )
{
    _Tick ( _CfrTil_->Context0 ) ;
}

void
CfrTil_Token ( )
{
    _DataStack_Push ( ( int32 ) Lexer_ReadToken ( _Context_->Lexer0 ) ) ;
}

void
CfrTil_Tick ( )
{
    if ( _ReadLine_PeekIndexedChar ( _ReadLiner_, 1 ) == '\'' )
    {
        byte c = _ReadLine_GetNextChar ( _ReadLiner_ ) ;
        _ReadLine_GetNextChar ( _ReadLiner_ ) ;
        char buffer [4] ; buffer[0]= '\'' ; buffer[1]= c ; buffer[2]= '\'' ; buffer[3]= 0 ;
        CfrTil_WordLists_PopWord ( ) ; // pop the "'" token
        //Word * word = _DataObject_New ( LITERAL, 0, buffer, 0, 0, 0, (uint32) c, 0 ) ;
        //Word * word = _DataObject_New ( LITERAL, 0, String_New (buffer, TEMPORARY), 0, 0, 0, (uint32) c, 0 ) ;
        Word * word = _Interpreter_TokenToWord ( _Interpreter_, buffer ) ;
        _Interpreter_DoWord ( _Interpreter_, word, _Lexer_->TokenStart_ReadLineIndex ) ;
        //_DataObject_Run ( word ) ;
    }
    else
    {
        CfrTil_Token ( ) ;
        _CfrTil_Tick ( ) ;
    }
}


#if 0

void
Parse_SkipUntil_EitherToken ( byte * end1, byte* end2 )
{
    while ( 1 )
    {
        byte * token ;
        int inChar = ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) ;
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
        int inChar = ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) ;
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

