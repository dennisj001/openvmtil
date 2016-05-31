
#include "../../includes/cfrtil.h"

// we could make this a SymbolList function if we refactor State field
Word *
Word_FindInOneNamespace ( Namespace * ns, byte * name )
{
    if ( ns && name )
    {
        _Context_->Finder0->FoundWord = 0 ;
        _Context_->Finder0->w_Word = 0 ;
        return _Context_->Finder0->w_Word = _Tree_Map_State_Flag_OneArg ( ( Word* ) dllist_First ( (dllist*) ns->W_List ), USING, 1, ( MapFunction_Cell_1 ) Symbol_CompareName, ( int32 ) name ) ;
    }
    return 0 ;
}

Symbol *
_Word_Find_Symbol ( dllist * list, uint64 state, byte * name )
{
    Symbol * s = ( Symbol* ) _Tree_Map_State_Flag_OneArg ( ( Word* ) dllist_First ( (dllist*) list ), state, 1, ( MapFunction_Cell_1 ) _Symbol_CompareName, ( int32 ) name ) ;
    return s ;
}

Word *
_Word_Find ( uint64 state, byte * name )
{
    _Context_->Finder0->FoundWord = 0 ;
    _Context_->Finder0->w_Word = 0 ;
#if 1    
    return _Tree_Map_State_Flag_OneArg ( _Q_->OVT_CfrTil->Namespaces, state, 0, ( MapFunction_Cell_1 ) Symbol_CompareName, ( int32 ) name ) ;
#else    
    _Context_->NlsWord = 0 ;
    return _Tree_Map_State_Flag_1Arg ( _Q_->OVT_CfrTil->Namespaces, state, 0, ( MapFunction_Cell_1 ) Symbol_CompareName, ( int32 ) name ) ;
#endif    
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
    byte * codeStart = ( byte* ) word->Definition ; // nb. this maybe more accurate ??
    //byte * codeStart = word->CodeStart ;
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
    byte * codeStart = ( byte* ) word->Definition ; // nb. this maybe more accurate ??
    //byte * codeStart = word->CodeStart ;
    if ( ( ! ( word->CProperty & ALIAS ) ) && codeStart && ( address >= codeStart ) && ( address <= ( codeStart + word->S_CodeSize ) ) )
    {
        return symbol ;
    }
    else return 0 ;
}

Word *
Finder_Address_FindInOneNamespace ( Finder * finder, Namespace * ns, byte * address )
{
    return finder->w_Word = _Tree_Map_State_Flag_OneArg ( ns, USING, 1, ( MapFunction_Cell_1 ) _Finder_CompareDefinitionAddress, ( int32 ) address ) ;
}

Word *
Finder_Address_FindAny ( Finder * finder, byte * address )
{
    return finder->w_Word = _Tree_Map_State_Flag_OneArg ( _Q_->OVT_CfrTil->Namespaces, USING | NOT_USING, 0, ( MapFunction_Cell_1 ) _Finder_CompareDefinitionAddress, ( int32 ) address ) ;
}

Word *
Finder_Address_FindAny_NoAlias ( Finder * finder, byte * address )
{
    return finder->w_Word = _Tree_Map_State_Flag_OneArg ( _Q_->OVT_CfrTil->Namespaces, USING | NOT_USING, 0, ( MapFunction_Cell_1 ) _Finder_CompareDefinitionAddress_NoAlias, ( int32 ) address ) ;
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
                }
            }
        }
        if ( ! word ) word = Word_FindUsing ( name ) ;
    }
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
        Printf ( ( byte* ) "\n%s ?", ( char* ) token ) ;
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



