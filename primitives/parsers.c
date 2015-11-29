
#include "../includes/cfrtil.h"

void
_CfrTil_Tick ( )
{
    _Tick ( _Q_->OVT_CfrTil->Context0 ) ;
}

void
CfrTil_Token ( )
{
    _DataStack_Push ( ( int32 ) Lexer_ReadToken ( _Q_->OVT_Context->Lexer0 ) ) ;
}

void
CfrTil_Tick ( )
{
    CfrTil_Token ( ) ;
    _CfrTil_Tick ( ) ;
}

void
CfrTil_PreProcessor ( )
{
    Finder_SetNamedQualifyingNamespace ( _Q_->OVT_Context->Finder0, "PreProcessor" ) ;
    _Interpret_ToEndOfLine ( _Q_->OVT_Context->Interpreter0 ) ;
}

#if 0
void
Parse_SkipUntil_EitherToken ( byte * end1, byte* end2 )
{
    while ( 1 )
    {
        byte * token ;
        int inChar = ReadLine_PeekNextChar ( _Q_->OVT_Context->ReadLiner0 ) ;
        if ( ( inChar == - 1 ) || ( inChar == eof ) ) break ;

        if ( ( token = ( char* ) Lexer_ReadToken ( _Q_->OVT_Context->Lexer0 ) ) )
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
        int inChar = ReadLine_PeekNextChar ( _Q_->OVT_Context->ReadLiner0 ) ;
        if ( ( inChar == 0 ) || ( inChar == - 1 ) || ( inChar == eof ) ) break ;

        if ( ( token = ( char* ) Lexer_ReadToken ( _Q_->OVT_Context->Lexer0 ) ) )
        {
            if ( String_Equal ( token, end ) ) break ;
        }
    }
}

void
CfrTil_Parse ( )
{
    Lexer * lexer = _Q_->OVT_Context->Lexer0 ;
    byte * token = ( byte* ) _DataStack_Pop ( ) ;
    Lexer_ParseObject ( lexer, token ) ;
    _DataStack_Push ( lexer->Literal ) ;
}

#if 0
void
CfrTil_DoubleQuote ( )
{
    CfrTil_SourceCode_Init ( ) ; // ?? an adhoc to work with ":" ??
    CfrTil_Token ( ) ;
}
#endif
