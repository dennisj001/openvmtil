
#include "../include/cfrtil.h"

void
CfrTil_Colon ( )
{
    CfrTil_RightBracket ( ) ;
    SetState ( _Context_->Compiler0, COMPILE_MODE, true ) ;
    CfrTil_SourceCode_Init ( ) ;
    CfrTil_Token ( ) ;
    CfrTil_Word_New ( ) ;
    CfrTil_BeginBlock ( ) ;
}

Word *
_CfrTil_SemiColon ( )
{
    block b = ( block ) _DataStack_Pop ( ) ;
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Word_InitFinal ( word, ( byte* ) b ) ;
    return word ;
}

void
CfrTil_SemiColon ( )
{
    CfrTil_EndBlock ( ) ;
    _CfrTil_SemiColon ( ) ;
}

void
CfrTil_SourceCodeCompileOff ( )
{
    //SetState ( _CfrTil_, SOURCE_CODE_MODE, false ) ;
    _CfrTil_DebugSourceCodeCompileOff ( ) ;
    if ( ! GetState ( _Context_, C_SYNTAX ) ) CfrTil_SemiColon ( ) ;
}

void
AddressToWord ( )
{
    _DataStack_Push ( ( int64 ) Finder_FindWordFromAddress_AnyNamespace ( _Context_->Finder0, ( byte* ) _DataStack_Pop ( ) ) ) ;
}

void
Word_Definition ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int64 ) word->Definition ) ;
}

void
Word_Value ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int64 ) word->W_Value ) ;
}

void
Word_Xt_LValue ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int64 ) & word->Definition ) ;
}

#if  0

void
Word_DefinitionStore ( )
{
    Word * word = ( Word * ) _DataStack_Pop ( ) ;
    block b = ( block ) TOS ; // leave word on tos for anticipated further processing
    _Word_DefinitionStore ( word, b ) ;
}
#endif

void
Word_DefinitionEqual ( )
{
    block b = ( block ) _DataStack_Pop ( ) ;
    Word * word = ( Word* ) TOS ; // leave word on tos for anticipated further processing
    _Word_DefinitionStore ( word, b ) ;
}

void
CfrTil_Word_Run ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Word_Run ( word ) ;
}

void
Word_CodeStart ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int64 ) word->CodeStart ) ;
}

void
Word_CodeSize ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int64 ) word->S_CodeSize ) ;
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
    byte * name = Lexer_ReadToken ( _Context_->Lexer0 ) ;
    return name ;
}

void
Word_Add ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Word_Add ( word, 1, 0 ) ;
}

void
CfrTil_Word_New ( )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int64 ) Word_New ( name ) ) ;
}

// ( token block -- word )
// postfix 'word' takes a token and a block

void
CfrTil_Word ( )
{
    block b = ( block ) _DataStack_Pop ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataObject_New ( CFRTIL_WORD, 0, name, 0, 0, 0, ( int64 ) b, 0 ) ;
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
    LC_CompileRun_C_ArgList ( ( Word * ) _DataStack_Pop ( ) ) ;
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
    _DataStack_Push ( ( int64 ) word->Name ) ;
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
    _DataStack_Push ( ( int64 ) _Word_Namespace ( word ) ) ;
}

void
CfrTil_Keyword ( void )
{
    if ( _CfrTil_->LastFinishedWord ) _CfrTil_->LastFinishedWord->CProperty |= KEYWORD ;
}

void
CfrTil_Immediate ( void )
{
    if ( _CfrTil_->LastFinishedWord ) _CfrTil_->LastFinishedWord->CProperty |= IMMEDIATE ;
}

void
CfrTil_IsImmediate ( void )
{
#if 0    
    Word * word = ( Word* ) TOS ;
    TOS = ( word->CProperty & IMMEDIATE ) ;
#else
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( word->CProperty & IMMEDIATE ) ;
#endif    
}

void
CfrTil_Inline ( void )
{
    if ( _CfrTil_->LastFinishedWord ) _CfrTil_->LastFinishedWord->CProperty |= INLINE ;
}

void
CfrTil_Prefix ( void )
{
    if ( _CfrTil_->LastFinishedWord )
    {
        //_CfrTil_->LastFinishedWord->CProperty |= PREFIX ;
        _CfrTil_->LastFinishedWord->WProperty = WT_PREFIX ;
    }
}

void
CfrTil_C_Prefix ( void )
{
    if ( _CfrTil_->LastFinishedWord )
    {
        _CfrTil_->LastFinishedWord->CProperty |= C_PREFIX | C_PREFIX_RTL_ARGS ;
        _CfrTil_->LastFinishedWord->WProperty = WT_C_PREFIX_RTL_ARGS ;
    }
}

void
CfrTil_C_Return ( void )
{
    if ( _CfrTil_->LastFinishedWord )
    {
        _CfrTil_->LastFinishedWord->CProperty |= C_RETURN | C_PREFIX_RTL_ARGS ;
        _CfrTil_->LastFinishedWord->WProperty = WT_C_PREFIX_RTL_ARGS ;
    }
}

void
CfrTil_Void_Return ( void )
{
    if ( _CfrTil_->LastFinishedWord )
    {
        _CfrTil_->LastFinishedWord->CProperty &= ~ C_RETURN ;
        _CfrTil_->LastFinishedWord->CProperty |= VOID_RETURN ;
        if ( GetState ( _Context_, C_SYNTAX ) )
        {
            _CfrTil_->LastFinishedWord->CProperty |= C_PREFIX_RTL_ARGS ;
            _CfrTil_->LastFinishedWord->WProperty = WT_C_PREFIX_RTL_ARGS ;
        }
    }
}

void
CfrTil_EAX_Return ( void )
{
    if ( _CfrTil_->LastFinishedWord )
    {
        _CfrTil_->LastFinishedWord->CProperty &= ~ C_RETURN ;
        _CfrTil_->LastFinishedWord->CProperty2 |= EAX_RETURN ;
    }
}

void
CfrTil_DebugWord ( void )
{
    if ( _CfrTil_->LastFinishedWord ) _CfrTil_->LastFinishedWord->CProperty |= DEBUG_WORD ;
}

void
_PrintWord ( dlnode * node, int64 * n )
{
    Word * word = ( Word * ) node ;
    _Word_Print ( word ) ;
    ( *n ) ++ ;
}

void
_Words ( Symbol * symbol, MapFunction1 mf, int64 n )
{
    Namespace * ns = ( Namespace * ) symbol ;
    _Printf ( ( byte* ) "\n - %s :> ", ns->Name ) ;
    dllist_Map1 ( ns->Lo_List, mf, n ) ;
}

void
_DoWords ( Symbol * symbol, int64 * n )
{
    _Words ( symbol, ( MapFunction1 ) _PrintWord, ( int64 ) n ) ;
}

int64
_CfrTil_PrintWords ( int64 state )
{
    int64 n = 0 ;
    _CfrTil_NamespacesMap ( ( MapSymbolFunction2 ) _DoWords, state, ( int64 ) & n, 0 ) ;
    if ( _Q_->Verbosity > 3 ) _Printf ( ( byte* ) "\nCfrTil : WordsAdded = %d : WordMaxCount = %d", _CfrTil_->WordsAdded, _CfrTil_->FindWordMaxCount ) ;
    return n ;
}

void
CfrTil_Words ( )
{
    _Printf ( ( byte* ) "\nWords :\n - <namespace> ':>' <word list>" ) ;
    int64 n = _CfrTil_PrintWords ( USING ) ;
    _Printf ( ( byte* ) "\n" INT_FRMT " words on the 'using' Namespaces List ::", n ) ;
    if ( _Q_->Verbosity > 3 ) _Printf ( ( byte* ) "\nCfrTil : WordsAdded = %d : WordMaxCount = %d", _CfrTil_->WordsAdded, _CfrTil_->FindWordMaxCount ) ;
}

void
_Variable_Print ( Word * word )
{
    _Printf ( ( byte* ) c_ud ( " %s = %x ;" ), word->Name, word->W_Value ) ;
}

void
_PrintVariable ( dlnode * node, int64 * n )
{
    Word * word = ( Word * ) node ;
    if ( word->CProperty & NAMESPACE_VARIABLE )
    {
        _Variable_Print ( word ) ;
        ( *n ) ++ ;
    }
}

void
_Variables ( Symbol * symbol, MapFunction1 mf, int64 n )
{
    int64 pre_n = * ( int64* ) n ;
    Namespace * ns = ( Namespace * ) symbol ;
    _Printf ( ( byte* ) "\n - %s :> ", ns->Name ) ;
    dllist_Map1 ( ns->Lo_List, mf, n ) ;
    if ( *( int64* ) n == pre_n ) _Printf ( ( byte* ) "\r" ) ;
}

void
_PrintVariables ( Symbol * symbol, int64 * n )
{
    _Variables ( symbol, ( MapFunction1 ) _PrintVariable, ( int64 ) n ) ;
}

int64
_CfrTil_PrintVariables ( int64 nsStatus )
{
    int64 n = 0 ;
    _CfrTil_NamespacesMap ( ( MapSymbolFunction2 ) _PrintVariables, nsStatus, ( int64 ) & n, 0 ) ;
    return n ;
}

void
CfrTil_Variables ( )
{
    _Printf ( ( byte* ) "\nGlobal Variables :\n - <namespace> ':>' <variable '=' value ';'>*" ) ;
    int64 n = _CfrTil_PrintVariables ( USING ) ;
    _Printf ( ( byte* ) "\n" INT_FRMT " global variables on the 'using' Namespaces List", n ) ;
}

void
_CfrTil_NamespaceWords ( )
{
    int64 n = 0 ;
    Namespace * ns = ( Namespace * ) _DataStack_Pop ( ) ;
    if ( ns )
    {
        _DoWords ( ( Symbol * ) ns, &n ) ;
        _Printf ( ( byte* ) "\n" INT_FRMT " words in %s namespace", n, ns->Name ) ;
    }
    else _Printf ( ( byte* ) "\nError : can't find that namespace" ) ;
}

void
CfrTil_NamespaceWords ( )
{
    byte * name = ( byte * ) _DataStack_Pop ( ) ;
    Namespace * ns = _Namespace_Find ( name, 0, 0 ) ;
    _DataStack_Push ( ( int64 ) ns ) ;
    _CfrTil_NamespaceWords ( ) ;
}

void
CfrTil_AllWords ( )
{
    _Printf ( ( byte* ) "\n - <namespace> ':>' <word list>" ) ;
    _Printf ( ( byte* ) "\n'using' Namespaces List ::" ) ;
    int64 n = _CfrTil_PrintWords ( USING ) ;
    _Printf ( ( byte* ) "\n" INT_FRMT " words on the Currently 'using' Namespaces List", n ) ;
    _Printf ( ( byte* ) "\n'notUsing' Namespaces List ::" ) ;
    int64 usingWords = _CfrTil_->FindWordCount ;
    int64 m = _CfrTil_PrintWords ( NOT_USING ) ;
    _Printf ( ( byte* ) "\n" INT_FRMT " words on the 'notUsing' List", m ) ;
    _Printf ( ( byte* ) "\n" INT_FRMT " total words", n + m ) ;
    int64 notUsingWords = _CfrTil_->FindWordCount ;
    _CfrTil_->FindWordCount = usingWords + notUsingWords ;
    CfrTil_WordAccounting ( "CfrTil_AllWords" ) ;
}

