
#include "../include/cfrtil.h"

void
CfrTil_IncludeFile ( )
{
    byte * filename = ( byte* ) _DataStack_Pop ( ) ;
    _CfrTil_ContextNew_IncludeFile ( filename ) ;
}

int64
_File_Size ( FILE * file )
{
    fseek ( file, 0, SEEK_END ) ;
    int64 size = ftell ( file ) ;
    rewind ( file ) ;
    return size ;
}

int64
_File_Exists ( byte * fname )
{
    struct stat sbuf ;
    if ( ! stat ( ( char* ) fname, &sbuf ) ) 
    {
        return true ; // nb. this logic is the reverse of the system call
    }
    else { return false ; }
}

#if 1

byte *
_File_ReadToString_ ( FILE * file )
{
    int64 size, result ;

    size = _File_Size ( file ) ;
    byte * fstr = Mem_Allocate ( size + 2, CONTEXT ) ; // 2 : an extra so readline doesn't read into another area of allocated mem
    result = fread ( fstr, 1, size, file ) ;
    //fclose ( file ) ;
    if ( result != size ) return 0 ;
    return fstr ;
}

byte *
_File_ReadToString ( byte * name )
{
    FILE * file ;
    file = fopen ( ( char* ) name, "rb" ) ;
    return _File_ReadToString_ ( file ) ;
}

void
File_ReadToString ( )
{
    byte * filename = ( byte* ) _DataStack_Pop ( ), *str ;
    str = _File_ReadToString ( filename ) ;
    _DataStack_Push ( ( int64 ) str ) ;
}

#endif

void
File_Open ( )
{
    byte * filename = ( byte* ) _DataStack_Pop ( ) ;
    FILE * file = fopen ( ( char* ) filename, "r" ) ;
    if ( file == NULL )
    {
        perror ( "\nFile_Open error : " ) ;
        CfrTil_Quit ( ) ;
    }
    else _DataStack_Push ( ( int64 ) file ) ;
}

void
File_Close ( )
{
    FILE * file = ( FILE* ) _DataStack_Pop ( ) ;
    fclose ( file ) ;
}

void
File_Read ( )
{
    int64 size = _DataStack_Pop ( ) ;
    byte * ptr = ( byte * ) _DataStack_Pop ( ) ;
    FILE * file = ( FILE * ) _DataStack_Pop ( ) ;
    int64 result = fread ( ptr, size, 1, file ) ;
    _DataStack_Push ( result ) ;
    if ( result != size )
    {
        if ( ! feof ( file ) )
        {
            if ( ferror ( file ) )
            {
                perror ( "\nFile_Read error : " ) ;
                CfrTil_Quit ( ) ;
            }
        }
    }
}

void
File_Write ( )
{
    int64 size = _DataStack_Pop ( ) ;
    byte * ptr = ( byte * ) _DataStack_Pop ( ) ;
    FILE * file = ( FILE * ) _DataStack_Pop ( ) ;
    fwrite ( ptr, size, 1, file ) ;
}

void
File_Seek ( )
{
    int64 offset = _DataStack_Pop ( ) ;
    FILE * file = ( FILE * ) _DataStack_Pop ( ) ;
    fseek ( file, offset, SEEK_SET ) ;
}

void
File_Tell ( )
{
    FILE * file = ( FILE * ) _DataStack_Pop ( ) ;
    int64 result = ftell ( file ) ;
    _DataStack_Push ( result ) ;
}

void
File_Size ( )
{
    FILE * file = ( FILE * ) _DataStack_Pop ( ) ;
    _DataStack_Push ( _File_Size ( file ) ) ;
}

void
File_Exists ( )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( _File_Exists ( name ) ) ;
}