
#include "../include/cfrtil.h"

void
MultipleEscape ( )
{
    _MultipleEscape ( _Context_->Lexer0 ) ;
}

void
CfrTil_StrLen ( )
{
    _DataStack_Push ( (int32) Strlen ( (char*) _DataStack_Pop ( ) ) ) ;
}

void
CfrTil_StrCmp ( )
{
    _DataStack_Push ( (int32) strcmp ( (char*) _DataStack_Pop ( ), (char*) _DataStack_Pop ( ) ) ) ;
}

void
CfrTil_StrICmp ( )
{
    _DataStack_Push ( (int32) stricmp ( (byte*) _DataStack_Pop ( ), (byte*) _DataStack_Pop ( ) ) ) ;
}

//char * strcat ( char * destination, const char * source );
void
CfrTil_StrCat ( )
{
    Buffer * b = Buffer_New ( BUFFER_SIZE ) ;  
    byte * buffer = Buffer_Data ( b );  byte *str ;
    char * src = (char*) _DataStack_Pop ( ) ;
    char * dst = (char*) _DataStack_Pop ( ) ;
    strcpy ( (char*) buffer, dst ) ;
    if (src) strcat ( (char *) buffer, src ) ; 
#if 0    
    if ( ! CompileMode ) 
    {
        str = TemporaryString_New ( buffer ) ; // only if not Compiling do we want to free the lexer->Literal
    }
    //else 
#endif    
    str = TemporaryString_New ( buffer ) ; //String_New ( (byte*) buffer, DICTIONARY ) ;
    _DataStack_Push ( (int32) str ) ;
    Buffer_SetAsUnused ( b ) ; ;
}

void
CfrTil_StrCpy ( )
{
    // !! nb. this cant really work !! what do we want here ??
    _DataStack_Push ( (int32) strcpy ( (char*) _DataStack_Pop ( ), (char*) _DataStack_Pop ( ) ) ) ;
}

void
String_GetStringToEndOfLine ( )
{
    _DataStack_Push ( (int32) _String_GetStringToEndOfLine ( ) ) ;
}
