
#include "../../include/cfrtil.h"

ByteArray *
_ByteArray_AppendSpace_MakeSure ( ByteArray * ba, int64 size ) // size in bytes
{
    NamedByteArray * nba = ba->OurNBA ;
    if ( nba )
    {
        while ( ba->MemRemaining < size )
        {
            int64 largestRemaining = 0 ;
            // check the other bas in the nba list to see if any have enough remaining
            {
                dlnode * node, *nodeNext ;
                for ( node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
                {
                    nodeNext = dlnode_Next ( node ) ;
                    ba = Get_BA_Symbol_To_BA ( node ) ;
                    if ( ba->MemRemaining > largestRemaining ) largestRemaining = ba->MemRemaining ;
                    if ( ba->MemRemaining >= size ) goto done ;
                }
            }
            _Q_->AllocationRequestLacks ++ ;
            nba->NBA_DataSize += ( ( ++ nba->CheckTimes ) * ( 2 * K ) ) + size ;
            if ( ba == _Q_CodeByteArray ) nba->NBA_DataSize += 5 * K ; // make sure we have enough code space
            //nba->NBA_DataSize += size ;
            if ( _Q_->Verbosity > 3 )
            {
                printf ( "\n%s size requested = %ld :: adding size = %ld :: largest remaining = %ld :: Nba total remaining = %ld :: checkTimes = %ld\n",
                    nba->NBA_Symbol.Name, size, nba->NBA_DataSize, largestRemaining, nba->MemRemaining, nba->CheckTimes ) ;
            }
            ba = _NamedByteArray_AddNewByteArray ( nba, nba->NBA_DataSize ) ; //( nba->NBA_DataSize > size ) ? nba->NBA_DataSize : ( nba->NBA_DataSize + size ) ) ; //size ) ;
        }
    }
    else Error_Abort ( ( byte* ) "\n_ByteArray_AppendSpace_MakeSure : no nba?!\n" ) ;
done:
    return ba ;
}

byte *
_ByteArray_AppendSpace ( ByteArray * ba, int64 size ) // size in bytes
{
    while ( ba->MemRemaining < size )
    {
        ba = _ByteArray_AppendSpace_MakeSure ( ba, size ) ;
    }
    ba->StartIndex = ba->EndIndex ; // move index to end of the last append
    ba->EndIndex += size ;
    if ( ba->OurNBA ) ba->OurNBA->MemRemaining -= size ; //nb. debugger->StepInstructionBA doesn't have an nba
    ba->MemRemaining -= size ;
    return ba->StartIndex ;
}

void
_ByteArray_UnAppendSpace ( ByteArray * ba, int64 size ) // size in bytes
{
    // ?? no error checking ??
    ba->EndIndex -= size ;
    ba->StartIndex -= size ;
}

void
_ByteArray_DataClear ( ByteArray * ba )
{
    Mem_Clear ( ba->BA_Data, ba->BA_DataSize ) ;
}

 void
_ByteArray_Init ( ByteArray * ba )
{
    ba->BA_Data = ( byte* ) ( ba + 1 ) ;
    ba->StartIndex = ba->BA_Data ;
    ba->EndIndex = ba->StartIndex ;
    ba->bp_Last = & ba->BA_Data [ ba->BA_DataSize - 1 ] ;
    ba->MemRemaining = ba->BA_DataSize ;
    _ByteArray_DataClear ( ba ) ;
}

int64
ByteArray_IsAddressWwitinTheArray ( ByteArray * ba, byte * address )
{
    if ( ( address >= ( byte* ) ba->BA_Data ) && ( address <= ( byte* ) ba->bp_Last ) ) return true ; // ?!? not quite accurate
    return false ;
}

void
_ByteArray_ReInit ( ByteArray * ba )
{
    _ByteArray_Init ( ba ) ;
}

ByteArray *
ByteArray_Init ( ByteArray * ba, int64 size, uint64 type )
{
    // we want to keep track of how much data for each type separate from MemChunk accounting
    ba->BA_DataSize = size ;
    ba->BA_AllocSize = size + sizeof (ByteArray ) ;
    ba->BA_AProperty = type ;
    Set_BA_Symbol_To_BA ( ba ) ; // nb! : ByteArray has two nodes, a MemChunk and a Symbol, each on different lists 
    _ByteArray_Init ( ba ) ;
    return ba ;
}

ByteArray *
ByteArray_AllocateNew ( int64 size, uint64 type )
{
    ByteArray * ba = ( ByteArray* ) _Mem_ChunkAllocate ( size + sizeof ( ByteArray ), type ) ;
    ByteArray_Init ( ba, size, type ) ;
    return ba ;
}

byte *
_ByteArray_GetEndIndex ( ByteArray * ba )
{
    return ba->EndIndex ;
}

byte *
_ByteArray_Here ( ByteArray * ba )
{
    return ba->EndIndex ;
}

void
_ByteArray_SetEndIndex ( ByteArray * ba, byte * index )
{
    ba->EndIndex = index ;
}

void
_ByteArray_SetHere ( ByteArray * ba, byte * index )
{
    ba->EndIndex = index ;
}

void
_ByteArray_SetHere_AndForDebug ( ByteArray * ba, byte * index )
{
    if ( index )
    {
        _ByteArray_SetEndIndex ( ba, index ) ;
        if ( _Debugger_ ) _Debugger_->PreHere = index ;
    }
}

byte *
_ByteArray_GetStartIndex ( ByteArray * ba )
{
    return ba->StartIndex ;
}

void
_ByteArray_SetStartIndex ( ByteArray * ba, byte * index )
{
    ba->StartIndex = index ;
}

// ! TODO : should be macros here !

void
ByteArray_AppendCopyItem ( ByteArray * ba, int64 size, int64 data ) // size in bytes
{
    _ByteArray_AppendSpace ( ba, size ) ; // size in bytes
    byte * index = ba->StartIndex ;
    if ( index )
    {
        switch ( size )
        {
            case 1:
            {
                *( ( byte* ) index ) = ( byte ) data ;
                break ;
            }
            case 2:
            {
                *( ( short* ) index ) = ( int16 ) data ;
                break ;
            }
            case 4:
            {
                *( ( int32* ) index ) = ( int32 ) data ;
                break ;
            }
            case 8:
            {
                *( ( int64* ) index ) = ( int64 ) data ;
                break ;
            }
        }
    }
    else Error ( "\nByteArray_AppendCopyItem : Out of memory", ABORT ) ;
}

void
ByteArray_AppendCopy ( ByteArray * ba, int64 size, byte * data ) // size in bytes
{
    _ByteArray_AppendSpace ( ba, size ) ; // size in bytes
    memcpy ( ba->StartIndex, data, size ) ;
}

void
ByteArray_AppendCopyUpToRET ( ByteArray * ba, byte * data ) // size in bytes
{
    int64 i ;
    for ( i = 0 ; 1 ; i ++ )
    {
        if ( data [ i ] == _RET ) break ;
    }
    ByteArray_AppendCopy ( ba, i, data ) ; // ! after we find out how big 'i' is
}

ByteArray *
_NamedByteArray_AddNewByteArray ( NamedByteArray *nba, int64 size )
{
    if ( size < nba->NBA_DataSize )
    {
        size = nba->NBA_DataSize ;
    }
    nba->MemAllocated += size ;
    nba->MemRemaining += size ;
    nba->ba_CurrentByteArray = ByteArray_AllocateNew ( size, nba->NBA_AProperty ) ; // the whole ba itself is allocated as a chunk then we can allocate with its specific type
    dllist_AddNodeToHead ( &nba->NBA_BaList, ( dlnode* ) & nba->ba_CurrentByteArray->BA_Symbol ) ; // ByteArrays are linked here in the NBA with their BA_Symbol node. BA_MemChunk is linked in PermanentMemList
    nba->ba_CurrentByteArray->BA_Symbol.S_Value = ( uint64 ) nba->ba_CurrentByteArray ; // for FreeNbaList
    nba->ba_CurrentByteArray->OurNBA = nba ;
    nba->TotalAllocSize += nba->ba_CurrentByteArray->BA_MemChunk.S_ChunkSize ;

    nba->NumberOfByteArrays ++ ;
    return nba->ba_CurrentByteArray ;
}

NamedByteArray *
_NamedByteArray_Allocate ( int64 allocType )
{
    return ( NamedByteArray* ) _Mem_ChunkAllocate ( sizeof ( NamedByteArray ), allocType ) ;
}

NamedByteArray *
NamedByteArray_Allocate ( )
{
    return _NamedByteArray_Allocate ( OPENVMTIL ) ;
}

void
_NamedByteArray_Init ( NamedByteArray * nba, byte * name, int64 size, int64 atype )
{
    _Symbol_NameInit ( ( Symbol* ) & nba->NBA_Symbol, name ) ;
    nba->NBA_AProperty = atype ;
    dllist_Init ( &nba->NBA_BaList, &nba->NBA_ML_HeadNode, &nba->NBA_ML_TailNode ) ;
    nba->NBA_DataSize = size ;
    nba->MemInitial = size ;
    nba->TotalAllocSize = sizeof ( NamedByteArray ) ;
    Set_NBA_Symbol_To_NBA ( nba ) ;
    nba->NBA_Symbol.S_unmap = nba->NBA_MemChunk.S_unmap ;
    nba->NumberOfByteArrays = 0 ;
    _NamedByteArray_AddNewByteArray ( nba, size ) ;
}

void
NamedByteArray_Delete ( NamedByteArray * nba )
{
    ByteArray * ba ;
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
    {
        nodeNext = dlnode_Next ( node ) ;
        ba = Get_BA_Symbol_To_BA ( node ) ;
        _Mem_ChunkFree ( ( MemChunk * ) ba ) ;
    }
    dlnode_Remove ( ( dlnode* ) & nba->NBA_Symbol ) ;
    _Mem_ChunkFree ( ( MemChunk * ) nba ) ; // mchunk )
}

NamedByteArray *
NamedByteArray_New ( byte * name, int64 size, int64 atype )
{
    NamedByteArray * nba = NamedByteArray_Allocate ( ) ; // else the nba would be deleted with MemList_FreeExactType ( nba->NBA_AProperty ) ;
    _NamedByteArray_Init ( nba, name, size, atype ) ;
    return nba ;
}

// returns true if address is in this nba memory space

int64
NamedByteArray_CheckAddress ( NamedByteArray * nba, byte * address )
{
    ByteArray * ba ;
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
    {
        nodeNext = dlnode_Next ( node ) ;
        ba = Get_BA_Symbol_To_BA ( node ) ;
        if ( ByteArray_IsAddressWwitinTheArray ( ba, address ) == true ) return true ;
        //if ( ( address >= ( byte* ) ba->BA_Data ) && ( address <= ( byte* ) ba->bp_Last ) ) return true ; // ?!? not quite accurate
    }
    return false ;
}

