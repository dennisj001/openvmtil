
#include "../../include/cfrtil.h"

void
_Compile_C_Call_1_Arg ( byte* function, int32 arg )
{
    _Compile_Esp_Push ( arg ) ;
    Compile_Call ( function ) ;
    _Compile_Rsp_Drop ( ) ;
}

void
_CompileN ( byte * data, int32 size )
{
    ByteArray_AppendCopy ( _Q_CodeByteArray, size, data ) ; // size in bytes
}

// nb : can be used by an compiler on
// a code body that has only one RET point

void
_CompileFromUptoRET ( byte * data )
{
    ByteArray_AppendCopyUpToRET ( _Q_CodeByteArray, data ) ; // size in bytes
}

// copy block from 'address' to Here

void
_Compile_WordInline ( Word * word ) // , byte * dstAddress )
{
    _Block_Copy ( ( byte* ) word->Definition, word->S_CodeSize ) ;
}

void
_CompileFromName ( byte * wordName )
{
    Word * word = Finder_Word_FindUsing ( _Context_->Finder0, wordName, 0 ) ;
    // ?? Exception : error message here
    if ( ! word ) _Throw ( QUIT ) ;
    _Word_Compile ( word ) ;
}

void
_CompileFromName_Inline ( byte * wordName )
{
    Word * word = Finder_Word_FindUsing ( _Context_->Finder0, wordName, 0 ) ;
    if ( ! word ) _Throw ( QUIT ) ;
    _Compile_WordInline ( word ) ;
}

void
_MoveGotoPoint ( dlnode * node, int32 srcAddress, int32 key, int32 dstAddress )
{
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    byte * address = gotoInfo->pb_JmpOffsetPointer ;
    if ( ( byte* ) srcAddress == address ) 
    {
        gotoInfo->pb_JmpOffsetPointer = ( byte* ) dstAddress ;
    }
}

void
_GotoInfo_SetAndDelete ( GotoInfo * gotoInfo, byte * address )
{
    _SetOffsetForCallOrJump ( gotoInfo->pb_JmpOffsetPointer, address ) ;
    GotoInfo_Delete ( ( dlnode* ) gotoInfo ) ;
}

void
_InstallGotoPoint_Key ( dlnode * node, int32 bi, int32 key )
{
    Word * word ;
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    byte * address = gotoInfo->pb_JmpOffsetPointer ;
    if ( *( int32* ) address == 0 ) // if we move a block its recurse offset remains, check if this looks like at real offset pointer
    {
        if ( ( gotoInfo->GI_CProperty & ( GI_GOTO | GI_CALL_LABEL ) ) && ( key & ( GI_GOTO | GI_CALL_LABEL ) ) )
        {
            //Namespace * ns = Namespace_FindOrNew_SetUsing ( ( byte* ) "__labels__", _CfrTil_->Namespaces, 1 ) ;
            Namespace * ns = _Namespace_Find ( ( byte* ) "__labels__", _CfrTil_->Namespaces, 0 ) ;
            if ( ns && ( word = Word_FindInOneNamespace ( ns, gotoInfo->pb_LabelName ) ) )
            {
                _GotoInfo_SetAndDelete ( gotoInfo, ( byte* ) word->W_Value ) ;
            }
        }
        else if ( ( gotoInfo->GI_CProperty & GI_RETURN ) && ( key & GI_RETURN ) )
        {
            _GotoInfo_SetAndDelete ( gotoInfo, Here ) ;
        }
        else if ( ( gotoInfo->GI_CProperty & GI_BREAK ) && ( key & GI_BREAK ) )
        {
            if ( _Context_->Compiler0->BreakPoint )
            {
                _GotoInfo_SetAndDelete ( gotoInfo, _Context_->Compiler0->BreakPoint ) ;
            }
        }
        else if ( ( gotoInfo->GI_CProperty & GI_CONTINUE ) && ( key & GI_CONTINUE ) )
        {
            if ( _Context_->Compiler0->ContinuePoint )
            {
                _GotoInfo_SetAndDelete ( gotoInfo, _Context_->Compiler0->ContinuePoint ) ;
            }
        }
        else if ( ( gotoInfo->GI_CProperty & GI_RECURSE ) && ( key & GI_RECURSE ) )
        {
            _GotoInfo_SetAndDelete ( gotoInfo, ( byte* ) ( ( BlockInfo * ) bi )->bp_First ) ;
        }
        else if ( ( gotoInfo->GI_CProperty & GI_TAIL_CALL ) && ( key & GI_TAIL_CALL ) )
        {
            _GotoInfo_SetAndDelete ( gotoInfo, ( ( BlockInfo * ) bi )->Start ) ; // ( byte* ) _DataStack_GetTop ( ) ) ; // , ( byte* ) bi->bi_FrameStart ) ;
        }
    }
}

void
_CheckForGotoPoint ( dlnode * node, int32 key, int32 * status )
{
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    if ( gotoInfo->GI_CProperty & key )
    {
        *status = DONE ;
    }
}

void
_RemoveGotoPoint ( dlnode * node, int32 key, int32 * status )
{
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    if ( gotoInfo->GI_CProperty & key )
    {
        dlnode_Remove ( ( dlnode* ) gotoInfo ) ;
        *status = DONE ;
    }
}

void
_CfrTil_InstallGotoCallPoints_Keyed ( BlockInfo * bi, int32 key )
{
    dllist_Map2 ( _Context_->Compiler0->GotoList, ( MapFunction2 ) _InstallGotoPoint_Key, ( int32 ) bi, key ) ;
}

int32 
_CfrTil_MoveGotoPoint ( int32 srcAddress, int32 key, int32 dstAddress )
{
    return dllist_Map3 ( _Context_->Compiler0->GotoList, ( MapFunction3 ) _MoveGotoPoint, srcAddress, key, dstAddress ) ;
}

int32
CfrTil_CheckForGotoPoints ( int32 key ) // compile time
{
    int32 status = 0 ;
    dllist_Map_OnePlusStatus ( _Context_->Compiler0->GotoList, ( MapFunction2 ) _CheckForGotoPoint, key, &status ) ;
    return status ;
}

int32
CfrTil_RemoveGotoPoints ( int32 key ) // compile time
{
    int32 status = 0 ;
    dllist_Map_OnePlusStatus ( _Context_->Compiler0->GotoList, ( MapFunction2 ) _RemoveGotoPoint, key, &status ) ;
    return status ;
}

