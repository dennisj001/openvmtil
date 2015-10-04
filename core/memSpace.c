
#include "../includes/cfrtil.h"

void
MemChunk_Show ( MemChunk * mchunk )
{
    Printf ( "\naddress : 0x%08x :      type = %8lu : size = %8d : data = 0x%08x", ( uint ) mchunk, ( long unsigned int ) mchunk->S_AType, ( int ) mchunk->S_ChunkSize, ( unsigned int ) mchunk->S_ChunkData ) ;
    //printf ( "\naddress : 0x%08x :      type = %8lu : size = %8d : data = 0x%08x", ( uint ) mchunk, ( long unsigned int ) mchunk->S_AType, ( int ) mchunk->S_ChunkSize, ( unsigned int ) mchunk->S_ChunkData ) ;
}

void
_MemChunk_Account ( MemChunk * mchunk, int32 size, int32 flag )
{
    if ( _Q_ )
    {
        if ( flag ) _Q_->Mmap_TotalMemoryAllocated += size ;
        else _Q_->Mmap_TotalMemoryAllocated -= size ;
        if ( ( _Q_->Verbosity > 2 ) && ( size >= 10 * M ) )
        {
            Symbol * sym = ( Symbol * ) ( mchunk + 1 ) ;
            _Printf ( "\nFree : %s : 0x%lld : %d, ", ( int ) ( sym->S_Name ) > 0x80000000 ? ( char* ) sym->S_Name : "(null)", mchunk->S_AType, mchunk->S_ChunkSize ) ;
        }
    }
}

//int32 diff ;

byte *
_Mem_Mmap ( int32 size )
{
    byte * mem = mmap_AllocMem ( size ) ;
    if ( ( mem == MAP_FAILED ) )
    {
        perror ( "mmap" ) ;
        CfrTil_MemoryAllocated ( ) ;
        CfrTil_Exit ( ) ;
    }
    return mem ;
}

void
_Mem_ChunkFree ( MemChunk * mchunk )
{
    _MemChunk_Account ( mchunk, mchunk->S_ChunkSize, 0 ) ;
    DLNode_Remove ( ( DLNode* ) mchunk ) ;
    munmap ( mchunk->S_unmap, mchunk->S_ChunkSize ) ;
}

byte *
_Mem_Allocate ( int32 size, uint64 type, int32 flags )
{
    int32 asize = size ;
    if ( flags & ADD_MEM_CHUNK_HEADER ) //.AddMemChunk )
    {
        asize += sizeof ( MemChunk ) ;
    }
    MemChunk * mchunk = ( MemChunk * ) _Mem_Mmap ( asize ) ;
    mchunk->S_unmap = ( byte* ) mchunk ;
    mchunk->S_ChunkSize = asize ; // S_ChunkSize is the total size of the chunk including any prepended accounting structure in that total
    mchunk->S_AType = type ;
    mchunk->S_ChunkData = ( byte* ) ( mchunk + 1 ) ; // nb. ptr arithmetic
    _MemChunk_Account ( ( MemChunk* ) mchunk, asize, 1 ) ;
    DLList_AddNodeToHead ( &_Q_->PermanentMemList, ( DLNode* ) mchunk ) ;
    ///_Calculate_CurrentNbaMemoryAllocationInfo ( 1 ) ;
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
            else return ( byte* ) _Mem_Allocate ( size, OPENVMTIL, ( ADD_MEM_CHUNK_HEADER | RETURN_CHUNK_DATA ) ) ;
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
_OVT_ShowPermanentMemList ( int32 flag )
{
    int32 size ;
    if ( _Q_ )
    {
        int32 diff ;
        DLNode * node, *nodeNext ;
        if ( flag > 1 ) printf ( "\nMemChunk List :: " ) ;
        if ( flag ) Printf ( c_dd ( "\nformat :: Type Name or Chunk Pointer : Type : Size, ...\n" ) ) ;
        for ( size = 0, node = DLList_First ( &_Q_->PermanentMemList ) ; node ; node = nodeNext )
        {
            nodeNext = DLNode_Next ( node ) ;
            if ( flag ) MemChunk_Show ( ( MemChunk * ) node ) ;
            size += ( ( MemChunk * ) node )->S_ChunkSize ;
        }
        diff = _Q_->Mmap_TotalMemoryAllocated - size ;
        if ( flag )
        {
            printf ( "\nTotal Size = %9d : _Q_->Mmap_TotalMemoryAllocated = %9d :: diff = %6d", size, _Q_->Mmap_TotalMemoryAllocated, diff ) ;
            fflush ( stdout ) ;
        }
    }
    return size ;
}

int32
_Calculate_CurrentNbaMemoryAllocationInfo ( int32 flag )
{
    DLNode * node, * nextNode ;
    NamedByteArray * nba ;
    int32 allocSize = 0 ;
    _Q_->MemRemaining = 0 ;
    if ( _Q_ && _Q_->MemorySpace0 )
    {
        for ( allocSize = 0, node = DLList_First ( &_Q_->MemorySpace0->NBAs ) ; node ; node = nextNode )
        {
            nextNode = DLNode_Next ( node ) ;
            nba = Get_NBA_Node_To_NBA ( node ) ;
            if ( flag > 1 ) NBA_Show ( nba, 0 ) ;
            allocSize += nba->TotalAllocSize ;
            _Q_->MemRemaining += nba->MemRemaining ; // Remaining
        }
        int32 diff = _Q_->Mmap_TotalMemoryAllocated - allocSize ;
        if ( flag && diff )
        {
            printf ( "\nTotal Allocated = %9d : _Q_->Mmap_TotalMemoryAllocated = %9d :: diff = %6d\n", allocSize, _Q_->Mmap_TotalMemoryAllocated, diff ) ;
            fflush ( stdout ) ;
        }
    }
    return allocSize ;
}

void
Calculate_CurrentNbaMemoryAllocationInfo ( )
{
    _Calculate_CurrentNbaMemoryAllocationInfo ( 0 ) ;
}

void
CfrTil_MemoryAllocated ( )
{
    _Q_->TotalAccountedMemAllocated = _Calculate_CurrentNbaMemoryAllocationInfo ( 0 ) ;
    _Q_->PermanentMemListAccounted = _OVT_ShowPermanentMemList ( 0 ) ;
    int32 sflag, memDiff1 = _Q_->Mmap_TotalMemoryAllocated - _Q_->TotalAccountedMemAllocated ; //- _Q_->OVT_InitialMemAllocated ;
    int32 memDiff2 = _Q_->Mmap_TotalMemoryAllocated - _Q_->PermanentMemListAccounted ; //- _Q_->OVT_InitialMemAllocated ;
    if ( _Q_ && _Q_->OVT_CfrTil && _DataStack_ ) // so we can use this function anywhere
    {
        int32 dsu = DataStack_Depth ( ) * sizeof (int32 ) ;
        int32 dsa = ( STACK_SIZE * sizeof (int32 ) ) - dsu ;
        Printf ( ( byte* ) "\nData Stack                                  Used = %9d : Available = %9d", dsu, dsa ) ;
    }
    Printf ( ( byte* ) "\nTotal Accounted Mem                         Used = %9d : Available = %9d", _Q_->TotalAccountedMemAllocated - _Q_->MemRemaining, _Q_->MemRemaining ) ;
    Printf ( ( byte* ) "\nMmap_TotalMemoryAllocated                        = %9d : <=: _Q_->Mmap_TotalMemoryAllocated", _Q_->Mmap_TotalMemoryAllocated ) ;
    Printf ( ( byte* ) "\nMem Total Accounted Allocated                    = %9d : <=: _Q_->TotalCategorizedMemAllocated", _Q_->TotalAccountedMemAllocated ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nMem Available                                    = %9d : <=: _Q_->MemRemaining", _Q_->MemRemaining ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nMem Used - Categorized                           = %9d : <=: _Q_->TotalCategorizedMemAllocated - _Q_->MemRemaining", _Q_->TotalAccountedMemAllocated - _Q_->MemRemaining ) ; //+ _Q_->UnaccountedMem ) ) ;
    if ( memDiff1 || memDiff2 || ( _Q_->TotalAccountedMemAllocated != _Q_->PermanentMemListAccounted ) ) sflag = 1 ;
    Printf ( ( byte* ) "\nMem PermanentMemListAccounted                    = %9d : <=: _Q_->PermanentMemListAccounted", _Q_->PermanentMemListAccounted ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nMem Used - Permanent                             = %9d : <=: _Q_->PermanentMemListAccounted - _Q_->MemRemaining", _Q_->PermanentMemListAccounted - _Q_->MemRemaining ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( ( byte* ) "\nCurrent Unaccounted Diff (leak?)                 = %9d : <=: _Q_->Mmap_TotalMemoryAllocated - _Q_->PermanentMemListAccounted", memDiff2 ) ; // + _Q_->OVT_InitialMemAllocated" ) ; //+ _Q_->UnaccountedMem ) ) ;
    if ( sflag )
    {
        Printf ( ( byte* ) "\nCurrent Unaccounted Diff (leak?)                 = %9d : <=: _Q_->Mmap_TotalMemoryAllocated - _Q_->TotalCategorizedMemAllocated", memDiff1 ) ; // + _Q_->OVT_InitialMemAllocated" ) ; //+ _Q_->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nCalculator ::%9d - (%9d +%9d ) = %9d", _Q_->Mmap_TotalMemoryAllocated, _Q_->TotalAccountedMemAllocated - _Q_->MemRemaining, _Q_->MemRemaining, memDiff1 ) ; //memReportedAllocated ) ; ;//+ _Q_->UnaccountedMem ) ) ;
        Printf ( ( byte* ) "\nCalculator ::%9d - (%9d +%9d ) = %9d", _Q_->Mmap_TotalMemoryAllocated, _Q_->PermanentMemListAccounted - _Q_->MemRemaining, _Q_->MemRemaining, memDiff2 ) ; //memReportedAllocated ) ; ;//+ _Q_->UnaccountedMem ) ) ;
    }
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
NBA_FreeChunkType ( Symbol * s, uint64 type, int32 exactFlag )
{
    NamedByteArray * nba = Get_NBA_Symbol_To_NBA ( s ) ; //( NBA* ) s->S_pb_Data ;
    if ( exactFlag )
    {
        if ( nba->NBA_AType != type ) return ;
    }
    else if ( ! ( nba->NBA_AType & type ) ) return ;
    FreeNbaList ( &nba->NBA_BaList ) ;
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
    ms->CfrTilInternalSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CfrTilInternalSpace", 5 * M, CFRTIL ) ;
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
MemorySpace_New ( )
{
    //MemorySpace *memSpace = ( MemorySpace* ) _Mem_Allocate ( sizeof ( MemorySpace ), OPENVMTIL, 0 ) ;
    MemorySpace *memSpace = ( MemorySpace* ) mmap_AllocMem ( sizeof ( MemorySpace ) ) ;
    DLList_Init ( &memSpace->NBAs, &memSpace->NBAsHeadNode, &memSpace->NBAsTailNode ) ; //= _DLList_New ( OPENVMTIL ) ;
    _Q_->MemorySpace0 = memSpace ; // Mem_Allocate needs this here
    MemorySpace_Init ( memSpace ) ; // can't be initialized until after it is hooked into it's System
    return memSpace ;
}

NamedByteArray *
_OVT_Find_NBA ( byte * name )
{
    // needs a Word_Find that can be called before everything is initialized
    Symbol * s = _Word_Find_Symbol ( &_Q_->MemorySpace0->NBAs, - 1, ( byte * ) name ) ;
    return Get_NBA_Symbol_To_NBA ( s ) ; //( NamedByteArray* ) s->S_pb_Data ;
}

// fuzzy still but haven't yet needed to adjust this one

void
OVT_MemList_FreeNBAMemory ( byte * name, uint32 moreThan, int32 always )
{
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    if ( always || ( nba->MemAllocated > ( nba->MemInitial + moreThan ) ) ) // this logic is a little fuzzy ?? what may be wanted is a way to fine tune mem allocation 
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
    OVT_MemList_FreeNBAMemory ( ( byte* ) "CfrTilInternalSpace", 1 * M, 1 ) ;
}

void
OVT_MemListFree_HistorySpace ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "HistorySpace", 1 * M, 0 ) ;
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
OVT_ShowPermanentMemList ( )
{
    _OVT_ShowPermanentMemList ( 1 ) ;
}

void
OVT_ShowNBAs ( )
{
    if ( _Q_ )
    {
        DLNode * node, *nodeNext ;
        if ( _Q_->MemorySpace0 && ( node = DLList_First ( &_Q_->MemorySpace0->NBAs ) ) )
        {
            for ( ; node ; node = nodeNext )
            {
                nodeNext = DLNode_Next ( node ) ;
                NamedByteArray * nba = Get_NBA_Symbol_To_NBA ( node ) ;
                NBA_Show ( nba, 1 ) ;
            }
        }
        printf ( "\n" ) ;
        fflush ( stdout ) ;
    }
}

