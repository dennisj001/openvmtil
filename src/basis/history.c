
#include "../include/cfrtil.h"

HistoryStringNode *
HistoryStringNode_New ( byte * hstring )
{
    HistoryStringNode * hsn = ( HistoryStringNode * ) Mem_Allocate ( sizeof ( HistoryStringNode ), HISTORY ) ;
    _Symbol_Init_AllocName ( ( Symbol* ) hsn, hstring, HISTORY ) ; // use Name for history string
    //hsn->S_CProperty = HISTORY_NODE ;
    return hsn ;
}

HistoryStringNode *
HistorySymbolList_Find ( byte * hstring )
{
    HistoryStringNode * hsn = 0 ;
    dlnode * node, * nextNode ;
#if 1   
    for ( node = dllist_First ( (dllist*) _Q_->OVT_HistorySpace.StringList ) ; node ; node = nextNode ) // index = dlnode_NextNode ( &_Q->HistoryList, (dlnode *) index ) )
    {
        nextNode = dlnode_Next ( node ) ;
        hsn = ( HistoryStringNode* ) node ;
        if ( ( hsn->S_Name ) && ( String_Equal ( ( char* ) hsn->S_Name, ( char* ) hstring ) ) )
        {
            return hsn ;
        }
    }
#else // some work towards eliminating the StringList and just using the MemList
    for ( node = dllist_First ( (dllist*) _Q_->OVT_HistorySpace.MemList ) ; node ; node = nextNode ) // index = dlnode_NextNode ( &_Q->HistoryList, (dlnode *) index ) )
    {
        nextNode = dlnode_Next ( node ) ;
        hsn = ( HistoryStringNode* ) ( ( MemChunk * ) node + 1 ) ;
        if ( ( hsn->S_Name ) && ( String_Equal ( ( char* ) hsn->S_Name, ( char* ) hstring ) ) )
        {
            return hsn ;
        }
    }
#endif    
    return 0 ;
}

void
ReadLine_ShowHistoryNode ( ReadLiner * rl )
{
    rl->EscapeModeFlag = 0 ;
    if ( rl->HistoryNode && rl->HistoryNode->S_Name )
    {
        byte * dst = Buffer_Data ( _CfrTil_->ScratchB1 ) ;
        dst = _String_ConvertStringToBackSlash ( dst, rl->HistoryNode->S_Name ) ;
        _ReadLine_PrintfClearTerminalLine ( ) ;
        __ReadLine_DoStringInput ( rl, String_FilterMultipleSpaces ( dst, TEMPORARY ), rl->AltPrompt ) ;
        ReadLine_SetCursorPosition ( rl, rl->EndPosition ) ;
    }
    else
    {
        ReadLine_ClearCurrentTerminalLine ( rl, rl->EndPosition ) ; // nb : this is also part of ShowString
        ReadLine_ShowNormalPrompt ( rl ) ;
    }
    _ReadLine_CursorToEnd ( rl ) ;
}

void
_OpenVmTil_AddStringToHistoryList ( byte * istring )
{
    HistoryStringNode * hsn ;
    if ( istring && strcmp ( ( char* ) istring, "" ) ) // don't add blank lines to history
    {
        //Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
        byte * nstring = Buffer_Data ( _CfrTil_->ScratchB1 ) ;
        nstring = _String_ConvertStringToBackSlash ( nstring, istring ) ;

        hsn = HistorySymbolList_Find ( nstring ) ;
        if ( ! hsn )
        {
            hsn = HistoryStringNode_New ( nstring ) ;
        }
        else dlnode_Remove ( ( dlnode* ) hsn ) ; // make it last with dllist_AddNodeToTail
        dllist_AddNodeToTail ( _Q_->OVT_HistorySpace.StringList, ( dlnode* ) hsn ) ; //
        d0 ( int64 ll = List_Length ( _Q_->OVT_HistorySpace.StringList ) ) ;
        dllist_SetCurrentNode_After ( _Q_->OVT_HistorySpace.StringList ) ; // ! properly set Object.dln_Node
        //Buffer_SetAsUnused ( buffer ) ;
    }
}

void
OpenVmTil_AddStringToHistory ( )
{
    byte * string = ( byte* ) _DataStack_Pop ( ) ;
    _OpenVmTil_AddStringToHistoryList ( string ) ;
}

void
OpenVmTil_AddStringToHistoryOn ( )
{
    SetState ( _Context_->ReadLiner0, ADD_TO_HISTORY, true ) ;
}

void
OpenVmTil_AddStringToHistoryOff ( )
{
    SetState ( _Context_->ReadLiner0, ADD_TO_HISTORY, false ) ;
}

void
HistorySpace_Delete ( )
{
    //MemList_FreeExactType ( HISTORY ) ;
    OVT_MemListFree_HistorySpace ( ) ;
}

HistorySpace *
_HistorySpace_Init ( OpenVmTil * ovt, int64 reset )
{
    ovt->OVT_HistorySpace.StringList = & ovt->OVT_HistorySpace._StringList ;
    dllist_Init ( ovt->OVT_HistorySpace.StringList, &ovt->OVT_HistorySpace._StringList_HeadNode, &ovt->OVT_HistorySpace._StringList_TailNode ) ;
    if ( ovt ) _Q_->OVT_HistorySpace.HistorySpaceNBA = ovt->MemorySpace0->HistorySpace ;
    if ( reset ) _NamedByteArray_Init ( _Q_->OVT_HistorySpace.HistorySpaceNBA, ( byte* ) "HistorySpace", HISTORY_SIZE, HISTORY ) ;
}

void
_HistorySpace_New ( OpenVmTil * ovt, int64 resetFlag )
{
    if ( resetFlag )
    {
        HistorySpace_Delete ( ) ;
    }
    _HistorySpace_Init ( ovt, resetFlag ) ;
}

void
HistorySpace_Reset ( void )
{
    _HistorySpace_New ( _Q_, 1 ) ;
}

