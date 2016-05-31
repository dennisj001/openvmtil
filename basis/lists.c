
#include "../includes/cfrtil.h"

int32
List_Length ( dllist * list )
{
    int32 i ;
    dlnode * node, *nextNode ;
    for ( i = 0, node = dllist_First ( (dllist*) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        i++ ;
    }
    return i ;
}

#if 0 // partially adjusted from _Stack_Show_N_Word_Names
dlnode *
List_PrintNames ( dllist * list, int32 count )
{
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( (dllist*) list ) ; node && count--; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        Printf ( (byte*) "\nName : %010s 0x%08x \t\tBefore : %s 0x%08x: \tAfter : %s 0x%08x,", ((Word*) node)->Name, node, ((Word*) node->before)->Name, node->before, ((Word*) node->after)->Name, node->after )  ;
    }
    return 0 ;
}

dlnode *
List_Search ( dllist * list, int32 value )
{
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( (dllist*) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        if ( ((Symbol *) node)->W_Value == value ) return node ;
    }
    return 0 ;
}

dlnode *
List_AddValue ( dllist * list, int32 value )
{
    return _dllist_AddValue ( list, value, SESSION );
}

dlnode *
List_AddNamedValue ( dllist * list, byte * name, int32 value )
{
    return _dllist_AddNamedValue ( list, name, value, SESSION );
}

void
_List_Show_N_Word_Names ( List * list, uint32 n, byte * listName, int32 dbgFlag )
{
    uint32 i ;
    int32 depth = Stack_Depth ( stack ) ;
    byte * buffer = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    if ( dbgFlag ) NoticeColors ;
    _Stack_PrintHeader ( stack, listName ) ;
    for ( i = 0 ; ( n > i ) && ( i < depth ) ; i ++ )
    {
        if ( Stack_N ( stack, - i ) ) _Stack_Show_Word_Name_AtN ( stack, i, listName, buffer ) ;
        else break ;
    }
    //_Stack_Print ( stack, stackName ) ;
    if ( dbgFlag ) DefaultColors ;
}
#endif

