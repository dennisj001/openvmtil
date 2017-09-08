
#include "../include/cfrtil.h"

void
MultipleEscape ( )
{
    _MultipleEscape ( _Context_->Lexer0 ) ;
}

void
CfrTil_StrLen ( )
{
    _DataStack_Push ( (int64) Strlen ( (char*) _DataStack_Pop ( ) ) ) ;
}

void
CfrTil_StrCmp ( )
{
    _DataStack_Push ( (int64) strcmp ( (char*) _DataStack_Pop ( ), (char*) _DataStack_Pop ( ) ) ) ;
}

void
CfrTil_StrICmp ( )
{
    _DataStack_Push ( (int64) stricmp ( (byte*) _DataStack_Pop ( ), (byte*) _DataStack_Pop ( ) ) ) ;
}

//char * strcat ( char * destination, const char * source );
void
CfrTil_StrCat ( )
{
    //Buffer * b = Buffer_New ( BUFFER_SIZE ) ;  
    byte * buffer = Buffer_Data ( _CfrTil_->StrCatBuffer );  byte *str ;
    char * src = (char*) _DataStack_Pop ( ) ;
    char * dst = (char*) _DataStack_Pop ( ) ;
    strcpy ( (char*) buffer, dst ) ;
    if (src) strcat ( (char *) buffer, src ) ; 
    str = String_New ( buffer, TEMPORARY ) ; //String_New ( (byte*) buffer, DICTIONARY ) ;
    _DataStack_Push ( (int64) str ) ;
    //Buffer_SetAsUnused ( b ) ; ;
}

void
CfrTil_StrCpy ( )
{
    // !! nb. this cant really work !! what do we want here ??
    _DataStack_Push ( (int64) strcpy ( (char*) _DataStack_Pop ( ), (char*) _DataStack_Pop ( ) ) ) ;
}

void
String_GetStringToEndOfLine ( )
{
    _DataStack_Push ( (int64) _String_GetStringToEndOfLine ( ) ) ;
}
