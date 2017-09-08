#include "../../include/cfrtil.h"

/*
 * cfrTil namespaces basic understandings :
 * 0. to allow for ordered search thru lists in use ...
 * 1. Namespaces are also linked on the _Context->System0->Namespaces list with status of USING or NOT_USING
 *      and are moved the front or back of that list if status is set to USING with the 'Symbol node'
 *      This list is ordered so we can set an order to our search thru the namespaces in use. As usual
 *      the first word found within the ordered USING list will be used.
 */

void
_Namespace_SetAsInNamespace ( Namespace * ns )
{
    _CfrTil_->InNamespace = ns ;
}

void
_Namespace_ResetFromInNamespace ( Namespace * ns )
{
    if ( ns == _CfrTil_->InNamespace ) _CfrTil_->InNamespace = _Namespace_FirstOnUsingList ( ) ; //( Namespace* ) dllist_First ( (dllist*) _CfrTil_->Namespaces->W_List ) ;
}

void
_Namespace_DoAddSymbol ( Namespace * ns, Symbol * symbol )
{
    dllist_AddNodeToHead ( ns->W_List, ( dlnode* ) symbol ) ;
}

void
Namespace_DoAddSymbol ( Namespace * ns, Symbol * symbol )
{
    if ( ! ns->W_List ) ns->W_List = dllist_New ( ) ;
    _Namespace_DoAddSymbol ( ns, symbol ) ;
    symbol->S_ContainingNamespace = ns ;
}

void
_Namespace_DoAddWord ( Namespace * ns, Word * word, int64 addFlag )
{
    Namespace_DoAddSymbol ( ns, ( Symbol* ) word ) ;
    if ( addFlag ) _CfrTil_->WordsAdded ++ ;
}

void
Namespace_DoAddWord ( Namespace * ns, Word * word )
{
    if ( ns && word ) _Namespace_DoAddWord ( ns, word, 1 ) ;
}

void
_Namespace_AddToNamespacesHead ( Namespace * ns )
{
    _Namespace_DoAddSymbol ( _CfrTil_->Namespaces, ns ) ;
}

void
_Namespace_AddToNamespacesHead_SetAsInNamespace ( Namespace * ns )
{
    _Namespace_AddToNamespacesHead ( ns ) ;
    _Namespace_SetAsInNamespace ( ns ) ;
}

void
_Namespace_AddToNamespacesTail ( Namespace * ns )
{
    dllist_AddNodeToTail ( _CfrTil_->Namespaces->W_List, ( dlnode* ) ns ) ;
}

void
_Namespace_AddToNamespacesTail_ResetFromInNamespace ( Namespace * ns )
{
    _Namespace_AddToNamespacesTail ( ns ) ;
    _Namespace_ResetFromInNamespace ( ns ) ;
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
_Namespace_SetState ( Namespace * ns, uint64 state )
{
    if ( ns )
    {
        d0 ( if ( Is_DebugModeOn )
        {
            //CfrTil_Namespaces_PrettyPrintTree ( ) ;
            //CfrTil_Using ( ) ;
            _Printf ( ( byte* ) "\n\nNamespace : %s :: Before _Namespace_SetState : \n\t", ns->Name ) ;
                List_PrintNames ( _CfrTil_->Namespaces->W_List, 5 ) ;
        } ) ;
        ns->State = state ;
        if ( state & USING ) _Namespace_AddToNamespacesHead_SetAsInNamespace ( ns ) ; // make it first on the list
        else _Namespace_AddToNamespacesTail_ResetFromInNamespace ( ns ) ;
        d0 ( if ( Is_DebugModeOn )
        {
            //CfrTil_Namespaces_PrettyPrintTree ( ) ;
            //CfrTil_Using ( ) ;
            _Printf ( ( byte* ) "\n\nNamespace : %s :: After _Namespace_SetState : \n\t", ns->Name ) ;
                List_PrintNames ( _CfrTil_->Namespaces->W_List, 5 ) ;
        } ) ;
    }
}

Word *
_Namespace_FirstOnUsingList ( )
{
    Word * ns, *nextNs ;
    for ( ns = ( Namespace* ) dllist_First ( ( dllist* ) _CfrTil_->Namespaces->W_List ) ; ns ; ns = nextNs )
    {
        nextNs = ( Word* ) dlnode_Next ( ( node* ) ns ) ;
        if ( Is_NamespaceType ( ns ) && ( ns->State & USING ) ) return ns ;
    }
    return 0 ;
}

void
_Namespace_AddToUsingList ( Namespace * ns )
{
    int64 i ;
    Stack * stack = _Context_->Compiler0->NamespacesStack ;
    Stack_Init ( stack ) ;
    do
    {
        if ( ns == _CfrTil_->Namespaces ) break ;
        _Stack_Push ( stack, ( int64 ) ns ) ;
        ns = ns->ContainingNamespace ;
    }
    while ( ns ) ;
    for ( i = Stack_Depth ( stack ) ; i > 0 ; i -- )
    {
        ns = ( Word* ) _Stack_Pop ( stack ) ;
        if ( ns->W_OriginalWord ) ns = ns->W_OriginalWord ; //_Namespace_Find ( ns->Name, 0, 0 ) ; // this is needed because of Compiler_PushCheckAndCopyDuplicates
        _Namespace_SetState ( ns, USING ) ;
    }
}

void
_Namespace_ActivateAsPrimary ( Namespace * ns )
{
    if ( ns )
    {
        Finder_SetQualifyingNamespace ( _Context_->Finder0, ns ) ;
        _Namespace_AddToUsingList ( ns ) ;
        _CfrTil_->InNamespace = ns ;
        _Context_->Interpreter0->BaseObject = 0 ;
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
    _Namespace_AddToNamespacesTail_ResetFromInNamespace ( ns ) ;
}

void
Namespace_SetAsNotUsing ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    SetState_TrueFalse ( ns, NOT_USING, USING ) ;
}

void
_Namespace_SetAsNotUsing_MoveToTail ( Namespace * ns )
{
    SetState_TrueFalse ( ns, NOT_USING, USING ) ;
    _Namespace_AddToNamespacesTail_ResetFromInNamespace ( ns ) ;
}

void
Namespace_SetAsNotUsing_MoveToTail ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    _Namespace_SetAsNotUsing_MoveToTail ( ns ) ;
}

int64
_Namespace_VariableValueGet ( Namespace * ns, byte * name )
{
    Word * word = _CfrTil_VariableGet ( ns, name ) ;
    if ( word ) return ( int64 ) word->W_Value ; // value of variable
    else return 0 ;
}

void
_Namespace_VariableValueSet ( Namespace * ns, byte * name, int64 value )
{
    Word * word = _CfrTil_VariableGet ( ns, name ) ;
    if ( word ) word->W_Value = value ; // value of variable
}

Namespace *
_CfrTil_Namespace_InNamespaceSet ( Namespace * ns )
{
    if ( ns )
    {
        SetState ( ns, USING, true ) ;
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
    if ( _CfrTil_->Namespaces && ( ! _CfrTil_->InNamespace ) )
    {
        _CfrTil_->InNamespace = _Namespace_FirstOnUsingList ( ) ; //( Namespace* ) _Tree_Map_FromANode ( ( dlnode* ) _CfrTil_->Namespaces, ( cMapFunction_1 ) _Namespace_IsUsing ) ;
    }
    return _CfrTil_->InNamespace ;
}

Namespace *
_CfrTil_InNamespace ( )
{
    Namespace * ins ;
    if ( ( ins = Finder_GetQualifyingNamespace ( _Context_->Finder0 ) ) ) return ins ;
    else return _CfrTil_Namespace_InNamespaceGet ( ) ;
}

Boolean
_CfrTil_IsContainingNamespace ( byte * wordName, byte * namespaceName )
{
    Word * word = Finder_FindWord_UsedNamespaces ( _Finder_, ( byte* ) wordName ) ;
    if ( word && String_Equal ( ( char* ) word->ContainingNamespace->Name, namespaceName ) ) return true ;
    else return false ;
}

void
Namespace_DoNamespace ( byte * name )
{
    _Namespace_DoNamespace ( Namespace_Find ( name ), 1 ) ;
}

void
Symbol_NamespacePrettyPrint ( Symbol * symbol, int64 indentFlag, int64 indentLevel )
{
    Namespace * ns = ( Namespace* ) symbol ;
    Namespace_PrettyPrint ( ns, indentFlag, indentLevel ) ;
}

// a namespaces internal finder, a wrapper for Symbol_Find - prefer Symbol_Find directly

Namespace *
_Namespace_Find ( byte * name, Namespace * superNamespace, int64 exceptionFlag )
{
    Word * word = 0 ;
    if ( superNamespace ) word = Finder_FindWord_InOneNamespace ( _Finder_, superNamespace, name ) ;
    if ( ! word ) word = Finder_FindWord_AnyNamespace ( _Finder_, name ) ;
    if ( word && ( word->CProperty & ( NAMESPACE | CLASS | DOBJECT ) ) ) return ( Namespace* ) word ;
    else if ( exceptionFlag )
    {
        _Printf ( ( byte* ) "\nUnable to find Namespace : %s\n", name ) ;
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
    _Namespace_AddToNamespacesTail_ResetFromInNamespace ( Namespace_Find ( name ) ) ;
}

void
_Namespace_RemoveFromUsingList ( Namespace * ns )
{
    //ns->State = NOT_USING ;
    _Namespace_SetAsNotUsing_MoveToTail ( ns ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) _RemoveSubNamespacesFromUsingList, ( int64 ) ns, 0 ) ;
}

void
Namespace_RemoveFromUsingList ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    if ( String_Equal ( ns->Name, "System" ) ) _Printf ( ( byte* ) "\n\nSystem namespace being cleared %s", _Context_Location ( _Context_ ) ) ;
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
    if ( ! String_Equal ( "Root", ( char* ) name ) )
    {
        Namespace_RemoveFromUsingList ( name ) ;
    }
    else Throw ( ( byte* ) "Error : can't remove Root namespace", QUIT ) ;
}

void
_Namespace_Clear ( Namespace * ns )
{
    if ( ns )
    {
        //DLList_RecycleWordList (  ns->W_List ) ;
        _dllist_Init ( ns->W_List ) ;
    }
}

void
Namespace_Clear ( byte * name )
{
    _Namespace_Clear ( _Namespace_Find ( name, 0, 0 ) ) ;
}

void
_Namespace_FreeNamespacesStack ( Stack * stack )
{
    int64 n ;
    for ( n = Stack_Depth ( stack ) ; n ; n -- )
    {
        Namespace * ns = ( Namespace* ) Stack_Pop ( stack ) ;
        if ( ns ) _Namespace_RemoveFromUsingListAndClear ( ns ) ;
    }
}

Namespace *
Namespace_FindOrNew_SetUsing ( byte * name, Namespace * containingNs, int64 setUsingFlag )
{
    //if ( ! isprint ( name [0] ) ) Error_Abort ( "\nNamespace must begin with printable character!" ) ;
    if ( ! containingNs ) containingNs = _CfrTil_->Namespaces ;
    Namespace * ns = _Namespace_Find ( name, containingNs, 0 ) ;
    if ( ! ns )
    {
        ns = _DataObject_New ( NAMESPACE, 0, name, NAMESPACE | IMMEDIATE, 0, 0, ( int64 ) containingNs, 0 ) ;
    }
    if ( setUsingFlag ) _Namespace_SetState ( ns, USING ) ;
    return ns ;
}

Namespace *
_Namespace_FindOrNew_Local ( Stack * nsStack )
{
    
    int64 d = Stack_Depth ( nsStack ) ; //, bsd = Stack_Depth ( _Context_->Compiler0->BlockStack ) ;
    byte bufferData [ 32 ], *buffer = ( byte* ) bufferData ;
    sprintf ( ( char* ) buffer, "locals_%ld", d ) ;
    Namespace * ns = Namespace_FindOrNew_SetUsing ( buffer, _CfrTil_->Namespaces, 1 ) ;
    _Namespace_ActivateAsPrimary ( ns ) ;
    Stack_Push ( nsStack, ( int64 ) ns ) ;
    BlockInfo * bi = ( BlockInfo * ) _Stack_Top ( _Context_->Compiler0->BlockStack ) ;
    bi->LocalsNamespace = ns ;
    return ns ;
}

void
_Namespace_PrintWords ( Namespace * ns )
{
    dllist_Map1 ( ns->Lo_List, ( MapFunction1 ) _Word_Print, 0 ) ;
}

void
_Namespace_MapAny_2Args ( MapSymbolFunction2 msf2, int64 one, int64 two )
{
    Tree_Map_State_2Args ( _CfrTil_->Namespaces->W_List, ANY, msf2, one, two ) ;
}

void
_Namespace_MapUsing_2Args ( MapSymbolFunction2 msf2, int64 one, int64 two )
{
    Tree_Map_State_2Args ( _CfrTil_->Namespaces->W_List, USING, msf2, one, two ) ;
}
