
#include "../includes/cfrtil.h"

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
            if ( nba->NBA_AType == CODE )
            {
                Error_Abort ( ( byte* ) "\nOut of Code Memory : Set Code Memory size higher at startup.\n" ) ;
            }
            _NamedByteArray_AddNewByteArray ( nba, size ) ;
            array = nba->ba_ByteArray ;
            goto tryAgain ;
        }
    }
    if ( nba ) nba->MemRemaining -= size ; //nb. debugger->StepInstructionBA doesn't have an nba
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
    Mem_Clear ( array->BA_Data, array->BA_Size ) ;
}

void
_ByteArray_Init ( ByteArray * array )
{
    array->BA_Data = ( byte* ) ( array + 1 ) ;
    array->StartIndex = array->BA_Data ;
    array->EndIndex = array->StartIndex ;
    array->bp_Last = & array->BA_Data [ array->BA_Size ] ;
    _ByteArray_DataClear ( array ) ;
}

void
_ByteArray_ReInit ( ByteArray * array )
{
    _ByteArray_Init ( array ) ;
}

ByteArray *
_ByteArray_AllocateNew_ ( DLList * list, int32 size, int64 type )
{
    //ByteArray * ba = ( ByteArray* ) Mem_AllocateAndAccountChunk ( sizeof ( ByteArray ) + size, type ) ;
    ByteArray * ba = ( ByteArray* ) _Mem_Allocate ( 0, sizeof ( ByteArray ) + size, OPENVMTIL, (MEM_CHUNK_ACCOUNT) )  ;
    ba->BA_Size = size ; // nb. not accounting for sizeof ByteArray here
    ba->BA_AType = type ;
    DLList_AddNodeToHead ( list, ( DLNode* ) ba ) ;
    _ByteArray_Init ( ba ) ;
    return ba ;
}

ByteArray *
_ByteArray_AllocateNew ( NamedByteArray * nba, int32 size )
{
    ByteArray * ba = _ByteArray_AllocateNew_ ( nba->NBA_MemoryList, size, nba->NBA_AType ) ;
    ba->OurNBA = nba ;
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
    _ByteArray_SetEndIndex ( array, index ) ;
    _Q_->OVT_CfrTil->Debugger0->OptimizedCodeAffected = index ;
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
NBA_Show ( NamedByteArray * nba )
{
    ByteArray * ba = nba->ba_ByteArray ;
    byte * name = nba->NBA_Symbol.S_Name ;
    Printf ( ( byte* ) "\n%-28s" "Used = " INT_FRMT_9 " : Available = " INT_FRMT_9, name, nba->MemAllocated - nba->MemRemaining, nba->MemRemaining ) ;
}

void
_NamedByteArray_AddNewByteArray ( NamedByteArray *nba, int32 size )
{
    if ( size > nba->NBA_Size )
    {
        size += nba->NBA_Size ;
    }
    nba->MemAllocated += size ;
    nba->MemRemaining += size ;
    nba->ba_ByteArray = _ByteArray_AllocateNew ( nba, size ) ; // the whole array itself is allocated as a chunk then we can allocate with its specific type
    nba->NumberOfByteArrays ++ ;
}

NamedByteArray *
_NamedByteArray_Allocate ( )
{
    //return ( NamedByteArray * ) MemList_AllocateAndAccount_MemChunkAdded ( _Q_->PermanentMemList, sizeof ( NamedByteArray ), OPENVMTIL ) ; // nb : this is just the nba structure and it must be type OPENVMTIL or ..
    return (NamedByteArray*) _Mem_Allocate ( _Q_->PermanentMemList, sizeof ( NamedByteArray ), OPENVMTIL, (ADD_TO_LIST|ADD_MEM_CHUNK|INIT_MEM_CHUNK|MEM_CHUNK_ACCOUNT|RETURN_MEM_CHUNK) )  ;
}

void
_NamedByteArray_Init ( NamedByteArray * nba, byte * name, int32 size, int32 atype )
{
    _Symbol_NameInit ( ( Symbol* ) nba, name ) ;
    nba->NBA_AType = atype ;
    nba->NBA_MemoryList = _DLList_New ( OPENVMTIL ) ;
    nba->NBA_Size = size ;
    nba->MemInitial = size ;
    nba->NumberOfByteArrays = 0 ;
    _NamedByteArray_AddNewByteArray ( nba, size ) ;
}

NamedByteArray *
NamedByteArray_New ( byte * name, int32 size, int32 atype )
{
    NamedByteArray * nba = _NamedByteArray_Allocate ( ) ; // else the nba would be deleted with MemList_FreeExactType ( nba->NBA_AType ) ;
    _NamedByteArray_Init ( nba, name, size, atype ) ;
    return nba ;
}

