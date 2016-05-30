
#include "../includes/cfrtil.h"

int32
List_Length ( DLList * list )
{
    int32 i ;
    DLNode * node, *nextNode ;
    for ( i = 0, node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        i++ ;
    }
    return i ;
}

DLNode *
List_PrintNames ( DLList * list, int32 count )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node && count--; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        Printf ( (byte*) "\nName : %010s 0x%08x \t\tBefore : %s 0x%08x: \tAfter : %s 0x%08x,", ((Word*) node)->Name, node, ((Word*) node->Before)->Name, node->Before, ((Word*) node->After)->Name, node->After )  ;
    }
    return 0 ;
}

DLNode *
List_Search ( DLList * list, int32 value )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        if ( ((Symbol *) node)->W_Value == value ) return node ;
    }
    return 0 ;
}

DLNode *
List_AddValue ( DLList * list, int32 value )
{
    return _DLList_AddValue ( list, value, SESSION );
}

DLNode *
List_AddNamedValue ( DLList * list, byte * name, int32 value )
{
    return _DLList_AddNamedValue ( list, name, value, SESSION );
}

#if 0 // partially adjusted from _Stack_Show_N_Word_Names
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

