
#include "../includes/cfrtil.h"

void
MemChunk_Show ( MemChunk * mchunk )
{
    //Printf ( "\naddress : 0x%08x : type = %6d : size = %7d : data = 0x%08x", mchunk, mchunk->S_AType, mchunk, mchunk->S_ChunkSize, mchunk->S_ChunkData ) ;
    printf ( "\naddress : 0x%08x : type = %6lu : size = %8d : data = 0x%08x", ( uint ) mchunk, ( long unsigned int ) mchunk->S_AType, ( int ) mchunk->S_ChunkSize, ( unsigned int ) mchunk->S_ChunkData ) ;
    //fflush (stdout) ;
}

//int32 tmem, lmem = 120000000 ;
byte *
_Mem_Allocate_ ( int32 size )
{
    byte * chunk = mmap_AllocMem ( size ) ;
    if ( ( chunk == MAP_FAILED ) )
    {
        perror ( "mmap" ) ;
        CfrTil_MemoryAllocated ( ) ;
        CfrTil_Exit ( ) ;
        //OpenVmTil_Throw ( ( byte* ) "Memory Allocation Error", FULL_RESTART ) ;
    }
#if 0     
    tmem+= size ;
    if ( tmem > lmem )
    {
        printf ("\ntmem = %d", tmem) ;
        fflush (stdout) ;
        lmem += 10000000 ;
    }
#endif    
    return chunk ;
}

void
_MemChunk_Account ( MemChunk * mchunk, int32 size )
{
    if ( _Q_ )
    {
        _Q_->Mmap_TotalMemoryAllocated += size ; // added here only for ovt allocation
        if ( ( _Q_->Verbosity > 2 ) && ( size > 10000000 ) )
        {
            _Printf ( "\nAllocate : %s : 0x%lld : %d, ", ( ( NamedByteArray* ) ( mchunk->S_pb_Data ) )->NBA_Symbol.S_Name, mchunk->S_AType, mchunk->S_ChunkSize ) ;
        }
    }
}

byte *
_Mem_Allocate ( DLList * list, int32 size, uint64 type, int32 flags )
{
    int32 asize = size ;
    if ( flags & ADD_MEM_CHUNK_HEADER ) //.AddMemChunk )
    {
        asize += sizeof ( MemChunk ) ;
    }
    MemChunk * mchunk = ( MemChunk * ) _Mem_Allocate_ ( asize ) ;
    mchunk->S_unmap = (byte*) mchunk ;
    if ( flags & ( INIT_HEADER ) ) // .InitMemChunk )
    {
        mchunk->S_AType = type ;
        mchunk->S_ChunkSize = asize ; // S_ChunkSize is the total size of the chunk including any prepended accounting structure in that total
        mchunk->S_ChunkData = ( byte* ) ( mchunk + 1 ) ; // nb. ptr arithmetic
    }
    if ( ( flags & ADD_TO_PML ) ) // & ( ADD_MEM_CHUNK_HEADER | INIT_HEADER ) ) //.AddToList )
    {
        DLList_AddNodeToHead ( &_Q_->PermanentMemList, ( DLNode* ) mchunk ) ;
        //Printf ( "" ) ;
    }
    else if ( flags & ADD_TO_LIST ) //.AddToList )
    {
        DLList_AddNodeToHead ( list, ( DLNode* ) mchunk ) ;
    }
    if ( flags & MEM_CHUNK_ACCOUNT ) //.Account )
    {
        _MemChunk_Account ( ( MemChunk* ) mchunk, asize ) ;
    }
    if ( flags & RETURN_CHUNK_DATA )
    {
        return ( byte* ) mchunk->S_ChunkData ;
    }
    else return ( byte* ) mchunk ; //default : RETURN_RAW_CHUNK 
}

byte *
Mem_Allocate ( int32 size, uint64 type )
{
    MemorySpace * ms = _Q_->MemorySpace0 ;
    switch ( type )
    {
        case OPENVMTIL:
        {
            if ( ms && ms->OpenVmTilSpace ) return _Allocate ( size, ms->OpenVmTilSpace ) ;
            else return ( byte* ) _Mem_Allocate ( 0, size, OPENVMTIL, ( ADD_TO_PML | ADD_MEM_CHUNK_HEADER | INIT_HEADER | MEM_CHUNK_ACCOUNT | RETURN_CHUNK_DATA ) ) ;
        }
        case LISP:
        case OBJECT_MEMORY: return _Allocate ( size, ms->ObjectSpace ) ;
        case TEMP_OBJECT_MEMORY: return _Allocate ( size, ms->TempObjectSpace ) ;
        case DICTIONARY: return _Allocate ( size, ms->DictionarySpace ) ;
        case SESSION: return _Allocate ( size, ms->SessionObjectsSpace ) ;
        case CODE: return _Allocate ( size, ms->CodeSpace ) ;
        case BUFFER: return _Allocate ( size, ms->BufferSpace ) ;
        case HISTORY: return _Allocate ( size, ms->HistorySpace ) ;
        case LISP_TEMP: return _Allocate ( size, ms->LispTempSpace ) ;
        case CONTEXT: return _Allocate ( size, ms->ContextSpace ) ;
        case COMPILER_TEMP_OBJECT_MEMORY: return _Allocate ( size, ms->CompilerTempObjectSpace ) ;
        case CFRTIL:
        case DATA_STACK: return _Allocate ( size, ms->CfrTilInternalSpace ) ;
        default: CfrTil_Exception ( MEMORY_ALLOCATION_ERROR, QUIT ) ;
    }
}

int32
_MemList_GetCurrentMemAllocated ( DLList * list, int32 flag )
{
    DLNode * node, *nodeNext ;
    int32 memAllocated = 0 ;
    if ( flag ) Printf ( c_dd ( "\nformat :: Type Name or Chunk Pointer : Type : Size, ...\n" ) ) ;
    for ( node = DLList_First ( list ) ; node ; node = nodeNext )
    {
        MemChunk * mchunk = ( MemChunk* ) node ;
        nodeNext = DLNode_Next ( node ) ;
        if ( mchunk->S_ChunkSize )
        {
            memAllocated += mchunk->S_ChunkSize ;
            if ( flag ) _Printf ( "\n0x%08x : 0x%08llx : %d, ", ( uint ) mchunk, mchunk->S_AType, mchunk->S_ChunkSize ) ;
        }
    }
    return memAllocated ;
}

void
Calculate_CurrentMemoryAllocationInfo ( )
{
    DLNode * node, * nextNode ;
    NamedByteArray * nba ;
    _Q_->MemAccountedFor = 0 ;
    _Q_->MemRemaining = 0 ;
    _Q_->NumberOfByteArrays = 0 ;
    if ( _Q_ && _Q_->MemorySpace0 )
    {
        for ( node = DLList_First ( &_Q_->MemorySpace0->NBAs ) ; node ; node = nextNode )
        {
            nextNode = DLNode_Next ( node ) ;
            nba = ( NamedByteArray* ) ( ( ( Symbol* ) node )->S_pb_Data ) ;
            NBA_Show ( nba, 0 ) ;
            _Q_->MemAccountedFor += nba->MemAllocated ;
            _Q_->MemRemaining += nba->MemRemaining ;
            _Q_->NumberOfByteArrays += nba->NumberOfByteArrays ;
        }
    }
    _Q_->MemAccountedFor += ( _Q_->NumberOfByteArrays * sizeof (ByteArray ) ) ;
    int32 plistTotal = _MemList_GetCurrentMemAllocated ( &_Q_->PermanentMemList, 0 ) ;
    //_Q_->MemAccountedFor += plistTotal ;
    _Q_->MemAccountedFor += _Q_->OVT_InitialUnAccountedMemory ;
}

void
CfrTil_MemoryAllocated ( )
{
    Calculate_CurrentMemoryAllocationInfo ( ) ;
    int32 memDiff = _Q_->Mmap_TotalMemoryAllocated - _Q_->MemAccountedFor ; //- _Q_->OVT_InitialMemAllocated ;
    int32 dsu = DataStack_Depth ( ) * sizeof (int32 ) ;
    int32 dsa = ( STACK_SIZE * sizeof (int32 ) ) - dsu ;
    Printf ( ( byte* ) "\n%-28s" "Used = %9d : Available = %9d", "Data Stack", dsu, dsa ) ;
    Printf ( ( byte* ) "\n%-28s" "Used = %9d : Available = %9d", "Total Categorized Mem", _Q_->MemAccountedFor - _Q_->MemRemaining, _Q_->MemRemaining ) ;
    Printf ( ( byte* ) "\nMem Alloc Continuous Total       =  %d : %s", _Q_->Mmap_TotalMemoryAllocated, "<=: _Q_->Mmap_TotalMemoryAllocated" ) ;
    Printf ( ( byte* ) "\nMem Currently Accounted For      = %9d : %s", _Q_->MemAccountedFor, "<=: _Q_->MemAccountedFor <=: Used + Available" ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nCurrent Unaccounted Diff (leak?) = %9d : %s", memDiff, "<=: _Q_->Mmap_TotalMemoryAllocated - _Q_->MemAccountedFor" ) ; // + _Q_->OVT_InitialMemAllocated" ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nCalculator :: %d - ( %d + %d ) = %d", _Q_->Mmap_TotalMemoryAllocated, _Q_->MemAccountedFor - _Q_->MemRemaining, _Q_->MemRemaining, memDiff ) ; //memReportedAllocated ) ; ;//+ _Q_->UnaccountedMem ) ) ;
    //Mem_ShowAllocated ( );
}

void
_Mem_ChunkFree ( MemChunk * mchunk )
{
    int32 size = mchunk->S_ChunkSize ;
    if ( _Q_ )
    {
        _Q_->Mmap_TotalMemoryAllocated -= size ;
        if ( ( _Q_->Verbosity > 2 ) && ( size > 10000000 ) )
        {
            Symbol * sym = ( Symbol * ) ( mchunk + 1 ) ;
            _Printf ( "\nFree : %s : 0x%lld : %d, ", ( int ) ( sym->S_Name ) > 0x80000000 ? ( char* ) sym->S_Name : "(null)", mchunk->S_AType, mchunk->S_ChunkSize ) ;
        }
    }
    DLNode_Remove ( ( DLNode* ) mchunk ) ;
    //MemChunk_Show ( mchunk ) ;
    munmap ( mchunk->S_unmap, size ) ;
    //tmem -= size ;
}

void
Mem_FreeItem ( DLList * mList, byte * item )
{
    DLNode * node, *nodeNext ;
    for ( node = DLList_First ( mList ) ; node ; node = nodeNext )
    {
        MemChunk * mchunk = ( MemChunk* ) node ;
        nodeNext = DLNode_Next ( node ) ;
        if ( ( byte* ) mchunk->S_pb_Data == item )
        {
            _Mem_ChunkFree ( mchunk ) ;
            return ;
        }
    }
}

void
FreeChunkList ( DLList * list )
{
    DLNode * node, *nodeNext ;
    for ( node = DLList_First ( list ) ; node ; node = nodeNext )
    {
        nodeNext = DLNode_Next ( node ) ;
        _Mem_ChunkFree ( ( MemChunk* ) node ) ;
    }
}

void
FreeNbaList ( DLList * list )
{
    DLNode * node, *nodeNext ;
    for ( node = DLList_First ( list ) ; node ; node = nodeNext )
    {
        nodeNext = DLNode_Next ( node ) ;
        _Mem_ChunkFree ( ( MemChunk* ) node ) ;
    }
}

void
Mem_ShowAllocated ( )
{
    if ( _Q_ )
    {
        DLNode * node, *nodeNext ;
        Printf ( "\nMemChunk List :: " ) ;
        for ( node = DLList_First ( &_Q_->PermanentMemList ) ; node ; node = nodeNext )
        {
            nodeNext = DLNode_Next ( node ) ;
            MemChunk_Show ( ( MemChunk * ) node ) ;
        }
        if ( _Q_->MemorySpace0 && ( node = DLList_First ( &_Q_->MemorySpace0->NBAs ) ) )
        {
            for ( ; node ; node = nodeNext )
            {
                nodeNext = DLNode_Next ( node ) ;
                NamedByteArray * nba = ( NamedByteArray* ) ( ( ( Symbol* ) node )->S_pb_Data ) ;
                NBA_Show ( nba, 1 ) ;
            }
        }
    }
}

void
NBA_FreeChunkType ( Symbol * s, uint64 type, int32 exactFlag )
{
    NamedByteArray * nba = ( NBA* ) s->S_pb_Data ;
    if ( exactFlag )
    {
        if ( nba->NBA_AType != type ) return ;
    }
    else if ( ! ( nba->NBA_AType & type ) ) return ;
    FreeNbaList ( &nba->NBA_BaList ) ;
}

void
_MemList_FreeExactType ( DLList * list, int type )
{
    DLList_Map2_64 ( list, ( MapFunction2_64 ) NBA_FreeChunkType, type, 1 ) ;
}

void
_MemList_FreeVariousTypes ( DLList * list, int type )
{
    DLList_Map2_64 ( list, ( MapFunction2_64 ) NBA_FreeChunkType, type, 0 ) ;
}

void
NBAsMemList_FreeExactType ( int type )
{
    _MemList_FreeExactType ( &_Q_->MemorySpace0->NBAs, type ) ;
}

void
NBAsMemList_FreeVariousTypes ( int type )
{
    _MemList_FreeVariousTypes ( &_Q_->MemorySpace0->NBAs, type ) ;
}

void
OVT_MemList_FreeNBAMemory ( byte * name, uint32 moreThan, int32 always )
{
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    if ( always || ( nba->MemAllocated > ( nba->MemInitial + moreThan ) ) )
    {
        NBAsMemList_FreeExactType ( nba->NBA_AType ) ;
        nba->MemAllocated = 0 ;
        nba->MemRemaining = 0 ;
        _NamedByteArray_AddNewByteArray ( nba, nba->NBA_Size ) ;
    }
}

void
OVT_MemListFree_Objects ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "ObjectSpace", 20 * M, 0 ) ;
}

void
OVT_MemListFree_TempObjects ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "TempObjectSpace", 1 * M, 0 ) ;
}

void
OVT_MemListFree_ContextMemory ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "ContextSpace", 10 * M, 0 ) ;
}

void
OVT_MemListFree_CompilerTempObjects ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "CompilerTempObjectSpace", 0, 0 ) ;
}

void
OVT_MemListFree_LispTemp ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "LispTempSpace", 2 * M, 0 ) ;
}

void
OVT_MemListFree_Session ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "SessionObjectsSpace", 1 * M, 0 ) ;
}

void
OVT_MemListFree_CfrTilInternal ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "CfrTilInternalSpace", 1 * M, 0 ) ;
}

void
OVT_MemListFree_HistorySpace ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "HistorySpace", 1 * M, 0 ) ;
}

NamedByteArray *
_OVT_Find_NBA ( byte * name )
{
    // needs a Word_Find that can be called before everything is initialized
    Symbol * s = _Word_Find_Symbol ( &_Q_->MemorySpace0->NBAs, - 1, ( byte * ) name ) ;
    return ( NamedByteArray* ) s->S_pb_Data ;
}

NamedByteArray *
MemorySpace_NBA_New ( MemorySpace * memSpace, byte * name, int32 size, int32 atype )
{
    NamedByteArray *nba = NamedByteArray_New ( name, size, atype ) ;
    DLList_AddNodeToHead ( &memSpace->NBAs, ( DLNode* ) & nba->NBA_Symbol ) ;
    return nba ;
}

void
MemorySpace_Init ( MemorySpace * ms )
{
    OpenVmTil * ovt = _Q_ ;

    ms->OpenVmTilSpace = MemorySpace_NBA_New ( ms, ( byte* ) "OpenVmTilSpace", 5 * M, OPENVMTIL ) ;
    ms->CfrTilInternalSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CfrTilInternalSpace", 5 * K, CFRTIL ) ;
    ms->ObjectSpace = MemorySpace_NBA_New ( ms, ( byte* ) "ObjectSpace", ovt->ObjectsSize, OBJECT_MEMORY ) ;
    ms->TempObjectSpace = MemorySpace_NBA_New ( ms, ( byte* ) "TempObjectSpace", ovt->TempObjectsSize, TEMPORARY ) ;
    ms->CompilerTempObjectSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CompilerTempObjectSpace", ovt->CompilerTempObjectsSize, COMPILER_TEMP_OBJECT_MEMORY ) ;
    ms->CodeSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CodeSpace", ovt->MachineCodeSize, CODE ) ;
    ms->SessionObjectsSpace = MemorySpace_NBA_New ( ms, ( byte* ) "SessionObjectsSpace", ovt->SessionObjectsSize, SESSION ) ;
    ms->DictionarySpace = MemorySpace_NBA_New ( ms, ( byte* ) "DictionarySpace", ovt->DictionarySize, DICTIONARY ) ;
    ms->LispTempSpace = MemorySpace_NBA_New ( ms, ( byte* ) "LispTempSpace", ovt->LispTempSize, LISP_TEMP ) ;
    ms->BufferSpace = MemorySpace_NBA_New ( ms, ( byte* ) "BufferSpace", 2 * M, BUFFER ) ;
    ms->ContextSpace = MemorySpace_NBA_New ( ms, ( byte* ) "ContextSpace", ovt->ContextSize, CONTEXT ) ;
    ms->HistorySpace = MemorySpace_NBA_New ( ms, ( byte* ) "HistorySpace", HISTORY_SIZE, HISTORY ) ;

    ms->BufferList = _DLList_New ( OPENVMTIL ) ; // put it here to minimize allocating chunks for each node and the list

    CompilerMemByteArray = ms->CodeSpace->ba_CurrentByteArray ; //init CompilerSpace ptr

    if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\nSystem Memory has been initialized.  " ) ;
}

MemorySpace *
MemorySpace_New ( OpenVmTil * ovt )
{
    MemorySpace *memSpace = ( MemorySpace* ) _Mem_Allocate ( 0, sizeof ( MemorySpace ), OPENVMTIL, ( ADD_TO_PML | ADD_MEM_CHUNK_HEADER | INIT_HEADER | MEM_CHUNK_ACCOUNT | RETURN_CHUNK_DATA ) ) ;
    ovt->MemorySpace0 = memSpace ;
    DLList_Init ( &memSpace->NBAs, &memSpace->NBAsHeadNode, &memSpace->NBAsTailNode ) ; //= _DLList_New ( OPENVMTIL ) ;
    MemorySpace_Init ( memSpace ) ; // can't be initialized until after it is hooked into it's System
    return memSpace ;
}

