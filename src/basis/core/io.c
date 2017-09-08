
#include "../../include/cfrtil.h"

int64
GetTerminalWidth ( )
{
#ifdef TIOCGSIZE
    struct ttysize ts ;
    ioctl ( STDIN_FILENO, TIOCGSIZE, &ts ) ;
    //cols = ts.ts_cols ;
    //lines = ts.ts_lines;
    return ts.ts_cols ;
#elif defined(TIOCGWINSZ)
    struct winsize ts ;
    ioctl ( STDIN_FILENO, TIOCGWINSZ, &ts ) ;
    //ioctl ( STDOUT_FILENO, TIOCGWINSZ, &ts ) ;
    //cols = ts.ws_col ;
    //lines = ts.ws_row;
    return ts.ws_col ;
#endif /* TIOCGSIZE */
}

char
kbhit ( void )
{
    int64 oldf ;

    oldf = fcntl ( STDIN_FILENO, F_GETFL, 0 ) ;
    fcntl ( STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK ) ;
    char ch = getchar ( ) ;
    if ( ch < 0 ) ch = 0 ;
    fcntl ( STDIN_FILENO, F_SETFL, oldf ) ;
    return ch ;
}

#if 0

int64
GetC ( )
{
    byte buffer [4] ;
    read ( STDIN_FILENO, buffer, 1 ) ;
    write ( STDOUT_FILENO, buffer, 1 ) ;
    return buffer [0] ;
}
#endif

// from : http://stackoverflow.com/questions/16026858/reading-the-device-status-report-ansi-escape-sequence-reply

void
getCursor ( int* x, int* y )
{
    _Printf ( (byte*) "\033[6n" ) ;
    scanf ( "\033[%d;%dR", x, y ) ;
    fflush ( stdin ) ; 
}

#define KEY() getc ( stdin )

int64
_Key ( FILE * f )
{
    int64 key = getc ( f ) ; // GetC () ;
    return key ;
}

int64
_Kbhit ()
{
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ;
}

int64
Key_Kbhit ( FILE * f )
{
    int64 key = _Key ( f ) ;
    _Kbhit () ;
    return key ;
}

int64
Key ( )
{
    return Key_Kbhit ( stdin ) ;
}

byte
_CfrTil_Key ( ReadLiner * rl )
{
    //return getc ( rl->InputFile ) ;
    return _Key ( rl->InputFile ) ;
}

void
Emit ( byte c )
{
    _Printf ( (byte*) "%c", c ) ;
}

void
Context_DoPrompt ( Context * cntx )
{
#if 1   
    int x = 0, y = 0 ;
    getCursor ( &x, &y ) ;
    if ( x > Strlen ( ( char* ) _ReadLiner_->Prompt ) ) _Printf ( (byte*) "\n" ) ;
#endif    
    _Printf ( ( byte* ) "%s", ( char* ) cntx->ReadLiner0->NormalPrompt ) ; // for when including files
}

void
CfrTil_DoPrompt ( )
{
    Context_DoPrompt ( _Context_ ) ;
}

// all output comes thru here

void
_Printf ( byte *format, ... )
{
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ;
    if ( _Q_->Verbosity ) //GetState ( _ReadLiner_, CHAR_ECHO ) )
    {
        va_list args ;
        va_start ( args, ( char* ) format ) ;
        // int64 olength = vprintf ( ( char* ) format, args ) ;
        vprintf ( ( char* ) format, args ) ;
        if ( _CfrTil_ && _CfrTil_->LogFlag ) vfprintf ( _CfrTil_->LogFILE, ( char* ) format, args ) ;
        va_end ( args ) ;
        fflush ( stdout ) ;
        fflush ( stdout ) ;
        //_ReadLiner_->EndPosition += ((olength > 0) ? olength : 0) ;
    }
}
#if 0
// try not to (don't) print extra newlines
// this is called on exceptions so alot of checking 

void
Printf ( byte *format, ... )
{
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ; //CfrTil_Quit ( ) ;
    if ( _Q_ && _CfrTil_ && _Q_->Verbosity )
    {
        va_list args ;
        va_start ( args, ( char* ) format ) ;
        char * out = ( char* ) Buffer_Data ( _CfrTil_->PrintfB ) ;
        vsprintf ( ( char* ) out, ( char* ) format, args ) ;
        va_end ( args ) ;
        int64 len = Strlen ( ( char* ) out ) ;
        byte final = out [ len - 1 ] ;
        if ( _Q_->psi_PrintStateInfo )
        {
            if ( out [0] == '\n' )
            {
                if ( ( _Q_->psi_PrintStateInfo->OutputLineCharacterNumber < 2 ) && ( GetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE ) ) ) out = & out [1] ;
                else if ( _Q_->psi_PrintStateInfo && GetState ( _Q_->psi_PrintStateInfo, PSI_PROMPT ) )
                {
                    out [0] = '\r' ;
                    SetState ( _Q_->psi_PrintStateInfo, PSI_PROMPT, false ) ;
                }
            }
        }
        printf ( "%s", out ) ;
        if ( _CfrTil_ && _CfrTil_->LogFlag ) fprintf ( _CfrTil_->LogFILE, "%s", out ) ;
        if ( _Q_->psi_PrintStateInfo )
        {
            if ( ( final == '\n' ) || ( final == '\r' ) )
            {
                _Q_->psi_PrintStateInfo->OutputLineCharacterNumber = 0 ;
                ConserveNewlines ;
            }
            else
            {
                _Q_->psi_PrintStateInfo->OutputLineCharacterNumber += len ;
                AllowNewlines ;
            }
        }
        fflush ( stdout ) ;
    }
}

PrintStateInfo *
PrintStateInfo_New ( )
{
    PrintStateInfo * psi = ( PrintStateInfo * ) Mem_Allocate ( sizeof ( PrintStateInfo ), OPENVMTIL ) ;
    //PrintStateInfo * psi = ( PrintStateInfo * ) MemList_AllocateChunk ( &_MemList_, sizeof ( PrintStateInfo ), OPENVMTIL ) ; ;
    SetState ( psi, PSI_PROMPT, false ) ;
    SetState ( psi, PSI_NEWLINE, true ) ;
    return psi ;
}
#endif

#if LISP_IO

byte *
_vprintf ( FILE * f, char *format, ... )
{
    va_list args ;
    va_start ( args, ( char* ) format ) ;
    __Printf ( ( byte* ) format, args ) ;
}

uint64
Getc ( FILE * f )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    if ( f != stdin ) return fgetc ( f ) ;
    if ( Maru_RawReadFlag ) return ReadLine_Key ( rl ) ;
    else return ( int64 ) ReadLine_NextChar ( rl ) ;
}

uint64
Getwc ( FILE * f )
{
    return btowc ( Getc ( f ) ) ;
}

void
UnGetc ( int64 c, FILE * f )
{
    if ( f == stdin )
        ReadLine_UnGetChar ( _Context_->ReadLiner0 ) ;
    else ungetc ( c, f ) ;
}

void
UnGetwc ( int64 c, FILE * f )
{
    return UnGetc ( wctob ( c ), f ) ;
}
#endif

#if 0

void
__CfrTil_Emit ( byte c )
{
    if ( ( c == '\n' ) || ( c == '\r' ) )
    {
        if ( _Context_->ReadLiner0->OutputLineCharacterNumber == 0 ) return ;
        else
        {
            //if ( ! overWrite ) 
            c = '\n' ; // don't overwrite the existing line
            _Context_->ReadLiner0->OutputLineCharacterNumber = 0 ;
        }
    }
    else _Context_->ReadLiner0->OutputLineCharacterNumber ++ ;
    if ( _Q_->Verbosity ) putc ( c, _Context_->ReadLiner0->OutputFile ) ;
}

void
_CfrTil_EmitString ( byte * string )
{
#if 1
    int64 i ;
    //if ( _Context->ReadLiner0->Flags & CHAR_ECHO )
    {
        for ( i = 0 ; string [ i ] ; i ++ )
        {
            if ( kbhit ( ) == ESC ) CfrTil_Quit ( ) ;
            __CfrTil_Emit ( string [ i ] ) ;
        }
    }
#else
    if ( kbhit ( ) == ESC ) CfrTil_Quit ( ) ;
    puts ( ( char* ) string ) ;
#endif
}
#endif

