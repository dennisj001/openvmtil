#include "../../include/cfrtil.h"

void
_dlnode_Init ( dlnode * node )
{
    node->afterWord = 0 ;
    node->beforeWord = 0 ;
}

dlnode *
_dlnode_New ( uint64 allocType )
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
        //D1 ( if ( (int64) (((Node)node)->n_Property.T_CProperty) & ( T_HEAD | T_TAIL ) ) Error ( "\nCan't remove the Head or Tail node!\n", QUIT ) ) ;
        if ( node->beforeWord ) node->beforeWord->afterWord = node->afterWord ;
        if ( node->afterWord ) node->afterWord->beforeWord = node->beforeWord ;
        node->afterWord = 0 ;
        node->beforeWord = 0 ;
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
        list->n_CurrentNode = 0 ;
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
_dllist_New ( uint64 allocType )
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

int64
dllist_Length ( dllist * list )
{
    int64 length ;
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
    if ( list && node )
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
        list->n_CurrentNode = 0 ;
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
        list->n_CurrentNode = node ;
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

// toward the HeadNode

dlnode *
_dllist_Before ( dllist * list )
{
    return dlnode_Previous ( list->n_CurrentNode ) ;
}

dlnode *
dllist_SetCurrentNode_Before ( dllist * list )
{
    list->n_CurrentNode = _dllist_Before ( list ) ;
    if ( list->n_CurrentNode == 0 )
    {
        list->n_CurrentNode = dllist_Head ( list ) ;
        return 0 ;
    }
    return list->n_CurrentNode ;
}
// toward the TailNode

dlnode *
_dllist_After ( dllist * list )
{
    return dlnode_Next ( list->n_CurrentNode ) ;
}
// toward the TailNode

dlnode *
dllist_SetCurrentNode_After ( dllist * list )
{
    list->n_CurrentNode = _dllist_After ( list ) ;
    if ( list->n_CurrentNode == 0 )
    {
        list->n_CurrentNode = dllist_Tail ( list ) ;
        return 0 ;
    }
    return ( dlnode* ) list->n_CurrentNode ;
}

dlnode *
_dllist_AddNamedValue ( dllist * list, byte * name, int64 value, uint64 allocType )
{
    Symbol * sym = _Symbol_New ( name, allocType ) ;
    sym->W_Value = value ;
    _dllist_AddNodeToHead ( list, ( dlnode* ) sym ) ;
}

dobject *
_dllist_PushNode ( dllist* list, dlnode * node )
{
    _dllist_AddNodeToHead ( list, node ) ;
}

dobject *
_dllist_Push_M_Slot_Node ( dllist* list, int64 dobjType, int64 allocType, int64 m_slots, ... )
{
    int64 i ;
    va_list args ;
    va_start ( args, m_slots ) ;
    dobject * dobj = _dobject_Allocate ( dobjType, m_slots, allocType ) ;
    for ( i = 0 ; i < m_slots ; i ++ ) dobj->do_iData[i] = va_arg ( args, int64 ) ;
    va_end ( args ) ;
    //_dllist_AddNodeToHead ( list, ( dlnode* ) dobj ) ;
    _dllist_PushNode ( list, ( dlnode* ) dobj ) ;

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
_dllist_DropN ( dllist * list, int64 n )
{
    dlnode * node ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node && ( -- n >= 0 ) ; node = dlnode_Next ( node ) )
    {
        dlnode_Remove ( node ) ;
    }
}

int64
_dllist_Get_N_Node_M_Slot ( dllist * list, int64 n, int64 m )
{
    dlnode * node ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node && ( -- n >= 0 ) ; node = dlnode_Next ( node ) ) ;
    return node ? dobject_Get_M_Slot ( node, m ) : 0 ; // LIST_EMPTY
}

void
_dllist_Set_N_Node_M_Slot ( dllist * list, int64 n, int64 m, int64 value )
{
    dlnode * node ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node && ( -- n >= 0 ) ; node = dlnode_Next ( node ) ) ;
    if ( node ) dobject_Set_M_Slot ( node, m, value ) ;
}

int64
_dllist_Depth ( dllist * list )
{
    int64 n ;
    dlnode * node ;
    for ( n = 0, node = dllist_First ( ( dllist* ) list ) ; node ; n ++, node = dlnode_Next ( node ) ) ;
    return n ;
}

int64
_dllist_GetTopValue ( dllist * list )
{
    _dllist_Get_N_Node_M_Slot ( list, 0, 0 ) ;
}

int64
_dllist_SetTopValue ( dllist * list, int64 value )
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
dllist_Map1 ( dllist * list, MapFunction1 mf, int64 one )
{
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        mf ( node, one ) ;
    }
}

void
dllist_Map2 ( dllist * list, MapFunction2 mf, int64 one, int64 two )
{
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        mf ( node, one, two ) ;
    }
}

int64
dllist_Map3 ( dllist * list, MapFunction3 mf, int64 one, int64 two, int64 three )
{
    int64 rtrn = 0 ;
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        if ( rtrn = mf ( node, one, two, three ) ) break ;
    }
    return rtrn ;
}

void
dllist_Map_OnePlusStatus ( dllist * list, MapFunction2 mf, int64 one, int64 * status )
{
    dlnode * node, *nextNode ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node && ( *status != DONE ) ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        mf ( node, one, ( int64 ) status ) ;
    }
}

void
Tree_Map_State_2Args ( dllist * list, uint64 state, MapSymbolFunction2 mf, int64 one, int64 two )
{
    dlnode * node, *nextNode ;
    Word * word ;
    d0 ( _CfrTil_->FindWordCount = 0 ) ;
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        word = ( Word * ) node ;
        d0 ( _CfrTil_->FindWordCount ++ ) ;
        if ( Is_NamespaceType ( word ) )
        {
            if ( word->State & state ) mf ( ( Symbol* ) word, one, two ) ;
            Tree_Map_State_2Args ( word->W_List, state, mf, one, two ) ;
        }
    }
    d0 ( CfrTil_WordAccounting ( ( byte* ) "Tree_Map_State_2" ) ) ;
}

Word *
Tree_Map_OneNamespace ( Word * word, MapFunction_1 mf, int64 one )
{
    Word *nextWord ;
    for ( ; word ; word = nextWord )
    {
        nextWord = ( Word* ) dlnode_Next ( ( node* ) word ) ;
        d0 ( _CfrTil_->FindWordCount ++ ) ;
        if ( mf ( ( Symbol* ) word, one ) ) return word ;
    }
    return 0 ;
}

#if 1

Word *
Tree_Map_State_Flag_OneArg_AnyNamespaceWithState ( uint64 state, MapFunction_1 mf, int64 one )
{
    d1 ( if (_CfrTil_->Namespaces) )
    {
    Word * word, * word2, *nextWord ;
    _CfrTil_->FindWordCount = 1 ;
    if ( mf ( ( Symbol* ) _CfrTil_->Namespaces, one ) ) return _CfrTil_->Namespaces ;
    for ( word = ( Word * ) dllist_First ( ( dllist* ) _CfrTil_->Namespaces->W_List ) ; word ; word = nextWord )
    {
        nextWord = ( Word* ) dlnode_Next ( ( node* ) word ) ;
        _CfrTil_->FindWordCount ++ ;
        if ( mf ( ( Symbol* ) word, one ) ) return word ;
        else if ( Is_NamespaceType ( word ) )
        {
            if ( ( word->State & state ) )
            {
                if ( ( word2 = Tree_Map_OneNamespace ( ( Word* ) dllist_First ( ( dllist* ) word->W_List ), mf, one ) ) ) return word2 ;
            }
        }
    }
    }
    return 0 ;
}
#else

Word *
//TC_Tree_Map_2 ( uint64 state, dllist * list, MapFunction mf, Word * iword )
Tree_Map_State_Flag_OneArg_AnyNamespaceWithState ( uint64 state, MapFunction_1 mf, int64 one )
{
    dllist * list = _CfrTil_->Namespaces->W_List ;
    Word * word, *nextWord, *oword, *oNextWord, *rword ;
start:
    for ( word = ( Word * ) dllist_First ( list ) ; word ; word = nextWord )
    {
        nextWord = ( Word* ) dlnode_Next ( ( node* ) word ) ;
        if ( mf ( ( Symbol* ) word, one ) ) return word ;
        else if ( Is_NamespaceType ( word ) )
        {
            if ( ( word->State & state ) )
            {
                oword = ( Word* ) dllist_First ( ( dllist* ) word->W_List ) ;
                for ( ; oword ; oword = oNextWord )
                {
                    oNextWord = ( Word* ) dlnode_Next ( ( node* ) oword ) ;
                    if ( mf ( ( Symbol* ) oword, one ) ) return oword ;
                }
            }
        }
    }
    return word ;
}
#endif

// we have to remember that namespace nodes are being moved around on the Namespaces list by namespace functions
#if 0

Word *
TC_Tree_Map_1 ( TabCompletionInfo * tci, dllist * list, MapFunction mf, Word * one, int64 * startFlag )
{
    dlnode * node, *nextNode ;
    Word * word, *word2 ;
    if ( ! one )
    {
        ( *startFlag ) = 1 ;
        tci->SearchNumber = rand ( ) ; // SearchNumber : keeps track of which words we have already found on a search so we don't return them again
        //tci->FoundMarker = rand () ;
    }
    for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode )
    {
        nextNode = dlnode_Next ( node ) ;
        word = ( Word * ) node ;
        if ( ( word == one ) && ( word->W_SearchNumber == tci->SearchNumber ) )
        {
            ( *startFlag ) = 1 ;
        }
        if ( ( *startFlag ) && ( word->W_SearchNumber != tci->SearchNumber ) && mf ( ( Symbol* ) word ) )
        {
            d0 ( _Printf ( "\nTC_Tree_Map_1 :: word->Name = %s : word = 0x%08x\n", word->Name, word ) ; )
            word->W_SearchNumber = tci->SearchNumber ;
            return ( Word * ) word ;
        }
            //else if ( Is_NamespaceType ( word ) && (list == _CfrTil_->Namespaces->W_List ) ) // all namespaces are on this list; cf namespace
        else if ( Is_NamespaceType ( word ) && ( word->W_SearchNumber != tci->SearchNumber ) && ( list == _CfrTil_->Namespaces->W_List ) ) // all namespaces are on this list; cf namespace
            //else if ( Is_NamespaceType ( word ) && ( word->W_SearchNumber != tci->SearchNumber ) ) //&& (list == _CfrTil_->Namespaces->W_List ) ) // all namespaces are on this list; cf namespace
        {
            if ( word2 = TC_Tree_Map_1 ( tci, word->W_List, mf, one, startFlag ) ) return word2 ;
            else word->W_SearchNumber = tci->SearchNumber ;
        }
    }
    return 0 ;
}

#else

Word *
TC_Tree_Map_2 ( dllist * list, MapFunction mf, Word * iword )
{
    Word * word, *nextWord, *oword, *oNextWord, *rword = 0 ;
first:
    if ( iword )
    {
        if ( mf ( ( Symbol* ) iword ) )
        {
            rword = ( Word* ) dlnode_Next ( ( node* ) iword ) ;
            goto doReturn ;
        }
        if ( Is_NamespaceType ( iword ) )
        {
            word = iword ;
            goto checkWord ;
        }
        else
        {
            word = iword->S_ContainingNamespace ;
            nextWord = ( Word* ) dlnode_Next ( ( node* ) word ) ;
            oword = iword ;
            goto checkOWord ;
        }
    }
    for ( word = ( Word * ) dllist_First ( list ) ; word ; word = nextWord )
    {
checkWord:
        nextWord = ( Word* ) dlnode_Next ( ( node* ) word ) ;
        if ( mf ( ( Symbol* ) word ) )
        {
            rword = nextWord ;
            goto doReturn ;
        }
        else if ( Is_NamespaceType ( word ) )
        {
            oword = ( Word* ) dllist_First ( ( dllist* ) word->W_List ) ;
            for ( ; oword ; oword = oNextWord )
            {
checkOWord:
                oNextWord = ( Word* ) dlnode_Next ( ( node* ) oword ) ;
                if ( mf ( ( Symbol* ) oword ) )
                {
                    rword = oNextWord ? oNextWord : nextWord ;
                    goto doReturn ;
                }
            }
        }
    }
    rword = word ;
doReturn:
    if ( iword && ( rword == iword ) ) rword = 0 ;
#if 1 // prevents from having to hit <tab> more than once in some cases
    else
    {
        if ( ! rword )
        {
            list = _CfrTil_->Namespaces->W_List ;
            rword = ( Word * ) dllist_First ( list ) ;
            iword = rword ;
            goto first ;
        }
    }
#endif
    return rword ;
}
#endif

