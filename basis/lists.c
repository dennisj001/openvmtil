
#include "../includes/cfrtil.h"

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
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        // get nextNode before map function (mf) in case mf changes list by a Remove of current node
        // problem could arise if mf removes Next node
        nextNode = dlnode_Next ( node ) ;
        dllist * list = ( dllist * ) DynoInt_GetValue ( node ) ;
        List_Interpret ( list ) ;
        dlnode_Remove ( node ) ;
    }
    List_Init ( list ) ;
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
        dllist * plist = ( dllist * ) DynoInt_GetValue ( node ) ; // plist created in CfrTil_IncDec
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
#if 0// partially adjusted from _Stack_Show_N_Word_Names

int32
List_Length ( dllist * list )
{
    int32 i ;
    dlnode * node, *nextNode ;
    for ( i = 0, node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        i ++ ;
    }
    return i ;
}

dlnode *
List_Search ( dllist * list, int32 value )
{
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        if ( ( ( Symbol * ) node )->W_Value == value ) return node ;
    }
    return 0 ;
}

dlnode *
List_AddValue ( dllist * list, int32 value )
{
    return _dllist_AddValue ( list, value, SESSION ) ;
}

dlnode *
List_AddNamedValue ( dllist * list, byte * name, int32 value )
{
    return _dllist_AddNamedValue ( list, name, value, SESSION ) ;
}
#endif

#if 1 // save
#define Str_Conv( d, s ) (byte*) _String_ConvertStringToBackSlash ( d, s )

void
_List_PrintNames ( dllist * list, int32 count, int32 flag )
{
    dlnode * node, *nextNode ;
    Word * nodeWord, *beforeWord, *afterWord ;
    byte * thisName, *beforeName, *afterName, *bt = Buffer_New_pbyte ( 64 ), *ba = Buffer_New_pbyte ( 64 ), *bb = Buffer_New_pbyte ( 64 ) ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node && count -- ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        nodeWord = ( node->afterWord && node->afterWord->afterWord ? ( Word* ) DynoInt_GetValue ( node ) : 0 ) ;
        if ( ! nodeWord ) break ;
        thisName = nodeWord ? Str_Conv ( bt, nodeWord->Name ) : ( byte* ) " ", node ;
        if ( flag )
        {
            beforeWord = ( node->beforeWord == list->afterWord ? 0 : ( Word * ) DynoInt_GetValue ( node->beforeWord ) ) ;
            afterWord = ( node->afterWord == list->afterWord ? 0 : ( Word* ) DynoInt_GetValue ( node->afterWord ) ) ;
            afterName = afterWord ? Str_Conv ( ba, afterWord->Name ) : ( byte* ) " ", node->afterWord ;
            beforeName = beforeWord ? Str_Conv ( bb, ( beforeWord )->Name ) : ( byte* ) " ", node->beforeWord ;
            Printf ( ( byte* ) "\n\tName : %s 0x%08x \t\tBefore : %s 0x%08x : \t\tAfter : %s 0x%08x,", thisName, node, beforeName, node->beforeWord, afterName, node->afterWord ) ;
        }
        else Printf ( ( byte* ) "\n\tName : %s", thisName ) ;
    }
}

void
_List_Show_N_Word_Names ( dllist * list, uint32 n, int32 showBeforeAfterFlag, int32 dbgFlag ) //byte * listName, int32 dbgFlag )
{
    uint32 i ;
    //int32 depth = Stack_Depth ( stack ) ;
    //byte * buffer = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    if ( dbgFlag ) NoticeColors ;
    //_Stack_PrintHeader ( stack, listName ) ;
    _List_PrintNames ( list, n, showBeforeAfterFlag ) ; //_Stack_Print ( stack, stackName ) ;
    if ( dbgFlag ) DefaultColors ;
}
#endif
