
#include "../include/cfrtil.h"

// ?? this file could be worked on ??

int32
_OpenVmTil_ShowExceptionInfo ( )
{
    AlertColors ;
    if ( _Q_->Verbosity )
    {
        if ( _Q_->SignalExceptionsHandled ++ < 2 )
        {
            Word * word = 0 ;
            Debugger * debugger ;
            if ( _CfrTil_ && ( debugger = _Debugger_ ) )
            {
                DebugOn ;
                if ( _Q_->Signal != 11 )
                {
                    if ( ! debugger->w_Word )
                    {
                        Word * word = 0 ;
                        if ( _Q_->SigAddress ) word = Word_GetFromCodeAddress ( ( byte* ) _Q_->SigAddress ) ;
                        if ( ! word ) word = _Context_->CurrentlyRunningWord ;
                        debugger->w_Word = word ;
                    }
                }
                SetState ( debugger, DBG_INFO, true ) ;
                Debugger_ShowInfo ( debugger, _Q_->ExceptionMessage, _Q_->Signal ) ;

                if ( GetState ( debugger, DBG_STEPPING ) ) Debugger_Registers ( debugger ) ;
                if ( _Q_->Signal != 11 )
                {
                    if ( word )
                    {
                        _CfrTil_Source ( word, 0 ) ;
                        if ( ! CompileMode && ( ! ( word->CProperty & CPRIMITIVE ) ) ) _CfrTil_Word_Disassemble ( word ) ;
                    }
                }
                else _Q_->SignalExceptionsHandled ++ ;
            }
            _DisplaySignal ( _Q_->Signal ) ;
        }
        else
        {
            _Q_->RestartCondition = FULL_RESTART ;
        }
    }
    _Q_->Signal = 0 ;
    int32 rtrn = _OpenVmTil_Pause ( ) ;
    return rtrn ;
}

int32
_OVT_Pause ( byte * prompt )
{
    int32 rtrn = 0 ;
    if ( ! GetState ( _Debugger_, DBG_AUTO_MODE | DBG_STEPPING ) && _Context_->ReadLiner0->Filename )
    {
        byte buffer [512], *defaultPrompt = ( byte * ) "\n%s\n%s : at %s :: %s\n'd' for debugger, '\\' for an interpret prompt, 'q' to (q)uit, 'x' to e(x)it, other <key> == continue%s" ;
        snprintf ( ( char* ) buffer, 512, ( char* ) prompt ? prompt : defaultPrompt, _Q_->ExceptionMessage ? _Q_->ExceptionMessage : ( byte* ) "", c_dd ( "pause" ),
            _Context_Location ( _Context_ ), c_dd ( _Debugger_->ShowLine ? _Debugger_->ShowLine : _Context_->ReadLiner0->InputLine ), c_dd ( "\n-> " ) ) ;
        int key ;
        DebugColors ;
        do
        {
            _Printf ( ( byte* ) "%s", buffer ) ;
            key = Key ( ) ;
            _ReadLine_PrintfClearTerminalLine ( ) ;
            if ( ( key == 'x' ) || ( key == 'X' ) )
            {
                byte * msg = ( byte * ) "Exit cfrTil from pause?" ;
                Printf ( ( byte* ) "\n%s : 'x' to e(x)it cfrTil : any other <key> to continue%s", msg, c_dd ( "\n-> " ) ) ;
                key = Key ( ) ;
                if ( ( key == 'x' ) || ( key == 'X' ) ) OVT_Exit ( ) ;
            }
            else if ( key == 'q' )
            {
                byte * msg = ( byte * ) "Quit to interpreter loop from pause?" ;
                Printf ( ( byte* ) "\n%s : 'q' to (q)uit : any other key to continue%s", msg, c_dd ( "\n-> " ) ) ;
                key = Key ( ) ;
                if ( ( key == 'q' ) || ( key == 'Q' ) ) DefaultColors, _OVT_Throw ( QUIT ) ;
            }
            else if ( key == 'd' )
            {
                Debugger * debugger = _Debugger_ ;
                //if ( Is_DebugOn || GetState ( debugger, DBG_COMMAND_LINE ) ) siglongjmp ( _Debugger_->JmpBuf0, 0 ) ;
                //else
                {
                    DebugOn ;
                    SetState ( _CfrTil_, DEBUG_MODE, true ) ;
                    debugger->TokenStart_ReadLineIndex = 0 ; // prevent turning off _Debugger_PreSetup
                    _Debugger_PreSetup ( debugger, _Context_->CurrentlyRunningWord ) ;
                }
                break ;
            }
            else if ( key == '\\' )
            {
                DebugOff ; //SetState ( _CfrTil_, DEBUG_MODE, false ) ;
                SetState ( _Debugger_, DBG_COMMAND_LINE, true ) ;
                Debugger_InterpretLine ( ) ;
                SetState ( _Debugger_, DBG_COMMAND_LINE, false ) ;
            }
            else
            {
                rtrn = 1 ;
                break ;
            }
        }
        while ( 1 ) ;
    }
    DefaultColors ;
    return rtrn ;
}

int32
_OpenVmTil_Pause ( )
{
    DebugColors ;
    return _OVT_Pause ( 0 ) ;
}

void
OpenVmTil_Pause ( byte * msg )
{
    Printf ( ( byte* ) "%s", msg ) ;
    _OpenVmTil_Pause ( ) ;
}

void
_OVT_Throw ( int32 restartCondition )
{
    _Q_->RestartCondition = restartCondition ;
    SetBuffersUnused ;
    if ( ( restartCondition > ABORT ) || ( ( _Q_->Signal == SIGSEGV ) ) )
    {
        if ( _Q_->Signal == SIGSEGV )
        {
            //siglongjmp ( _Q_->JmpBuf0, 1 ) ;

            sigset_t signal_set ;
            sigemptyset ( &signal_set ) ;
            sigaddset ( &signal_set, SIGSEGV ) ;
            sigprocmask ( SIG_UNBLOCK, &signal_set, NULL ) ;
#if 1            
            if ( ++ _Q_->SigSegvs < 2 )
            {
                //_OpenVmTil_ShowExceptionInfo ( ) ;
                _Q_->RestartCondition = ABORT ;
                siglongjmp ( _CfrTil_->JmpBuf0, 1 ) ;
            }
            else
#endif            
                _Q_->RestartCondition = INITIAL_START ;
        }
        siglongjmp ( _Q_->JmpBuf0, 1 ) ;
    }
    else siglongjmp ( _CfrTil_->JmpBuf0, 1 ) ;
}

void
OpenVmTil_Throw ( byte * excptMessage, int32 restartCondition, int32 infoFlag )
{
    _Q_->ExceptionMessage = excptMessage ;
    _Q_->Thrown = restartCondition ;
    if ( _Q_ && ( infoFlag && _OpenVmTil_ShowExceptionInfo ( ) ) || ( _Q_->Signal == SIGSEGV ) ) _OVT_Throw ( restartCondition ) ;
}

void
_OpenVmTil_LongJmp_WithMsg ( int32 restartCondition, byte * msg )
{
    OpenVmTil_Throw ( msg, restartCondition, 1 ) ;
}

void
OpenVmTil_SignalAction ( int signal, siginfo_t * si, void * uc )
{
    d0 ( Printf ( ( byte* ) "\nOpenVmTil_SignalAction :: signal = %d\n", signal ) ) ;
    if ( ( signal >= SIGCHLD ) ) _Q_->SigAddress = 0 ; //|| ( signal == SIGWINCH ) ) _Q_->SigAddress = 0 ; // 17 : "CHILD TERMINATED" : ignore; its just back from a shell fork
    else
    {
        _Q_->Signal = signal ;
        _Q_->SigAddress = si->si_addr ;
        _Q_->SigLocation = ( ( signal != SIGSEGV ) && _Context_ ) ? ( byte* ) c_dd ( Context_Location ( ) ) : ( byte* ) "" ;

        siglongjmp ( _Q_->JmpBuf0, 1 ) ;

        _OVT_Throw ( _Q_->RestartCondition ) ;
    }
}

void
CfrTil_Exception ( int32 signal, int32 restartCondition )
{
    byte * b = Buffer_Data ( _CfrTil_->ScratchB1 ) ;
    AlertColors ;
    switch ( signal )
    {
        case CASE_NOT_LITERAL_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Syntax Error : \"case\" only takes a literal/constant as its parameter after the block", restartCondition, 1 ) ;
            break ;
        }
        case DEBUG_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Debug Error : User is not in debug mode", restartCondition, 1 ) ;
            break ;
        }
        case OBJECT_REFERENCE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Object Reference Error", restartCondition, 1 ) ;
            break ;
        }
        case OBJECT_SIZE_ERROR:
        {
            sprintf ( ( char* ) b, "Exception : Warning : Class object size is 0. Did you declare 'size' for %s? ",
                _Context_->CurrentlyRunningWord->ContainingNamespace->Name ) ;
            OpenVmTil_Throw ( b, restartCondition, 1 ) ;
            break ;
        }
        case STACK_OVERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Stack Overflow", restartCondition, 1 ) ;
            break ;
        }
        case STACK_UNDERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Stack Underflow", restartCondition, 1 ) ;
            break ;
        }
        case STACK_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Stack Error", restartCondition, 1 ) ;
            break ;
        }
        case SEALED_NAMESPACE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : New words can not be added to sealed namespaces", restartCondition, 1 ) ;
            break ;
        }
        case NAMESPACE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Namespace (Not Found?) Error", restartCondition, 1 ) ;
            break ;
        }
        case SYNTAX_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Syntax Error", restartCondition, 1 ) ;
            break ;
        }
        case NESTED_COMPILE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Nested Compile Error", restartCondition, 1 ) ;
            break ;
        }
        case COMPILE_TIME_ONLY:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Compile Time Use Only", restartCondition, 1 ) ;
            break ;
        }
        case BUFFER_OVERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Buffer Overflow", restartCondition, 1 ) ;
            break ;
        }
        case MEMORY_ALLOCATION_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Memory Allocation Error", restartCondition, 1 ) ;
            break ;
        }
        case LABEL_NOT_FOUND_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Word not found. Misssing namespace qualifier?\n", QUIT, 1 ) ;
            break ;
        }
        case NOT_A_KNOWN_OBJECT:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Not a known object.\n", QUIT, 1 ) ;
            break ;
        }
        case ARRAY_DIMENSION_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Array has no dimensions!? ", QUIT, 1 ) ;
            break ;
        }
        case INLINE_MULTIPLE_RETURN_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Multiple return points in a inlined function", restartCondition, 1 ) ;
            break ;
        }
        case MACHINE_CODE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : in machine coding", restartCondition, 1 ) ;
            break ;
        }
        case VARIABLE_NOT_FOUND_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Variable not found error", restartCondition, 1 ) ;
            break ;
        }
        case FIX_ME_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Fix Me", restartCondition, 1 ) ;
            break ;
        }
        default:
        {
            OpenVmTil_Throw ( 0, restartCondition, 1 ) ;
            break ;
        }
    }
    return ;
}

// ?? the logic of exceptions could be reworked ??

void
Error3 ( byte * format, byte * one, byte * two, int three )
{
    char buffer [ 128 ] ;
    sprintf ( ( char* ) buffer, ( char* ) format, one, two ) ;
    Error ( ( byte* ) buffer, three ) ;
}

void
Error2 ( byte * format, byte * one, int two )
{
    char buffer [ 128 ] ;
    sprintf ( ( char* ) buffer, ( char* ) format, one ) ;
    Error ( ( byte* ) buffer, two ) ;
}

