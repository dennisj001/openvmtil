
#include "../include/cfrtil.h"

void
List_Interpret ( dllist * list )
{
    dllist_Map ( list, _Interpret_ListNode ) ;
    List_Init ( list ) ;
}

// list : a list of lists of postfix operations needing to be interpreted

void
List_InterpretLists ( dllist * list )
{
    Compiler * compiler = _Compiler_ ;
    int64 svs = GetState ( compiler, C_INFIX_EQUAL ) ;
    SetState ( compiler, C_INFIX_EQUAL, false ) ;
    SetState ( compiler, INFIX_LIST_INTERPRET, true ) ;
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        // get nextNode before map function (mf) in case mf changes list by a Remove of current node
        // problem could arise if mf removes Next node
        nextNode = dlnode_Next ( node ) ;
        dllist * list = ( dllist * ) dobject_Get_M_Slot ( node, 0 ) ;
        List_Interpret ( list ) ;
        dlnode_Remove ( node ) ;
    }
    List_Init ( list ) ;
    SetState ( compiler, INFIX_LIST_INTERPRET, false ) ;
    SetState ( compiler, C_INFIX_EQUAL, svs ) ;
}

// list : a list of lists of postfix operations needing to be interpreted

void
List_CheckInterpretLists_OnVariable ( dllist * list, byte * token )
{
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        // get nextNode before map function (mf) in case mf changes list by a Remove of current node
        // problem could arise if mf removes Next node
        nextNode = dlnode_Next ( node ) ;
        dllist * plist = ( dllist * ) dobject_Get_M_Slot ( node, 0 ) ; // plist created in CfrTil_IncDec
        Word * word = ( Word * ) List_Top ( plist ) ; 
        byte *checkPostfixToken = word ? word->Name : 0 ;
        if ( checkPostfixToken && String_Equal ( checkPostfixToken, token ) )
        {
            List_Interpret ( plist ) ;
            dlnode_Remove ( node ) ;
        }
    }
    //List_Init ( list ) ;
}

void
_List_PrintNames ( dllist * list, int64 count, int64 flag )
{
    dlnode * node, *nextNode ;
    Word * nodeWord, *beforeWord, *afterWord ;
    byte * thisName, *beforeName, *afterName, *bt = Buffer_New_pbyte ( 64 ), *ba = Buffer_New_pbyte ( 64 ), *bb = Buffer_New_pbyte ( 64 ) ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node && count -- ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        nodeWord = ( node->afterWord && node->afterWord->afterWord ? ( Word* ) dobject_Get_M_Slot ( node, 0 ) : 0 ) ;
        if ( ! nodeWord ) break ;
        thisName = nodeWord ? sconvbs ( bt, nodeWord->Name ) : ( byte* ) " ", node ;
        if ( flag )
        {
            beforeWord = ( node->beforeWord == list->afterWord ? 0 : ( Word * ) dobject_Get_M_Slot ( node->beforeWord, 0 ) ) ;
            afterWord = ( node->afterWord == list->afterWord ? 0 : ( Word* ) dobject_Get_M_Slot ( node->afterWord, 0 ) ) ;
            afterName = afterWord ? sconvbs ( ba, afterWord->Name ) : ( byte* ) " ", node->afterWord ;
            beforeName = beforeWord ? sconvbs ( bb, ( beforeWord )->Name ) : ( byte* ) " ", node->beforeWord ;
            _Printf ( ( byte* ) "\n\tName : %s 0x%08x \t\tBefore : %s 0x%08x : \t\tAfter : %s 0x%08x,", 
                thisName, node, beforeName, node->beforeWord, afterName, node->afterWord ) ;
        }
        else _Printf ( ( byte* ) "\n\tName : %s", thisName ) ;
    }
}

void
_List_Show_N_Word_Names ( dllist * list, uint64 n, int64 showBeforeAfterFlag, int64 dbgFlag ) //byte * listName, int64 dbgFlag )
{
    if ( dbgFlag ) NoticeColors ;
    _List_PrintNames ( list, n, showBeforeAfterFlag ) ; 
    if ( dbgFlag ) DefaultColors ;
}
