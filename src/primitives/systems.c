#include "../include/cfrtil.h"

// void getStdin(void) {Chr = getc(InFile), Env.put(Chr) ; }
// void putStdout(int64 c) {putc(c, OutFile);}
#if PICOLISP
extern int64 Chr ;

void
key ( )
{
    Chr = _Context_->Lexer0->NextChar ( ) ;
    //putc ( Chr, stdout ) ;
    //emit ( Chr ) ;
}

void
emit ( int64 c )
{
    putc ( Chr, stdout ) ;
    //_Printf ( (byte*)"%c", (char) c ) ;
}
#endif

#if 0
int64 doTest = 3, testDone = 1 ;

void
TestAnd ( )
{
    //{ doTest testDone @ not && } { testDone ++ sp basicT testX } if
    if ( doTest && ( ! testDone ) ) _Printf ( "true" ) ;
    else _Printf ( "false" ) ;
}
#endif

void
CfrTil_InitTime ( )
{
    System_InitTime ( _Context_->System0 ) ;
}

void
CfrTil_TimerInit ( )
{
    int64 timer = _DataStack_Pop ( ) ;
    if ( timer < 8 )
    {
        _System_TimerInit ( _Context_->System0, timer ) ;
    }
    else Throw ( ( byte* ) "Error: timer index must be less than 8", QUIT ) ;
}

void
CfrTil_Time ( )
{
    int64 timer = DataStack_Pop ( ) ;
    System_Time ( _Context_->System0, timer, ( char* ) "Timer", 1 ) ;
}

void
CfrTil_Throw ( )
{
    byte * msg = ( byte * ) DataStack_Pop ( ) ;
    Throw ( msg, 0 ) ;
}

void
_ShellEscape ( char * str )
{
    int status = 0 ;
#if 0    
    status = system ( str ) ;
#elif 0  
    char *cmd[] = { str, ( char * ) 0 } ; //{ "ls", "-l", ( char * ) 0 } ;
    char *env[] = { ( char * ) 0 } ; //{ "HOME=/usr/home", "LOGNAME=home", ( char * ) 0 } ;
    returned = execve ( "", cmd, env ) ;
#elif 0
    status = execlp ( "str", "str", "", ( char * ) 0 ) ;
#elif 0
    status = execl ( "", "sh", "-c", str, ( char * ) 0 ) ;
#elif 0
    {
        pid_t pid ;

        pid = fork ( ) ;
        if ( pid == 0 )
        {
            status = system ( str ) ;
        }
        else return ;
    }
#elif 1
    {
        extern char **environ ;
        pid_t pid ;
        char *argv[] = { (char*) "bash", (char*) "-c", str, NULL } ;
        d0( _Q_->Verbosity = 2 ) ;
        if ( _Q_->Verbosity > 1 ) printf ( "\nposix_spawn :: command = %s\n", str ) ;
        //else printf ("\n") ;
        status = posix_spawn ( &pid, "/bin/bash", NULL, NULL, argv, environ ) ;
#if 1        
        if ( status == 0 )
        {
            if ( _Q_->Verbosity > 1 ) printf ( "\nposix_spawn : child : pid = %d\n", pid ) ;
            //if ( wait ( &status ) != -1 ) //( waitpid ( pid, &status, 0 ) != - 1 )
            if ( waitpid ( pid, &status, 0 ) != - 1 )
            {
                if ( _Q_->Verbosity > 1 ) printf ( "\nposix_spawn : child : pid = %d : %s :: exited with status %d\n", pid, (char*) String_ConvertToBackSlash ( (byte*) str ), status ) ;
            }
            else
            {
                if ( _Q_->Verbosity > 0 ) perror ( "\nwaitpid" ) ;
            }
        }
        else
        {
            if ( _Q_->Verbosity > 1 ) printf ( "\nposix_spawn: %s\n", strerror ( status ) ) ;
        }
#endif        
    }
#endif    
    if ( _Q_->Verbosity > 1 ) printf ( (char*) c_dd ( "\n_ShellEscape : command = \"%s\" : returned %d.\n" ), str, status ) ;
}

void
ShellEscape_Postfix ( )
{
    byte * str = ( byte* ) _DataStack_Pop ( ) ;
    _ShellEscape ( (char*) str ) ;
    NewLine ( _Context_->Lexer0 ) ;
    SetState ( _Context_->Lexer0, LEXER_DONE, true ) ;
}

#if 0

void
ShellEscape ( )
{
    //ReadLiner * rl = _Context_->ReadLiner0 ;
    //CString str = String_New ( ( CString ) & rl->InputLine [rl->ReadIndex], TEMPORARY ) ;
    byte * str = _String_GetStringToEndOfLine ( ) ;
    _ShellEscape ( ( char* ) str ) ;
    //ReadLiner_CommentToEndOfLine ( rl ) ; //
    //SetState ( _Context_->Lexer0, LEXER_DONE, true ) ;
    //SetState ( _Context_->Interpreter0, END_OF_STRING, true ) ;
    //SetState ( _Context_->Interpreter0, DONE, true ) ; // 
}
#endif

#if 1 // designed to be parallel to '$' in c_syntax.cft to compare the compiler output
typedef struct
{
    char buf [ 256 ] ;
} Buffer0 ;

void
shell ( )
{
    char * atoken, *  buffer ;
    Buffer0 buffer0 ; 
    buffer = buffer0.buf ;
    memset ( buffer, 0, sizeof (Buffer0) ) ;
    sprintf ( buffer, "%s", "" ) ;
    while ( atoken = (char*) Lexer_ReadToken ( _Context_->Lexer0 ) )
    {
        if ( strcmp ( atoken, ";" ) )
        {
            strcat ( buffer, atoken ) ;
            if ( strcmp ( atoken, "." ) )
            {
                strcat ( buffer, " " ) ;
            }
            if ( _Q_->Verbosity > 2 ) printf ( "\n\tbuffer = %s\n", buffer ) ; //pause () ;
        }
        else
        {
            if ( _Q_->Verbosity > 1 ) printf ( "\n\tbuffer = %s\n", buffer ) ; //pause () ;
            _ShellEscape ( buffer ) ;
            break ;
        }
    }
}
#endif

void
CfrTil_Filename ( )
{
    byte * filename = _Context_->ReadLiner0->Filename ;
    if ( ! filename ) filename = ( byte* ) "command line" ;
    _DataStack_Push ( ( int64 ) filename ) ;
}

void
CfrTil_Location ( )
{
    _Printf ( _Context_Location ( _Context_ ) ) ;
}

void
CfrTil_LineNumber ( )
{
    _DataStack_Push ( ( int64 ) _Context_->ReadLiner0->LineNumber ) ;
}

void
CfrTil_LineCharacterNumber ( )
{
    _DataStack_Push ( ( int64 ) _Context_->ReadLiner0->InputLineCharacterNumber ) ;
}

void
_CfrTil_Version ( int64 flag )
{
    if ( flag || ( ( _Q_->Verbosity ) && ( _Q_->StartedTimes == 1 ) ) )
    {
        //AllowNewlines ;
        _Printf ( ( byte* ) "\ncfrTil %s", _Q_->VersionString ) ;
    }
}

void
CfrTil_Version ( )
{
    _CfrTil_Version ( 1 ) ;
}


void
CfrTil_SystemState_Print ( )
{
    _CfrTil_SystemState_Print ( 1 ) ;
}

void
_SetEcho ( int64 boolFlag )
{
    SetState ( _Context_->ReadLiner0, CHAR_ECHO, boolFlag ) ;
    SetState ( _CfrTil_, READLINE_ECHO_ON, boolFlag ) ;
}

void
CfrTil_Echo ( )
{
    // toggle flag
    if ( GetState ( _CfrTil_, READLINE_ECHO_ON ) )
    {
        _SetEcho ( false ) ;
    }
    else
    {
        _SetEcho ( true ) ;
    }
}

void
CfrTil_EchoOn ( )
{
    _SetEcho ( true ) ;
}

void
CfrTil_EchoOff ( )
{
    _SetEcho ( false ) ;
}

// ?? optimize state should be in either CfrTil or OpenVmTil not System structure

void
CfrTil_NoOp ( void )
{
    //if ( CompileMode ) _Compile_Return ( ) ;
}

void
CfrTil_Hex ( ) // !
{

    _Context_->System0->NumberBase = 16 ;
}

void
CfrTil_Binary ( ) // !
{

    _Context_->System0->NumberBase = 2 ;
}

void
CfrTil_Decimal ( ) // !
{

    _Context_->System0->NumberBase = 10 ;
}

void
CfrTil_Dump ( )
{
    byte * location = Context_IsInFile ( _Context_ ) ? Context_Location ( ) : (byte*) "" ;
    _Printf ( ( byte* ) "\nDump at : %s :" , location ) ; 
    _CfrTil_Dump ( 8 ) ;
    //_Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_Source_AddToHistory ( )
{
    Word *word = ( Word* ) _DataStack_Pop ( ) ;
    if ( word )
    {
        _CfrTil_Source ( word, 1 ) ;
    }
    //else CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
}

void
CfrTil_Source_DontAddToHistory ( )
{
    Word *word = ( Word* ) _DataStack_Pop ( ) ;
    if ( word )
    {
        _CfrTil_Source ( word, 0 ) ;
    }
    //else CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
}

void
CfrTil_AllocateNew ( )
{
    _DataStack_Push ( ( int64 ) Mem_Allocate ( _DataStack_Pop ( ), OBJECT_MEMORY ) ) ;
}

void
CfrTil_SystemBreak ( )
{
    _OpenVmTil_LongJmp_WithMsg ( BREAK, ( byte* ) "System.interpreterBreak : returning to main interpreter loop." ) ;
}

void
CfrTil_Quit ( )
{
    _OpenVmTil_LongJmp_WithMsg ( QUIT, ( byte* ) "Quit : Session Memory, temporaries, are reset." ) ;
}

void
CfrTil_Abort ( )
{
    _OpenVmTil_LongJmp_WithMsg ( ABORT, ( byte* ) "Abort : Session Memory and the DataStack are reset (as in a cold restart)." ) ;
}

void
CfrTil_DebugStop ( )
{
    _OpenVmTil_LongJmp_WithMsg ( STOP, ( byte* ) "Stop : Debug Stop. " ) ;
}

void
CfrTil_ResetAll ( )
{
    _OpenVmTil_LongJmp_WithMsg ( RESET_ALL, ( byte* ) "ResetAll. " ) ;
}

void
CfrTil_Restart ( )
{
    _OpenVmTil_LongJmp_WithMsg ( RESTART, ( byte* ) "Restart. " ) ;
}

// cold init

void
CfrTil_RestartInit ( )
{
    _OpenVmTil_LongJmp_WithMsg ( RESET_ALL, ( byte* ) "Restart Init... " ) ;
}

void
CfrTil_FullRestart ( )
{
    _Q_->Signal = 0 ;
     _OVT_Throw ( INITIAL_START ) ;
}

void
CfrTil_WarmInit ( )
{
    _CfrTil_Init_SessionCore ( _CfrTil_, 1, 1 ) ;
}

void
CfrTil_ReturnFromFile ( )
{
    _EOF ( _Context_->Lexer0 ) ;
}

void
CfrTil_ShellEscape ( )
{
    _ShellEscape ( ( char* ) _DataStack_Pop ( ) ) ;
    NewLine ( _Context_->Lexer0 ) ;
}

void
OVT_Mem_ShowAllocated ( )
{
    _OVT_ShowPermanentMemList ( _Q_, 1 ) ;
    OVT_ShowNBAs ( _Q_, 1 ) ;
}

#if 0
void foxWindow ( int64 argc, char **argv ) ;

void
CfrTil_Window ( )
{   
    int64 argc = 0 ;
    char ** argv = 0 ;
    foxWindow ( argc, argv ) ;
}
#endif
