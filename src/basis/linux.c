
#include "../include/cfrtil.h"

#if LINUX

void
_DisplaySignal ( int64 signal )
{
    if ( signal )
    {
        //byte * location = _Context_->Location ;
        byte * location = ( byte* ) Context_Location ( ) ;
        switch ( signal )
        {
            case SIGSEGV:
            {
                _Printf ( ( byte* ) "\nSIGSEGV : memory access violation - %s", location ) ;
                break ;
            }
            case SIGFPE:
            {
                _Printf ( ( byte* ) "\nSIGFPE : arithmetic exception - %s", location ) ;
                break ;
            }
            case SIGILL:
            {
                _Printf ( ( byte* ) "\nSIGILL : illegal instruction - %s", location ) ;
                break ;
            }
            case SIGTRAP:
            {
                _Printf ( ( byte* ) "\nSIGTRAP : int3/trap - %s", location ) ;
                break ;
            }
            default: break ;
        }
    }
}

void
Linux_SetupSignals ( sigjmp_buf * sjb, int64 startTimes )
{
    struct sigaction signalAction ;
    // from http://www.linuxjournal.com/article/6483
    int64 i, result ;
    Mem_Clear ( ( byte* ) & signalAction, sizeof ( struct sigaction ) ) ;
    Mem_Clear ( ( byte* ) sjb, sizeof ( *sjb ) ) ;
    signalAction.sa_sigaction = OpenVmTil_SignalAction ;
    signalAction.sa_flags = SA_SIGINFO | SA_RESTART ; // restarts the set handler after being used instead of the default handler
    for ( i = SIGHUP ; i <= _NSIG ; i ++ )
    {
        result = sigaction ( i, &signalAction, NULL ) ;
        d0 ( if ( ( result && ( startTimes ) && ( _Q_ && ( _Q_->Verbosity > 2 ) ) ) printf ( "\nLinux_SetupSignals : signal number = " INT_FRMT_02 " : result = " INT_FRMT " : This signal can not have a handler.", i, result ) ) ) ;
    }
    //signal ( SIGWINCH, SIG_IGN ) ; // a fix for a netbeans problem but causes crash with gcc 6.x -O2+
}

void
Linux_RestoreTerminalAttributes ( )
{
    tcsetattr ( STDIN_FILENO, TCSANOW, _Q_->SavedTerminalAttributes ) ;
}
struct termios term ;

void
Linux_SetInputMode ( struct termios * savedTerminalAttributes )
{
    struct termios term ; //terminalAttributes ;
    // Make sure stdin is a terminal. /
    if ( ! isatty ( STDIN_FILENO ) )
    {
        _Printf ( ( byte* ) "Not a terminal.\n" ) ;
        exit ( EXIT_FAILURE ) ;
    }

    // Save the terminal attributes so we can restore them later. /
    memset ( savedTerminalAttributes, 0, sizeof ( struct termios ) ) ;
    tcgetattr ( STDIN_FILENO, savedTerminalAttributes ) ;
    atexit ( Linux_RestoreTerminalAttributes ) ;

    tcgetattr ( STDIN_FILENO, &term ) ; //&terminalAttributes ) ;
#if 0
    //terminalAttributes.c_iflag &= ~ ( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR
    //        | IGNCR | ICRNL | IXON ) ;
    //terminalAttributes.c_lflag &= ~ ( ICANON | ECHO | ECHONL | ISIG ) ; // | IEXTEN ) ;
    terminalAttributes.c_lflag &= ~ ( ICANON | ECHO | ECHONL ) ; // | ISIG ) ; // | IEXTEN ) ;
    //terminalAttributes.c_cflag &= ~ ( CSIZE | PARENB ) ;
    //terminalAttributes.c_cflag |= CS8 ;
    //terminalAttributes.c_cc [ VMIN ] = 1 ;
    //terminalAttributes.c_cc [ VTIME ] = 0 ;
    tcsetattr ( STDIN_FILENO, TCSANOW, &terminalAttributes ) ;
#else
    // from http://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c
    term.c_iflag |= IGNBRK ;
    term.c_iflag &= ~ ( INLCR | ICRNL | IXON | IXOFF ) ;
    term.c_lflag &= ~ ( ICANON | ECHO | ECHOK | ECHOE | ECHONL | ISIG | IEXTEN ) ;
    term.c_cc[VMIN] = 1 ;
    term.c_cc[VTIME] = 0 ;
    tcsetattr ( fileno ( stdin ), TCSANOW, &term ) ;
#endif    
}

void
LinuxInit ( struct termios * savedTerminalAttributes )
{
    Linux_SetInputMode ( savedTerminalAttributes ) ; // nb. save first !! then copy to _Q_ so atexit reset from global _Q_->SavedTerminalAttributes
    //Linux_SetupSignals ( 1 ) ; //_Q_ ? ! _Q_->StartedTimes : 1 ) ;
}

#endif


