#include "../includes/cfrtil.h"

void
_DLNode_Init ( DLNode * node )
{
    node->After = 0 ;
    node->Before = 0 ;
}

DLNode *
_DLNode_New ( uint64 allocType )
{
    DLNode * node = ( DLNode* ) Mem_Allocate ( sizeof (DLNode ), allocType ) ;
    return node ;
}

// toward the TailNode
#define _DLNode_Next( node ) node->After
// toward the HeadNode
#define _DLNode_Previous( node ) node->Before

// toward the TailNode

DLNode *
DLNode_Next ( DLNode * node )
{
    // don't return TailNode return 0
    if ( node && node->After && node->After->After )
    {
        return _DLNode_Next ( node ) ;
    }
    return 0 ;
}

// toward the HeadNode

DLNode *
DLNode_Previous ( DLNode * node )
{
    // don't return HeadNode return 0
    if ( node && node->Before && node->Before->Before )
    {
        return _DLNode_Previous ( node ) ;
    }
    return 0 ;
}

void
DLNode_InsertThisAfterANode ( DLNode * node, DLNode * anode ) // Insert this After node : toward the tail of the list - "after" the Head
{
    if ( node && anode )
    {
        D0 ( if ( anode->N_CType & T_TAIL ) Error ( "\nCan't Insert a node after the TailNode!\n", QUIT ) ; ) ;
        if ( anode->After ) anode->After->Before = node ; // don't overwrite a Head or Tail node 
        node->After = anode->After ;
        anode->After = node ; // after the above statement ! obviously
        node->Before = anode ;
    }
}

void
DLNode_InsertThisBeforeANode ( DLNode * node, DLNode * anode ) // Insert this Before node : toward the head of the list - "before" the Tail
{
    if ( node && anode )
    {
        D0 ( if ( anode->N_CType & T_HEAD ) Error ( "\nCan't Insert a node before the HeadNode!\n", QUIT ) ; ) ;
        if ( anode->Before ) anode->Before->After = node ; // don't overwrite a Head or Tail node
        node->Before = anode->Before ;
        anode->Before = node ; // after the above statement ! obviously
        node->After = anode ;
    }
}

DLNode *
DLNode_Remove ( DLNode * node )
{
    if ( node )
    {
        D1 ( if ( node->N_Type.T_CType & ( T_HEAD | T_TAIL ) ) Error ( "\nCan't remove the Head or Tail node!\n", QUIT ) ) ;
        if ( node->Before ) node->Before->After = node->After ;
        if ( node->After ) node->After->Before = node->Before ;
    }
    return node ;
}

void
DLNode_ReplaceNodeWithANode ( DLNode * node, DLNode * anode )
{
    if ( node && anode )
    {
        DLNode * after = node->N_After ;
        D1 ( if ( after->N_Type.T_CType & ( T_HEAD | T_TAIL ) ) Error ( "\nCan't remove the Head or Tail node!\n", QUIT ) ) ;
        DLNode_Remove ( node ) ;
        DLNode_InsertThisBeforeANode ( anode, after ) ;
    }
}

void
DLNode_Replace ( DLNode * replacedNode, DLNode * replacingNode )
{
    if ( replacedNode && replacingNode )
    {
        D1 ( if ( replacedNode->N_Type.T_CType & ( T_HEAD | T_TAIL ) ) Error ( "\nCan't remove the Head or Tail replacedNode!\n", QUIT ) ) ;
        if ( replacedNode->Before ) replacedNode->Before->After = replacingNode ;
        if ( replacedNode->After ) replacedNode->After->Before = replacingNode ;
    }
    //return replacingNode ;
}

void
_DLList_Init ( DLList * list )
{
    if ( list )
    {
        list->Head->After = ( DLNode * ) list->Tail ;
        list->Head->Before = ( DLNode * ) 0 ;
        list->Tail->After = ( DLNode* ) 0 ;
        list->Tail->Before = ( DLNode * ) list->Head ;
        list->Head->N_Type.T_CType = T_HEAD ;
        list->Tail->N_Type.T_CType = T_TAIL ;
        list->S_CurrentNode = 0 ;
    }
}

void
DLList_Init ( DLList * list, DLNode * head, DLNode *tail )
{
    list->Head = head ;
    list->Tail = tail ;
    _DLList_Init ( list ) ;
}

DLList *
_DLList_New ( uint64 allocType )
{
    DLList * list = ( DLList* ) Mem_Allocate ( sizeof ( DLList ), allocType ) ;
    list->Head = _DLNode_New ( allocType ) ;
    list->Tail = _DLNode_New ( allocType ) ;
    _DLList_Init ( list ) ;
    return list ;
}

DLList *
DLList_New ( )
{
    return _DLList_New ( DICTIONARY ) ;
}

void
DLList_ReInit ( DLList * list )
{
    DLNode * node, * nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        DLNode_Remove ( node ) ;
    }
    _DLList_Init ( list ) ;
}

int32
DLList_Length ( DLList * list )
{
    int32 length ;
    DLNode * node, * nextNode ;
    for ( length = 0, node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        length ++ ;
    }
    return length ;
}

void
_DLList_AddNodeToHead ( DLList *list, DLNode * node )
{
    if ( node )
    {
        DLNode_InsertThisAfterANode ( node, list->Head ) ; // after Head toward Tail
    }
}

void
DLList_AddNodeToHead ( DLList *list, DLNode * node )
{
    if ( node )
    {
        // prevent trying to add nodes already on the list; this will move it to the beginning
        DLNode_Remove ( node ) ; // if the node is already on a list it will be first removed
        _DLList_AddNodeToHead ( list, node ) ;
        list->S_CurrentNode = 0 ;
    }
}

void
DLList_AddNodeToTail ( DLList *list, DLNode * node )
{
    if ( node )
    {
        // prevent trying to add nodes already on the list; this will move it to the beginning
        DLNode_Remove ( node ) ; // prevent trying to add nodes already on the list
        DLNode_InsertThisBeforeANode ( node, list->Tail ) ; // before Tail toward Head
        list->S_CurrentNode = node ;
    }
}

DLNode *
DLList_Head ( DLList * list )
{
    if ( ! list ) return 0 ;
    return list->Head ;
}

DLNode *
DLList_Tail ( DLList * list )
{
    if ( ! list ) return 0 ;
    return list->Tail ;
}

DLNode *
_DLList_First ( DLList * list )
{
    return DLNode_Next ( list->Head ) ;
}

DLNode *
DLList_First ( DLList * list )
{
    if ( ! list ) return 0 ;
    return DLNode_Next ( list->Head ) ;
}

DLNode *
_DLList_Last ( DLList * list )
{
    return DLNode_Previous ( list->Tail ) ;
}

DLNode *
DLList_Last ( DLList * list )
{
    if ( ! list ) return 0 ;
    return DLNode_Previous ( list->Tail ) ;
}

DLNode *
DLList_NodePrevious ( DLList * list, DLNode * node )
{
    if ( node )
    {
        node = _DLNode_Previous ( node ) ;
    }
    if ( ! node ) node = DLList_Head ( list ) ;
    return node ;
}

DLNode *
DLList_NodeNext ( DLList * list, DLNode * node )
{
    if ( node )
    {
        node = _DLNode_Next ( node ) ;
    }
    if ( ! node ) node = DLList_Tail ( list ) ;
    return node ;
}

DLNode *
_DLList_Before ( DLList * list )
{
    return DLNode_Previous ( list->S_CurrentNode ) ;
}

DLNode *
DLList_Before ( DLList * list )
{
    list->S_CurrentNode = _DLList_Before ( list ) ;
    if ( list->S_CurrentNode == 0 )
    {
        list->S_CurrentNode = DLList_Head ( list ) ;
        //list->CurrentNode = DLList_First ( list ) ;
        return 0 ;
    }
    return list->S_CurrentNode ;
}
// toward the TailNode

DLNode *
_DLList_After ( DLList * list )
{
    return DLNode_Next ( list->S_CurrentNode ) ;
}
// toward the TailNode

DLNode *
DLList_After ( DLList * list )
{
    list->S_CurrentNode = _DLList_After ( list ) ;
    if ( list->S_CurrentNode == 0 )
    {
        list->S_CurrentNode = DLList_Tail ( list ) ;
        return 0 ;
    }
    return ( DLNode* ) list->S_CurrentNode ;
}

DLNode *
_DLList_AddNamedValue ( DLList * list, byte * name, int32 value, int32 allocType )
{
    Symbol * sym = _Symbol_New ( name, allocType ) ;
    sym->W_Value = value ;
    _DLList_AddNodeToHead ( list, ( DLNode* ) sym ) ;
}

DLNode *
_DLList_AddValue ( DLList * list, int32 value, int32 allocType )
{
    Symbol * sym = Symbol_NewValue ( value, allocType ) ;
    _DLList_AddNodeToHead ( list, ( DLNode* ) sym ) ;
}

void
DLList_Map ( DLList * list, MapFunction0 mf )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        // get nextNode before map function (mf) in case mf changes list by a Remove of current node
        // problem could arise if mf removes Next node
        nextNode = DLNode_Next ( node ) ;
        mf ( node ) ;
    }
}

void
DLList_Map1 ( DLList * list, MapFunction1 mf, int32 one )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        mf ( node, one ) ;
    }
}

void
DLList_Map2 ( DLList * list, MapFunction2 mf, int32 one, int32 two )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        mf ( node, one, two ) ;
    }
}

void
DLList_Map2_64 ( DLList * list, MapFunction2_64 mf, uint64 one, int32 two )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        if ( two ) // EXACT
        {
            if ( node->N_Type.T_AType != one ) continue ;
        }
        else if ( ! ( node->N_Type.T_AType & one ) ) continue ;
        mf ( node, one, two ) ;
    }
}

void
DLList_Map3 ( DLList * list, MapFunction3 mf, int32 one, int32 two, int32 three )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        mf ( node, one, two, three ) ;
    }
}

void
DLList_Map_OnePlusStatus ( DLList * list, MapFunction2 mf, int32 one, int32 * status )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node && ( *status != DONE ) ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
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
        if ( ! _Q_->OVT_Context->NlsWord )
        {
            nextNs = ( Word * ) DLList_First ( _Q_->OVT_CfrTil->Namespaces->W_List ) ;
        }
        else
        {
            nextNs = 0 ;
            do
            {
                if ( nextNs ) nextNs->W_SearchNumber = 0 ; // reset 
                nextNs = ( Word* ) DLNode_Next ( ( Node* ) _Q_->OVT_Context->NlsWord ) ;
            }
            while ( nextNs && nextNs->W_SearchNumber ) ;
        }
        _Q_->OVT_Context->NlsWord = nextNs ;
        if ( nextNs ) nextWord = ( Word* ) DLList_First ( nextNs->Lo_List ) ; 
        else nextWord = 0 ; // will restart the cycle thru the _Q_->OVT_CfrTil->Namespaces word lists
    }
    else
    {
        nextWord = ( Word* ) DLNode_Next ( ( Node* ) thisWord ) ;
        if ( ! nextWord )
        {
            if ( thisWord->S_ContainingNamespace )
            {
                if ( thisWord->S_ContainingNamespace == _Q_->OVT_Context->NlsWord ) thisWord->S_ContainingNamespace->W_SearchNumber ++ ;
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
_Tree_Map_State_2 ( DLList * list, uint64 state, MapSymbolFunction2 mf, int32 one, int32 two )
{
    DLNode * node, *nextNode ;
    Namespace * ns ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        ns = ( Namespace * ) node ;
        if ( Is_NamespaceType ( ns ) )
        {
            if ( ns->State & state ) mf ( ( Symbol* ) ns, one, two ) ;
            _Tree_Map_State_2 ( ns->W_List, state, mf, one, two ) ;
        }
    }
}

Word *
_TreeList_DescendMap_State_Flag_OneArg ( Word * word, uint64 state, int32 oneNamespaceFlag, MapFunction_Cell_1 mf, int32 one )
{
    Word * word2, *nextWord ;
    for ( ; word ; word = nextWord )
    {
        nextWord = ( Word* ) DLNode_Next ( ( Node* ) word ) ;
        if ( mf ( ( Symbol* ) word, one ) ) return word ;
        else if ( Is_NamespaceType ( word ) )
        {
            if ( ! oneNamespaceFlag )
            {
                if ( word->State & state )
                {
                    if ( ( word2 = _TreeList_DescendMap_State_Flag_OneArg ( ( Word* ) DLList_First ( word->W_List ), state, oneNamespaceFlag, mf, one ) ) ) return word2 ;
                }
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
        nextWord = ( Word* ) DLNode_Next ( ( Node* ) word ) ;
        if ( mf ( ( Symbol* ) word ) ) return nextWord ;
        if ( Is_NamespaceType ( word ) )
        {
            if ( ( word = _TreeMap_FromAWord ( ( Word* ) DLList_First ( word->W_List ), mf ) ) ) return word ;
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
        nextWord = ( Word* ) DLNode_Next ( ( Node* ) nowWord ) ;
        if ( ! nextWord )
        {
            nextWord = Q_->OVT_Context->NlsWord_Q_->OVT_Context->NlsWord ? ( Word* ) DLNode_Next ( ( Node* ) _Q_->OVT_Context->NlsWord ) : ( Word* ) DLList_First ( _Q_->OVT_CfrTil->Namespaces->W_List ) ;
        }
        if ( mf ( ( Symbol* ) nextWord ) ) return nextWord ;
        else if ( Is_NamespaceType ( nextWord ) && ( nextWord->W_SearchNumber != tci->SearchNumber ) )
        {
            if ( ( word2 = _TC_TreeList_DescendMap ( tci, ( Word* ) DLList_First ( nextWord->W_List ), mf ) ) ) return word2 ;
            if ( nextWord->S_ContainingNamespace && nextWord->S_ContainingNamespace->S_ContainingNamespace )
            {
                nextWord->S_ContainingNamespace->W_SearchNumber = tci->SearchNumber ; // end of list; mark it as searched with SearchNumber
            }
        }
    }
    return 0 ;
}
#endif

