#include "../includes/cfrtil.h"

// void getStdin(void) {Chr = getc(InFile), Env.put(Chr) ; }
// void putStdout(int c) {putc(c, OutFile);}
#if PICOLISP
extern int Chr ;

void
key ( )
{
    Chr = _Q_->OVT_Context->Lexer0->NextChar ( ) ;
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
    System_InitTime ( _Q_->OVT_Context->System0 ) ;
}

void
CfrTil_TimerInit ( )
{
    int32 timer = _DataStack_Pop ( ) ;
    if ( timer < 8 )
    {
        _System_TimerInit ( _Q_->OVT_Context->System0, timer ) ;
    }
    else Throw ( ( byte* ) "Error: timer index must be less than 8", QUIT ) ;
}

void
CfrTil_Time ( )
{
    int32 timer = DataStack_Pop ( ) ;
    System_Time ( _Q_->OVT_Context->System0, timer, ( char* ) "Timer", 1 ) ;
}

void
_ShellEscape ( char * str )
{
    int returned = system ( str ) ;
    if ( _Q_->Verbosity > 1 ) Printf ( c_dd ( "\nCfrTil : system ( \"%s\" ) returned %d.\n" ), str, returned ) ;
    D0 ( CfrTil_PrintDataStack ( ) ) ;
    Interpreter_SetState ( _Q_->OVT_Context->Interpreter0, DONE, true ) ; // 
}

void
CfrTil_Throw ( )
{
    byte * msg = ( byte * ) DataStack_Pop ( ) ;
    Throw ( msg, 0 ) ;
}

void
ShellEscape ( )
{
    ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
    _ShellEscape ( ( CString ) & rl->InputLine [rl->ReadIndex] ) ;
    Lexer_SetState ( _Q_->OVT_Context->Lexer0, LEXER_DONE, true ) ;
    Interpreter_SetState ( _Q_->OVT_Context->Interpreter0, END_OF_STRING, true ) ;
}

void
CfrTil_Filename ( )
{
    byte * filename = _Q_->OVT_Context->ReadLiner0->Filename ;
    if ( ! filename ) filename = ( byte* ) "command line" ;
    _DataStack_Push ( ( int32 ) filename ) ;
}

void
CfrTil_Location ( )
{
    Printf ( _Context_Location ( _Q_->OVT_Context ) ) ;
}

void
CfrTil_LineNumber ( )
{
    _DataStack_Push ( ( int32 ) _Q_->OVT_Context->ReadLiner0->LineNumber ) ;
}

void
CfrTil_LineCharacterNumber ( )
{
    _DataStack_Push ( ( int32 ) _Q_->OVT_Context->ReadLiner0->InputLineCharacterNumber ) ;
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
    SetState ( _Q_->OVT_Context->ReadLiner0, CHAR_ECHO, boolFlag ) ;
    SetState ( _Q_->OVT_CfrTil, READLINE_ECHO_ON, boolFlag ) ;
}

void
CfrTil_Echo ( )
{
    // toggle flag
    if ( CfrTil_GetState ( _Q_->OVT_CfrTil, READLINE_ECHO_ON ) )
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

    _Q_->OVT_Context->System0->NumberBase = 16 ;
}

void
CfrTil_Binary ( ) // !
{

    _Q_->OVT_Context->System0->NumberBase = 2 ;
}

void
CfrTil_Decimal ( ) // !
{

    _Q_->OVT_Context->System0->NumberBase = 10 ;
}

void
CfrTil_Dump ( )
{

    _CfrTil_Dump ( 8 ) ;
    Printf ( ( byte* ) "\n" ) ;
}

void
_PrintNReturnStack ( int32 * esp, int32 size )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    Buffer * b = Buffer_New ( BUFFER_SIZE ) ;
    int32 * i, saveSize = size ;
    byte * buffer = Buffer_Data ( b ) ;
    if ( esp )
    {
        Printf ( ( byte* ) "\nReturnStack   :%3i  : Esp (ESP) = " UINT_FRMT_0x08 " : Top = " UINT_FRMT_0x08 "", size, ( uint ) esp, ( uint ) esp ) ;
        // print return stack in reverse of usual order first
        while ( size -- > 1 )
        {
            Word * word = Word_GetFromCodeAddress ( ( byte* ) ( esp [ size ] ) ) ;
            if ( word ) sprintf ( ( char* ) buffer, "< %s.%s >", word->ContainingNamespace->Name, word->Name ) ;
            Printf ( ( byte* ) "\n\t\t    ReturnStack   [ %3d ] < " UINT_FRMT_0x08 " > = " UINT_FRMT_0x08 "\t\t%s", size, ( uint ) & esp [ size ], esp [ size ], word ? ( char* ) buffer : "" ) ;
        }
        _Stack_PrintValues ( ( byte* ) "ReturnStack", esp, saveSize ) ;
    }
    Buffer_SetAsUnused ( b ) ;
}

void
_CfrTil_PrintNReturnStack ( int32 size )
{
    _CfrTil_WordName_Run ( ( byte* ) "getESP" ) ;
    int32 * esp = ( int32 * ) _DataStack_Pop ( ) ;
    _PrintNReturnStack ( esp, size ) ;
}

void
CfrTil_PrintNReturnStack ( )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    int32 size = _DataStack_Pop ( ) ;
    _CfrTil_PrintNReturnStack ( size ) ;
}

void
CfrTil_PrintReturnStack ( )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    _CfrTil_PrintNReturnStack ( 8 ) ;
}

void
CfrTil_PrintDataStack ( )
{
    CfrTil_SyncStackPointerFromDsp ( _Q_->OVT_CfrTil ) ;
    _Stack_Print ( _DataStack_, ( byte* ) "DataStack" ) ;
    Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_CheckInitDataStack ( )
{
    CfrTil_SyncStackPointerFromDsp ( _Q_->OVT_CfrTil ) ;
    if ( Stack_Depth ( _DataStack_ ) < 0 )
    {
        _Stack_PrintHeader ( _DataStack_, ( byte* ) "DataStack" ) ;
        Printf ( ( byte* ) c_ad ( "\n\nError : %s : %s : Stack Underflow!" ), _Q_->OVT_Context->CurrentRunWord ? _Q_->OVT_Context->CurrentRunWord->Name : ( byte * ) "", _Context_Location ( _Q_->OVT_Context ) ) ;
        Printf ( ( byte* ) c_dd ( "\nReseting DataStack.\n" ) ) ;
        _CfrTil_DataStack_Init ( _Q_->OVT_CfrTil ) ;
        _Stack_PrintHeader ( _DataStack_, ( byte* ) "DataStack" ) ;
    }
    Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_DataStack_Size ( )
{
    _DataStack_Push ( DataStack_Depth ( ) ) ;
}

void
CfrTil_Source_AddToHistory ( )
{
    Word *word = ( Word* ) _DataStack_Pop ( ) ;
    if ( word )
    {
        _CfrTil_Source ( word, 1 ) ;
    }
    else CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
}

void
CfrTil_Source_DontAddToHistory ( )
{
    Word *word = ( Word* ) _DataStack_Pop ( ) ;
    if ( word )
    {
        _CfrTil_Source ( word, 0 ) ;
    }

    else CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
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
    jmp_buf * sjb = & _Q_->JmpBuf0 ;
    _Q_->Signal = 0 ;
    _OpenVmTil_Throw ( sjb, ( byte* ) "Full Restart. ", INITIAL_START ) ;
}

void
CfrTil_WarmInit ( )
{
    _CfrTil_Init_SessionCore ( _Q_->OVT_CfrTil, 1, 1 ) ;
}

void
CfrTil_ReturnFromFile ( )
{
    _EOF ( _Q_->OVT_Context->Lexer0 ) ;
}

void
CfrTil_ShellEscape ( )
{
    _ShellEscape ( ( char* ) _DataStack_Pop ( ) ) ;
    NewLine ( _Q_->OVT_Context->Lexer0 ) ;
}

void
OVT_Mem_ShowAllocated ( )
{
    OVT_ShowPermanentMemList ( ) ;
    OVT_ShowNBAs ( ) ;
}

