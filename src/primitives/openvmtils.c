
#include "../include/cfrtil.h"

void
OpenVmTil_Verbosity ( )
{
    _DataStack_Push ( ( int64 ) & _Q_->Verbosity ) ;
}

void
Ovt_Optimize ( )
{
    _DataStack_Push ( ( int64 ) GetState ( _CfrTil_, OPTIMIZE_ON ) ? 1 : 0 ) ;
}

void
Ovt_Inlining ( )
{
    _DataStack_Push ( ( int64 ) GetState ( _CfrTil_, INLINE_ON ) ? 1 : 0 ) ;
}

// allows variables to be created on first use without a "var" declaration

void
Ovt_AutoVar ( )
{
    _DataStack_Push ( ( int64 ) GetState ( _Q_, AUTO_VAR ) ? 1 : 0 ) ;
}

void
Ovt_AutoVarOff ( )
{
    SetState ( _Q_, AUTO_VAR, false ) ;
}

// allows variables to be created on first use without a "var" declaration

void
Ovt_AutoVarOn ( )
{
    SetState ( _Q_, AUTO_VAR, true ) ;
}

void
OpenVmTil_HistorySize ( )
{
    _DataStack_Push ( ( int64 ) & _Q_->HistorySize ) ;
}

void
OpenVmTil_DataStackSize ( )
{
    _DataStack_Push ( ( int64 ) & _Q_->DataStackSize ) ;
}

void
OpenVmTil_CodeSize ( )
{
    _DataStack_Push ( ( int64 ) & _Q_->MachineCodeSize ) ;
}

void
OpenVmTil_SessionObjectsSize ( )
{
    _DataStack_Push ( ( int64 ) & _Q_->SessionObjectsSize ) ;
}

void
OpenVmTil_CompilerTempObjectsSize ( )
{
    _DataStack_Push ( ( int64 ) & _Q_->CompilerTempObjectsSize ) ;
}

void
OpenVmTil_ObjectsSize ( )
{
    _DataStack_Push ( ( int64 ) & _Q_->ObjectsSize ) ;
}

void
OpenVmTil_DictionarySize ( )
{
    _DataStack_Push ( ( int64 ) & _Q_->DictionarySize ) ;
}

void
OpenVmTil_Print_DataSizeofInfo ( int64 flag )
{
    if ( flag || ( _Q_->Verbosity > 1 ) )
    {
        _Printf ( ( byte* ) "\nOpenVimTil size : %d bytes, ", sizeof (OpenVmTil ) ) ;
        _Printf ( ( byte* ) "Object size : %d bytes, ", sizeof (Object ) ) ;
        _Printf ( ( byte* ) "dobject size : %d bytes, ", sizeof ( dobject ) ) ;
        _Printf ( ( byte* ) "DLNode size : %d bytes, ", sizeof ( DLNode ) ) ;
        _Printf ( ( byte* ) "PropInfo size : %d bytes, ", sizeof (PropInfo ) ) ;
        //_Printf ( ( byte* ) "\nCProperty0 size : %d bytes, ", sizeof (struct _T_CProperty0 ) ) ;
        _Printf ( ( byte* ) "CfrTil size : %d bytes, ", sizeof (CfrTil ) ) ;
        _Printf ( ( byte* ) "Context size : %d bytes, ", sizeof (Context ) ) ;
        _Printf ( ( byte* ) "System size : %d bytes, ", sizeof (System ) ) ;
        _Printf ( ( byte* ) "Debugger size : %d bytes, ", sizeof (Debugger ) ) ;
        _Printf ( ( byte* ) "\nMemorySpace size : %d bytes, ", sizeof (MemorySpace ) ) ;
        _Printf ( ( byte* ) "ReadLiner size : %d bytes, ", sizeof (ReadLiner ) ) ;
        _Printf ( ( byte* ) "Lexer size : %d bytes, ", sizeof (Lexer ) ) ;
        _Printf ( ( byte* ) "Interpreter size : %d bytes, ", sizeof (Interpreter ) ) ;
        _Printf ( ( byte* ) "\nFinder size : %d bytes, ", sizeof (Finder ) ) ;
        _Printf ( ( byte* ) "Compiler size : %d bytes, ", sizeof (Compiler ) ) ;
        _Printf ( ( byte* ) "Word size : %d bytes, ", sizeof (Word ) ) ;
        _Printf ( ( byte* ) "Symbol size : %d bytes, ", sizeof (Symbol ) ) ;
        _Printf ( ( byte* ) "\ndlnode size : %d bytes, ", sizeof (dlnode ) ) ;
        _Printf ( ( byte* ) "dllist size : %d bytes, ", sizeof (dllist ) ) ;
        _Printf ( ( byte* ) "WordData size : %d bytes, ", sizeof (WordData ) ) ;
        _Printf ( ( byte* ) "ListObject size : %d bytes, ", sizeof ( ListObject ) ) ;
        _Printf ( ( byte* ) "\nByteArray size : %d bytes, ", sizeof (ByteArray ) ) ;
        _Printf ( ( byte* ) "NamedByteArray size : %d bytes, ", sizeof (NamedByteArray ) ) ;
        _Printf ( ( byte* ) "MemChunk size : %d bytes", sizeof ( MemChunk ) ) ;
    }
}

void
OVT_ShowMemoryAllocated ()
{
    _OVT_ShowMemoryAllocated ( _Q_ ) ;
}

void
OVT_Exit ( )
{
    if ( _Q_->Verbosity > 0 ) _Printf ( ( byte* ) "bye\n" ) ;
    exit ( 0 ) ;
}

void
_OVT_Ok ( int64 promptFlag )
{
    if ( _Q_->Verbosity > 3 )
    {
        _CfrTil_SystemState_Print ( 0 ) ;
        _Printf ( ( byte* ) "\n<Esc> - break, <Ctrl-C> - quit, <Ctrl-D> - restart, \'bye\'/\'exit\' - leave." ) ;
    }
    if ( ( _Q_->Verbosity >  0 ) && ( _Q_->InitSessionCoreTimes ++ == 1 ) )
    {
        System_Time ( _CfrTil_->Context0->System0, 0, ( char* ) "Startup", 1 ) ; //_Q_->StartedTimes == 1 ) ;
        _CfrTil_Version ( 0 ) ;
        _Printf ( (byte*) "\nOpenVmTil : cfrTil comes with ABSOLUTELY NO WARRANTY; for details type `license' in the source directory." ) ;
        _Printf ( (byte*) "\nType 'testCfrTil' or 'test' <tab><enter>; 'demo' for starters" ) ;
        _Printf ( (byte*) "\nType 'bye' to exit" ) ;
        //fflush ( stdout ) ;
    }
    _Context_Prompt ( _Q_->Verbosity && promptFlag ) ;
}

void
OVT_Ok ( )
{
    _OVT_Ok ( 1 ) ;
    //_CfrTil_Prompt ( _Q_->Verbosity && ( ( _Q_->RestartCondition < RESET_ALL ) || _Q_->StartTimes > 1 ) ) ;
}

#if 0 // not used

void
OVT_Prompt ( )
{
    if ( GetState ( _Context_->System0, DO_PROMPT ) ) // && ( ( _Context->OutputLineCharacterNumber == 0 ) || ( _Context->OutputLineCharacterNumber > 3 ) ) )
    {
        _Context_Prompt ( 1 ) ;
    }
}
#endif

