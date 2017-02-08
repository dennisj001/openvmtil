
#include "../include/cfrtil.h"
#define VERSION ((byte*) "0.801.680" )

// the only extern variable but there are two global structures in primitives.c
OpenVmTil * _Q_ ;
struct termios SavedTerminalAttributes ;

int
main ( int argc, char * argv [ ] )
{
    openvmtil ( argc, argv ) ;
}

int
openvmtil ( int argc, char * argv [ ] )
{
    LinuxInit ( &SavedTerminalAttributes ) ;
    _OpenVmTil ( argc, argv ) ;
}

void
_OpenVmTil ( int argc, char * argv [ ] )
{
    int32 restartCondition = INITIAL_START ;
    while ( 1 )
    {
        OpenVmTil * ovt = _Q_ = _OpenVmTil_New ( _Q_, argc, argv, & SavedTerminalAttributes ) ;
        ovt->RestartCondition = restartCondition ;
        if ( ! sigsetjmp ( ovt->JmpBuf0, 0 ) )
        {
            _OpenVmTil_Run ( ovt ) ;
        }
        restartCondition = ovt->RestartCondition ;
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
    dllist_Init ( &ovt->PermanentMemList, &ovt->PML_HeadNode, &ovt->PML_TailNode ) ;
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
    if ( _File_Exists ( ( byte* ) "./.init.cft" ) )
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

void
_OpenVmTil_CalculateMemSpaceSizes ( OpenVmTil * ovt, int32 restartCondition, int32 totalMemSizeTarget )
{
    int32 minimalCoreMemorySize, minStaticMemSize, coreMemTargetSize, exceptionsHandled, verbosity, objectsSize, tempObjectsSize, codeSize, dictionarySize,
        sessionObjectsSize, dataStackSize, historySize, lispTempSize, compilerTempObjectsSize, contextSize, bufferSpaceSize,
        openVmTilSize, cfrTilSize ;

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
        bufferSpaceSize = ovt->BufferSpaceSize ;
        openVmTilSize = ovt->OpenVmTilSize ;
        cfrTilSize = ovt->CfrTilSize ;
        exceptionsHandled = ovt->SignalExceptionsHandled ;
    }
    else if ( totalMemSizeTarget > 0 )
    {
        verbosity = 1 ;

        // volatile mem sizes
        tempObjectsSize = 10 * K ; //TEMP_OBJECTS_SIZE ;
        sessionObjectsSize = 50 * K ; //SESSION_OBJECTS_SIZE ;
        lispTempSize = 10 * K ; //LISP_TEMP_SIZE ;
        compilerTempObjectsSize = 10 * K ; //COMPILER_TEMP_OBJECTS_SIZE ;
        historySize = 1 * K ; //HISTORY_SIZE ;
        contextSize = 10 * K ; //CONTEXT_SIZE ;
        bufferSpaceSize = 10 * K ; //BUFFER_SPACE_SIZE ;

        // static mem sizes
        openVmTilSize = 2 * K ; //OPENVMTIL_SIZE ;
        dataStackSize = 2 * K ; // STACK_SIZE
        cfrTilSize = ( dataStackSize * 4 ) + ( 12.5 * K ) ; // CFRTIL_SIZE
    }
    else // 0 or -1 get default
    {
        verbosity = 1 ;

        tempObjectsSize = 1 * MB ; //TEMP_OBJECTS_SIZE ;
        sessionObjectsSize = 1 * MB ; // SESSION_OBJECTS_SIZE ;
        lispTempSize = 1 * MB ; // LISP_TEMP_SIZE ;
        compilerTempObjectsSize = 1 * MB ; //COMPILER_TEMP_OBJECTS_SIZE ;
        contextSize = 1 * MB ; //CONTEXT_SIZE ;
        bufferSpaceSize = 1 * MB ; //BUFFER_SPACE_SIZE ;
        historySize = 1 * MB ; //HISTORY_SIZE ;

        dataStackSize = 8 * KB ; //STACK_SIZE ;
        openVmTilSize = 4 * KB ; //OPENVMTIL_SIZE ;
        cfrTilSize = ( dataStackSize * sizeof (int ) ) + ( 5 * KB ) ; //CFRTIL_SIZE ;

        exceptionsHandled = 0 ;
    }
    minStaticMemSize = tempObjectsSize + sessionObjectsSize + dataStackSize + historySize + lispTempSize + compilerTempObjectsSize +
        contextSize + bufferSpaceSize + openVmTilSize + cfrTilSize ;

    minimalCoreMemorySize = 150 * K, coreMemTargetSize = totalMemSizeTarget - minStaticMemSize ;
    coreMemTargetSize = ( coreMemTargetSize > minimalCoreMemorySize ) ? coreMemTargetSize : minimalCoreMemorySize ;
    // core memory
    objectsSize = ( int32 ) ( 0.125 * ( ( double ) coreMemTargetSize ) ) ; // we can easily allocate more object and dictionary space but not code space
    dictionarySize = ( int32 ) ( 0.125 * ( ( double ) coreMemTargetSize ) ) ;
    codeSize = ( int32 ) ( 0.75 * ( ( double ) coreMemTargetSize ) ) ;
    codeSize = ( codeSize > (500 * K) ) ? codeSize : 500 * K ;
    //if ( codeSize < (100 * K) ) codeSize = 100 * K ; // not necessary with "minimalCoreMemorySize = 150 * K"

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
    ovt->BufferSpaceSize = bufferSpaceSize ;
    ovt->CfrTilSize = cfrTilSize ;
    ovt->OpenVmTilSize = openVmTilSize ;
}

void
OVT_GetStartupOptions ( OpenVmTil * ovt )
{
    int32 i ;
    for ( i = 0 ; i < ovt->Argc ; i ++ )
    {
        if ( String_Equal ( "-m", ovt->Argv [ i ] ) )
        {
            ovt->TotalMemSizeTarget = ( atoi ( ovt->Argv [ ++ i ] ) * MB ) ;
        }
            // -s : a script file with "#! cfrTil -s" -- as first line includes the script file, the #! whole line is treated as a comment
        else if ( String_Equal ( "-f", ovt->Argv [ i ] ) || ( String_Equal ( "-s", ovt->Argv [ i ] ) ) )
        {
            ovt->StartupFilename = ( byte* ) ovt->Argv [ ++ i ] ;
        }
        else if ( String_Equal ( "-e", ovt->Argv [ i ] ) ) ovt->StartupString = ( byte* ) ovt->Argv [ ++ i ] ;
    }
    d0 (
        Printf ( "\n\nOVT_GetStartupOptions :: _Q_->Argv [0] = %s\n\n", _Q_->Argv [0] ) ;
        Printf ( "\n\nOVT_GetStartupOptions :: _Q_->Argv [1] = %s\n\n", _Q_->Argv [1] ) ;
        Printf ( "\n\nOVT_GetStartupOptions :: _Q_->Argv [2] = %s\n\n", _Q_->Argv [2] ) ;
        Printf ( "\n\nOVT_GetStartupOptions :: _Q_->StartupFilename = %s\n\n", _Q_->StartupFilename ) ;
        _Pause ( ) ;
        ) ;
}

OpenVmTil *
_OpenVmTil_New ( OpenVmTil * ovt, int argc, char * argv [ ], struct termios * savedTerminalAttributes )
{
    char errorFilename [256] ;
    int32 fullRestart, restartCondition, startIncludeTries, exceptionsHandled ;
    if ( ! ovt )
    {
        fullRestart = INITIAL_START ;
    }
    else fullRestart = ( ovt->RestartCondition == INITIAL_START ) ;


    startIncludeTries = ( ovt && ( ! fullRestart ) ) ? ovt->StartIncludeTries : 0 ;
    if ( startIncludeTries )
    {
        if ( ovt && ovt->OVT_Context && ovt->OVT_Context->ReadLiner0 && ovt->OVT_Context->ReadLiner0->Filename ) strcpy ( errorFilename, ( char* ) ovt->OVT_Context->ReadLiner0->Filename ) ;
        else strcpy ( errorFilename, "Debug Context" ) ;
    }
    else errorFilename [ 0 ] = 0 ;
    restartCondition = ( ! fullRestart ) && ( startIncludeTries < 2 ) ? ovt->RestartCondition : RESTART ;

    OpenVmTil_Delete ( ovt ) ;
    _Q_ = ovt = _OpenVmTil_Allocate ( ) ;
    ovt->RestartCondition = FULL_RESTART ;
    ovt->Argc = argc ;
    ovt->Argv = argv ;
    ovt->SavedTerminalAttributes = savedTerminalAttributes ;

    OVT_GetStartupOptions ( ovt ) ;
    int32 MIN_TotalMemSizeTarget = (300 * K) ;
    if ( ovt->TotalMemSizeTarget < MIN_TotalMemSizeTarget ) ovt->TotalMemSizeTarget = MIN_TotalMemSizeTarget ;
    int32 totalMemSizeTarget = ( ovt->TotalMemSizeTarget < 5 * M ) ? ovt->TotalMemSizeTarget : -1 ; // 0 or -1 : gets default values     
    _OpenVmTil_CalculateMemSpaceSizes ( ovt, restartCondition, totalMemSizeTarget ) ;

    _OpenVmTil_Init ( ovt, exceptionsHandled > 1 ) ; // try to keep history if we can
    Linux_SetupSignals ( &ovt->JmpBuf0, 1 ) ;
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
OVT_ShowMemoryAllocated ( )
{
    _Q_->TotalAccountedMemAllocated = _Calculate_CurrentNbaMemoryAllocationInfo ( _Q_->Verbosity > 0 ) ;
    _Q_->PermanentMemListAccounted = _OVT_ShowPermanentMemList ( 0 ) ;
    int32 sflag = 0, memDiff1 = _Q_->Mmap_TotalMemoryAllocated - _Q_->TotalAccountedMemAllocated ; //- _Q_->OVT_InitialMemAllocated ;
    int32 memDiff2 = _Q_->Mmap_TotalMemoryAllocated - _Q_->PermanentMemListAccounted ; //- _Q_->OVT_InitialMemAllocated ;
    if ( _Q_ && _CfrTil_ && _DataStack_ ) // so we can use this function anywhere
    {
        int32 dsu = DataStack_Depth ( ) * sizeof (int32 ) ;
        int32 dsa = ( STACK_SIZE * sizeof (int32 ) ) - dsu ;
        Printf ( ( byte* ) "\nData Stack                                  Used = %9d : Available = %9d", dsu, dsa ) ;
    }
    Printf ( ( byte* ) "\nTotal Accounted Mem                         Used = %9d : Available = %9d", _Q_->TotalAccountedMemAllocated - _Q_->MemRemaining, _Q_->MemRemaining ) ;
    Printf ( ( byte* ) "\nTotalMemSizeTarget                               = %9d : <=: _Q_->TotalMemSizeTarget", _Q_->TotalMemSizeTarget ) ;
    Printf ( ( byte* ) "\nMmap_TotalMemoryAllocated                        = %9d : <=: _Q_->Mmap_TotalMemoryAllocated", _Q_->Mmap_TotalMemoryAllocated ) ;
    Printf ( ( byte* ) "\nMem Total Accounted Allocated                    = %9d : <=: _Q_->TotalAccountedMemAllocated", _Q_->TotalAccountedMemAllocated ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nMem Available                                    = %9d : <=: _Q_->MemRemaining", _Q_->MemRemaining ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nMem Used - Categorized                           = %9d : <=: _Q_->TotalAccountedMemAllocated - _Q_->MemRemaining", _Q_->TotalAccountedMemAllocated - _Q_->MemRemaining ) ; //+ _Q_->UnaccountedMem ) ) ;
    if ( memDiff1 || memDiff2 || ( _Q_->TotalAccountedMemAllocated != _Q_->PermanentMemListAccounted ) ) sflag = 1 ;
    Printf ( ( byte* ) "\nMem PermanentMemListAccounted                    = %9d : <=: _Q_->PermanentMemListAccounted", _Q_->PermanentMemListAccounted ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nMem Used - Permanent                             = %9d : <=: _Q_->PermanentMemListAccounted - _Q_->MemRemaining", _Q_->PermanentMemListAccounted - _Q_->MemRemaining ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nTotal Mem Allocated                              = %9d : <=: _Q_->TotalMemAllocated", _Q_->TotalMemAllocated ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nTotal Mem Freed                                  = %9d : <=: _Q_->TotalMemFreed", _Q_->TotalMemFreed ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nTotal Mem Remaining                              = %9d : <=: _Q_->TotalMemAllocated - _Q_->TotalMemFreed", _Q_->TotalMemAllocated - _Q_->TotalMemFreed ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nTotal Mem Allocated                             %12lld : <=: TotalMemAllocated", TotalMemAllocated ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nTotal Mem Freed                                 %12lld : <=: TotalMemFreed", TotalMemFreed ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nTotal Mem Remaining                              = %9d : <=: TotalMemAllocated - TotalMemFreed", TotalMemAllocated - TotalMemFreed ) ; //+ _Q_->UnaccountedMem ) ) ;
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
        Printf ( "\nOpenVmTil : cfrTil comes with ABSOLUTELY NO WARRANTY; for details type `license' in the source directory." ) ;
        Printf ( "\nType 'bye' to exit" ) ;
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

