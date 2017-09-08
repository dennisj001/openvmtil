
#include "../include/cfrtil.h"

/*
 * cfrTil namespaces basic understandings :
 * 0. to allow for ordered search thru lists in use ...
 * 1. Namespaces are also linked on the _Context->System0->Namespaces list with status of USING or NOT_USING
 *      and are moved the front or back of that list if status is set to USING with the 'Symbol node'
 *      This list is ordered so we can set an order to our search thru the namespaces in use. 
 *      The first word found within the ordered USING list will be used.
 */

// "root" -- ?? is this all the namespaces we want/need in this list ??
// no ! -- this needs updating

#if 0

void
CfrTil_Namespaces_Root ( )
{
    //dllist_Map1 ( _Context->System0->Words->Lo_List, ( MapFunction1 ) _Namespace_DoSetState, USING ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) _Namespace_DoSetState, USING, 0 ) ;
    Namespace * ns1 = _Namespace_Find ( ( byte* ) "Root", 0, 1 ) ;
    Namespace * ns2 = _Namespace_Find ( ( byte* ) "Namespace", 0, 1 ) ;
    Namespace * ns3 = _Namespace_Find ( ( byte* ) "Io", 0, 1 ) ;
    Namespace * ns4 = _Namespace_Find ( ( byte* ) "System", 0, 1 ) ;
    Namespace * ns5 = _Namespace_Find ( ( byte* ) "OpenVmTil", 0, 1 ) ;
    Namespace * ns6 = _Namespace_Find ( ( byte* ) "Logic", 0, 1 ) ;
    Namespace * ns7 = _Namespace_Find ( ( byte* ) "Combinators", 0, 1 ) ;
    Namespace * ns8 = _Namespace_Find ( ( byte* ) "Compiler", 0, 1 ) ;
    Namespace * ns9 = _Namespace_Find ( ( byte* ) "Interpreter", 0, 1 ) ;
    Namespace * ns10 = _Namespace_Find ( ( byte* ) "Int", 0, 1 ) ;
    Namespace * ns11 = _Namespace_Find ( ( byte* ) "Stack", 0, 1 ) ;
    Namespace * ns12 = _Namespace_Find ( ( byte* ) "Word", 0, 1 ) ;

    //dllist_Map1 ( _Context->System0->Words->Lo_List, ( MapFunction1 ) _Namespace_DoSetState, NOT_USING ) ;
    //_Namespace_Map_2 ( dllist * list, cell state, MapSymbolFunction2 mf, cell one, cell two )
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) _Namespace_DoSetState, NOT_USING, 0 ) ;

    _Namespace_SetState ( ns1, USING ) ;
    _Namespace_SetState ( ns2, USING ) ;
    _Namespace_SetState ( ns3, USING ) ;
    _Namespace_SetState ( ns4, USING ) ;
    _Namespace_SetState ( ns5, USING ) ;
    _Namespace_SetState ( ns6, USING ) ;
    _Namespace_SetState ( ns7, USING ) ;
    _Namespace_SetState ( ns8, USING ) ;
    _Namespace_SetState ( ns9, USING ) ;
    _Namespace_SetState ( ns10, USING ) ;
    _Namespace_SetState ( ns11, USING ) ;
    _Namespace_SetState ( ns12, USING ) ;
    //
}
#endif

void
Do_Namespace_WithStatus_2 ( dlnode * node, MapFunction2 nsf, int64 nsStateFlag, int64 one, int64 two )
{
    Namespace * ns = ( Namespace * ) node ;
    if ( ns->State == nsStateFlag )
    {
        nsf ( node, one, two ) ;
    }
}

void
_CfrTil_TreeMap ( MapSymbolFunction2 msf2, uint64 state, int64 one, int64 two )
{
    Tree_Map_State_2Args ( _CfrTil_->Namespaces->Lo_List, state, msf2, one, two ) ;
}

void
_CfrTil_NamespacesMap ( MapSymbolFunction2 msf2, uint64 state, int64 one, int64 two )
{
    Tree_Map_State_2Args ( _CfrTil_->Namespaces->Lo_List, state, msf2, one, two ) ;
}

// list/print namespaces

void
_CfrTil_ForAllNamespaces ( MapSymbolFunction2 msf2 )
{
    _Printf ( ( byte* ) "\nusing :" ) ;
    _CfrTil_NamespacesMap ( msf2, USING, 1, 1 ) ;
    _Printf ( ( byte* ) "\nnotUsing :" ) ;
    int64 usingWords = _CfrTil_->FindWordCount ;
    _CfrTil_NamespacesMap ( msf2, NOT_USING, 1, 1 ) ;
    int64 notUsingWords = _CfrTil_->FindWordCount ;
    _CfrTil_->FindWordCount = usingWords + notUsingWords ;
    CfrTil_WordAccounting ( "_CfrTil_ForAllNamespaces" ) ;
}

void
Namespace_PrettyPrint ( Namespace* ns, int64 indentFlag, int64 indentLevel )
{
    if ( indentFlag )
    {
        _Printf ( ( byte* ) "\n" ) ;
        while ( indentLevel -- ) _Printf ( ( byte* ) "\t" ) ;
    }
    if ( ns->State & NOT_USING ) _Printf ( ( byte* ) " - %s", c_dd ( ns->Name ) ) ;
    else _Printf ( ( byte* ) " - %s", ns->Name ) ;
    _Context_->NsCount ++ ;
}

void
CfrTil_Namespace_New ( )
{
    Namespace * ns = Namespace_FindOrNew_SetUsing ( ( byte* ) _DataStack_Pop ( ), _CfrTil_Namespace_InNamespaceGet ( ), 1 ) ;
    _Namespace_DoNamespace ( ns, 1 ) ;

}

void
_CfrTil_Namespace_NotUsing ( byte * name )
{
    Namespace * ns = Namespace_Find ( name ) ;
    if ( ns )
    {
        _Namespace_RemoveFromUsingList ( ns ) ;
        _CfrTil_->InNamespace = _Namespace_FirstOnUsingList ( ) ; //( Namespace* ) _Tree_Map_FromANode ( dlnode_Next ( ( dlnode* ) ns ), ( cMapFunction_1 ) _Namespace_IsUsing ) ;
    }
}

void
CfrTil_Namespace_NotUsing ( )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _CfrTil_Namespace_NotUsing ( name ) ;
}

void
CfrTil_Namespace_UsingFirst ( )
{
    Namespace * ns = Namespace_Find ( ( byte* ) _DataStack_Pop ( ) ) ;
    if ( ns )
    {
        _Namespace_AddToUsingList ( ns ) ;
    }
}

void
CfrTil_Namespace_UsingLast ( )
{
    _Namespace_UsingLast ( ( byte* ) _DataStack_Pop ( ) ) ;
}

// "in"

void
CfrTil_PrintInNamespace ( )
{
    _Printf ( ( byte* ) "\nCurrent Namespace Being Compiled : %s\n",
        _CfrTil_Namespace_InNamespaceGet ( )->Name ) ;
}

// list/print namespaces

void
CfrTil_Namespaces ( )
{
    _Printf ( ( byte* ) "\nAll Namespaces : \n<list> ':' '-' <namespace>" ) ;
    _CfrTil_ForAllNamespaces ( ( MapSymbolFunction2 ) Symbol_NamespacePrettyPrint ) ;
    _Printf ( ( byte* ) "\n" ) ;
}

void
Symbol_SetNonTREED ( Symbol * symbol, int64 one, int64 two )
{
    Namespace * ns = ( Namespace * ) symbol ;
    ns->State &= ~ TREED ;
}

void
Symbol_Namespaces_PrintTraverse ( Symbol * symbol, int64 containingNamespace, int64 indentLevel )
{
    Namespace * ns = ( Namespace * ) symbol ;
    if ( ns->ContainingNamespace == ( Namespace* ) containingNamespace )
    {
        if ( ! ( ns->State & TREED ) )
        {
            ns->State |= TREED ;
            Namespace_PrettyPrint ( ns, 1, indentLevel ) ;
            _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_Namespaces_PrintTraverse, ( int64 ) ns, indentLevel + 1 ) ;
        }
    }
}

void
Symbol_Namespaces_PrintTraverseWithWords ( Symbol * symbol, int64 containingNamespace, int64 indentLevel )
{
    Namespace * ns = ( Namespace * ) symbol ;
    if ( ns->ContainingNamespace == ( Namespace* ) containingNamespace )
    {
        if ( ! ( ns->State & TREED ) )
        {
            ns->State |= TREED ;
            Namespace_PrettyPrint ( ns, 1, indentLevel ) ;
            dllist_Map1 ( ns->Lo_List, ( MapFunction1 ) _Word_Print, 0 ) ;
            _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_Namespaces_PrintTraverseWithWords, ( int64 ) ns, indentLevel + 1 ) ;
        }
    }
}

void
CfrTil_Namespaces_PrettyPrintTree ( )
{
    _Context_->NsCount = 0 ;
    _Context_->WordCount = 0 ;
    //SetState ( _Q_->psi_PrintStateInfo, PSI_PROMPT, false ) ;
    _Printf ( ( byte* ) "\nNamespaceTree - All Namespaces : %s%s%s", c_ud ( "using" ), " : ", c_dd ( "not using" ) ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_SetNonTREED, 0, 0 ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_Namespaces_PrintTraverse, ( int64 ) _CfrTil_->Namespaces, 1 ) ;
    _Printf ( ( byte* ) "\nTotal namespaces = %d :: Total words = %d\n", _Context_->NsCount, _Context_->WordCount ) ;
}

void
CfrTil_Namespaces_PrettyPrintTreeWithWords ( )
{
    _Context_->NsCount = 0 ;
    _Context_->WordCount = 0 ;
    //SetState ( _Q_->psi_PrintStateInfo, PSI_PROMPT, false ) ;
    _Printf ( ( byte* ) "%s%s%s%s%s%s%s", "\nNamespaceTree - All Namespaces : ", "using", " : ", c_dd ( "not using" ), " :: ", "with", c_ud ( " : words" ) ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_SetNonTREED, 0, 0 ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_Namespaces_PrintTraverseWithWords, ( int64 ) _CfrTil_->Namespaces, 1 ) ;
    _Printf ( ( byte* ) "\nTotal namespaces = %d :: Total words = %d\n", _Context_->NsCount, _Context_->WordCount ) ;
}

void
_Namespace_Symbol_Print ( Symbol * symbol, int64 printFlag, int64 str )
{
    char buffer [128] ;
    Namespace * ns = ( Namespace * ) symbol ;
    sprintf ( buffer, "%s ", ns->Name ) ;
    if ( printFlag )
    {
        _Printf ( ( byte* ) "%s", buffer ) ;
    }
    else strcat ( ( char* ) str, buffer ) ;
}

// prints all the namespaces on the SearchList
// 'using'

byte *
_CfrTil_UsingToString ( )
{
    //Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
    byte * b = Buffer_Data ( _CfrTil_->ScratchB1 ) ;
    strcpy ( ( char* ) b, "" ) ;
    Tree_Map_State_2Args ( _CfrTil_->Namespaces->Lo_List, USING, ( MapSymbolFunction2 ) _Namespace_Symbol_Print, 0, ( int64 ) b ) ;
    b = String_New ( ( byte* ) b, TEMPORARY ) ;
    //Buffer_SetAsUnused ( buffer ) ;
    return b ;
}

void
CfrTil_Using ( )
{
    _Printf ( ( byte* ) "\nUsing Namespaces :> " ) ;
    Tree_Map_State_2Args ( _CfrTil_->Namespaces->Lo_List, USING, ( MapSymbolFunction2 ) _Namespace_Symbol_Print, 1, 0 ) ;
    _Printf ( ( byte* ) "\n" ) ;
}

// this namespace is will be taken out of the system

void
_Namespace_RemoveFromUsingListAndClear ( Namespace * ns )
{
    if ( ns )
    {
        if ( ns == _CfrTil_->InNamespace ) _CfrTil_->InNamespace = ( Namespace* ) dlnode_Next ( ( dlnode* ) ns ) ; //dllist_First ( (dllist*) _Q_->CfrTil->Namespaces->Lo_List ) ;
        if ( ns == _Context_->Finder0->QualifyingNamespace ) Finder_SetQualifyingNamespace ( _Context_->Finder0, 0 ) ;
        _Namespace_Clear ( ns ) ;
        dlnode_Remove ( ( dlnode* ) ns ) ;
    }
}

void
_CfrTil_RemoveNamespaceFromUsingListAndClear ( byte * name )
{
    _Namespace_RemoveFromUsingListAndClear ( Namespace_Find ( name ) ) ;
}

