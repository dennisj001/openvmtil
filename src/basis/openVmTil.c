
#include "../include/cfrtil.h"
#define VERSION ((byte*) "0.808.200" )

OpenVmTil * _Q_ ; // the only globally used variable except for two extern structures in primitives.c and a couple int64 in memSpace.c and 
static struct termios SavedTerminalAttributes ;

int
main ( int argc, char * argv [ ] )
{
    openvmtil ( argc, argv ) ;
}

void
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
    ovt->OVT_InitialUnAccountedMemory = sizeof ( OpenVmTil ) ; // needed here because 'ovt' was not initialized yet for MemChunk accounting
    return ovt ;
}

void
_OpenVmTil_Init ( OpenVmTil * ovt, int resetHistory )
{
    MemorySpace_New ( ) ; // nb : memory must be after we set Size values and before lists; list are allocated from memory
    _HistorySpace_New ( ovt, resetHistory ) ;
    //ovt->psi_PrintStateInfo = PrintStateInfo_New ( ) ; // variable init needed by any allocation which call _Printf
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
        _Printf ( ( byte* ) "\nRestart : All memory freed, allocated and initialized as at startup. "
            "termios, verbosity and memory category allocation sizes preserved. verbosity = %d.", ovt->Verbosity ) ;
        OpenVmTil_Print_DataSizeofInfo ( 0 ) ;
    }
    _OpenVmTil_ColorsInit ( ovt ) ;
}

void
Ovt_RunInit ( OpenVmTil * ovt )
{
    //ovt->SignalExceptionsHandled = 0 ;
    ovt->StartedTimes ++ ;
    ovt->RestartCondition = STOP ;
}

void
OpenVmTil_Delete ( OpenVmTil * ovt )
{
    if ( ovt )
    {
        if ( ovt->Verbosity > 2 ) _Printf ( ( byte* ) "\nAll allocated memory is being freed.\nRestart : verbosity = %d.", ovt->Verbosity ) ;
        FreeChunkList ( &ovt->PermanentMemList ) ;
        mmap_FreeMem ( ( byte* ) ovt->MemorySpace0, sizeof ( MemorySpace ) ) ;
        mmap_FreeMem ( ( byte* ) ovt, sizeof ( OpenVmTil ) ) ;
    }
    _Q_ = 0 ;
}

void
_OpenVmTil_CalculateMemSpaceSizes ( OpenVmTil * ovt, int32 restartCondition, int32 totalMemSizeTarget )
{
    int32 minimalCoreMemorySize, minStaticMemSize, coreMemTargetSize, exceptionsHandled, verbosity, objectsSize, tempObjectsSize,
        sessionObjectsSize, dataStackSize, historySize, lispTempSize, compilerTempObjectsSize, contextSize, bufferSpaceSize, stringSpaceSize,
        openVmTilSize, cfrTilSize, codeSize, dictionarySize ;

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
        stringSpaceSize = ovt->StringSpaceSize ;
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
        contextSize = 5 * K ; //CONTEXT_SIZE ;
        bufferSpaceSize = 10 * K ; //BUFFER_SPACE_SIZE ;
        stringSpaceSize = 10 * K ; //BUFFER_SPACE_SIZE ;

        // static mem sizes
        dataStackSize = 2 * K ; // STACK_SIZE
        openVmTilSize = 2 * K ; //OPENVMTIL_SIZE ;
        cfrTilSize = 24 * K ; //( dataStackSize * 4 ) + ( 12.5 * K ) ; // CFRTIL_SIZE
        exceptionsHandled = 0 ;
    }
    else // 0 or -1 get default
    {
        verbosity = 1 ;

        tempObjectsSize = 1 * MB ; //TEMP_OBJECTS_SIZE ;
        sessionObjectsSize = 1 * MB ; // SESSION_OBJECTS_SIZE ;
        lispTempSize = 1 * MB ; // LISP_TEMP_SIZE ;
        compilerTempObjectsSize = 1 * MB ; //COMPILER_TEMP_OBJECTS_SIZE ;
        contextSize = 5 * K ; // CONTEXT_SIZE ;
        bufferSpaceSize = 1 * MB ; //BUFFER_SPACE_SIZE ;
        stringSpaceSize = 1 * MB ; //BUFFER_SPACE_SIZE ;
        historySize = 1 * MB ; //HISTORY_SIZE ;

        dataStackSize = 8 * KB ; //STACK_SIZE ;
        openVmTilSize = 2 * KB ; //OPENVMTIL_SIZE ;
        cfrTilSize = 24 * K ; //( dataStackSize * sizeof (int ) ) + ( 5 * KB ) ; //CFRTIL_SIZE ;

        exceptionsHandled = 0 ;
    }
    minStaticMemSize = tempObjectsSize + sessionObjectsSize + dataStackSize + historySize + lispTempSize + compilerTempObjectsSize +
        contextSize + bufferSpaceSize + openVmTilSize + cfrTilSize, stringSpaceSize ;

    minimalCoreMemorySize = 150 * K, coreMemTargetSize = totalMemSizeTarget - minStaticMemSize ;
    coreMemTargetSize = ( coreMemTargetSize > minimalCoreMemorySize ) ? coreMemTargetSize : minimalCoreMemorySize ;
    // core memory
    objectsSize = ( int32 ) ( 0.333 * ( ( double ) coreMemTargetSize ) ) ; // we can easily allocate more object and dictionary space but not code space
    dictionarySize = ( int32 ) ( 0.333 * ( ( double ) coreMemTargetSize ) ) ;
    codeSize = ( int32 ) ( 0.333 * ( ( double ) coreMemTargetSize ) ) ;
    codeSize = ( codeSize > ( 500 * K ) ) ? codeSize : 100 * K ;

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
    ovt->StringSpaceSize = stringSpaceSize ;
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
        _Printf ( ( byte* ) "\n\nOVT_GetStartupOptions :: ovt->Argv [0] = %s\n\n", ovt->Argv [0] ) ;
        _Printf ( ( byte* ) "\n\nOVT_GetStartupOptions :: ovt->Argv [1] = %s\n\n", ovt->Argv [1] ) ;
        _Printf ( ( byte* ) "\n\nOVT_GetStartupOptions :: ovt->Argv [2] = %s\n\n", ovt->Argv [2] ) ;
        _Printf ( ( byte* ) "\n\nOVT_GetStartupOptions :: ovt->StartupFilename = %s\n\n", ovt->StartupFilename ) ;
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

    int32 ium = ovt ? ovt->OVT_InitialUnAccountedMemory : 0, ovtv = ovt ? ovt->Verbosity : 0 ;
    OpenVmTil_Delete ( ovt ) ;
    d0 ( if ( ovtv > 1 )
    {
        printf ( ( byte* ) "\nTotal Mem Remaining = %9lld : <=: mmap_TotalMemAllocated - mmap_TotalMemFreed - ovt->OVT_InitialUnAccountedMemory", mmap_TotalMemAllocated - mmap_TotalMemFreed - ium ) ;
            fflush ( stdout ) ;
    } )
    _Q_ = ovt = _OpenVmTil_Allocate ( ) ;
    ovt->RestartCondition = FULL_RESTART ;
    ovt->Argc = argc ;
    ovt->Argv = argv ;
    ovt->SavedTerminalAttributes = savedTerminalAttributes ;

    OVT_GetStartupOptions ( ovt ) ;
    int32 MIN_TotalMemSizeTarget = ( 300 * K ) ;
    if ( ovt->TotalMemSizeTarget < MIN_TotalMemSizeTarget ) ovt->TotalMemSizeTarget = MIN_TotalMemSizeTarget ;
    int32 totalMemSizeTarget = ( ovt->TotalMemSizeTarget < 5 * M ) ? ovt->TotalMemSizeTarget : - 1 ; // 0 or -1 : gets default values     
    _OpenVmTil_CalculateMemSpaceSizes ( ovt, restartCondition, totalMemSizeTarget ) ;

    _OpenVmTil_Init ( ovt, exceptionsHandled > 1 ) ; // try to keep history if we can
    Linux_SetupSignals ( &ovt->JmpBuf0, 1 ) ;
    if ( startIncludeTries ) ovt->ErrorFilename = String_New ( ( byte* ) errorFilename, STRING_MEM ) ;
    return ovt ;
}

void
OVT_ShowNBAs ( OpenVmTil * ovt, int32 flag )
{
    if ( ovt )
    {
        dlnode * node, *nodeNext ;
        if ( ovt->MemorySpace0 && ( node = dllist_First ( ( dllist* ) & ovt->MemorySpace0->NBAs ) ) )
        {
            for ( ; node ; node = nodeNext )
            {
                nodeNext = dlnode_Next ( node ) ;
                NamedByteArray * nba = Get_NBA_Symbol_To_NBA ( node ) ;
                NBA_Show ( nba, flag ) ;
            }
        }
        printf ( "\n" ) ;
        fflush ( stdout ) ;
    }
}

int32
_OVT_ShowPermanentMemList ( OpenVmTil * ovt, int32 flag )
{
    int32 size ;
    if ( ovt )
    {
        int32 diff ;
        dlnode * node, *nodeNext ;
        if ( flag > 1 ) printf ( "\nMemChunk List :: " ) ;
        if ( flag ) _Printf ( ( byte* ) c_dd ( "\nformat :: Type Name or Chunk Pointer : Type : Size, ...\n" ) ) ;
        for ( size = 0, node = dllist_First ( ( dllist* ) & ovt->PermanentMemList ) ; node ; node = nodeNext )
        {
            nodeNext = dlnode_Next ( node ) ;
            if ( flag ) MemChunk_Show ( ( MemChunk * ) node ) ;
            size += ( ( MemChunk * ) node )->S_ChunkSize ;
        }
        diff = ovt->Mmap_RemainingMemoryAllocated - size ;
        if ( diff && flag )
        {
            printf ( "\nTotal Size = %9d : ovt->Mmap_RemainingMemoryAllocated = %9d :: diff = %6d", size, ovt->Mmap_RemainingMemoryAllocated, diff ) ;
            fflush ( stdout ) ;
        }
    }
    ovt->PermanentMemListRemainingAccounted = size ;
    return size ;
}

int32
_Calculate_TotalNbaAccountedMemAllocated ( OpenVmTil * ovt, int32 flag )
{
    dlnode * node, * nextNode ;
    NamedByteArray * nba ;
    ovt->TotalNbaAccountedMemRemaining = 0 ;
    ovt->TotalNbaAccountedMemAllocated = 0 ;
    if ( ovt && ovt->MemorySpace0 )
    {
        for ( node = dllist_First ( ( dllist* ) & ovt->MemorySpace0->NBAs ) ; node ; node = nextNode )
        {
            nextNode = dlnode_Next ( node ) ;
            nba = Get_NBA_Node_To_NBA ( node ) ;
            if ( flag ) NBA_Show ( nba, 0 ) ;
            ovt->TotalNbaAccountedMemAllocated += nba->TotalAllocSize ;
            ovt->TotalNbaAccountedMemRemaining += nba->MemRemaining ;
        }
        if ( _CfrTil_ && _DataStack_ ) // so we can use this function anywhere
        {
            int32 dsu = DataStack_Depth ( ) * sizeof (int32 ) ;
            int32 dsa = ( STACK_SIZE * sizeof (int32 ) ) - dsu ;
            _Printf ( ( byte* ) "\nData Stack                                  Used = %9d : Unused = %9d", dsu, dsa ) ;
        }
        _Printf ( ( byte* ) "\nTotal Accounted Mem                         Used = %9d : Unused = %9d",
            ovt->TotalNbaAccountedMemAllocated - ovt->TotalNbaAccountedMemRemaining, ovt->TotalNbaAccountedMemRemaining ) ;
        int32 diff = ovt->Mmap_RemainingMemoryAllocated - ovt->TotalNbaAccountedMemAllocated ; //- ovt->MemorySpace0->ContextSpace->TotalAllocSize ;
        if ( flag && diff )
        {
            printf ( "\n\novt->Mmap_RemainingMemoryAllocated               = %9d"
                "\novt->TotalNbaAccountedMemAllocated               = %9d"
                "\novt->Mmap_RemainingMemoryAllocated - ovt->TotalNbaAccountedMemAllocated - ovt->MemorySpace0->ContextSpace->TotalAllocSize = %6d\n",
                ovt->Mmap_RemainingMemoryAllocated, ovt->TotalNbaAccountedMemAllocated, diff ) ; //ovt->MemorySpace0->ContextSpace->TotalAllocSize, diff ) ;
            fflush ( stdout ) ;
        }
    }
    return ovt->TotalNbaAccountedMemAllocated ;
}

void
_OVT_ShowMemoryAllocated ( OpenVmTil * ovt )
{
    if ( ovt->Verbosity <= 1 ) _Printf ( ( byte* ) c_du ( "Increase the verbosity setting to 2 or more for more info here. ( Eg. : verbosity 2 = )" ) ) ;
    int32 leak = ( mmap_TotalMemAllocated - mmap_TotalMemFreed ) - ( ovt->TotalMemAllocated - ovt->TotalMemFreed ) - ovt->OVT_InitialUnAccountedMemory ;
    _Calculate_TotalNbaAccountedMemAllocated ( ovt, leak || ( ovt->Verbosity > 0 ) ) ;
    _OVT_ShowPermanentMemList ( ovt, 0 ) ;
    int32 memDiff1 = ovt->Mmap_RemainingMemoryAllocated - ovt->TotalNbaAccountedMemAllocated ; //- ovt->OVT_InitialMemAllocated ;
    int32 memDiff2 = ovt->Mmap_RemainingMemoryAllocated - ovt->PermanentMemListRemainingAccounted ; //- ovt->OVT_InitialMemAllocated ;
        _Printf ( ( byte* ) "\nTotal Accounted Mem Allocated                    = %9d : <=: ovt->TotalNbaAccountedMemAllocated", ovt->TotalNbaAccountedMemAllocated ) ;
    if ( memDiff1 || memDiff2 || leak ) _Printf ( ( byte* ) c_ad ( "\nCurrent Unaccounted Diff (leak?)                 = %9d : <=: ovt->Mmap_RemainingMemoryAllocated - ovt->PermanentMemListAccounted" ), memDiff2 ) ; // + ovt->OVT_InitialMemAllocated" ) ; //+ ovt->UnaccountedMem ) ) ;
    else _Printf ( ( byte* ) c_ud ( "\nCurrent Unaccounted Diff (leak?)                 = %9d : <=: ovt->Mmap_RemainingMemoryAllocated - ovt->PermanentMemListAccounted" ), memDiff2 ) ; // + ovt->OVT_InitialMemAllocated" ) ; //+ ovt->UnaccountedMem ) ) ;
    if ( memDiff1 || memDiff2 || leak || ( ovt->Verbosity > 1 ) )
    {
        _Printf ( ( byte* ) "\n\nTotalNbaAccountedMemAllocated                    = %9d : <=: ovt->TotalNbaAccountedMemAllocated", ovt->TotalNbaAccountedMemAllocated ) ;
        _Printf ( ( byte* ) "\nMmap_RemainingMemoryAllocated                    = %9d : <=: ovt->Mmap_RemainingMemoryAllocated", ovt->Mmap_RemainingMemoryAllocated ) ;
        _Printf ( ( byte* ) "\nPermanentMemListRemainingAccounted               = %9d : <=: ovt->PermanentMemListRemainingAccounted", ovt->PermanentMemListRemainingAccounted ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\nTotal Mem Allocated                              = %9d : <=: ovt->TotalMemAllocated", ovt->TotalMemAllocated ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\nTotal Mem Freed                                  = %9d : <=: ovt->TotalMemFreed", ovt->TotalMemFreed ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\nTotal Mem Remaining                              = %9d : <=: ovt->TotalMemAllocated - ovt->TotalMemFreed", ovt->TotalMemAllocated - ovt->TotalMemFreed ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\n\nMem Total Accounted Allocated                    = %9d : <=: ovt->TotalNbaAccountedMemAllocated", ovt->TotalNbaAccountedMemAllocated ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\nTotalNbaAccountedMemRemaining                    = %9d : <=: ovt->TotalNbaAccountedMemRemaining", ovt->TotalNbaAccountedMemRemaining ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\nMem Used - Categorized                           = %9d : <=: ovt->TotalNbaAccountedMemAllocated - ovt->TotalNbaAccountedMemRemaining", ovt->TotalNbaAccountedMemAllocated - ovt->TotalNbaAccountedMemRemaining ) ; //+ ovt->UnaccountedMem ) ) ;
        //_Printf ( ( byte* ) "\nMem Used - Permanent                             = %9d : <=: ovt->PermanentMemListAccounted - ovt->TotalNbaAccountedMemRemaining", ovt->PermanentMemListRemainingAccounted - ovt->TotalNbaAccountedMemRemaining ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\nmmap_TotalMemAllocated                          %12lld : <=: mmap_TotalMemAllocated", mmap_TotalMemAllocated ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\nmmap_TotalMemFreed                              %12lld : <=: mmap_TotalMemFreed", mmap_TotalMemFreed ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\nmmap Total Mem Remaining                         = %9d : <=: mmap_TotalMemAllocated - mmap_TotalMemFreed", mmap_TotalMemAllocated - mmap_TotalMemFreed ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\nTotalMemSizeTarget                               = %9d : <=: ovt->TotalMemSizeTarget", ovt->TotalMemSizeTarget ) ;
        _Printf ( ( byte* ) "\nCurrent Unaccounted Diff (leak?)                 = %9d : <=: ovt->Mmap_RemainingMemoryAllocated - ovt->PermanentMemListAccounted", memDiff2 ) ; // + ovt->OVT_InitialMemAllocated" ) ; //+ ovt->UnaccountedMem ) ) ;
        if ( leak ) _Printf ( ( byte* ) "\nOVT_InitialUnAccountedMemory                     = %9d : <=: ovt->OVT_InitialUnAccountedMemory", ovt->OVT_InitialUnAccountedMemory ) ; //+ ovt->UnaccountedMem ) ) ;

        _Printf ( ( byte* ) "\nCurrent Unaccounted Diff (leak?)                 = %9d : <=: ovt->Mmap_RemainingMemoryAllocated - ovt->TotalNbaAccountedMemAllocated", memDiff1 ) ; // + ovt->OVT_InitialMemAllocated" ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\nTotalMemRemainings leak                          = %9d", leak ) ; //+ ovt->UnaccountedMem ) ) ;
        _Printf ( ( byte* ) "\nCalculator ::%9d - (%9d +%9d ) = %9d", ovt->Mmap_RemainingMemoryAllocated, ovt->TotalNbaAccountedMemAllocated - ovt->TotalNbaAccountedMemRemaining, ovt->TotalNbaAccountedMemRemaining, memDiff1 ) ; //memReportedAllocated ) ; ;//+ ovt->UnaccountedMem ) ) ;
        //_Printf ( ( byte* ) "\nCalculator ::%9d - (%9d +%9d ) = %9d :: ovt->Mmap_RemainingMemoryAllocated - ( ovt->PermanentMemListRemainingAccounted + ovt->TotalNbaAccountedMemRemaining ) = ovt->TotalNbaAccountedMemRemaining", ovt->Mmap_RemainingMemoryAllocated, ovt->PermanentMemListRemainingAccounted - ovt->TotalNbaAccountedMemRemaining, ovt->TotalNbaAccountedMemRemaining, memDiff2 ) ; //memReportedAllocated ) ; ;//+ ovt->UnaccountedMem ) ) ;
    }
    //else _Printf ( "\n\n" ) ;
}

