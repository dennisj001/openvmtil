
#include "../include/cfrtil.h"

byte*
mmap_AllocMem ( int32 size )
{
    return ( byte* ) mmap ( NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, - 1, 0 ) ;
}

byte *
_Mem_Mmap ( int32 size )
{
    byte * mem = mmap_AllocMem ( size ) ;
    if ( ( mem == MAP_FAILED ) )
    {
        perror ( "_Mem_Mmap" ) ;
        OVT_ShowMemoryAllocated ( ) ;
        OVT_Exit ( ) ;
    }
    return mem ;
}

void
MemChunk_Show ( MemChunk * mchunk )
{
    //Printf ( ( byte* ) "\naddress : 0x%08x : allocType = %8lu : size = %8d : data = 0x%08x", ( uint ) mchunk, ( long unsigned int ) mchunk->S_AProperty, ( int ) mchunk->S_ChunkSize, ( unsigned int ) mchunk->S_ChunkData ) ;
    Printf ( ( byte* ) "\naddress : 0x%08x : allocType = %8lu : size = %8d", ( uint ) mchunk, ( long unsigned int ) mchunk->S_AProperty, ( int ) mchunk->S_ChunkSize ) ;
}

int64 TotalMemAllocated = 0, TotalMemFreed = 0 ;
void
_MemChunk_Account ( MemChunk * mchunk, int32 flag )
{
    if ( _Q_ )
    {
        if ( flag ) 
        {
            TotalMemAllocated += mchunk->S_ChunkSize ;
            _Q_->TotalMemAllocated += mchunk->S_ChunkSize ;
            _Q_->Mmap_TotalMemoryAllocated += mchunk->S_ChunkSize ;
        }
        else 
        {
            TotalMemFreed += mchunk->S_ChunkSize ;
            _Q_->TotalMemFreed += mchunk->S_ChunkSize ;
            _Q_->Mmap_TotalMemoryAllocated -= mchunk->S_ChunkSize ;
        }
#if 0        
        if ( ( _Q_->Verbosity > 2 ) && ( mchunk->S_ChunkSize >= 10 * M ) )
        {
            Symbol * sym = ( Symbol * ) ( mchunk + 1 ) ;
            _Printf ( ( byte* ) "\n%s : %s : 0x%lld : %d, ", flag ? "Alloc" : "Free", ( int ) ( sym->S_Name ) > 0x80000000 ? ( char* ) sym->S_Name : "(null)", mchunk->S_AProperty, mchunk->S_ChunkSize ) ;
        }
#endif        
    }
}

void
_Mem_ChunkFree ( MemChunk * mchunk )
{
    _MemChunk_Account ( mchunk, 0 ) ;
    dlnode_Remove ( ( dlnode* ) mchunk ) ;
    munmap ( mchunk->S_unmap, mchunk->S_ChunkSize ) ;
}

byte *
_Mem_Allocate ( int32 size, uint32 allocType )
{
    int32 asize = size ;
    MemChunk * mchunk = ( MemChunk * ) _Mem_Mmap ( asize ) ;
    mchunk->S_unmap = ( byte* ) mchunk ;
    mchunk->S_ChunkSize = asize ; // S_ChunkSize is the total size of the chunk including any prepended accounting structure in that total
    mchunk->S_AProperty = allocType ;
    //mchunk->S_ChunkData = ( byte* ) ( mchunk + 1 ) ; // nb. ptr arithmetic
    _MemChunk_Account ( ( MemChunk* ) mchunk, 1 ) ;
    dllist_AddNodeToHead ( &_Q_->PermanentMemList, ( dlnode* ) mchunk ) ;
    return ( byte* ) mchunk ;
}

byte *
Mem_Allocate ( int32 size, uint32 allocType )
{
    MemorySpace * ms = _Q_->MemorySpace0 ;
    switch ( allocType )
    {
        case OPENVMTIL: return _Allocate ( size, ms->OpenVmTilSpace ) ;
        case LISP: case OBJECT_MEMORY: return _Allocate ( size, ms->ObjectSpace ) ;
        case TEMPORARY: return _Allocate ( size, ms->TempObjectSpace ) ;
        case DICTIONARY: return _Allocate ( size, ms->DictionarySpace ) ;
        case SESSION: return _Allocate ( size, ms->SessionObjectsSpace ) ;
        case CODE: return _Allocate ( size, ms->CodeSpace ) ;
        case BUFFER: return _Allocate ( size, ms->BufferSpace ) ;
        case HISTORY: return _Allocate ( size, ms->HistorySpace ) ;
        case LISP_TEMP: return _Allocate ( size, ms->LispTempSpace ) ;
        case CONTEXT: return _Allocate ( size, ms->ContextSpace ) ;
        case COMPILER_TEMP_OBJECT_MEMORY: return _Allocate ( size, ms->CompilerTempObjectSpace ) ;
        case CFRTIL: case DATA_STACK: return _Allocate ( size, ms->CfrTilInternalSpace ) ;
        default: CfrTil_Exception ( MEMORY_ALLOCATION_ERROR, QUIT ) ;
    }
}

byte *
_object_Allocate ( int32 size, int32 allocType )
{
    return Mem_Allocate ( size, allocType ) ;
}

void
Mem_FreeItem ( dllist * mList, byte * item )
{
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( (dllist*) mList ) ; node ; node = nodeNext )
    {
        MemChunk * mchunk = ( MemChunk* ) node ;
        nodeNext = dlnode_Next ( node ) ;
        if ( ( byte* ) mchunk->S_pb_Data == item )
        {
            _Mem_ChunkFree ( mchunk ) ;
            return ;
        }
    }
}

void
FreeChunkList ( dllist * list )
{
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( (dllist*) list ) ; node ; node = nodeNext )
    {
        nodeNext = dlnode_Next ( node ) ;
        _Mem_ChunkFree ( ( MemChunk* ) node ) ;
    }
}

void
FreeNba_BaList ( NamedByteArray * nba )
{
    dllist * list = & nba->NBA_BaList ;
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( (dllist*) list ) ; node ; node = nodeNext )
    {
        nodeNext = dlnode_Next ( node ) ;
        dlnode_Remove ( node ) ; // remove BA_Symbol from nba->NBA_BaList cf. _NamedByteArray_AddNewByteArray
        MemChunk* mchunk = ( MemChunk* ) ( ( Symbol * ) node )->S_Value ;
        nba->TotalAllocSize -= mchunk->S_ChunkSize ;
        _Mem_ChunkFree ( mchunk ) ;
    }
}

void
NBA_FreeChunkType ( Symbol * s, uint32 allocType, int32 exactFlag )
{
    NamedByteArray * nba = Get_NBA_Symbol_To_NBA ( s ) ;
    if ( exactFlag )
    {
        if ( nba->NBA_AProperty != allocType ) return ;
    }
    else if ( ! ( nba->NBA_AProperty & allocType ) ) return ;
    FreeNba_BaList ( nba ) ;
    nba->MemRemaining = 0 ;
    nba->MemAllocated = 0 ;
    _NamedByteArray_AddNewByteArray ( nba, nba->NBA_Size ) ;
}

NamedByteArray *
MemorySpace_NBA_New ( MemorySpace * memSpace, byte * name, int32 size, int32 allocType )
{
    NamedByteArray *nba = NamedByteArray_New ( name, size, allocType ) ;
    dllist_AddNodeToHead ( &memSpace->NBAs, ( dlnode* ) & nba->NBA_Symbol ) ;
    return nba ;
}

void
MemorySpace_Init ( MemorySpace * ms )
{
    OpenVmTil * ovt = _Q_ ;

    ms->OpenVmTilSpace = MemorySpace_NBA_New ( ms, ( byte* ) "OpenVmTilSpace", ovt->OpenVmTilSize, OPENVMTIL ) ;
    ms->CfrTilInternalSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CfrTilInternalSpace", ovt->CfrTilSize, CFRTIL ) ;
    ms->ObjectSpace = MemorySpace_NBA_New ( ms, ( byte* ) "ObjectSpace", ovt->ObjectsSize, OBJECT_MEMORY ) ;
    ms->TempObjectSpace = MemorySpace_NBA_New ( ms, ( byte* ) "TempObjectSpace", ovt->TempObjectsSize, TEMPORARY ) ;
    ms->CompilerTempObjectSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CompilerTempObjectSpace", ovt->CompilerTempObjectsSize, COMPILER_TEMP_OBJECT_MEMORY ) ;
    ms->CodeSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CodeSpace", ovt->MachineCodeSize, CODE ) ;
    ms->SessionObjectsSpace = MemorySpace_NBA_New ( ms, ( byte* ) "SessionObjectsSpace", ovt->SessionObjectsSize, SESSION ) ;
    ms->DictionarySpace = MemorySpace_NBA_New ( ms, ( byte* ) "DictionarySpace", ovt->DictionarySize, DICTIONARY ) ;
    ms->LispTempSpace = MemorySpace_NBA_New ( ms, ( byte* ) "LispTempSpace", ovt->LispTempSize, LISP_TEMP ) ;
    ms->BufferSpace = MemorySpace_NBA_New ( ms, ( byte* ) "BufferSpace", ovt->BufferSpaceSize, BUFFER ) ;
    ms->ContextSpace = MemorySpace_NBA_New ( ms, ( byte* ) "ContextSpace", ovt->ContextSize, CONTEXT ) ;
    ms->HistorySpace = MemorySpace_NBA_New ( ms, ( byte* ) "HistorySpace", HISTORY_SIZE, HISTORY ) ;

    ms->BufferList = _dllist_New ( OPENVMTIL ) ; // put it here to minimize allocating chunks for each node and the list

    _Q_CodeByteArray = ms->CodeSpace->ba_CurrentByteArray ; //init CompilerSpace ptr

    if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\nSystem Memory has been initialized.  " ) ;
}

MemorySpace *
MemorySpace_New ( )
{
    MemorySpace *memSpace = ( MemorySpace* ) mmap_AllocMem ( sizeof ( MemorySpace ) ) ;
    _Q_->MemorySpace0 = memSpace ;
    _Q_->OVT_InitialUnAccountedMemory += sizeof ( MemorySpace ) ; // needed here because '_Q_' was not initialized yet for MemChunk accounting
    dllist_Init ( &memSpace->NBAs, &memSpace->NBAsHeadNode, &memSpace->NBAsTailNode ) ; //= _dllist_New ( OPENVMTIL ) ;
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
    if ( always || ( nba->MemAllocated > ( nba->MemInitial + moreThan ) ) ) // this logic is fuzzy ?? what is wanted is a way to fine tune mem allocation 
    {
        NBAsMemList_FreeExactType ( nba->NBA_AProperty ) ;
        nba->MemAllocated = 0 ;
        nba->MemRemaining = 0 ;
        _NamedByteArray_AddNewByteArray ( nba, nba->NBA_Size ) ;
    }
}

void
OVT_MemListFree_ContextMemory ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "ContextSpace", 1 * M, 1 ) ;
}

void
OVT_MemListFree_TempObjects ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "TempObjectSpace", 1 * M, 1 ) ;
}

void
OVT_MemListFree_CompilerTempObjects ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "CompilerTempObjectSpace", 0, 1 ) ;
}

void
OVT_MemListFree_LispTemp ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "LispTempSpace", 2 * M, 1 ) ;
}

void
OVT_MemListFree_Session ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "SessionObjectsSpace", 2 * M, 1 ) ;
}

void
OVT_MemListFree_Buffers ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "BufferSpace", 2 * M, 0 ) ;
}

void
OVT_MemListFree_HistorySpace ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "HistorySpace", 1 * M, 0 ) ;
}

void
_OVT_MemListFree_CfrTilInternal ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "CfrTilInternalSpace", 0, 1 ) ;
}

void
_MemList_FreeExactType ( dllist * list, int allocType )
{
    dllist_Map2 ( list, ( MapFunction2 ) NBA_FreeChunkType, allocType, 1 ) ;
}

void
_MemList_FreeVariousTypes ( dllist * list, int allocType )
{
    dllist_Map2 ( list, ( MapFunction2 ) NBA_FreeChunkType, allocType, 0 ) ;
}

void
NBAsMemList_FreeExactType ( int allocType )
{
    _MemList_FreeExactType ( &_Q_->MemorySpace0->NBAs, allocType ) ;
}

void
NBAsMemList_FreeVariousTypes ( int allocType )
{
    _MemList_FreeVariousTypes ( &_Q_->MemorySpace0->NBAs, allocType ) ;
}

void
NBA_Show ( NamedByteArray * nba, int32 flag )
{
    byte * name = nba->NBA_Symbol.S_Name ;
    if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\n%-27s type = %8lu Used = " INT_FRMT_9 " : Available = " INT_FRMT_9, name, ( long unsigned int ) nba->NBA_AProperty, nba->MemAllocated - nba->MemRemaining, nba->MemRemaining ) ;
    else Printf ( ( byte* ) "\n%-43s Used = " INT_FRMT_9 " : Available = " INT_FRMT_9, name, nba->MemAllocated - nba->MemRemaining, nba->MemRemaining ) ;
    if ( flag )
    {
        dlnode * node, *nodeNext ;
        for ( node = dllist_First ( (dllist*) &nba->NBA_BaList ) ; node ; node = nodeNext )
        {
            nodeNext = dlnode_Next ( node ) ;
            ByteArray * ba = Get_BA_Symbol_To_BA ( node ) ;
            MemChunk_Show ( &ba->BA_MemChunk ) ;
        }
    }
}

void
OVT_ShowNBAs ( )
{
    if ( _Q_ )
    {
        dlnode * node, *nodeNext ;
        if ( _Q_->MemorySpace0 && ( node = dllist_First ( (dllist*) &_Q_->MemorySpace0->NBAs ) ) )
        {
            for ( ; node ; node = nodeNext )
            {
                nodeNext = dlnode_Next ( node ) ;
                NamedByteArray * nba = Get_NBA_Symbol_To_NBA ( node ) ;
                NBA_Show ( nba, 1 ) ;
            }
        }
        printf ( "\n" ) ;
        fflush ( stdout ) ;
    }
}

int32
_OVT_ShowPermanentMemList ( int32 flag )
{
    int32 size ;
    if ( _Q_ )
    {
        int32 diff ;
        dlnode * node, *nodeNext ;
        if ( flag > 1 ) printf ( "\nMemChunk List :: " ) ;
        if ( flag ) Printf ( ( byte* ) c_dd ( "\nformat :: Type Name or Chunk Pointer : Type : Size, ...\n" ) ) ;
        for ( size = 0, node = dllist_First ( (dllist*) &_Q_->PermanentMemList ) ; node ; node = nodeNext )
        {
            nodeNext = dlnode_Next ( node ) ;
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
    _Q_->PermanentMemListAccounted = size ;
    return size ;
}

void
OVT_ShowPermanentMemList ( )
{
    _OVT_ShowPermanentMemList ( 1 ) ;
}

int32
_Calculate_CurrentNbaMemoryAllocationInfo ( int32 flag )
{
    dlnode * node, * nextNode ;
    NamedByteArray * nba ;
    _Q_->MemRemaining = 0 ;
    _Q_->TotalAccountedMemAllocated = 0 ;
    if ( _Q_ && _Q_->MemorySpace0 )
    {
        for ( node = dllist_First ( (dllist*) &_Q_->MemorySpace0->NBAs ) ; node ; node = nextNode )
        {
            nextNode = dlnode_Next ( node ) ;
            nba = Get_NBA_Node_To_NBA ( node ) ;
            if ( flag ) NBA_Show ( nba, 0 ) ;
            _Q_->TotalAccountedMemAllocated += nba->TotalAllocSize ;
            _Q_->MemRemaining += nba->MemRemaining ;
        }
        int32 diff = _Q_->Mmap_TotalMemoryAllocated - _Q_->TotalAccountedMemAllocated ;
        if ( flag && diff )
        {
            printf ( "\n_Q_->Mmap_TotalMemoryAllocated = %9d : _Q_->TotalAccountedMemAllocated = %9d :: diff = %6d\n", _Q_->Mmap_TotalMemoryAllocated, 
                _Q_->TotalAccountedMemAllocated, diff ) ;
            fflush ( stdout ) ;
        }
    }
    return _Q_->TotalAccountedMemAllocated ;
}

void
Calculate_CurrentNbaMemoryAllocationInfo ( )
{
    _Calculate_CurrentNbaMemoryAllocationInfo ( 0 ) ;
}

#if 0


void
OVT_MemListFree_Objects ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "ObjectSpace", 20 * M, 0 ) ;
}


#endif
