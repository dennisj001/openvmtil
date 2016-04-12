
#include "../includes/cfrtil.h"

// ?? this file could be worked on ??

int32
_OpenVmTil_ShowExceptionInfo ( )
{
    AlertColors ;
    if ( _Q_->Verbosity )
    {
        if ( _Q_->SignalExceptionsHandled ++ < 2 )
        {
            Debugger * debugger ;
            if ( _Q_->OVT_CfrTil && (debugger = _Q_->OVT_CfrTil->Debugger0 ))
            {
                Debugger_ShowInfo ( debugger, _Q_->ExceptionMessage, _Q_->Signal ) ;
                if ( GetState ( debugger, DBG_STEPPING ) ) Debugger_Registers ( debugger ) ;
                if ( _Q_->Signal != 11 )
                {
                    Word * word = Word_GetFromCodeAddress ( ( byte* ) _Q_->SigAddress ) ;
                    if ( ! word ) word = _Q_->OVT_Context->CurrentRunWord ;
                    if ( word )
                    {
                        _CfrTil_Source ( word, 0 ) ;
                        if ( ! CompileMode && ( ! ( word->CType & CPRIMITIVE ) ) ) _CfrTil_Word_Disassemble ( word ) ;
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
    int key ;
    DebugColors ;
    do
    {
        _Printf ( ( byte* ) "%s", prompt ) ;
        key = Key ( ) ;
        _ReadLine_PrintfClearTerminalLine ( ) ;
        if ( key == 'd' )
        {
            SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, true ) ;
            _Q_->OVT_CfrTil->Debugger0->TokenStart_ReadLineIndex = 0 ; // prevent turning off _Debugger_PreSetup
            _Debugger_PreSetup ( _Q_->OVT_CfrTil->Debugger0, 0, _Q_->OVT_Context->CurrentRunWord ) ;
            return 0 ;//break ;
        }
        else if ( key == '\\' )
        {
            SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, false ) ;
            SetState ( _Q_->OVT_CfrTil->Debugger0, DBG_COMMAND_LINE, true ) ;
            Debugger_InterpretLine ( ) ;
        }
        else break ;
    }
    while ( 1 ) ;
    DefaultColors ;
    return 1 ;
}

int32
_OpenVmTil_Pause ( )
{
    Context * cntx = _Q_->OVT_Context ;
    byte buffer [256] ;
    snprintf ( ( char* ) buffer, 256, "\nPausing at %s : Any <key> to continue... :: 'd' for debugger, '\\' for a command prompt ...", _Context_Location ( cntx ) ) ;
    return _OVT_Pause ( buffer ) ;
}

void
OpenVmTil_Pause ( )
{
    //_DataStack_Push ( 
    _OpenVmTil_Pause ( ) ;
}

void
_OpenVmTil_Throw ( jmp_buf * jb, byte * excptMessage, int32 restartCondition )
{
    _Q_->ExceptionMessage = excptMessage ;
    _Q_->RestartCondition = restartCondition ;
    _Q_->Thrown = restartCondition ;
    SetBuffersUnused ;
    if ( _OpenVmTil_ShowExceptionInfo ( ) || ( _Q_->Signal == SIGSEGV ) ) longjmp ( *jb, - 1 ) ;
}

void
OpenVmTil_Throw ( byte * excptMessage, int32 restartCondition )
{
    if ( _Q_ ) _OpenVmTil_Throw ( &_Q_->OVT_CfrTil->JmpBuf0, excptMessage, restartCondition ) ;
}

void
_OpenVmTil_LongJmp_WithMsg ( int32 restartCondition, byte * msg )
{
    OpenVmTil_Throw ( msg, restartCondition ) ;
}

void
OpenVmTil_SignalAction ( int signal, siginfo_t * si, void * uc )
{
    if ( signal == SIGCHLD ) _Q_->SigAddress = 0 ; // 17 : "CHILD TERMINATED" : ignore; its just back from a shell fork
    else
    {
        _Q_->Signal = signal ;
        _Q_->SigAddress = si->si_addr ;
        _Q_->SigLocation = signal != SIGSEGV ? ( byte* ) c_dd ( Context_Location ( ) ) : ( byte* ) "" ;
        OpenVmTil_Throw ( 0, 0 ) ;
        //siglongjmp ( _Q_->OVT_CfrTil->JmpBuf0, - 1 ) ;
    }
}

void
CfrTil_Exception ( int32 signal, int32 restartCondition )
{
    byte * b = Buffer_Data ( _Q_->OVT_CfrTil->Scratch1B ) ;
    AlertColors ;
    switch ( signal )
    {
        case CASE_NOT_LITERAL_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Syntax Error : \"case\" only takes a literal/constant as its parameter after the block", restartCondition ) ;
            break ;
        }
        case DEBUG_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Debug Error : User is not in debug mode", restartCondition ) ;
            break ;
        }
        case OBJECT_REFERENCE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Object Reference Error", restartCondition ) ;
            break ;
        }
        case OBJECT_SIZE_ERROR:
        {
            sprintf ( ( char* ) b, "Exception : Warning : Class object size is 0. Did you declare 'size' for %s? ",
                _Q_->OVT_Context->CurrentRunWord->ContainingNamespace->Name ) ;
            OpenVmTil_Throw ( b, restartCondition ) ;
            break ;
        }
        case STACK_OVERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Stack Overflow", restartCondition ) ;
            break ;
        }
        case STACK_UNDERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Stack Underflow", restartCondition ) ;
            break ;
        }
        case STACK_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Stack Error", restartCondition ) ;
            break ;
        }
        case SEALED_NAMESPACE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : New words can not be added to sealed namespaces", restartCondition ) ;
            break ;
        }
        case NAMESPACE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Namespace (Not Found?) Error", restartCondition ) ;
            break ;
        }
        case SYNTAX_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Syntax Error", restartCondition ) ;
            break ;
        }
        case NESTED_COMPILE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Nested Compile Error", restartCondition ) ;
            break ;
        }
        case COMPILE_TIME_ONLY:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Compile Time Use Only", restartCondition ) ;
            break ;
        }
        case BUFFER_OVERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Buffer Overflow", restartCondition ) ;
            break ;
        }
        case MEMORY_ALLOCATION_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Memory Allocation Error", restartCondition ) ;
            break ;
        }
        case NOT_A_KNOWN_OBJECT:
        case LABEL_NOT_FOUND_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Word not found. Misssing namespace qualifier? ", QUIT ) ;
            break ;
        }
        case ARRAY_DIMENSION_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Array has no dimensions!? ", QUIT ) ;
            break ;
        }
        case INLINE_MULTIPLE_RETURN_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Multiple return points in a inlined function", restartCondition ) ;
            break ;
        }
        case MACHINE_CODE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : in machine coding", restartCondition ) ;
            break ;
        }
        case VARIABLE_NOT_FOUND_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Variable not found error", restartCondition ) ;
            break ;
        }
        case FIX_ME_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Exception : Fix Me", restartCondition ) ;
            break ;
        }
        default:
        {
            OpenVmTil_Throw ( 0, restartCondition ) ;
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

