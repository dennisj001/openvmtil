
#include "../../include/cfrtil.h"

Symbol *
_Word_FindSymbol_InOneNamespace ( dllist * list, byte * name )
{
    Symbol * s = ( Symbol* ) _Tree_Map_OneNamespace ( ( Word* ) dllist_First ( ( dllist* ) list ),
        ( MapFunction_1 ) _Symbol_CompareName, ( int32 ) name ) ;
    return s ;
}

Word *
Word_FindInOneNamespace ( Namespace * ns, byte * name )
{
    if ( ns && name )
    {
        _Context_->Finder0->FoundWord = 0 ;
        _Context_->Finder0->w_Word = 0 ;
        Word * word = _Word_FindSymbol_InOneNamespace ( ( dllist* ) ns->W_List, name ) ;
        _Context_->Finder0->w_Word = word ;
        return word ;
    }
    return 0 ;
}

Word *
_Word_Find ( uint64 state, byte * name )
{
    _Context_->Finder0->FoundWord = 0 ;
    _Context_->Finder0->w_Word = 0 ;
    return _Tree_Map_State_Flag_OneArg_AnyNamespaceWithState ( state, ( MapFunction_1 ) Symbol_CompareName, ( int32 ) name ) ;
}

Word *
Word_FindUsing ( byte * name )
{
    return _Word_Find ( USING, name ) ;
}

Word *
_Word_FindAny ( byte * name )
{
    return _Word_Find ( ANY, name ) ;
}

Word *
Word_Find ( byte * name )
{
    return _Word_Find ( ANY, name ) ;
}

void
Finder_Init ( Finder * finder )
{
    memset ( finder, 0, sizeof (Finder ) ) ;
}

Finder *
Finder_New ( uint32 allocationType )
{
    Finder * finder = ( Finder * ) Mem_Allocate ( sizeof (Finder ), allocationType ) ;
    //Finder_Init ( finder ) ; // not needed assuming _Mem_Allocate returns clear mem
    return finder ;
}

Symbol *
_Finder_CompareDefinitionAddress ( Symbol * symbol, byte * address )
{
    Word * word = ( Word * ) symbol ;
    //byte * codeStart = ( byte* ) word->Definition ; // nb. this maybe more accurate ??
    byte * codeStart = word->CodeStart ;
    if ( codeStart && ( address >= codeStart ) && ( address <= ( codeStart + word->S_CodeSize ) ) )
    {
        return symbol ;
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
Finder_Address_FindInOneNamespace ( Finder * finder, Namespace * ns, byte * address )
{
    if ( ns ) return finder->w_Word = _Tree_Map_OneNamespace ( ( Word* ) dllist_First ( ( dllist* ) ns->S_SymbolList ),
        ( MapFunction_1 ) _Finder_CompareDefinitionAddress, ( int32 ) address ) ;
}

void
CfrTil_WordAccounting ( byte * functionName )
{
    if ( _CfrTil_->WordCount > _CfrTil_->WordMaxCount ) _CfrTil_->WordMaxCount = _CfrTil_->WordCount ;
    if ( _Q_->Verbosity > 3 ) _Printf ( "\n%s :: words searched = %d : words max searched = %d", functionName,
        _CfrTil_->WordCount, _CfrTil_->WordMaxCount ) ;
}

Word *
Finder_Address_FindAny ( Finder * finder, byte * address )
{
    finder->w_Word = _Tree_Map_State_Flag_OneArg_AnyNamespaceWithState ( USING | NOT_USING,
        ( MapFunction_1 ) _Finder_CompareDefinitionAddress, ( int32 ) address ) ;
    CfrTil_WordAccounting ( "Finder_Address_FindAny" ) ;
    return finder->w_Word ;
}

Word *
Finder_Address_FindAny_NoAlias ( Finder * finder, byte * address )
{
    return finder->w_Word = _Tree_Map_State_Flag_OneArg_AnyNamespaceWithState ( USING | NOT_USING,
        ( MapFunction_1 ) _Finder_CompareDefinitionAddress_NoAlias, ( int32 ) address ) ;
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
Finder_Word_FindUsing ( Finder * finder, byte * name, int32 saveQns )
{
    Word * word = 0 ;
    if ( name )
    {
        // the InNamespace takes precedence with this one exception is this best logic ??               
        if ( finder->QualifyingNamespace )
        {
            if ( String_Equal ( ".", ( char* ) name ) ) word = Word_FindUsing ( name ) ; // keep QualifyingNamespace intact // ?? assumes function of CfrTil_Dot is always and only named "." ??
            else
            {
                word = Word_FindInOneNamespace ( finder->QualifyingNamespace, name ) ;
                if ( ( ! saveQns ) && ( ! GetState ( finder, QID ) ) && ( ! Lexer_IsTokenForwardDotted ( _Context_->Lexer0 ) ) )
                {
                    Finder_SetQualifyingNamespace ( finder, 0 ) ; // nb. QualifyingNamespace is only good for one find unless we are in a quid
                    //finder->QualifyingNamespace = 0 ;
                }
            }
        }
        if ( ! word ) word = Word_FindUsing ( name ) ;
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
    return finder->w_Word ;
}

Word *
Finder_FindToken ( Finder * finder, byte * token )
{
    return Finder_FindToken_WithException ( finder, token ) ;
}

Word *
_CfrTil_FindInAnyNamespace ( byte * name )
{
    return _Word_FindAny ( name ) ;
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
    _DataStack_Push ( ( int32 ) Finder_FindToken ( _Context_->Finder0, _Context_->Lexer0->OriginalToken ) ) ;
}

void
CfrTil_Postfix_Find ( )
{
    Word * word = Finder_Word_FindUsing ( _Context_->Finder0, ( byte* ) _DataStack_Pop ( ), 0 ) ;
    _DataStack_Push ( ( int32 ) word ) ;
}



