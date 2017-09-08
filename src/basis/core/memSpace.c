
#include "../../include/cfrtil.h"

const int64 MEM_FREE = 0 ;
const int64 MEM_ALLOC = 1 ;
uint64 mmap_TotalMemAllocated = 0, mmap_TotalMemFreed = 0 ;

byte*
_mmap_AllocMem ( int64 size )
{
    mmap_TotalMemAllocated += size ;
    return ( byte* ) mmap ( NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, - 1, 0 ) ;
}

void
mmap_FreeMem ( byte * chunk, int64 size )
{
    mmap_TotalMemFreed += size ;
    munmap ( chunk, size ) ;
}

byte *
mmap_AllocMem ( int64 size )
{
    byte * mem = _mmap_AllocMem ( size ) ;
    if ( ( mem == MAP_FAILED ) )
    {
        perror ( "_Mem_Mmap" ) ;
        OVT_ShowMemoryAllocated ( ) ;
        //OVT_Exit ( ) ;
        CfrTil_FullRestart ( ) ;
    }
    _Kbhit ( ) ;
    memset ( mem, 0, size ) ; // ?? : is this necessary??
    return mem ;
}

void
MemChunk_Show ( MemChunk * mchunk )
{
    //_Printf ( ( byte* ) "\naddress : 0x%08x : allocType = %8lu : size = %8d : data = 0x%08x", ( uint64 ) mchunk, ( uint64 ) mchunk->S_AProperty, ( int64 ) mchunk->S_ChunkSize, ( uint64 ) mchunk->S_ChunkData ) ;
    _Printf ( ( byte* ) "\naddress : 0x%08x : allocType = %8lu : size = %8d", ( uint64 ) mchunk, ( uint64 ) mchunk->S_AProperty, ( int64 ) mchunk->S_ChunkSize ) ;
}

void
_MemChunk_WithSymbol_Show ( MemChunk * mchunk, int64 flag )
{
    Symbol * sym = ( Symbol * ) ( mchunk + 1 ) ;
    _Printf ( ( byte* ) "\n%s : %s : 0x%lld : %d, ", ( flag == MEM_ALLOC ) ? "Alloc" : "Free",
        ( ( int64 ) ( sym->S_Name ) > 0x80000000 ) ? ( char* ) sym->S_Name : "(null)", mchunk->S_AProperty, mchunk->S_ChunkSize ) ;
}

void
_MemChunk_Account ( MemChunk * mchunk, int64 flag )
{
    if ( _Q_ )
    {
        if ( flag )
        {
            _Q_->TotalMemAllocated += mchunk->S_ChunkSize ;
            _Q_->Mmap_RemainingMemoryAllocated += mchunk->S_ChunkSize ;
        }
        else
        {
            _Q_->TotalMemFreed += mchunk->S_ChunkSize ;
            _Q_->Mmap_RemainingMemoryAllocated -= mchunk->S_ChunkSize ;
        }
#if 0        
        if ( ( _Q_->Verbosity > 2 ) ) && ( mchunk->S_ChunkSize >= 10 * M ) _MemChunk_WithSymbol_Show ( mchunk, flag ) ;
#endif        
    }
}

void
_Mem_ChunkFree ( MemChunk * mchunk )
{
    _MemChunk_Account ( mchunk, MEM_FREE ) ;
    dlnode_Remove ( ( dlnode* ) mchunk ) ;
    mmap_FreeMem ( mchunk->S_unmap, mchunk->S_ChunkSize ) ;
}

byte *
_Mem_ChunkAllocate ( int64 size, uint64 allocType )
{
    int64 asize = size ;
    MemChunk * mchunk = ( MemChunk * ) mmap_AllocMem ( asize ) ;
    mchunk->S_unmap = ( byte* ) mchunk ;
    mchunk->S_ChunkSize = asize ; // S_ChunkSize is the total size of the chunk including any prepended accounting structure in that total
    mchunk->S_AProperty = allocType ;
    //mchunk->S_ChunkData = ( byte* ) ( mchunk + 1 ) ; // nb. ptr arithmetic
    _MemChunk_Account ( ( MemChunk* ) mchunk, MEM_ALLOC ) ;
    dllist_AddNodeToHead ( &_Q_->PermanentMemList, ( dlnode* ) mchunk ) ;
    return ( byte* ) mchunk ;
}

byte *
Mem_Allocate ( int64 size, uint64 allocType )
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
        case COMPILER_TEMP: return _Allocate ( size, ms->CompilerTempObjectSpace ) ;
        case CFRTIL: case DATA_STACK: return _Allocate ( size, ms->CfrTilInternalSpace ) ;
        case STRING_MEMORY: return _Allocate ( size, ms->StringSpace ) ;
            //case SIZED_ALLOCATE: return Sized_Allocate ( size ) ;
        case RUNTIME: return mmap_AllocMem ( size ) ;
        default: CfrTil_Exception ( MEMORY_ALLOCATION_ERROR, QUIT ) ;
    }
}

void
Mem_FreeItem ( dllist * mList, byte * item )
{
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( ( dllist* ) mList ) ; node ; node = nodeNext )
    {
        MemChunk * mchunk = ( MemChunk* ) node ;
        nodeNext = dlnode_Next ( node ) ;
        if ( ( byte* ) mchunk->S_pb_Data2 == item )
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
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nodeNext )
    {
        nodeNext = dlnode_Next ( node ) ;
        _Mem_ChunkFree ( ( MemChunk* ) node ) ;
    }
}

void
FreeNba_BaNode ( NamedByteArray * nba, dlnode * node )
{
    ByteArray *ba = ( ByteArray * ) node ;
    dlnode_Remove ( node ) ; // remove BA_Symbol from nba->NBA_BaList cf. _NamedByteArray_AddNewByteArray
    MemChunk* mchunk = ( MemChunk* ) ( ( Symbol * ) node )->S_Value ;
    nba->TotalAllocSize -= mchunk->S_ChunkSize ;
    _Mem_ChunkFree ( mchunk ) ;
}

void
FreeNba_BaList ( NamedByteArray * nba )
{
    dllist * list = & nba->NBA_BaList ;
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nodeNext )
    {
        nodeNext = dlnode_Next ( node ) ;
        FreeNba_BaNode ( nba, node ) ;
    }
}

void
NBA_FreeChunkType ( Symbol * s, uint64 allocType, int64 exactFlag )
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
    _NamedByteArray_AddNewByteArray ( nba, nba->NBA_DataSize ) ;
}

NamedByteArray *
MemorySpace_NBA_New ( MemorySpace * memSpace, byte * name, int64 size, int64 allocType )
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
    ms->CompilerTempObjectSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CompilerTempObjectSpace", ovt->CompilerTempObjectsSize, COMPILER_TEMP ) ;
    ms->CodeSpace = MemorySpace_NBA_New ( ms, ( byte* ) "CodeSpace", ovt->MachineCodeSize, CODE ) ;
    ms->SessionObjectsSpace = MemorySpace_NBA_New ( ms, ( byte* ) "SessionObjectsSpace", ovt->SessionObjectsSize, SESSION ) ;
    ms->SessionCodeSpace = MemorySpace_NBA_New ( ms, ( byte* ) "SessionCodeSpace", ovt->SessionCodeSize, SESSION_CODE ) ;
    ms->DictionarySpace = MemorySpace_NBA_New ( ms, ( byte* ) "DictionarySpace", ovt->DictionarySize, DICTIONARY ) ;
    ms->LispTempSpace = MemorySpace_NBA_New ( ms, ( byte* ) "LispTempSpace", ovt->LispTempSize, LISP_TEMP ) ;
    ms->BufferSpace = MemorySpace_NBA_New ( ms, ( byte* ) "BufferSpace", ovt->BufferSpaceSize, BUFFER ) ;
    //ms->ContextSpace = MemorySpace_NBA_New ( ms, ( byte* ) "ContextSpace", ovt->ContextSize, CONTEXT ) ;
    ms->HistorySpace = MemorySpace_NBA_New ( ms, ( byte* ) "HistorySpace", HISTORY_SIZE, HISTORY ) ;
    ms->StringSpace = MemorySpace_NBA_New ( ms, ( byte* ) "StringSpace", ovt->StringSpaceSize, STRING_MEMORY ) ;

    ms->BufferList = _dllist_New ( OPENVMTIL ) ; // put it here to minimize allocating chunks for each node and the list
    ms->RecycledWordList = _dllist_New ( OPENVMTIL ) ; // put it here to minimize allocating chunks for each node and the list

    _Q_CodeByteArray = ms->CodeSpace->ba_CurrentByteArray ; //init CompilerSpace ptr

    if ( _Q_->Verbosity > 2 ) _Printf ( ( byte* ) "\nSystem Memory has been initialized.  " ) ;
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
    Symbol * s = DLList_FindName_InOneNamespaceList ( &_Q_->MemorySpace0->NBAs, ( byte * ) name ) ;
    if ( s ) return Get_NBA_Symbol_To_NBA ( s ) ; //( NamedByteArray* ) s->S_pb_Data ;
    else return 0 ;
}

// fuzzy still but haven't yet needed to adjust this one

void
OVT_MemList_FreeNBAMemory ( byte * name, uint64 moreThan, int64 always )
{
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    if ( nba && ( always || ( nba->MemAllocated > ( nba->MemInitial + moreThan ) ) ) ) // this logic is fuzzy ?? what is wanted is a way to fine tune mem allocation 
    {
        dlnode * node, *nodeNext ;
        int64 flag ;
        for ( flag = 0, node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
        {
            nodeNext = dlnode_Next ( node ) ;
            ByteArray * ba = Get_BA_Symbol_To_BA ( node ) ;
            if ( ba )
            {
                if ( ! flag ++ )
                {
                    _ByteArray_Init ( ba ) ;
                    nba->ba_CurrentByteArray = ba ;
                    int64 size = ba->BA_DataSize ;
                    nba->MemAllocated = size ;
                    nba->MemRemaining = size ;
                }
                else
                {
                    FreeNba_BaNode ( nba, node ) ;
                    nba->NumberOfByteArrays -- ;
                }
            }
        }
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

#if 1

void
OVT_MemListFree_Session ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "SessionObjectsSpace", 2 * M, 1 ) ;
}
#endif

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
_MemList_FreeExactType ( dllist * list, int64 allocType )
{
    dllist_Map2 ( list, ( MapFunction2 ) NBA_FreeChunkType, allocType, 1 ) ;
}

void
_MemList_FreeVariousTypes ( dllist * list, int64 allocType )
{
    dllist_Map2 ( list, ( MapFunction2 ) NBA_FreeChunkType, allocType, 0 ) ;
}

void
NBAsMemList_FreeExactType ( int64 allocType )
{
    _MemList_FreeExactType ( &_Q_->MemorySpace0->NBAs, allocType ) ;
}

void
NBAsMemList_FreeVariousTypes ( int64 allocType )
{
    _MemList_FreeVariousTypes ( &_Q_->MemorySpace0->NBAs, allocType ) ;
}

void
NBA_Show ( NamedByteArray * nba, int64 flag )
{
    byte * name = nba->NBA_Symbol.S_Name ;
    if ( _Q_->Verbosity > 2 ) _Printf ( ( byte* ) "\n%-27s type = %8lu Used = " INT_FRMT_9 " : Unused = " INT_FRMT_9, name, ( uint64 ) nba->NBA_AProperty, nba->MemAllocated - nba->MemRemaining, nba->MemRemaining ) ;
    else _Printf ( ( byte* ) "\n%-43s Used = " INT_FRMT_9 " : Unused = " INT_FRMT_9, name, nba->MemAllocated - nba->MemRemaining, nba->MemRemaining ) ;
    if ( flag )
    {
        dlnode * node, *nodeNext ;
        for ( node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
        {
            nodeNext = dlnode_Next ( node ) ;
            ByteArray * ba = Get_BA_Symbol_To_BA ( node ) ;
            MemChunk_Show ( &ba->BA_MemChunk ) ;
        }
    }
}

byte *
OVT_CheckRecyclableAllocate ( dllist * list, int64 size )
{
    dlnode * node = 0 ;
    if ( _Q_ && _Q_->MemorySpace0 ) node = dllist_First ( ( dllist* ) list ) ;
    if ( node )
    {
        dlnode_Remove ( node ) ; // necessary else we destroy the list!
        Mem_Clear ( ( byte* ) node, size ) ;
        _Q_->MemorySpace0->RecycledWordCount ++ ;
    }
    return ( byte* ) node ;
}

void
Word_Recycle ( Word * w )
{
    if ( w ) dllist_AddNodeToHead ( _Q_->MemorySpace0->RecycledWordList, ( dlnode* ) w ) ;
}

void
_CheckRecycleWord ( Word * w )
{
    if ( w && ( w->S_CProperty & RECYCLABLE_COPY ) )
    {
        if ( ! ( IsSourceCodeOn && w->State & W_SOURCE_CODE_MODE ) )
        {
            d0 ( _Printf ( ( byte* ) "\nrecycling : %s", w->Name ) ) ;
            d0 ( if ( String_Equal ( w->Name, "power" ) ) _Printf ( "\nRecycle : Got it! : %s\n", w->Name ) ) ;
            Word_Recycle ( w ) ;
        }
    }
}

void
CheckRecycleWord ( Node * node )
{
    Word *w = ( Word* ) ( dlnode_Next ( ( dlnode* ) node ) ? dobject_Get_M_Slot ( node, 0 ) : 0 ) ;
    _CheckRecycleWord ( w ) ;
}

void
DLList_RecycleWordList ( dllist * list )
{
    //if ( list == (_Compiler_? _Compiler_->WordList : (dllist*) 0) )
    dllist_Map ( list, ( MapFunction0 ) CheckRecycleWord ) ;
    //else dllist_Map ( list, ( MapFunction0 ) _CheckRecycleWord ) ;
}

#if 0

void
OVT_ShowPermanentMemList ( )
{
    _OVT_ShowPermanentMemList ( 1 ) ;
}

void
Calculate_CurrentNbaMemoryAllocationInfo ( )
{
    _Calculate_TotalNbaAccountedMemAllocated ( 0 ) ;
}

void
OVT_MemListFree_Objects ( )
{
    OVT_MemList_FreeNBAMemory ( ( byte* ) "ObjectSpace", 20 * M, 0 ) ;
}


#if 0

void
Interpreter_DebugNow ( Interpreter * interp )
{
    if ( Is_DebugModeOn )
    {
        _Printf ( "\nInterpreter_DebugNow : %s", interp->w_Word->Name ) ;
        Word * word = Finder_Word_FindUsing ( interp->Finder0, "dbOn", 0 ) ;
        if ( word == 0 )
            _Printf ( "\nProblem here!\n" ) ;
    }
}
#endif

#endif
