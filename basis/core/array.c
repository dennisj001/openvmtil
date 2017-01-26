
#include "../../include/cfrtil.h"

byte *
_ByteArray_AppendSpace ( ByteArray * array, int32 size ) // size in bytes
{
    NamedByteArray * nba = array->OurNBA ;
tryAgain:
    array->StartIndex = array->EndIndex ; // move index to end of the last append
    array->EndIndex += size ;
    if ( array->EndIndex >= array->bp_Last )
    {
        if ( nba )
        {
            if ( nba->NBA_AProperty == CODE )
            {
                Error_Abort ( ( byte* ) "\nOut of Code Memory : Set Code Memory size higher at startup.\n" ) ;
            }
            _NamedByteArray_AddNewByteArray ( nba, nba->NBA_DataSize > size ? nba->NBA_DataSize : size ) ; //size ) ;
            array = nba->ba_CurrentByteArray ;
            goto tryAgain ;
        }
    }
    if ( nba ) nba->MemRemaining -= size ; //nb. debugger->StepInstructionBA doesn't have an nba
    array->MemRemaining -= size ;
    //memset ( array->StartIndex, 0, size ) ; // shouldn't need to do this since we clear the array at allocation time
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
    ba->bp_Last = & ba->BA_Data [ ba->BA_DataSize ] ;
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
    //ba->BA_Symbol.S_unmap = ba->BA_MemChunk.S_unmap ; 
    _ByteArray_Init ( ba ) ;
    return ba ;
}

ByteArray *
ByteArray_AllocateNew ( int32 size, uint32 type )
{
    //ByteArray * ba = _ByteArray_Allocate ( size, type ) ;
    ByteArray * ba = ( ByteArray* ) _Mem_Allocate ( size + sizeof ( ByteArray ), type ) ;
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
    byte * index ;
    if ( ! ( index = array->StartIndex ) )
        Error ( "\nByteArray_AppendCopyItem : Out of memory", ABORT ) ;
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

void
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
}

NamedByteArray *
_NamedByteArray_Allocate ( )
{
    return ( NamedByteArray* ) _Mem_Allocate ( sizeof ( NamedByteArray ), OPENVMTIL ) ;
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

NamedByteArray *
NamedByteArray_New ( byte * name, int32 size, int32 atype )
{
    NamedByteArray * nba = _NamedByteArray_Allocate ( ) ; // else the nba would be deleted with MemList_FreeExactType ( nba->NBA_AProperty ) ;
    _NamedByteArray_Init ( nba, name, size, atype ) ;
    return nba ;
}

