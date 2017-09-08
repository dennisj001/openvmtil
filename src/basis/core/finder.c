
#include "../../include/cfrtil.h"

Symbol *
DLList_FindName_InOneNamespaceList ( dllist * list, byte * name )
{
    Symbol * s = ( Symbol* ) Tree_Map_OneNamespace ( ( Word* ) dllist_First ( ( dllist* ) list ),
        ( MapFunction_1 ) _Symbol_CompareName, ( int64 ) name ) ;
    return s ;
}

Word *
Finder_Word_Find ( Finder * finder, uint64 state, byte * name )
{
    d1 ( if ( _CfrTil_->Namespaces ) )
    {
        return finder->FoundWord = Tree_Map_State_Flag_OneArg_AnyNamespaceWithState ( state, ( MapFunction_1 ) Symbol_CompareName, ( int64 ) name ) ;
    }
    d1 ( else return 0 ) ;
}

Symbol *
_Finder_CompareDefinitionAddress ( Symbol * symbol, byte * address )
{
    if ( symbol )
    {
        Word * word = ( Word * ) symbol ;
        //byte * codeStart = ( byte* ) word->Definition ; // nb. this maybe more accurate ??
        byte * codeStart = word->CodeStart ;
        //if ( ((byte*) symbol == address) || ( codeStart && ( address >= codeStart ) && ( address <= ( codeStart + word->S_CodeSize ) ) ) )
        if ( ( ( byte* ) symbol == address ) || ( codeStart && ( address >= codeStart ) && ( address <= ( codeStart + word->S_CodeSize ) ) ) )
        {
            return symbol ;
        }
    }
    return 0 ;
}

Symbol *
_Finder_CompareDefinitionAddress_NoAlias ( Symbol * symbol, byte * address )
{
    Word * word = ( Word * ) symbol ;
    if ( ( ! ( word->CProperty & ALIAS ) ) ) return _Finder_CompareDefinitionAddress ( symbol, address ) ;
}

Word *
Finder_FindWordFromAddress_InOneNamespace ( Finder * finder, Namespace * ns, byte * address )
{
    if ( ns ) return finder->FoundWord = Tree_Map_OneNamespace ( ( Word* ) dllist_First ( ( dllist* ) ns->S_SymbolList ),
        ( MapFunction_1 ) _Finder_CompareDefinitionAddress, ( int64 ) address ) ;
}

Word *
Finder_FindWordFromAddress_AnyNamespace ( Finder * finder, byte * address )
{
    finder->FoundWord = Tree_Map_State_Flag_OneArg_AnyNamespaceWithState ( USING | NOT_USING,
        ( MapFunction_1 ) _Finder_CompareDefinitionAddress, ( int64 ) address ) ;
    CfrTil_WordAccounting ( "Finder_Address_FindAny" ) ;
    return finder->FoundWord ;
}

Word *
Finder_FindWordFromAddress_AnyNamespace_NoAlias ( Finder * finder, byte * address )
{
    return finder->FoundWord = Tree_Map_State_Flag_OneArg_AnyNamespaceWithState ( USING | NOT_USING,
        ( MapFunction_1 ) _Finder_CompareDefinitionAddress_NoAlias, ( int64 ) address ) ;
}

void
Finder_SetQualifyingNamespace ( Finder * finder, Namespace * ns )
{
    finder->QualifyingNamespace = ns ;
}

void
Finder_SetNamedQualifyingNamespace ( Finder * finder, byte * name )
{
    finder->QualifyingNamespace = Namespace_Find ( name ) ;
}

Namespace *
Finder_GetQualifyingNamespace ( Finder * finder )
{
    return finder->QualifyingNamespace ;
}

Word *
Finder_Word_FindUsing ( Finder * finder, byte * name, int64 saveQns )
{
    Word * word = 0 ;
    if ( name )
    {
        // the InNamespace takes precedence with this one exception is this best logic ??               
        if ( finder->QualifyingNamespace )
        {
            if ( String_Equal ( ".", ( char* ) name ) ) word = Finder_FindWord_UsedNamespaces ( _Finder_, name ) ; // keep QualifyingNamespace intact // ?? assumes function of CfrTil_Dot is always and only named "." ??
            else
            {
                word = Finder_FindWord_InOneNamespace ( _Finder_, finder->QualifyingNamespace, name ) ;
                if ( ( ! saveQns ) && ( ! GetState ( finder, QID ) ) && ( ! Lexer_IsTokenForwardDotted ( _Context_->Lexer0 ) ) )
                {
                    Finder_SetQualifyingNamespace ( finder, 0 ) ; // nb. QualifyingNamespace is only good for one find unless we are in a quid
                    //finder->QualifyingNamespace = 0 ;
                }
            }
        }
        if ( ! word ) word = Finder_FindWord_UsedNamespaces ( _Finder_, name ) ;
    }
    CfrTil_WordAccounting ( "Finder_Word_FindUsing" ) ;
    return word ;
}

Word *
Finder_FindQualifiedIDWord ( Finder * finder, byte * token )
{
    Word * word ;
    while ( ( word = Finder_Word_FindUsing ( finder, token, 0 ) ) )
    {
        if ( word->CProperty & ( NAMESPACE ) )
        {
            Namespace * ns = ( Namespace * ) word ;
            Finder_SetQualifyingNamespace ( finder, ns ) ;
        }
        else if ( word->CProperty & ( OBJECT ) )
        {
            Finder_SetQualifyingNamespace ( finder, word->ContainingNamespace ) ;
        }
        else if ( word->CProperty & ( OBJECT_FIELD ) )
        {
            Finder_SetQualifyingNamespace ( finder, word->ClassFieldTypeNamespace ) ;
        }
        else return word ;
        if ( Lexer_IsTokenForwardDotted ( _Context_->Lexer0 ) )
        {
            Lexer_ReadToken ( _Context_->Lexer0 ) ; // the '.'
            token = Lexer_ReadToken ( _Context_->Lexer0 ) ; // the namespace
            continue ;
        }

        else return word ;
    }
    return 0 ;
}

byte *
Finder_GetTokenDefinitionAddress ( Finder * finder, byte * token )
{
    byte * definitionAddress = 0 ;
    if ( token )
    {
        Word * word = Finder_Word_FindUsing ( finder, token, 1 ) ;

        if ( word ) definitionAddress = ( byte* ) word->Definition ;
    }
    return definitionAddress ;
}

Word *
Finder_FindToken_WithException ( Finder * finder, byte * token )
{
    if ( Finder_Word_FindUsing ( finder, token, 0 ) == 0 )
    {
        _Printf ( ( byte* ) "\n%s ?", ( char* ) token ) ;
        CfrTil_Using ( ) ;
        CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
    }
    return finder->FoundWord ;
}

Word *
Finder_FindToken ( Finder * finder, byte * token )
{
    return Finder_FindToken_WithException ( finder, token ) ;
}

void
Finder_Init ( Finder * finder )
{
    memset ( finder, 0, sizeof (Finder ) ) ;
}

Finder *
Finder_New ( uint64 allocationType )
{
    Finder * finder = ( Finder * ) Mem_Allocate ( sizeof (Finder ), allocationType ) ;
    //Finder_Init ( finder ) ; // not needed assuming _Mem_Allocate returns clear mem
    return finder ;
}

Word *
Finder_FindWord_InOneNamespace ( Finder * finder, Namespace * ns, byte * name )
{
    if ( ns && name )
    {
        return finder->FoundWord = DLList_FindName_InOneNamespaceList ( ( dllist* ) ns->W_List, name ) ;
    }
    return 0 ;
}

Word *
Finder_FindWord_UsedNamespaces ( Finder * finder, byte * name )
{
    return Finder_Word_Find ( finder, USING, name ) ;
}

Word *
Finder_FindWord_AnyNamespace ( Finder * finder, byte * name )
{
    return Finder_Word_Find ( finder, ANY, name ) ;
}

Word *
_CfrTil_FindInAnyNamespace ( byte * name )
{
    return Finder_FindWord_AnyNamespace ( _Finder_, name ) ;
}

Word *
_CfrTil_Token_FindUsing ( byte * token )
{
    return Finder_Word_FindUsing ( _Context_->Finder0, token, 0 ) ;
}

void
CfrTil_Token_Find ( )
{
    _CfrTil_Token_FindUsing ( _Context_->Lexer0->OriginalToken ) ;
}

void
CfrTil_Find ( )
{
    _DataStack_Push ( ( int64 ) Finder_FindToken ( _Context_->Finder0, _Context_->Lexer0->OriginalToken ) ) ;
}

void
CfrTil_Postfix_Find ( )
{
    Word * word = Finder_Word_FindUsing ( _Context_->Finder0, ( byte* ) _DataStack_Pop ( ), 0 ) ;
    _DataStack_Push ( ( int64 ) word ) ;
}



