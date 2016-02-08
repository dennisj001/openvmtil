
#include "../includes/cfrtil.h"
#define VERSION ((byte*) "0.778.510" ) 

// the only extern variable but there are two global structures in primitives.c
OpenVmTil * _Q_ ;
struct termios SavedTerminalAttributes ;

int
main ( int argc, char * argv [ ] )
{
    LinuxInit ( &SavedTerminalAttributes ) ;
    _OpenVmTil ( argc, argv ) ;
}

void
_OpenVmTil ( int argc, char * argv [ ] ) 
{
    while ( 1 )
    {
        OpenVmTil * ovt = _Q_ = _OpenVmTil_New ( _Q_, argc, argv, & SavedTerminalAttributes ) ;
        if ( ! setjmp ( ovt->JmpBuf0 ) )
        {
            _OpenVmTil_Run ( ovt ) ;
        }
    }
}

void
_OpenVmTil_Run ( OpenVmTil * ovt )
{
    _CfrTil_Run ( ovt->OVT_CfrTil, ovt->RestartCondition ) ;
}

OpenVmTil *
_OpenVmTil_Allocate ( )
{
    OpenVmTil * ovt = ( OpenVmTil* ) mmap_AllocMem ( sizeof ( OpenVmTil ) ) ; //_Mem_Allocate ( 0, sizeof ( OpenVmTil ), 0, ( RETURN_CHUNK_HEADER ) ) ; // don't add this to mem alloc system ; ummap it when done
    DLList_Init ( &ovt->PermanentMemList, &ovt->PML_HeadNode, &ovt->PML_TailNode ) ;
    ovt->OVT_InitialUnAccountedMemory = sizeof ( OpenVmTil ) ; // needed here because '_Q_' was not initialized yet for MemChunk accounting
    return ovt ;
}

void
_OpenVmTil_Init ( OpenVmTil * ovt, int resetHistory )
{
    MemorySpace_New ( ) ; // nb : memory must be after we set Size values and before lists; list are allocated from memory
    _HistorySpace_New ( ovt, resetHistory ) ;
    ovt->psi_PrintStateInfo = PrintStateInfo_New ( ) ; // variable init needed by any allocation which call Printf
    ovt->VersionString = VERSION ;
    // ? where do we want the init file ?
    if ( _File_Exists ( (byte*) "./.init.cft" ) )
    {
        ovt->InitString = ( byte* ) "\"./.init.cft\" include" ; // could allow override with a startup parameter
        SetState ( ovt, OVT_IN_USEFUL_DIRECTORY, true ) ;
    }
    else
    {
        ovt->InitString = ( byte* ) "\"/usr/local/lib/cfrTil/.init.cft\" include" ; // could allow override with a startup parameter
        SetState ( ovt, OVT_IN_USEFUL_DIRECTORY, false ) ;
    }
    if ( ovt->Verbosity > 1 )
    {
        Printf ( ( byte* ) "\nRestart : All memory freed, allocated and initialized as at startup. "
            "termios, verbosity and memory category allocation sizes preserved. verbosity = %d.", ovt->Verbosity ) ;
        OpenVmTil_Print_DataSizeofInfo ( 0 ) ;
    }
    _OpenVmTil_ColorsInit ( ovt ) ;
}

void
Ovt_RunInit ( OpenVmTil * ovt )
{
    ovt->SignalExceptionsHandled = 0 ;
    ovt->StartedTimes ++ ;
    ovt->RestartCondition = STOP ;
}

void
OpenVmTil_Delete ( OpenVmTil * ovt )
{
    if ( ovt )
    {
        if ( ovt->Verbosity > 2 ) Printf ( ( byte* ) "\nAll allocated memory is being freed.\nRestart : verbosity = %d.", ovt->Verbosity ) ;
        FreeChunkList ( &ovt->PermanentMemList ) ;
        munmap ( ovt->MemorySpace0, sizeof ( MemorySpace ) ) ;
        munmap ( ovt, sizeof ( OpenVmTil ) ) ;
    }
    _Q_ = 0 ;
}

OpenVmTil *
_OpenVmTil_New ( OpenVmTil * ovt, int argc, char * argv [ ], struct termios * savedTerminalAttributes )
{
    char errorFilename [256] ;
    int32 fullRestart, restartCondition, startIncludeTries, verbosity, objectsSize, tempObjectsSize, codeSize, dictionarySize,
        sessionObjectsSize, dataStackSize, historySize, lispTempSize, compilerTempObjectsSize, exceptionsHandled, contextSize ; // inlining, optimize ;
    if ( ! ovt )
    {
        fullRestart = INITIAL_START ;
    }
    else fullRestart = ( ovt->RestartCondition == INITIAL_START ) ;


    startIncludeTries = ( ovt && ( ! fullRestart ) ) ? ovt->StartIncludeTries : 0 ;
    if ( startIncludeTries )
    {
        if ( ovt->OVT_Context->ReadLiner0->Filename ) strcpy ( errorFilename, ( char* ) ovt->OVT_Context->ReadLiner0->Filename ) ;
        else strcpy ( errorFilename, "Debug Context" ) ;
    }
    else errorFilename [ 0 ] = 0 ;
    restartCondition = ( ! fullRestart ) && ( startIncludeTries < 2 ) ? ovt->RestartCondition : RESTART ;

    if ( restartCondition < RESTART )
    {
        verbosity = ovt->Verbosity ;
        // preserve values across partial restarts
        sessionObjectsSize = ovt->SessionObjectsSize ;
        dictionarySize = ovt->DictionarySize ;
        lispTempSize = ovt->LispTempSize ;
        codeSize = ovt->MachineCodeSize ;
        objectsSize = ovt->ObjectsSize ;
        tempObjectsSize = ovt->TempObjectsSize ;
        compilerTempObjectsSize = ovt->CompilerTempObjectsSize ;
        dataStackSize = ovt->DataStackSize ;
        historySize = ovt->HistorySize ;
        contextSize = ovt->ContextSize ;
        exceptionsHandled = ovt->SignalExceptionsHandled ;
    }
    else // default values
    {
        verbosity = 1 ;
        objectsSize = OBJECTS_SIZE ;
        tempObjectsSize = TEMP_OBJECTS_SIZE ;
        sessionObjectsSize = SESSION_OBJECTS_SIZE ;
        codeSize = CODE_SIZE ;
        dictionarySize = DICTIONARY_SIZE ;
        dataStackSize = STACK_SIZE ;
        historySize = HISTORY_SIZE ;
        lispTempSize = LISP_TEMP_SIZE ;
        compilerTempObjectsSize = COMPILER_TEMP_OBJECTS_SIZE ;
        contextSize = CONTEXT_SIZE ;
        exceptionsHandled = 0 ;
    }
    OpenVmTil_Delete ( ovt ) ;
    ovt = _OpenVmTil_Allocate ( ) ;
    ovt->RestartCondition = FULL_RESTART ; //restartCondition ;
    ovt->StartIncludeTries = startIncludeTries ;
    ovt->SignalExceptionsHandled = exceptionsHandled ;
    ovt->Verbosity = verbosity ;
    ovt->MachineCodeSize = codeSize ;
    ovt->DictionarySize = dictionarySize ;
    ovt->ObjectsSize = objectsSize ;
    ovt->TempObjectsSize = tempObjectsSize ;
    ovt->SessionObjectsSize = sessionObjectsSize ;
    ovt->DataStackSize = dataStackSize ;
    ovt->HistorySize = historySize ;
    ovt->LispTempSize = lispTempSize ;
    ovt->ContextSize = contextSize ;
    ovt->CompilerTempObjectsSize = compilerTempObjectsSize ;

    ovt->Argc = argc ;
    ovt->Argv = argv ;
    ovt->SavedTerminalAttributes = savedTerminalAttributes ;
    _Q_ = ovt ;
    _OpenVmTil_Init ( ovt, exceptionsHandled > 1 ) ; // try to keep history if we can
    if ( startIncludeTries ) ovt->ErrorFilename = String_New ( ( byte* ) errorFilename, DICTIONARY ) ;
    return ovt ;
}

void
OpenVmTil_Verbosity ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->Verbosity ) ;
}

void
Ovt_Optimize ( )
{
    _DataStack_Push ( ( int32 ) GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) ? 1 : 0 ) ;
}

void
Ovt_Inlining ( )
{
    _DataStack_Push ( ( int32 ) GetState ( _Q_->OVT_CfrTil, INLINE_ON ) ? 1 : 0 ) ;
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
        Printf ( ( byte* ) "Type size : %d bytes, ", sizeof (Type ) ) ;
        Printf ( ( byte* ) "CType0 size : %d bytes, ", sizeof (struct _T_CType0 ) ) ;
        Printf ( ( byte* ) "\nCfrTil size : %d bytes, ", sizeof (CfrTil ) ) ;
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
        Printf ( ( byte* ) "\nDLNode size : %d bytes, ", sizeof (DLNode ) ) ;
        Printf ( ( byte* ) "DLList size : %d bytes, ", sizeof (DLList ) ) ;
        Printf ( ( byte* ) "WordData size : %d bytes, ", sizeof (WordData ) ) ;
        Printf ( ( byte* ) "ListObject size : %d bytes, ", sizeof ( ListObject ) ) ;
        Printf ( ( byte* ) "\nByteArray size : %d bytes, ", sizeof (ByteArray ) ) ;
        Printf ( ( byte* ) "NamedByteArray size : %d bytes, ", sizeof (NamedByteArray ) ) ;
        Printf ( ( byte* ) "MemChunk size : %d bytes", sizeof ( MemChunk ) ) ;
    }
}

void
OVT_MemoryAllocated ( )
{
    _Q_->TotalAccountedMemAllocated = _Calculate_CurrentNbaMemoryAllocationInfo ( _Q_->Verbosity > 0 ) ;
    _Q_->PermanentMemListAccounted = _OVT_ShowPermanentMemList ( 0 ) ;
    int32 sflag = 0, memDiff1 = _Q_->Mmap_TotalMemoryAllocated - _Q_->TotalAccountedMemAllocated ; //- _Q_->OVT_InitialMemAllocated ;
    int32 memDiff2 = _Q_->Mmap_TotalMemoryAllocated - _Q_->PermanentMemListAccounted ; //- _Q_->OVT_InitialMemAllocated ;
    if ( _Q_ && _Q_->OVT_CfrTil && _DataStack_ ) // so we can use this function anywhere
    {
        int32 dsu = DataStack_Depth ( ) * sizeof (int32 ) ;
        int32 dsa = ( STACK_SIZE * sizeof (int32 ) ) - dsu ;
        Printf ( ( byte* ) "\nData Stack                                  Used = %9d : Available = %9d", dsu, dsa ) ;
    }
    Printf ( ( byte* ) "\nTotal Accounted Mem                         Used = %9d : Available = %9d", _Q_->TotalAccountedMemAllocated - _Q_->MemRemaining, _Q_->MemRemaining ) ;
    Printf ( ( byte* ) "\nMmap_TotalMemoryAllocated                        = %9d : <=: _Q_->Mmap_TotalMemoryAllocated", _Q_->Mmap_TotalMemoryAllocated ) ;
    Printf ( ( byte* ) "\nMem Total Accounted Allocated                    = %9d : <=: _Q_->TotalAccountedMemAllocated", _Q_->TotalAccountedMemAllocated ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nMem Available                                    = %9d : <=: _Q_->MemRemaining", _Q_->MemRemaining ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nMem Used - Categorized                           = %9d : <=: _Q_->TotalAccountedMemAllocated - _Q_->MemRemaining", _Q_->TotalAccountedMemAllocated - _Q_->MemRemaining ) ; //+ _Q_->UnaccountedMem ) ) ;
    if ( memDiff1 || memDiff2 || ( _Q_->TotalAccountedMemAllocated != _Q_->PermanentMemListAccounted ) ) sflag = 1 ;
    Printf ( ( byte* ) "\nMem PermanentMemListAccounted                    = %9d : <=: _Q_->PermanentMemListAccounted", _Q_->PermanentMemListAccounted ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nMem Used - Permanent                             = %9d : <=: _Q_->PermanentMemListAccounted - _Q_->MemRemaining", _Q_->PermanentMemListAccounted - _Q_->MemRemaining ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nCurrent Unaccounted Diff (leak?)                 = %9d : <=: _Q_->Mmap_TotalMemoryAllocated - _Q_->PermanentMemListAccounted", memDiff2 ) ; // + _Q_->OVT_InitialMemAllocated" ) ; //+ _Q_->UnaccountedMem ) ) ;
    if ( sflag )
    {
        Printf ( ( byte* ) "\nCurrent Unaccounted Diff (leak?)                 = %9d : <=: _Q_->Mmap_TotalMemoryAllocated - _Q_->TotalAccountedMemAllocated", memDiff1 ) ; // + _Q_->OVT_InitialMemAllocated" ) ; //+ _Q_->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nCalculator ::%9d - (%9d +%9d ) = %9d", _Q_->Mmap_TotalMemoryAllocated, _Q_->TotalAccountedMemAllocated - _Q_->MemRemaining, _Q_->MemRemaining, memDiff1 ) ; //memReportedAllocated ) ; ;//+ _Q_->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nCalculator ::%9d - (%9d +%9d ) = %9d", _Q_->Mmap_TotalMemoryAllocated, _Q_->PermanentMemListAccounted - _Q_->MemRemaining, _Q_->MemRemaining, memDiff2 ) ; //memReportedAllocated ) ; ;//+ _Q_->UnaccountedMem ) ) ;
    }
}

void
OVT_Exit ( )
{
    if ( _Q_->Verbosity > 3 ) Printf ( ( byte* ) "\nbye\n" ) ;
    if ( _Q_->SignalExceptionsHandled ) _OpenVmTil_ShowExceptionInfo ( ) ;
    _ShowColors ( Color_Default, Color_Default ) ;
    //Linux_RestoreTerminalAttributes ( ) ;
    exit ( 0 ) ;
}

