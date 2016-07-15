#include "../../include/cfrtil.h"

void
_dlnode_Init ( dlnode * node )
{
    node->afterWord = 0 ;
    node->beforeWord = 0 ;
}

dlnode *
_dlnode_New ( uint32 allocType )
{
    dlnode * node = ( dlnode* ) Mem_Allocate ( sizeof (dlnode ), allocType ) ;
    return node ;
}

// toward the TailNode
#define _dlnode_Next( node ) node->afterWord
// toward the HeadNode
#define _dlnode_Previous( node ) node->beforeWord

// toward the TailNode

dlnode *
dlnode_Next ( dlnode * node )
{
    // don't return TailNode return 0
    if ( node && node->afterWord && node->afterWord->afterWord )
    {
        return _dlnode_Next ( node ) ;
    }
    return 0 ;
}

// toward the HeadNode

dlnode *
dlnode_Previous ( dlnode * node )
{
    // don't return HeadNode return 0
    if ( node && node->beforeWord && node->beforeWord->beforeWord )
    {
        return _dlnode_Previous ( node ) ;
    }
    return 0 ;
}

void
dlnode_InsertThisAfterANode ( dlnode * node, dlnode * anode ) // Insert this After node : toward the tail of the list - "after" the Head
{
    if ( node && anode )
    {
        D0 ( if ( anode->N_CProperty & T_TAIL ) Error ( "\nCan't Insert a node after the TailNode!\n", QUIT ) ; ) ;
        if ( anode->afterWord ) anode->afterWord->beforeWord = node ; // don't overwrite a Head or Tail node 
        node->afterWord = anode->afterWord ;
        anode->afterWord = node ; // after the above statement ! obviously
        node->beforeWord = anode ;
    }
}

void
dlnode_InsertThisBeforeANode ( dlnode * node, dlnode * anode ) // Insert this Before node : toward the head of the list - "before" the Tail
{
    if ( node && anode )
    {
        D0 ( if ( anode->N_CProperty & T_HEAD ) Error ( "\nCan't Insert a node before the HeadNode!\n", QUIT ) ; ) ;
        if ( anode->beforeWord ) anode->beforeWord->afterWord = node ; // don't overwrite a Head or Tail node
        node->beforeWord = anode->beforeWord ;
        anode->beforeWord = node ; // after the above statement ! obviously
        node->afterWord = anode ;
    }
}

dlnode *
dlnode_Remove ( dlnode * node )
{
    if ( node )
    {
        d0 ( if ( Is_DebugOn )
        {
            //CfrTil_Namespaces_PrettyPrintTree ( ) ;
            //CfrTil_Using ( ) ;
            Printf ( ( byte* ) "\n\n%s : Before dlnode_Remove : \n\t\t", ( ( Word* ) node )->Name ) ;
                List_PrintNames ( _Q_->OVT_CfrTil->Namespaces->W_List, 10 ) ;
        } ) ;
        D0 ( if ( node->N_Property.T_CProperty & ( T_HEAD | T_TAIL ) ) Error ( "\nCan't remove the Head or Tail node!\n", QUIT ) ) ;
        if ( node->beforeWord ) node->beforeWord->afterWord = node->afterWord ;
        if ( node->afterWord ) node->afterWord->beforeWord = node->beforeWord ;
        node->afterWord = 0 ;
        node->beforeWord = 0 ;
        d0 ( if ( Is_DebugOn )
        {
            //CfrTil_Namespaces_PrettyPrintTree ( ) ;
            //CfrTil_Using ( ) ;
            Printf ( ( byte* ) "\n\n%s : After dlnode_Remove : \n\t\t", ( ( Word* ) node )->Name ) ;
                List_PrintNames ( _Q_->OVT_CfrTil->Namespaces->W_List, 10 ) ;
        } ) ;
    }
    return node ;
}

void
dlnode_ReplaceNodeWithANode ( dlnode * node, dlnode * anode )
{
    if ( node && anode )
    {
        dlnode * afterWord = node->n_After ;
        D0 ( if ( afterWord->N_Property.T_CProperty & ( T_HEAD | T_TAIL ) ) Error ( "\nCan't remove the Head or Tail node!\n", QUIT ) ) ;
        dlnode_Remove ( node ) ;
        dlnode_InsertThisBeforeANode ( anode, afterWord ) ;
    }
}

void
dlnode_Replace ( dlnode * replacedNode, dlnode * replacingNode )
{
    if ( replacedNode && replacingNode )
    {
        D0 ( if ( replacedNode->N_Property.T_CProperty & ( T_HEAD | T_TAIL ) ) Error ( "\nCan't remove the Head or Tail replacedNode!\n", QUIT ) ) ;
        if ( replacedNode->beforeWord ) replacedNode->beforeWord->afterWord = replacingNode ;
        if ( replacedNode->afterWord ) replacedNode->afterWord->beforeWord = replacingNode ;
    }
    //return replacingNode ;
}

void
_dllist_Init ( dllist * list )
{
    if ( list )
    {
        list->head->afterWord = ( dlnode * ) list->tail ;
        list->head->beforeWord = ( dlnode * ) 0 ;
        list->tail->afterWord = ( dlnode* ) 0 ;
        list->tail->beforeWord = ( dlnode * ) list->head ;
        //list->Head->N_Property.T_CProperty = T_HEAD ;
        //list->Tail->N_Property.T_CProperty = T_TAIL ;
        list->S_CurrentNode = 0 ;
    }
}

void
dllist_Init ( dllist * list, dlnode * head, dlnode *tail )
{
    list->head = head ;
    list->tail = tail ;
    _dllist_Init ( list ) ;
}

dllist *
_dllist_New ( uint32 allocType )
{
    dllist * list = ( dllist* ) Mem_Allocate ( sizeof ( dllist ), allocType ) ;
    list->head = _dlnode_New ( allocType ) ;
    list->tail = _dlnode_New ( allocType ) ;
    _dllist_Init ( list ) ;
    return list ;
}

dllist *
dllist_New ( )
{
    return _dllist_New ( DICTIONARY ) ;
}

void
dllist_ReInit ( dllist * list )
{
    dlnode * node, * nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        dlnode_Remove ( node ) ;
    }
    _dllist_Init ( list ) ;
}

int32
dllist_Length ( dllist * list )
{
    int32 length ;
    dlnode * node, * nextNode ;
    for ( length = 0, node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        length ++ ;
    }
    return length ;
}

void
_dllist_AddNodeToHead ( dllist *list, dlnode * node )
{
    if ( node )
    {
        dlnode_InsertThisAfterANode ( node, list->head ) ; // after Head toward Tail
    }
}

void
dllist_AddNodeToHead ( dllist *list, dlnode * node )
{
    if ( node )
    {
        // prevent trying to add nodes already on the list; this will move it to the beginning
        dlnode_Remove ( node ) ; // if the node is already on a list it will be first removed
        _dllist_AddNodeToHead ( list, node ) ;
        list->S_CurrentNode = 0 ;
    }
}

void
dllist_AddNodeToTail ( dllist *list, dlnode * node )
{
    if ( node )
    {
        // prevent trying to add nodes already on the list; this will move it to the beginning
        dlnode_Remove ( node ) ; // prevent trying to add nodes already on the list
        dlnode_InsertThisBeforeANode ( node, list->tail ) ; // before Tail toward Head
        list->S_CurrentNode = node ;
    }
}

dlnode *
dllist_Head ( dllist * list )
{
    if ( ! list ) return 0 ;
    return ( dlnode * ) list->head ;
}

dlnode *
dllist_Tail ( dllist * list )
{
    if ( ! list ) return 0 ;
    return ( dlnode * ) list->tail ;
}

dlnode *
_dllist_First ( dllist * list )
{
    return dlnode_Next ( list->head ) ;
}

dlnode *
dllist_First ( dllist * list )
{
    if ( ! list ) return 0 ;
    return dlnode_Next ( list->head ) ;
}

dlnode *
_dllist_Last ( dllist * list )
{
    return dlnode_Previous ( list->tail ) ;
}

dlnode *
dllist_Last ( dllist * list )
{
    if ( ! list ) return 0 ;
    return dlnode_Previous ( list->tail ) ;
}

dlnode *
dllist_NodePrevious ( dllist * list, dlnode * node )
{
    if ( node )
    {
        node = _dlnode_Previous ( node ) ;
    }
    if ( ! node ) node = dllist_Head ( list ) ;
    return node ;
}

dlnode *
dllist_NodeNext ( dllist * list, dlnode * node )
{
    if ( node )
    {
        node = _dlnode_Next ( node ) ;
    }
    if ( ! node ) node = dllist_Tail ( list ) ;
    return node ;
}

dlnode *
_dllist_Before ( dllist * list )
{
    return dlnode_Previous ( list->S_CurrentNode ) ;
}

dlnode *
dllist_Before ( dllist * list )
{
    list->S_CurrentNode = _dllist_Before ( list ) ;
    if ( list->S_CurrentNode == 0 )
    {
        list->S_CurrentNode = dllist_Head ( list ) ;
        return 0 ;
    }
    return list->S_CurrentNode ;
}
// toward the TailNode

dlnode *
_dllist_After ( dllist * list )
{
    return dlnode_Next ( list->S_CurrentNode ) ;
}
// toward the TailNode

dlnode *
dllist_After ( dllist * list )
{
    list->S_CurrentNode = _dllist_After ( list ) ;
    if ( list->S_CurrentNode == 0 )
    {
        list->S_CurrentNode = dllist_Tail ( list ) ;
        return 0 ;
    }
    return ( dlnode* ) list->S_CurrentNode ;
}

dlnode *
_dllist_AddNamedValue ( dllist * list, byte * name, int32 value, uint32 allocType )
{
    Symbol * sym = _Symbol_New ( name, allocType ) ;
    sym->W_Value = value ;
    _dllist_AddNodeToHead ( list, ( dlnode* ) sym ) ;
}

dobject *
_dllist_Push_M_Slot_Node ( dllist* list, int32 dobjType, int32 allocType, int m_slots, ... )
{
    dobject *dobj ;
    va_list args ;
    int i ;
    va_start ( args, m_slots ) ;
    dobj = _dobject_Allocate ( dobjType, m_slots, allocType ) ;
    for ( i = 0 ; i < m_slots ; i ++ ) dobj->do_iData[i] = va_arg ( args, int32 ) ;
    va_end ( args ) ;
    _dllist_AddNodeToHead ( list, ( dlnode* ) dobj ) ;
    return dobj ;
}
// use list like a endless stack

dlnode *
_dllist_PopNode ( dllist * list )
{
    dlnode *node = dllist_First ( ( dllist* ) list ) ;
    if ( node )
    {
        dlnode_Remove ( node ) ;
        return node ; 
    }
    else return 0 ; // LIST_EMPTY
}

void
_dllist_DropN ( dllist * list, int32 n )
{
    dlnode * node ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node && ( -- n >= 0 ) ; node = dlnode_Next ( node ) )
    {
        dlnode_Remove ( node ) ;
    }
}

int32
_dllist_Get_N_Node_M_Slot ( dllist * list, int32 n, int32 m )
{
    dlnode * node ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node && ( -- n >= 0 ) ; node = dlnode_Next ( node ) ) ;
    return node ? dobject_Get_M_Slot ( node, m ) : 0 ; // LIST_EMPTY
}

void
_dllist_Set_N_Node_M_Slot ( dllist * list, int32 n, int32 m, int32 value )
{
    dlnode * node ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node && ( -- n >= 0 ) ; node = dlnode_Next ( node ) ) ;
    if ( node ) dobject_Set_M_Slot ( node, m, value ) ;
}

int
_dllist_Depth ( dllist * list )
{
    int32 n ;
    dlnode * node ;
    for ( n = 0, node = dllist_First ( ( dllist* ) list ) ; node ; n ++, node = dlnode_Next ( node ) ) ;
    return n ;
}

int32
_dllist_GetTopValue ( dllist * list )
{
    _dllist_Get_N_Node_M_Slot ( list, 0, 0 ) ;
}

int32
_dllist_SetTopValue ( dllist * list, int32 value )
{
    _dllist_Set_N_Node_M_Slot ( list, 0, 0, value ) ;
}

void
dllist_Map ( dllist * list, MapFunction0 mf )
{
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        // get nextNode before map function (mf) in case mf changes list by a Remove of current node
        // problem could arise if mf removes Next node
        nextNode = dlnode_Next ( node ) ;
        mf ( node ) ;
    }
}

void
dllist_Map1 ( dllist * list, MapFunction1 mf, int32 one )
{
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        mf ( node, one ) ;
    }
}

void
dllist_Map2 ( dllist * list, MapFunction2 mf, int32 one, int32 two )
{
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        mf ( node, one, two ) ;
    }
}

int32
dllist_Map3 ( dllist * list, MapFunction3 mf, int32 one, int32 two, int32 three )
{
    int32 rtrn = 0 ;
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        if ( rtrn = mf ( node, one, two, three ) ) break ;
    }
    return rtrn ;
}

void
dllist_Map_OnePlusStatus ( dllist * list, MapFunction2 mf, int32 one, int32 * status )
{
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node && ( *status != DONE ) ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        mf ( node, one, ( int32 ) status ) ;
    }
}

// we have to remember that namespace nodes are being moved around on the Namespaces list by namespace functions

Word *
_TreeMap_NextWord ( Word * thisWord )
{
    Word * nextWord, *nextNs ;
    if ( ! thisWord )
    {
        if ( ! _Context_->NlsWord )
        {
            nextNs = ( Word * ) dllist_First ( ( dllist* ) ( dllist* ) _Q_->OVT_CfrTil->Namespaces->W_List ) ;
        }
        else
        {
            nextNs = 0 ;
            do
            {
                if ( nextNs ) nextNs->W_SearchNumber = 0 ; // reset already visited namespaces
                nextNs = ( Word* ) dlnode_Next ( ( node* ) _Context_->NlsWord ) ;
            }
            while ( nextNs && nextNs->W_SearchNumber ) ;
        }
        _Context_->NlsWord = nextNs ;
        if ( nextNs ) nextWord = nextNs ; //return the list first then next time thru ( Word* ) dllist_First ( (dllist*) nextNs->Lo_List ) ; 
        else nextWord = 0 ; // will restart the cycle thru the _Q_->OVT_CfrTil->Namespaces word lists
    }
    else if ( thisWord == _Context_->NlsWord ) nextWord = ( Word * ) dllist_First ( ( dllist* ) thisWord->Lo_List ) ;
    else
    {
        nextWord = ( Word* ) dlnode_Next ( ( node* ) thisWord ) ;
        if ( ! nextWord )
        {
            if ( thisWord->S_ContainingNamespace )
            {
                if ( thisWord->S_ContainingNamespace == _Context_->NlsWord ) thisWord->S_ContainingNamespace->W_SearchNumber ++ ;
            }
        }
    }
    return nextWord ;
}

Word *
_Tree_Map_0 ( Word * first, MapFunction mf )
{
    Word * word = first ;
    do
    {
        word = _TreeMap_NextWord ( word ) ;
        if ( mf ( ( Symbol* ) word ) ) return word ;
        if ( kbhit ( ) ) return word ; // allow to break search 
    }
    while ( ( ! word ) || ( word != first ) ) ;
    return 0 ;
}

void
_Tree_Map_State_2 ( dllist * list, uint64 state, MapSymbolFunction2 mf, int32 one, int32 two )
{
    dlnode * node, *nextNode ;
    Namespace * ns ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        ns = ( Namespace * ) node ;
        if ( Is_NamespaceType ( ns ) )
        {
            if ( ns->State & state ) mf ( ( Symbol* ) ns, one, two ) ;
            _Tree_Map_State_2 ( ns->W_List, state, mf, one, two ) ;
        }
    }
}

Word *
_Tree_Map_State_Flag_OneArg ( Word * word, uint64 state, int32 oneNamespaceFlag, MapFunction_Cell_1 mf, int32 one )
{
    Word * word2, *nextWord ;
    for ( ; word ; word = nextWord )
    {
        nextWord = ( Word* ) dlnode_Next ( ( node* ) word ) ;
        if ( mf ( ( Symbol* ) word, one ) ) return word ;
        else if ( Is_NamespaceType ( word ) )
        {
            if ( ( ! oneNamespaceFlag ) && ( word->State & state ) )
            {
                if ( ( word2 = _Tree_Map_State_Flag_OneArg ( ( Word* ) dllist_First ( ( dllist* ) word->W_List ), state, oneNamespaceFlag, mf, one ) ) ) return word2 ;
            }
        }
    }
    return 0 ;
}

#if 0 // haven't got this working with tab completion yet

// depth first search

Word *
_TreeMap_FromAWord ( Word * word, MapFunction mf )
{
    Word *nextWord ;
    for ( ; word ; word = nextWord )
    {
        nextWord = ( Word* ) dlnode_Next ( ( Node* ) word ) ;
        if ( mf ( ( Symbol* ) word ) ) return nextWord ;
        if ( Is_NamespaceType ( word ) )
        {
            if ( ( word = _TreeMap_FromAWord ( ( Word* ) dllist_First ( ( dllist* ) word->W_List ), mf ) ) ) return word ;
        }
    }
    return 0 ;
}

Word *
_TC_TreeList_DescendMap ( TabCompletionInfo * tci, Word * nowWord, MapFunction mf )
{
    Word * word2, *nextWord, *firstWord ;
    for ( firstWord = nowWord ; nextWord != firstWord ; nowWord = nextWord )
    {
        nextWord = ( Word* ) dlnode_Next ( ( Node* ) nowWord ) ;
        if ( ! nextWord )
        {
            nextWord = Q_->OVT_Context->NlsWord_Context_->NlsWord ? ( Word* ) dlnode_Next ( ( Node* ) _Context_->NlsWord ) : ( Word* ) dllist_First ( ( dllist* ) _Q_->OVT_CfrTil->Namespaces->W_List ) ;
        }
        if ( mf ( ( Symbol* ) nextWord ) ) return nextWord ;
        else if ( Is_NamespaceType ( nextWord ) && ( nextWord->W_SearchNumber != tci->SearchNumber ) )
        {
            if ( ( word2 = _TC_TreeList_DescendMap ( tci, ( Word* ) dllist_First ( ( dllist* ) nextWord->W_List ), mf ) ) ) return word2 ;
            if ( nextWord->S_ContainingNamespace && nextWord->S_ContainingNamespace->S_ContainingNamespace )
            {
                nextWord->S_ContainingNamespace->W_SearchNumber = tci->SearchNumber ; // end of list; mark it as searched with SearchNumber
            }
        }
    }
    return 0 ;
}
#endif

