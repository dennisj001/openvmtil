#include "../includes/cfrtil.h"

/*
 * cfrTil namespaces basic understandings :
 * 0. to allow for ordered search thru lists in use ...
 * 1. Namespaces are also linked on the _Context->System0->Namespaces list with status of USING or NOT_USING
 *      and are moved the front or back of that list if status is set to USING with the 'Symbol node'
 *      This list is ordered so we can set an order to our search thru the namespaces in use. As usual
 *      the first word found within the ordered USING list will be used.
 */


Namespace *
Namespace_NextNamespace ( Word * w )
{
    Word * next = w ;
    do
    {
        next = ( Word* ) DLNode_Next ( ( DLNode* ) next ) ;
    }
    while ( next && ( ! Is_NamespaceType ( next ) ) ) ;
    return next ;
}

void
_Namespace_DoSetState ( Namespace * ns, uint64 state )
{
    SetState ( ns, state, true ) ; //ns->State = state ;
}

Namespace *
_Namespace_IsUsing ( Namespace * ns )
{
    if ( GetState ( ns, USING ) ) return ns ;
    return 0 ;
}

Namespace *
Namespace_IsUsing ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    return _Namespace_IsUsing ( ns ) ;
}

void
_Namespace_AddToNamespacesHead_SetAsInNamespace ( Namespace * ns )
{
    DLList_AddNodeToHead ( _Q_->OVT_CfrTil->Namespaces->W_List, ( DLNode* ) ns ) ;
    _Q_->OVT_CfrTil->InNamespace = ns ;
}

void
_Namespace_AddToNamespacesTail ( Namespace * ns )
{
    DLList_AddNodeToTail ( _Q_->OVT_CfrTil->Namespaces->W_List, ( DLNode* ) ns ) ;
    if ( ns == _Q_->OVT_CfrTil->InNamespace ) _Q_->OVT_CfrTil->InNamespace = ( Namespace* ) DLList_First ( _Q_->OVT_CfrTil->Namespaces->W_List ) ;
}

void
_Namespace_SetState ( Namespace * ns, uint64 state )
{
    if ( ns )
    {
        ns->State = state ;
        if ( state == USING ) _Namespace_AddToNamespacesHead_SetAsInNamespace ( ns ) ; // make it first on the list
        else _Namespace_AddToNamespacesTail ( ns ) ;
    }
}

Word *
_Namespace_FirstOnUsingList ( )
{
    Word * ns, *nextNs ;
    for ( ns = ( Namespace* ) DLList_First ( _Q_->OVT_CfrTil->Namespaces->W_List ) ; ns ; ns = nextNs )
    {
        nextNs = ( Word* ) DLNode_Next ( ( Node* ) ns ) ;
        if ( Is_NamespaceType ( ns ) && ( ns->State & USING ) ) return ns ;
    }
    return 0 ;
}

void
_Namespace_AddToUsingList ( Namespace * ns )
{
    Namespace * savedNs = ns ;
    while ( ( ns = ns->ContainingNamespace ) )
    {
        if ( ns == _Q_->OVT_CfrTil->Namespaces ) break ;
        _Namespace_SetState ( ns, USING ) ;
    }
    _Namespace_SetState ( savedNs, USING ) ; // do it last so it is at the Head of the list
}

void
_Namespace_ActivateAsPrimary ( Namespace * ns )
{
    if ( ns )
    {
        Finder_SetQualifyingNamespace ( _Q_->OVT_Context->Finder0, ns ) ;
        _Namespace_AddToUsingList ( ns ) ;
        _Q_->OVT_CfrTil->InNamespace = ns ;
        _Q_->OVT_Context->Interpreter0->BaseObject = 0 ;
    }
}

void
Namespace_ActivateAsPrimary ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    _Namespace_ActivateAsPrimary ( ns ) ;
}

void
Namespace_MoveToTail ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    _Namespace_AddToNamespacesTail ( ns ) ;
}

void
Namespace_SetAsNotUsing ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    _Namespace_DoSetState ( ns, NOT_USING ) ;
}

void
_Namespace_SetAsNotUsing_MoveToTail ( Namespace * ns )
{
    _Namespace_DoSetState ( ns, NOT_USING ) ;
    _Namespace_AddToNamespacesTail ( ns ) ;
}

void
Namespace_SetAsNotUsing_MoveToTail ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    _Namespace_SetAsNotUsing_MoveToTail ( ns ) ;
}

int32
_Namespace_VariableValueGet ( Namespace * ns, byte * name )
{
    Word * word = _CfrTil_VariableGet ( ns, name ) ;
    if ( word ) return ( int32 ) word->W_Value ; // value of variable
    else return 0 ;
}

void
_Namespace_VariableValueSet ( Namespace * ns, byte * name, int32 value )
{
    Word * word = _CfrTil_VariableGet ( ns, name ) ;
    if ( word ) word->W_Value = value ; // value of variable
}

Namespace *
_CfrTil_Namespace_InNamespaceSet ( Namespace * ns )
{
    if ( ns )
    {
        _Namespace_DoSetState ( ns, USING ) ;
        _Namespace_AddToNamespacesHead_SetAsInNamespace ( ns ) ;
    }
    return ns ;
}

Namespace *
CfrTil_Namespace_InNamespaceSet ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    _CfrTil_Namespace_InNamespaceSet ( ns ) ;
    return ns ;
}

Namespace *
_CfrTil_Namespace_InNamespaceGet ( )
{
    if ( _Q_->OVT_CfrTil->Namespaces && ( ! _Q_->OVT_CfrTil->InNamespace ) )
    {
        _Q_->OVT_CfrTil->InNamespace = ( Namespace* ) _Tree_Map_FromANode ( ( DLNode* ) _Q_->OVT_CfrTil->Namespaces, ( cMapFunction_1 ) _Namespace_IsUsing ) ;
    }
    return _Q_->OVT_CfrTil->InNamespace ;
}

Namespace *
_CfrTil_InNamespace ( )
{
    Namespace * ins ;
    if ( ( ins = Finder_GetQualifyingNamespace ( _Q_->OVT_Context->Finder0 ) ) ) return ins ;
    else return _CfrTil_Namespace_InNamespaceGet ( ) ;
}

Boolean
_CfrTil_AreWeInThisNamespace ( byte * name )
{
    Namespace * ins = _CfrTil_InNamespace ( ) ;
    if ( ins && String_Equal ( ( char* ) ins->Name, name ) ) return true ;
    return false ;
}

Boolean
_CfrTil_IsContainingNamespace ( byte * wordName, byte * namespaceName )
{
    Word * word = Word_FindUsing ( ( byte* ) wordName ) ;
    if ( word && String_Equal ( ( char* ) word->ContainingNamespace->Name, namespaceName ) ) return true ;
    else return false ;
}

void
Namespace_DoNamespace ( byte * name )
{
    _Namespace_DoNamespace ( Namespace_Find ( name ), 1 ) ;
}

void
Symbol_NamespacePrettyPrint ( Symbol * symbol, int32 indentFlag, int32 indentLevel )
{
    Namespace * ns = ( Namespace* ) symbol ;
    Namespace_PrettyPrint ( ns, indentFlag, indentLevel ) ;
}

void
_Namespace_DoAddSymbol ( Namespace * ns, Symbol * symbol )
{
    if ( ! ns->W_List ) ns->W_List = DLList_New ( ) ;
    DLList_AddNodeToHead ( ns->W_List, ( DLNode* ) symbol ) ;
    symbol->S_ContainingNamespace = ns ;
}

void
_Namespace_DoAddWord ( Namespace * ns, Word * word )
{
    _Namespace_DoAddSymbol ( ns, ( Symbol* ) word ) ;
}

// a namespaces internal finder, a wrapper for Symbol_Find - prefer Symbol_Find directly

Namespace *
_Namespace_Find ( byte * name, Namespace * superNamespace, int32 exceptionFlag )
{
    Word * word = 0 ;
    if ( superNamespace ) word = Word_FindInOneNamespace ( superNamespace, name ) ;
    if ( ! word ) word = _Word_FindAny ( name ) ;
    if ( word && ( word->CType & ( NAMESPACE | CLASS | DOBJECT ) ) ) return ( Namespace* ) word ;
    else if ( exceptionFlag )
    {
        Printf ( ( byte* ) "\nUnable to find Namespace : %s\n", name ) ;
        CfrTil_Exception ( NAMESPACE_ERROR, 1 ) ;
        return 0 ;
    }
    return 0 ;
}

Namespace *
Namespace_Find ( byte * name )
{
    return _Namespace_Find ( name, 0, 0 ) ;
}

void
_Namespace_UsingLast ( byte * name )
{
    _Namespace_AddToNamespacesTail ( Namespace_Find ( name ) ) ;
}

void
_Namespace_RemoveFromUsingList ( Namespace * ns )
{
    //ns->State = NOT_USING ;
    _Namespace_SetAsNotUsing_MoveToTail ( ns ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) _RemoveSubNamespacesFromUsingList, ( int32 ) ns, 0 ) ;
}

void
Namespace_RemoveFromUsingList ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    if ( ns ) _Namespace_RemoveFromUsingList ( ns ) ;
}
// this is simple, for more complete use _Namespace_RemoveFromSearchList
// removes all namespaces dependant on 'ns', the whole dependancy tree from the 'ns' root

void
_RemoveSubNamespacesFromUsingList ( Symbol * symbol, Namespace * ns )
{
    Namespace * ns1 = ( Namespace* ) symbol ;
    // if ns contains ns1 => ns1 is dependent on ns ; we are removing ns => we have to remove ns1
    if ( ns1->ContainingNamespace == ns )
    {
        _Namespace_RemoveFromUsingList ( ns1 ) ;
    }
}

void
Namespace_MoveToFirstOnUsingList ( byte * name )
{
    _Namespace_AddToUsingList ( Namespace_Find ( name ) ) ; // so it will be first on the list where Find will find it first
}

void
Namespace_RemoveFromUsingList_WithCheck ( byte * name )
{
    if ( strcmp ( "Root", ( char* ) name ) != 0 )
    {
        //Namespace * ns = Namespace_Find ( name ) ;
        //if ( ns ) _Namespace_RemoveFromUsingList ( ns ) ;
        Namespace_RemoveFromUsingList ( name ) ;
    }
    else Throw ( ( byte* ) "Error : can't remove Root namespace", QUIT ) ;
}

void
_Namespace_Clear ( Namespace * ns )
{
    if ( ns )
    {
        //DLList_Map ( ns->W_List, (MapFunction0) DLNode_Remove ) ; // not necessary
        _DLList_Init ( ns->W_List ) ;
    }
}

void
Namespace_Clear ( byte * name )
{
    _Namespace_Clear ( _Namespace_Find ( name, 0, 0 ) ) ;
}

void
Namespace_AddWord ( Namespace * ns, Word * word )
{
    if ( ns )
    {
#if 0  
        if ( ns->S_Type & SEALED )
        {
            Printf ( ( byte* ) "\nNamespace : %s is sealed :", ns->S_Name ) ;
            CfrTil_Exception ( SEALED_NAMESPACE_ERROR, 0 ) ;
        }
        else
#endif   
            _Namespace_DoAddWord ( ns, word ) ;
    }
}

Namespace *
Namespace_FindOrNew_SetUsing ( byte * name, Namespace * containingNs, int32 setUsingFlag )
{
    //if ( ! isprint ( name [0] ) ) Error_Abort ( "\nNamespace must begin with printable character!" ) ;
    if ( ! containingNs ) containingNs = _Q_->OVT_CfrTil->Namespaces ;
    Namespace * ns = _Namespace_Find ( name, containingNs, 0 ) ;
    if ( ! ns )
    {
        ns = _DataObject_New ( NAMESPACE, name, 0, 0, 0, ( int32 ) containingNs ) ;
    }
    if ( setUsingFlag ) _Namespace_SetState ( ns, USING ) ;
    return ns ;
}

Namespace *
Namespace_FindOrNew_Local ( )
{
    byte buffer [ 32 ] ; //truncate 
    sprintf ( ( char* ) buffer, "locals_%d", Stack_Depth ( _Q_->OVT_Context->Compiler0->LocalNamespaces ) ) ;
    Namespace * ns = Namespace_FindOrNew_SetUsing ( buffer, _Q_->OVT_CfrTil->Namespaces, 1 ) ;
    BlockInfo * bi = ( BlockInfo * ) _Stack_Top ( _Q_->OVT_Context->Compiler0->BlockStack ) ;
    if ( ! bi->LocalsNamespace )
    {
        _Namespace_ActivateAsPrimary ( ns ) ;
        bi->LocalsNamespace = ns ;
        Stack_Push ( _Q_->OVT_Context->Compiler0->LocalNamespaces, ( int32 ) ns ) ;
    }
    return ns ;
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

int32
_Tree_Map_FromANode ( DLNode *node, cMapFunction_1 mf )
{
    DLNode *nextNode, *returnNode ;
    for ( ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        if ( ( returnNode = ( DLNode* ) mf ( ( Symbol* ) node ) ) ) return ( int32 ) returnNode ;
        if ( Is_NamespaceType ( node ) )
        {
            if ( _Tree_Map ( ( ( Namespace* ) node )->W_List, mf ) == true ) return ( int32 ) node ;
        }
    }
    return 0 ;
}

int32
_Tree_Map ( DLList * list, cMapFunction_1 mf )
{
    return _Tree_Map_FromANode ( DLList_First ( list ), mf ) ;
}

void
_Namespace_MapAny_2Args ( MapSymbolFunction2 msf2, int32 one, int32 two )
{
    _Tree_Map_State_2 ( _Q_->OVT_CfrTil->Namespaces->W_List, ANY, msf2, one, two ) ;
}

void
_Namespace_MapUsing_2Args ( MapSymbolFunction2 msf2, int32 one, int32 two )
{
    _Tree_Map_State_2 ( _Q_->OVT_CfrTil->Namespaces->W_List, USING, msf2, one, two ) ;
}
