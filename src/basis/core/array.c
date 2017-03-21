
#include "../../include/cfrtil.h"

ByteArray *
_ByteArray_AppendSpace_MakeSure ( ByteArray * array, int32 size ) // size in bytes
{
    NamedByteArray * nba = array->OurNBA ;
    if ( nba )
    {
        while ( array->MemRemaining < size )
        {
            int32 largestRemaining = 0 ;
            // check the other arrays in the nba list to see if any have enough remaining
            {
                dlnode * node, *nodeNext ;
                for ( node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
                {
                    nodeNext = dlnode_Next ( node ) ;
                    array = Get_BA_Symbol_To_BA ( node ) ;
                    if ( array->MemRemaining > largestRemaining ) largestRemaining = array->MemRemaining ;
                    if ( array->MemRemaining >= size ) goto done ;
                }
            }
            _Q_->AllocationRequestLacks ++ ;
            nba->NBA_DataSize += ( ( ++ nba->CheckTimes ) * ( 2 * K ) ) + size ;
            if ( array == _Q_CodeByteArray ) nba->NBA_DataSize += 5 * K ; // make sure we have enough code space
            //nba->NBA_DataSize += size ;
            if ( _Q_->Verbosity > 1 )
            {
                printf ( "\n%s size requested = %d :: adding size = %d :: largest remaining = %d :: nba remaining = %d :: checkTimes = %d",
                    nba->NBA_Symbol.Name, size, nba->NBA_DataSize, largestRemaining, nba->MemRemaining, nba->CheckTimes ) ;
            }
            array = _NamedByteArray_AddNewByteArray ( nba, nba->NBA_DataSize ) ; //( nba->NBA_DataSize > size ) ? nba->NBA_DataSize : ( nba->NBA_DataSize + size ) ) ; //size ) ;
        }
    }
    else Error_Abort ( ( byte* ) "\n_ByteArray_AppendSpace_MakeSure : no nba?!\n" ) ;
done:
    return array ;
}

byte *
_ByteArray_AppendSpace ( ByteArray * array, int32 size ) // size in bytes
{
    while ( array->MemRemaining < size )
    {
        array = _ByteArray_AppendSpace_MakeSure ( array, size ) ;
    }
    array->StartIndex = array->EndIndex ; // move index to end of the last append
    array->EndIndex += size ;
    if ( array->OurNBA ) array->OurNBA->MemRemaining -= size ; //nb. debugger->StepInstructionBA doesn't have an nba
    array->MemRemaining -= size ;
    return array->StartIndex ;
}

void
_ByteArray_UnAppendSpace ( ByteArray * array, int32 size ) // size in bytes
{
    // ?? no error checking ??
    array->EndIndex -= size ;
    array->StartIndex -= size ;
}

void
_ByteArray_DataClear ( ByteArray * array )
{
    Mem_Clear ( array->BA_Data, array->BA_DataSize ) ;
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

void
_ByteArray_ReInit ( ByteArray * array )
{
    _ByteArray_Init ( array ) ;
}

ByteArray *
ByteArray_Init ( ByteArray * ba, int32 size, uint32 type )
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
ByteArray_AllocateNew ( int32 size, uint32 type )
{
    ByteArray * ba = ( ByteArray* ) _Mem_ChunkAllocate ( size + sizeof ( ByteArray ), type ) ;
    ByteArray_Init ( ba, size, type ) ;
    return ba ;
}

byte *
_ByteArray_GetEndIndex ( ByteArray * array )
{
    return array->EndIndex ;
}

byte *
_ByteArray_Here ( ByteArray * array )
{
    return array->EndIndex ;
}

void
_ByteArray_SetEndIndex ( ByteArray * array, byte * index )
{
    array->EndIndex = index ;
}

void
_ByteArray_SetHere ( ByteArray * array, byte * index )
{
    array->EndIndex = index ;
}

void
_ByteArray_SetHere_AndForDebug ( ByteArray * array, byte * index )
{
    if ( index )
    {
        _ByteArray_SetEndIndex ( array, index ) ;
        if ( _Debugger_ ) _Debugger_->OptimizedCodeAffected = index ;
    }
}

byte *
_ByteArray_GetStartIndex ( ByteArray * array )
{
    return array->StartIndex ;
}

void
_ByteArray_SetStartIndex ( ByteArray * array, byte * index )
{
    array->StartIndex = index ;
}

// ! TODO : should be macros here !

void
ByteArray_AppendCopyItem ( ByteArray * array, int32 size, int32 data ) // size in bytes
{
    _ByteArray_AppendSpace ( array, size ) ; // size in bytes
    byte * index = array->StartIndex ;
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
                *( ( short* ) index ) = ( short ) data ;
                break ;
            }
            case 4:
            {
                *( ( int* ) index ) = ( int ) data ;
                break ;
            }
            case 8:
            {
                *( ( long int* ) index ) = ( long int ) data ;
                break ;
            }
        }
    }
    else Error ( "\nByteArray_AppendCopyItem : Out of memory", ABORT ) ;
}

void
ByteArray_AppendCopy ( ByteArray * array, int32 size, byte * data ) // size in bytes
{
    _ByteArray_AppendSpace ( array, size ) ; // size in bytes
    memcpy ( array->StartIndex, data, size ) ;
}

void
ByteArray_AppendCopyUpToRET ( ByteArray * array, byte * data ) // size in bytes
{
    int32 i ;
    for ( i = 0 ; 1 ; i ++ )
    {
        if ( data [ i ] == _RET ) break ;
    }
    ByteArray_AppendCopy ( array, i, data ) ; // ! after we find out how big 'i' is
}

ByteArray *
_NamedByteArray_AddNewByteArray ( NamedByteArray *nba, int32 size )
{
    if ( size < nba->NBA_DataSize )
    {
        size = nba->NBA_DataSize ;
    }
    nba->MemAllocated += size ;
    nba->MemRemaining += size ;
    nba->ba_CurrentByteArray = ByteArray_AllocateNew ( size, nba->NBA_AProperty ) ; // the whole array itself is allocated as a chunk then we can allocate with its specific type
    dllist_AddNodeToHead ( &nba->NBA_BaList, ( dlnode* ) & nba->ba_CurrentByteArray->BA_Symbol ) ; // ByteArrays are linked here in the NBA with their BA_Symbol node. BA_MemChunk is linked in PermanentMemList
    nba->ba_CurrentByteArray->BA_Symbol.S_Value = ( uint32 ) nba->ba_CurrentByteArray ; // for FreeNbaList
    nba->ba_CurrentByteArray->OurNBA = nba ;
    nba->TotalAllocSize += nba->ba_CurrentByteArray->BA_MemChunk.S_ChunkSize ;

    nba->NumberOfByteArrays ++ ;
    return nba->ba_CurrentByteArray ;
}

NamedByteArray *
_NamedByteArray_Allocate ( int32 allocType )
{
    return ( NamedByteArray* ) _Mem_ChunkAllocate ( sizeof ( NamedByteArray ), allocType ) ;
}

NamedByteArray *
NamedByteArray_Allocate ( )
{
    return _NamedByteArray_Allocate ( OPENVMTIL ) ;
}

void
_NamedByteArray_Init ( NamedByteArray * nba, byte * name, int32 size, int32 atype )
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
    ByteArray * array ;
    dlnode * node, *nodeNext ;
    for ( node = dllist_First ( ( dllist* ) & nba->NBA_BaList ) ; node ; node = nodeNext )
    {
        nodeNext = dlnode_Next ( node ) ;
        array = Get_BA_Symbol_To_BA ( node ) ;
        _Mem_ChunkFree ( ( MemChunk * ) array ) ;
    }
    dlnode_Remove ( ( dlnode* ) & nba->NBA_Symbol ) ;
    _Mem_ChunkFree ( ( MemChunk * ) nba ) ; // mchunk )
}

NamedByteArray *
NamedByteArray_New ( byte * name, int32 size, int32 atype )
{
    NamedByteArray * nba = NamedByteArray_Allocate ( ) ; // else the nba would be deleted with MemList_FreeExactType ( nba->NBA_AProperty ) ;
    _NamedByteArray_Init ( nba, name, size, atype ) ;
    return nba ;
}

