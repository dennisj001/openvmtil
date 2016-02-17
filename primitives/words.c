
#include "../includes/cfrtil.h"

void
CfrTil_SourceCode_Init ( )
{
    _CfrTil_InitSourceCode_WithName ( WordStack ( 0 )->Name ) ;
}

void
CfrTil_Colon ( )
{
    CfrTil_RightBracket ( ) ;
    Compiler_SetState ( _Q_->OVT_Context->Compiler0, COMPILE_MODE, true ) ;
    CfrTil_SourceCode_Init ( ) ;
    CfrTil_Token ( ) ;
    CfrTil_Word_Create ( ) ;
    CfrTil_BeginBlock ( ) ;
}

void
CfrTil_SemiColon ( )
{
    CfrTil_EndBlock ( ) ;
    block b = ( block ) _DataStack_Pop ( ) ;
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Word ( word, ( byte* ) b ) ;
}

void
AddressToWord ( )
{
    _DataStack_Push ( ( int32 ) Finder_Address_FindAny ( _Q_->OVT_Context->Finder0, ( byte* ) _DataStack_Pop ( ) ) ) ;
}

void
Word_Definition ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int32 ) word->Definition ) ;
}

void
Word_Value ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int32 ) word->W_Value ) ;
}

void
Word_Xt_LValue ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int32 ) & word->Definition ) ;
}

void
Word_DefinitionStore ( )
{
    block b = ( block ) _DataStack_Pop ( ) ;
    Word * word = ( Word* ) TOS ; // leave word on tos for anticipated further processing
    DebugDontShow_On ;
    _Word_DefinitionStore ( word, b ) ;
    DebugDontShow_Off ;
}

void
Word_CodeStart ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int32 ) word->CodeStart ) ;
}

void
Word_CodeSize ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int32 ) word->S_CodeSize ) ;
}

void
Word_Run ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Word_Run ( word ) ;
}

void
Word_Eval ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Word_Eval ( word ) ;
}

void
Word_Finish ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Word_Finish ( word ) ;
}

// ?!? : nb. macros and _Word_Begin may need to be reworked

byte *
_Word_Begin ( )
{
    CfrTil_SourceCode_Init ( ) ;
    byte * name = Lexer_ReadToken ( _Q_->OVT_Context->Lexer0 ) ;
    return name ;
}

void
Word_Add ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Word_Add ( word, 1, 0 ) ;
}

void
CfrTil_Word_Create ( )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int32 ) Word_Create ( name ) ) ;
}

// ( token block -- word )
// postfix 'word' takes a token and a block

void
CfrTil_Word ( )
{
    block b = ( block ) _DataStack_Pop ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
#if 0    
    Word * word = Word_Create ( name ) ;
    _Word ( word, ( byte* ) b ) ;
#else
    _DataObject_New ( CFRTIL_WORD, name, 0, 0, 0, (int32) b, 0 ) ;
#endif    
}

void
CfrTil_Alias ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _CfrTil_Alias ( word, name ) ;
}

void
CfrTil_Eval_C_Rtl_ArgList ( ) // C protocol : right to left arguments from a list pushed on the stack
{
    LC_CompileRun_ArgList ( ( Word * ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_TextMacro ( )
{
    _CfrTil_Macro ( TEXT_MACRO, ( byte* ) Do_TextMacro ) ;
}

void
CfrTil_StringMacro ( )
{
    _CfrTil_Macro ( STRING_MACRO, ( byte* ) Do_StringMacro ) ;
}

void
Word_Name ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int32 ) word->Name ) ;
}

void
Word_Location ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Word_Location_Printf ( word ) ;
}

void
Word_Namespace ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int32 ) _Word_Namespace ( word ) ) ;
}

void
CfrTil_Keyword ( void )
{
    if ( _Q_->OVT_CfrTil->LastFinishedWord ) _Q_->OVT_CfrTil->LastFinishedWord->CType |= KEYWORD ;
}

void
CfrTil_Immediate ( void )
{
    if ( _Q_->OVT_CfrTil->LastFinishedWord ) _Q_->OVT_CfrTil->LastFinishedWord->CType |= IMMEDIATE ;
}

void
CfrTil_IsImmediate ( void )
{
#if 0    
    Word * word = ( Word* ) TOS ; 
    TOS = ( word->CType & IMMEDIATE ) ;
#else
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( word->CType & IMMEDIATE ) ;
#endif    
}

void
CfrTil_Inline ( void )
{
    if ( _Q_->OVT_CfrTil->LastFinishedWord ) _Q_->OVT_CfrTil->LastFinishedWord->CType |= INLINE ;
}

void
CfrTil_Prefix ( void )
{
    if ( _Q_->OVT_CfrTil->LastFinishedWord )
    {
        //_Q_->OVT_CfrTil->LastFinishedWord->CType |= PREFIX ;
        _Q_->OVT_CfrTil->LastFinishedWord->WType = WT_PREFIX ;
    }
}

void
CfrTil_C_Prefix ( void )
{
    if ( _Q_->OVT_CfrTil->LastFinishedWord )
    {
        _Q_->OVT_CfrTil->LastFinishedWord->CType |= C_PREFIX | C_PREFIX_RTL_ARGS ;
        _Q_->OVT_CfrTil->LastFinishedWord->WType = WT_C_PREFIX_RTL_ARGS ;
    }
}

void
CfrTil_C_Return ( void )
{
    if ( _Q_->OVT_CfrTil->LastFinishedWord )
    {
        _Q_->OVT_CfrTil->LastFinishedWord->CType |= C_RETURN | C_PREFIX_RTL_ARGS ;
        _Q_->OVT_CfrTil->LastFinishedWord->WType = WT_C_PREFIX_RTL_ARGS ;
    }
}

void
CfrTil_Void_Return ( void )
{
    if ( _Q_->OVT_CfrTil->LastFinishedWord )
    {
        _Q_->OVT_CfrTil->LastFinishedWord->CType &= ~C_RETURN ;
        _Q_->OVT_CfrTil->LastFinishedWord->CType |= VOID_RETURN ;
    }
}

void
CfrTil_EAX_Return ( void )
{
    if ( _Q_->OVT_CfrTil->LastFinishedWord )
    {
        _Q_->OVT_CfrTil->LastFinishedWord->CType &= ~C_RETURN ;
        _Q_->OVT_CfrTil->LastFinishedWord->CType2 |= EAX_RETURN ;
    }
}

void
CfrTil_DebugWord ( void )
{
    if ( _Q_->OVT_CfrTil->LastFinishedWord ) _Q_->OVT_CfrTil->LastFinishedWord->CType |= DEBUG_WORD ;
}

void
_PrintWord ( DLNode * node, int32 * n )
{
    Word * word = ( Word * ) node ;
    _Word_Print ( word ) ;
    ( *n ) ++ ;
}

void
_Words ( Symbol * symbol, MapFunction1 mf, int32 n )
{
    Namespace * ns = ( Namespace * ) symbol ;
    Printf ( ( byte* ) "\n - %s :> ", ns->Name ) ;
    DLList_Map1 ( ns->Lo_List, mf, n ) ;
}

void
_DoWords ( Symbol * symbol, int32 * n )
{
    _Words ( symbol, ( MapFunction1 ) _PrintWord, ( int32 ) n ) ;
}

int32
_CfrTil_PrintWords ( int32 state )
{
    int32 n = 0 ;
    _CfrTil_NamespacesMap ( ( MapSymbolFunction2 ) _DoWords, state, ( int32 ) & n, 0 ) ;
    if ( _Q_->Verbosity > 3 ) Printf ( (byte*) "\nCfrTil : WordsAdded = %d", _Q_->OVT_CfrTil->WordsAdded ) ;
    return n ;
}

void
CfrTil_Words ( )
{
    Printf ( ( byte* ) "\nWords :\n - <namespace> ':>' <word list>" ) ;
    int n = _CfrTil_PrintWords ( USING ) ;
    Printf ( ( byte* ) "\n" INT_FRMT " words on the Searched Namespaces List", n ) ;
}

void
_Variable_Print ( Word * word )
{
    Printf ( ( byte* ) c_ud ( " %s = %x ;" ), word->Name, word->W_Value ) ;
}

void
_PrintVariable ( DLNode * node, int32 * n )
{
    Word * word = ( Word * ) node ;
    if ( word->CType & VARIABLE )
    {
        _Variable_Print ( word ) ;
        ( *n ) ++ ;
    }
}

void
_Variables ( Symbol * symbol, MapFunction1 mf, int32 n )
{
    int32 pre_n = * ( int32* ) n ;
    Namespace * ns = ( Namespace * ) symbol ;
    Printf ( ( byte* ) "\n - %s :> ", ns->Name ) ;
    DLList_Map1 ( ns->Lo_List, mf, n ) ;
    if ( *( int32* ) n == pre_n ) Printf ( ( byte* ) "\r" ) ;
}

void
_PrintVariables ( Symbol * symbol, int32 * n )
{
    _Variables ( symbol, ( MapFunction1 ) _PrintVariable, ( int32 ) n ) ;
}

int32
_CfrTil_PrintVariables ( int32 nsStatus )
{
    int32 n = 0 ;
    _CfrTil_NamespacesMap ( ( MapSymbolFunction2 ) _PrintVariables, nsStatus, ( int32 ) & n, 0 ) ;
    return n ;
}

void
CfrTil_Variables ( )
{
    Printf ( ( byte* ) "\nGlobal Variables :\n - <namespace> ':>' <variable '=' value ';'>*" ) ;
    int n = _CfrTil_PrintVariables ( USING ) ;
    Printf ( ( byte* ) "\n" INT_FRMT " global variables on the Searched Namespaces List", n ) ;
}

void
_CfrTil_NamespaceWords ( )
{
    int32 n = 0 ;
    Namespace * ns = ( Namespace * ) _DataStack_Pop ( ) ;
    if ( ns )
    {
        _DoWords ( ( Symbol * ) ns, &n ) ;
        Printf ( ( byte* ) "\n" INT_FRMT " words in %s namespace", n, ns->Name ) ;
    }
    else Printf ( ( byte* ) "\nError : can't find that namespace" ) ;
}

void
CfrTil_NamespaceWords ( )
{
    byte * name = ( byte * ) _DataStack_Pop ( ) ;
    Namespace * ns = _Namespace_Find ( name, 0, 0 ) ;
    _DataStack_Push ( ( int32 ) ns ) ;
    _CfrTil_NamespaceWords ( ) ;
}

void
CfrTil_AllWords ( )
{
    Printf ( ( byte* ) "\n - <namespace> ':>' <word list>" ) ;
    Printf ( ( byte* ) "\nSearched Namespaces List" ) ;
    int n = _CfrTil_PrintWords ( USING ) ;
    Printf ( ( byte* ) "\n" INT_FRMT " words on the Currently Searched Namespaces List", n ) ;
    Printf ( ( byte* ) "\nNon - Searched Namespaces List" ) ;
    int m = _CfrTil_PrintWords ( NOT_USING ) ;
    Printf ( ( byte* ) "\n" INT_FRMT " words on the Non-SearchedList", m ) ;
    Printf ( ( byte* ) "\n" INT_FRMT " total words", n + m ) ;
}

