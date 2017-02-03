#include "../include/cfrtil.h"

// void getStdin(void) {Chr = getc(InFile), Env.put(Chr) ; }
// void putStdout(int c) {putc(c, OutFile);}
#if PICOLISP
extern int Chr ;

void
key ( )
{
    Chr = _Context_->Lexer0->NextChar ( ) ;
    //putc ( Chr, stdout ) ;
    //emit ( Chr ) ;
}

void
emit ( int c )
{
    putc ( Chr, stdout ) ;
    //Printf ( (byte*)"%c", (char) c ) ;
}
#endif

#if 0
int32 doTest = 3, testDone = 1 ;

void
TestAnd ( )
{
    //{ doTest testDone @ not && } { testDone ++ sp basicT testX } if
    if ( doTest && ( ! testDone ) ) Printf ( "true" ) ;
    else Printf ( "false" ) ;
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
    int32 timer = _DataStack_Pop ( ) ;
    if ( timer < 8 )
    {
        _System_TimerInit ( _Context_->System0, timer ) ;
    }
    else Throw ( ( byte* ) "Error: timer index must be less than 8", QUIT ) ;
}

void
CfrTil_Time ( )
{
    int32 timer = DataStack_Pop ( ) ;
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
    int returned = system ( str ) ;
    if ( _Q_->Verbosity > 1 ) Printf ( c_dd ( "\nCfrTil : system ( \"%s\" ) returned %d.\n" ), str, returned ) ;
}

void
ShellEscape ( )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    CString str = String_New ( ( CString ) & rl->InputLine [rl->ReadIndex], TEMPORARY ) ;
    _ShellEscape ( str ) ;
    ReadLiner_CommentToEndOfLine ( rl ) ; //
    SetState ( _Context_->Lexer0, LEXER_DONE, true ) ;
    //SetState ( _Context_->Interpreter0, END_OF_STRING, true ) ;
    //SetState ( _Context_->Interpreter0, DONE, true ) ; // 
}

void
ShellEscape_Postfix ( )
{
    byte * str = (byte* ) _DataStack_Pop ( ) ;
    _ShellEscape ( str ) ;
    SetState ( _Context_->Lexer0, LEXER_DONE, true ) ;
}

void
ShellEscape_Postfix2 ( )
{
    char * str1 = (char* ) _DataStack_Pop ( ) ;
    char * str0 = (char* ) _DataStack_Pop ( ) ;
    char * buffer = (char*) Buffer_Data ( _CfrTil_->ScratchB1 ) ;
    memset ( buffer, 0, BUFFER_SIZE ) ;
    strncat ( buffer, str0, BUFFER_SIZE ) ;
    strncat ( buffer, " ", BUFFER_SIZE ) ;
    strncat ( buffer, str1, BUFFER_SIZE ) ;
    _ShellEscape ( buffer ) ;
    SetState ( _Context_->Lexer0, LEXER_DONE, true ) ;
}

void
CfrTil_Filename ( )
{
    byte * filename = _Context_->ReadLiner0->Filename ;
    if ( ! filename ) filename = ( byte* ) "command line" ;
    _DataStack_Push ( ( int32 ) filename ) ;
}

void
CfrTil_Location ( )
{
    Printf ( _Context_Location ( _Context_ ) ) ;
}

void
CfrTil_LineNumber ( )
{
    _DataStack_Push ( ( int32 ) _Context_->ReadLiner0->LineNumber ) ;
}

void
CfrTil_LineCharacterNumber ( )
{
    _DataStack_Push ( ( int32 ) _Context_->ReadLiner0->InputLineCharacterNumber ) ;
}

void
_CfrTil_Version ( int flag )
{
    if ( flag || ( ( _Q_->Verbosity ) && ( _Q_->StartedTimes == 1 ) ) )
    {
        AllowNewlines ;
        Printf ( ( byte* ) "\ncfrTil %s", _Q_->VersionString ) ;
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
_SetEcho ( int32 boolFlag )
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

    _CfrTil_Dump ( 8 ) ;
    Printf ( ( byte* ) "\n" ) ;
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
    _DataStack_Push ( ( int32 ) Mem_Allocate ( _DataStack_Pop ( ), OBJECT_MEMORY ) ) ;
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
    //OpenVmTil_Throw ( ( byte* ) "Full Restart. ", INITIAL_START, 1 ) ;
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
    OVT_ShowPermanentMemList ( ) ;
    OVT_ShowNBAs ( ) ;
}

