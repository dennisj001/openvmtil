
#include "../include/cfrtil.h"

void
OpenVmTil_Verbosity ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->Verbosity ) ;
}

void
Ovt_Optimize ( )
{
    _DataStack_Push ( ( int32 ) GetState ( _CfrTil_, OPTIMIZE_ON ) ? 1 : 0 ) ;
}

void
Ovt_Inlining ( )
{
    _DataStack_Push ( ( int32 ) GetState ( _CfrTil_, INLINE_ON ) ? 1 : 0 ) ;
}

// allows variables to be created on first use without a "var" declaration

void
Ovt_AutoVar ( )
{
    _DataStack_Push ( ( int32 ) GetState ( _Q_, AUTO_VAR ) ? 1 : 0 ) ;
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
    _DataStack_Push ( ( int32 ) & _Q_->HistorySize ) ;
}

void
OpenVmTil_DataStackSize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->DataStackSize ) ;
}

void
OpenVmTil_CodeSize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->MachineCodeSize ) ;
}

void
OpenVmTil_SessionObjectsSize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->SessionObjectsSize ) ;
}

void
OpenVmTil_CompilerTempObjectsSize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->CompilerTempObjectsSize ) ;
}

void
OpenVmTil_ObjectsSize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->ObjectsSize ) ;
}

void
OpenVmTil_DictionarySize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->DictionarySize ) ;
}

void
OpenVmTil_Print_DataSizeofInfo ( int flag )
{
    if ( flag || ( _Q_->Verbosity > 1 ) )
    {
        Printf ( ( byte* ) "\nOpenVimTil size : %d bytes, ", sizeof (OpenVmTil ) ) ;
        Printf ( ( byte* ) "Object size : %d bytes, ", sizeof (Object ) ) ;
        Printf ( ( byte* ) "dobject size : %d bytes, ", sizeof ( dobject ) ) ;
        Printf ( ( byte* ) "DLNode size : %d bytes, ", sizeof ( DLNode ) ) ;
        Printf ( ( byte* ) "PropInfo size : %d bytes, ", sizeof (PropInfo ) ) ;
        Printf ( ( byte* ) "\nCProperty0 size : %d bytes, ", sizeof (struct _T_CProperty0 ) ) ;
        Printf ( ( byte* ) "CfrTil size : %d bytes, ", sizeof (CfrTil ) ) ;
        Printf ( ( byte* ) "Context size : %d bytes, ", sizeof (Context ) ) ;
        Printf ( ( byte* ) "System size : %d bytes, ", sizeof (System ) ) ;
        Printf ( ( byte* ) "Debugger size : %d bytes, ", sizeof (Debugger ) ) ;
        Printf ( ( byte* ) "\nMemorySpace size : %d bytes, ", sizeof (MemorySpace ) ) ;
        Printf ( ( byte* ) "ReadLiner size : %d bytes, ", sizeof (ReadLiner ) ) ;
        Printf ( ( byte* ) "Lexer size : %d bytes, ", sizeof (Lexer ) ) ;
        Printf ( ( byte* ) "Interpreter size : %d bytes, ", sizeof (Interpreter ) ) ;
        Printf ( ( byte* ) "\nFinder size : %d bytes, ", sizeof (Finder ) ) ;
        Printf ( ( byte* ) "Compiler size : %d bytes, ", sizeof (Compiler ) ) ;
        Printf ( ( byte* ) "Word size : %d bytes, ", sizeof (Word ) ) ;
        Printf ( ( byte* ) "Symbol size : %d bytes, ", sizeof (Symbol ) ) ;
        Printf ( ( byte* ) "\ndlnode size : %d bytes, ", sizeof (dlnode ) ) ;
        Printf ( ( byte* ) "dllist size : %d bytes, ", sizeof (dllist ) ) ;
        Printf ( ( byte* ) "WordData size : %d bytes, ", sizeof (WordData ) ) ;
        Printf ( ( byte* ) "ListObject size : %d bytes, ", sizeof ( ListObject ) ) ;
        Printf ( ( byte* ) "\nByteArray size : %d bytes, ", sizeof (ByteArray ) ) ;
        Printf ( ( byte* ) "NamedByteArray size : %d bytes, ", sizeof (NamedByteArray ) ) ;
        Printf ( ( byte* ) "MemChunk size : %d bytes\n\n", sizeof ( MemChunk ) ) ;
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
    if ( _Q_->Verbosity > 0 ) Printf ( ( byte* ) "\nbye\n" ) ;
    exit ( 0 ) ;
}

void
_OVT_Ok ( int32 promptFlag )
{
    if ( _Q_->Verbosity > 2 )
    {
        _CfrTil_SystemState_Print ( 0 ) ;
        //CfrTil_MemorySpaceAllocated ( ( byte* ) "SessionObjectsSpace" ) ;
        Printf ( ( byte* ) "\n<Esc> - break, <Ctrl-C> - quit, <Ctrl-D> - restart, \"exit\" - leave.\n ok " ) ;
    }
    if ( ( _Q_->Verbosity ) && ( _Q_->InitSessionCoreTimes ++ == 1 ) )
    {
        System_Time ( _CfrTil_->Context0->System0, 0, ( char* ) "Startup", 1 ) ; //_Q_->StartedTimes == 1 ) ;
        _CfrTil_Version ( 0 ) ;
        Printf ( (byte*) "\nOpenVmTil : cfrTil comes with ABSOLUTELY NO WARRANTY; for details type `license' in the source directory." ) ;
        Printf ( (byte*) "\nType 'bye' to exit" ) ;
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

