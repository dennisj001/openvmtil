
#include "../includes/cfrtil.h"

byte *
_Mem_Allocate_ ( int32 size )
{
    byte * chunk = mmap_AllocMem(  size ) ;
    if ( ( chunk == MAP_FAILED ) )
    {
        perror ( "mmap" ) ;
        OpenVmTil_Throw ( ( byte* ) "Memory Allocation Error", FULL_RESTART ) ;
    }
    return chunk ;
}

void
_MemChunk_Account ( MemChunk * mchunk, int32 size, uint64 type )
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

#if 0
byte *
Mem_AllocateAndAccountChunk ( int32 osize, uint64 type )
{
    byte * ba = mmap_AllocMem(  osize ) ; ;
    _MemChunk_Account ( (MemChunk*) ba, osize, type ) ;
    return ( byte* ) ba ;
}

MemChunk *
_MemList_Allocate_MemChunkAdded ( DLList * list, int32 osize, uint64 type )
{
    int32 nsize = osize + sizeof ( MemChunk ) ;
    MemChunk * mchunk = mmap_AllocMem(  nsize ) ; ;
    mchunk->S_AType = type ;
    mchunk->S_ChunkSize = nsize ; // S_ChunkSize is the total size of the chunk including any prepended accounting structure in that total
    mchunk->S_Chunk = ( byte* ) ( mchunk + 1 ) ; // nb. ptr arithmetic
    DLList_AddNodeToHead ( list, ( DLNode* ) mchunk ) ;
    return mchunk ;
}
byte *
MemList_AllocateAndAccount_MemChunkAdded ( DLList * list, int32 osize, uint64 type )
{
    MemChunk * mchunk = _MemList_Allocate_MemChunkAdded ( list, osize, type ) ;
    _MemChunk_Account ( mchunk, mchunk->S_ChunkSize, type ) ;
    return ( byte* ) mchunk->S_Chunk ;
}
#endif

byte *
_Mem_Allocate ( DLList * list, int32 size, uint64 type, int32 flags )
{
    int32 asize = size ;
    MemChunk * mchunk ;
    if ( flags & ADD_MEM_CHUNK ) //.AddMemChunk )
    {
        asize = size + sizeof ( MemChunk ) ;
    }
    byte * chunk = _Mem_Allocate_ ( asize ) ; ;
    if ( flags & ADD_TO_LIST ) //.AddToList )
    {
        DLList_AddNodeToHead ( list, ( DLNode* ) chunk ) ;
    }
#if 0    
    else if ( flags.AddMemChunkToList )
    {
        DLList_AddNodeToHead ( list, ( DLNode* ) chunk ) ;
    }
#endif    
    if ( flags & INIT_MEM_CHUNK ) // .InitMemChunk )
    {
        mchunk = (MemChunk *) chunk ;
        mchunk->S_AType = type ;
        mchunk->S_ChunkSize = size ; // S_ChunkSize is the total size of the chunk including any prepended accounting structure in that total
        mchunk->S_Chunk = ( byte* ) ( mchunk + 1 ) ; // nb. ptr arithmetic
    }
    if ( flags & MEM_CHUNK_ACCOUNT ) //.Account )
    {
        _MemChunk_Account ( (MemChunk*) chunk, size, type ) ;
    }
    //if ( flags.ReturnMemChunk && flags.InitMemChunk )
    if ( flags & (RETURN_MEM_CHUNK | INIT_MEM_CHUNK) ) 
    {
        return ( byte* ) mchunk->S_Chunk ;
    }
    else return chunk ;
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
            //else return MemList_AllocateChunk ( _Q_->PermanentMemList, size, OPENVMTIL ) ;
            //else return (byte*) MemList_AllocateAndAccount_MemChunkAdded ( _Q_->PermanentMemList, size, OPENVMTIL )  ;
            else return (byte*) _Mem_Allocate ( _Q_->PermanentMemList, size, OPENVMTIL, (ADD_TO_LIST|ADD_MEM_CHUNK|INIT_MEM_CHUNK|MEM_CHUNK_ACCOUNT|RETURN_MEM_CHUNK) )  ;
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
        for ( node = DLList_First ( _Q_->MemorySpace0->NBAs ) ; node ; node = nextNode )
        {
            nextNode = DLNode_Next ( node ) ;
            nba = ( NamedByteArray* ) node ;
            NBA_Show ( nba ) ;
            _Q_->MemAccountedFor += nba->MemAllocated ;
            _Q_->MemRemaining += nba->MemRemaining ;
            _Q_->NumberOfByteArrays += nba->NumberOfByteArrays ;
        }
    }
    _Q_->MemAccountedFor += ( _Q_->NumberOfByteArrays * sizeof (ByteArray ) ) ;
    int32 plistTotal = _MemList_GetCurrentMemAllocated ( _Q_->PermanentMemList, 0 ) ;
    _Q_->MemAccountedFor += plistTotal ; 
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
    Printf ( ( byte* ) "\nMem Alloc Accounted Info         = %9d : %s", _Q_->MemAccountedFor, "<=: _Q_->MemAccountedFor <=: Used + Available" ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nCurrent Unaccounted Diff (leak?) = %9d : %s", memDiff, "<=: _Q_->Mmap_TotalMemoryAllocated - _Q_->MemAccountedFor" ) ; // + _Q_->OVT_InitialMemAllocated" ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nCalculator :: %d - ( %d + %d ) = %d", _Q_->Mmap_TotalMemoryAllocated, _Q_->MemAccountedFor - _Q_->MemRemaining, _Q_->MemRemaining, memDiff ) ; //memReportedAllocated ) ; ;//+ _Q_->UnaccountedMem ) ) ;
    fflush ( stdout ) ;
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
    munmap ( mchunk, size ) ;
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
        MemChunk * mchunk = ( MemChunk* ) node ;
        nodeNext = DLNode_Next ( node ) ;
        _Mem_ChunkFree ( mchunk ) ;
    }
}

void
NBA_FreeChunkType ( NBA * nba, uint64 type, int32 exactFlag )
{
    if ( exactFlag ) if ( nba->NBA_AType != type ) return ;
        else if ( ! ( nba->NBA_AType & type ) ) return ;
    FreeChunkList ( nba->NBA_MemoryList ) ;
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
    //DLList_Map2_64 ( _Q_->MemorySpace0->NBAs, (MapFunction2_64) NBA_FreeChunkType, type, 1 ) ;
    _MemList_FreeExactType ( _Q_->MemorySpace0->NBAs, type ) ;
}

void
NBAsMemList_FreeVariousTypes ( int type )
{
    //DLList_Map2_64 ( _Q_->MemorySpace0->NBAs, (MapFunction2_64) NBA_FreeChunkType, type, 0 ) ;
    _MemList_FreeVariousTypes ( _Q_->MemorySpace0->NBAs, type ) ;
}

void
OVT_MemList_FreeNBAMemory ( byte * name, uint32 moreThan, int32 always )
{
#if 1 // ?!? how much do we need this at all ?!?
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    if ( always || ( nba->MemAllocated > ( nba->MemInitial + moreThan ) ) )
    {
        NBAsMemList_FreeExactType ( nba->NBA_AType ) ;
        nba->MemAllocated = 0 ;
        nba->MemRemaining = 0 ;
        _NamedByteArray_AddNewByteArray ( nba, nba->NBA_Size ) ;
    }
#endif    
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
    NamedByteArray *nba = ( NamedByteArray* ) _Word_Find_Minimal ( _Q_->MemorySpace0->NBAs, - 1, ( byte * ) name ) ;
    return nba ;
}

NamedByteArray *
MemorySpace_NBA_New ( MemorySpace * memSpace, byte * name, int32 size, int32 atype )
{
    NamedByteArray *nba = NamedByteArray_New ( name, size, atype ) ;
    DLList_AddNodeToHead ( memSpace->NBAs, ( DLNode* ) nba ) ;
    return nba ;
}

void
MemorySpace_Init ( MemorySpace * ms )
{
    OpenVmTil * ovt = _Q_ ;
    _Q_->MemorySpace0 = ms ;
    ms->NBAs = _DLList_New ( OPENVMTIL ) ;

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

    CompilerMemByteArray = ms->CodeSpace->ba_ByteArray ; //init CompilerSpace ptr

    if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\nSystem Memory has been initialized.  " ) ;
}

MemorySpace *
MemorySpace_New ( OpenVmTil * ovt )
{
    //MemorySpace *memSpace = ( MemorySpace* ) MemList_AllocateChunk ( ovt->PermanentMemList, sizeof ( MemorySpace ), OPENVMTIL ) ;
    //MemorySpace *memSpace = ( MemorySpace* ) MemList_AllocateAndAccount_MemChunkAdded ( _Q_->PermanentMemList, sizeof ( MemorySpace ), OPENVMTIL )  ;
    MemorySpace *memSpace = ( MemorySpace* ) _Mem_Allocate ( _Q_->PermanentMemList, sizeof ( MemorySpace ), OPENVMTIL, (ADD_TO_LIST|ADD_MEM_CHUNK|INIT_MEM_CHUNK|MEM_CHUNK_ACCOUNT|RETURN_MEM_CHUNK) )  ;
    ovt->MemorySpace0 = memSpace ;
    MemorySpace_Init ( memSpace ) ; // can't be initialized until after it is hooked into it's System
    return memSpace ;
}

