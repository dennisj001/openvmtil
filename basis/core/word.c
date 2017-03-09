#include "../../include/cfrtil.h"

void
Word_PrintOffset ( Word * word, int32 increment, int32 totalIncrement )
{
    Context * cntx = _Context_ ;
    if ( Is_DebugOn ) NoticeColors ;
    byte * name = String_ConvertToBackSlash ( word->Name ) ;
    if ( String_Equal ( "]", name ) )
    {
        _Printf ( ( byte* ) "\n\'%s\' = array end :: base object \'%s\' : increment = %d : total totalIncrement = %d", name,
            cntx->Interpreter0->BaseObject->Name, increment, totalIncrement ) ;
    }
    else
    {
        _Printf ( ( byte* ) "\n\'%s\' = object field :: type = %s : size = %d : base object \'%s\' : offset = %d : total offset = %d", name,
            word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "",
            TypeNamespace_Get ( word ) ? ( int32 ) _CfrTil_VariableValueGet ( TypeNamespace_Get ( word )->Name, ( byte* ) "size" ) : 0,
            cntx->Interpreter0->BaseObject ? String_ConvertToBackSlash ( cntx->Interpreter0->BaseObject->Name ) : ( byte* ) "",
            word->Offset, cntx->Compiler0->AccumulatedOptimizeOffsetPointer ? *cntx->Compiler0->AccumulatedOptimizeOffsetPointer : - 1 ) ;
    }
    if ( Is_DebugOn ) DefaultColors ;
}

void
_Word_Location_Printf ( Word * word )
{
    if ( word ) _Printf ( ( byte* ) "\n%s.%s : %s %d.%d", word->ContainingNamespace->Name, word->Name, word->S_WordData->Filename, word->S_WordData->LineNumber, word->W_CursorPosition ) ;
}

byte *
_Word_Location_pbyte ( Word * word )
{
    //Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
    byte * b = Buffer_Data ( _CfrTil_->ScratchB2 ) ;
    if ( word ) sprintf ( ( char* ) b, "%s.%s : %s %d.%d", word->ContainingNamespace->Name, word->Name, word->S_WordData->Filename, word->S_WordData->LineNumber, word->W_CursorPosition ) ;
    return b ;
}

void
Word_PrintName ( Word * word )
{
    if ( word ) _Printf ( ( byte* ) "%s ", word->Name ) ;
}

void
_Word_Print ( Word * word )
{
    _Context_->WordCount ++ ;
    _Printf ( ( byte* ) c_ud ( " %s" ), word->Name ) ;
}

void
__Word_ShowSourceCode ( Word * word )
{
    if ( word && word->S_WordData && word->SourceCode ) //word->CProperty & ( CPRIMITIVE | BLOCK ) )
    {
#if 1        
        Buffer *dstb = Buffer_NewLocked ( BUFFER_SIZE ) ;
        byte * dst = dstb->B_Data ;
        _String_ConvertStringToBackSlash ( dst, word->SourceCode ) ;
        byte * name = c_dd ( word->Name ), *dest = c_dd ( String_FilterMultipleSpaces ( dst, TEMPORARY ) ) ;
        _Printf ( ( byte* ) "\nSourceCode for ""%s"" :> \n%s", name, dest ) ;
        Buffer_Unlock ( dstb ) ;
        Buffer_SetAsUnused ( dstb ) ;
#else
        _Printf ( ( byte* ) "\nSourceCode for ""%s"" :> \n%s", c_dd ( word->Name ), c_dd ( word->SourceCode ) ) ;
#endif        
    }
}

void
_Word_ShowSourceCode ( Word * word )
{
    _CfrTil_Source ( word, 0 ) ;
}

Word *
Word_GetFromCodeAddress ( byte * address )
{
    return Finder_Address_FindAny ( _Context_->Finder0, address ) ;
}

Word *
Word_GetFromCodeAddress_NoAlias ( byte * address )
{
    return Finder_Address_FindAny_NoAlias ( _Context_->Finder0, address ) ;
}

void
_CfrTil_WordName_Run ( byte * name )
{
    _Block_Eval ( Finder_Word_FindUsing ( _Context_->Finder0, name, 0 )->Definition ) ;
}

void
_Word_Compile ( Word * word )
{
    Set_SCA ( 0 ) ;
    if ( ! word->Definition )
    {
        CfrTil_SetupRecursiveCall ( ) ;
    }
    else if ( ( GetState ( _CfrTil_, INLINE_ON ) ) && ( word->CProperty & INLINE ) && ( word->S_CodeSize ) )
    {
        _Compile_WordInline ( word ) ;
    }
    else
    {
        Compile_Call ( ( byte* ) word->Definition ) ;
    }
}

void
_Word_Run ( Word * word )
{
    if ( ! sigsetjmp ( _Context_->JmpBuf0, 0 ) )
    {
        CfrTil_Set_DebugSourceCodeIndex ( word ) ;
        _Context_->CurrentlyRunningWord = word ;
        _Block_Eval ( word->Definition ) ;
    }
}

void
_Word_Eval_Debug ( Word * word )
{
    if ( word )
    {
        Set_SCA ( 0 ) ;
        DEBUG_SETUP ( word ) ;
        //if ( ! ( GetState ( word, STEPPED ) ) )
        {
            //SetState ( word, STEPPED, false ) ;
            word->Coding = Here ;
            if ( ( word->CProperty & IMMEDIATE ) || ( ! CompileMode ) )
            {
                _Word_Run ( word ) ;
            }
            else
            {
                _Word_Compile ( word ) ;
            }
        }
        //SetState ( word, STEPPED, false ) ; // clear the state
        DEBUG_SHOW ;
    }
}

void
_Word_Eval ( Word * word )
{
    if ( word )
    {
        if ( word->CProperty & DEBUG_WORD ) DebugColors ;
        _Context_->CurrentlyRunningWord = word ;
        word->StackPushRegisterCode = 0 ; // nb. used! by the rewriting optInfo
        // keep track in the word itself where the machine code is to go, if this word is compiled or causes compiling code - used for optimization
        word->Coding = Here ;
        _Word_Eval_Debug ( word ) ;
        if ( word->CProperty & DEBUG_WORD ) DefaultColors ; // reset colors after a debug word
    }
}

void
_Word_Interpret ( Word * word )
{
    CfrTil_Set_DebugSourceCodeIndex ( word ) ;
    _Interpreter_DoWord ( _Interpreter_, word, - 1 ) ;
}

Namespace *
_Word_Namespace ( Word * word )
{
    if ( word->CProperty & NAMESPACE ) return ( Namespace * ) word ;
    else return word->ContainingNamespace ;
}

void
_CfrTil_AddSymbol ( Symbol * symbol )
{
    _Namespace_DoAddSymbol ( _CfrTil_Namespace_InNamespaceGet ( ), symbol ) ;
}

void
_CfrTil_AddWord ( Word * word )
{
    Namespace_AddWord ( _CfrTil_Namespace_InNamespaceGet ( ), word ) ;
}

Word *
_Word_Allocate ( uint32 allocType )
{
    Word * word ;
    if ( allocType & ( COMPILER_TEMP | LISP_TEMP ) ) allocType = TEMPORARY ;
    else allocType = DICTIONARY ;
    word = ( Word* ) Mem_Allocate ( sizeof ( Word ) + sizeof ( WordData ), allocType ) ;
    word->S_WordData = ( WordData * ) ( word + 1 ) ; // nb. "pointer arithmetic"
    return word ;
}

// deep copy from word0 to word

void
_Word_Copy ( Word * word, Word * word0 )
{
    WordData * swdata = word->S_WordData ;
    memcpy ( word, word0, sizeof (Word ) ) ;
    word->S_WordData = swdata ; // restore the WordData pointer we overwrote by the above memcpy
    memcpy ( word->S_WordData, word0->S_WordData, sizeof (WordData ) ) ;
}

Word *
Word_Copy ( Word * word0, uint32 allocType )
{
    Word * word = _Word_Allocate ( allocType ) ;
    _Word_Copy ( word, word0 ) ;
    return word ;
}

void
_Word_Finish ( Word * word )
{
    _DObject_Finish ( word ) ;
    _CfrTil_FinishSourceCode ( _CfrTil_, word ) ;
    Compiler_Init ( _Context_->Compiler0, 0 ) ; // not really necessary should always be handled by EndBlock ?? but this allows for some syntax errors with a '{' but no '}' ??
}

void
_Word_DefinitionStore ( Word * word, block code )
{
    _DObject_ValueDefinition_Init ( word, ( int32 ) code, BLOCK, 0, 0 ) ;
}

void
_Word_InitFinal ( Word * word, byte * code )
{
    _Word_DefinitionStore ( word, ( block ) code ) ;
    if ( ! word->S_ContainingNamespace ) _Word_Add ( word, 1, 0 ) ; // don't re-add if it is a recursive word cf. CfrTil_BeginRecursiveWord
    _Word_Finish ( word ) ;
}

#if 0

void
_Word_Add ( Word * word, int32 addToInNs, Namespace * addToNs )
{
    uint64 ctype = word->CProperty ;
    Namespace * ins = ( addToInNs && ( ! ( word->CProperty & ( LITERAL ) ) ) ) ? _CfrTil_Namespace_InNamespaceGet ( ) : 0 ;
    if ( ins ) _Namespace_DoAddWord ( ins, word ) ;
    else if ( addToNs ) _Namespace_DoAddWord ( addToNs, word ) ;
    if ( addToInNs && ( ! CompileMode ) && ( _Q_->Verbosity > 2 ) && ( ! ( ctype & ( SESSION | LOCAL_VARIABLE | PARAMETER_VARIABLE ) ) ) )
    {
        if ( ctype & BLOCK ) _Printf ( ( byte* ) "\nnew Word :: %s.%s\n", ins->Name, word->Name ) ;
        else _Printf ( ( byte* ) "\nnew DObject :: %s.%s\n", ins->Name, word->Name ) ;
    }
}
#else

void
_Word_Add ( Word * word, int32 addToInNs, Namespace * addToNs )
{
    Namespace * ins, *ns ;
    if ( addToInNs || ins )
    {
        if ( addToNs ) _Namespace_DoAddWord ( addToNs, word ) ;
        else if ( addToInNs )
        {
            ins = ( addToInNs && ( ! ( word->CProperty & ( LITERAL ) ) ) ) ? _CfrTil_Namespace_InNamespaceGet ( ) : 0 ;
            if ( ins ) _Namespace_DoAddWord ( ins, word ) ;
        }
        if ( _Q_->Verbosity > 2 ) // ( ! CompileMode ) && ( ! ( word->CProperty & ( SESSION | LOCAL_VARIABLE | PARAMETER_VARIABLE ) ) ) )
        {
            ns = addToNs ? addToNs : ins ;
            if ( ns )
            {
                if ( word->CProperty & BLOCK ) _Printf ( ( byte* ) "\nnew Word :: %s.%s\n", ns->Name, word->Name ) ;
                else _Printf ( ( byte* ) "\nnew DObject :: %s.%s\n", ns->Name, word->Name ) ;
            }
        }
    }
}
#endif

Word *
_Word_New ( byte * name, uint64 ctype, uint64 ltype, uint32 allocType )
{
    Word * word = _Word_Allocate ( allocType ? allocType : DICTIONARY ) ;
    //if ( ! ( allocType & EXISTING ) ) _Symbol_Init_AllocName ( ( Symbol* ) word, name, OBJECT_MEMORY ) ;
    if ( allocType & ( EXISTING ) ) _Symbol_NameInit ( ( Symbol * ) word, name ) ;
    else _Symbol_Init_AllocName ( ( Symbol* ) word, name, STRING_MEM ) ;
    word->CProperty = ctype ;
    word->LProperty = ltype ;
    if ( Is_NamespaceType ( word ) ) word->Lo_List = dllist_New ( ) ;
    return word ;
}

Word *
_Word_Create ( byte * name, uint64 ctype, uint64 ltype, uint32 allocType )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    Word * word = _Word_New ( name, ctype, ltype, allocType ) ; // CFRTIL_WORD : cfrTil compiled words as opposed to C compiled words
    if ( rl->InputStringOriginal )
    {
        word->S_WordData->Filename = rl->Filename ;
        word->S_WordData->LineNumber = rl->LineNumber ;
        word->W_CursorPosition = rl->CursorPosition ;
    }
    return word ;
}

Word *
Word_Create ( byte * name )
{
    Word * word = _Word_Create ( name, CFRTIL_WORD | WORD_CREATE, 0, DICTIONARY ) ;
    _Context_->Compiler0->CurrentWord = word ;
    if ( GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) )
    {
        word->DebugWordList = _dllist_New ( DICTIONARY ) ;
        _CfrTil_->DebugWordList = word->DebugWordList ;
    }
    _Word_Add ( word, 1, 0 ) ;
    return word ;
}

// alias : postfix

Word *
_CfrTil_Alias ( Word * word, byte * name )
{
    Word * alias = _Word_Create ( name, word->CProperty | ALIAS, word->LProperty, DICTIONARY ) ; // inherit type from original word
    while ( ( ! word->Definition ) && word->W_AliasOf ) word = word->W_AliasOf ;
    _Word_InitFinal ( alias, ( byte* ) word->Definition ) ;
    alias->S_CodeSize = word->S_CodeSize ;
    alias->W_AliasOf = word ;
    return alias ;
}

void
Do_TextMacro ( )
{
    Interpreter * interp = _Context_->Interpreter0 ;
    ReadLiner * rl = _Context_->ReadLiner0 ;
    ReadLiner_InsertTextMacro ( rl, interp->w_Word ) ;
    SetState ( interp, END_OF_LINE | END_OF_FILE | END_OF_STRING | DONE, false ) ; // reset a possible read newline
}

void
Do_StringMacro ( )
{
    Interpreter * interp = _Context_->Interpreter0 ;
    ReadLiner * rl = _Context_->ReadLiner0 ;
    String_InsertDataIntoStringSlot ( rl->InputLine, rl->ReadIndex, rl->ReadIndex, _String_UnBox ( ( byte* ) interp->w_Word->W_Value ) ) ; // size in bytes
    SetState ( interp, END_OF_LINE | END_OF_FILE | END_OF_STRING | DONE, false ) ; // reset a possible read newline
}

void
_CfrTil_Macro ( int64 mtype, byte * function )
{
    byte * name = _Word_Begin ( ), *macroString ;
    macroString = Parse_Macro ( mtype ) ;
    byte * code = String_New ( macroString, STRING_MEM ) ;
    //_DObject_New ( byte * name, uint32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, uint32 allocType )
    _DObject_New ( name, ( uint32 ) code, IMMEDIATE, 0, mtype, function, 0, 1, 0, DICTIONARY ) ;
}

Word *
Word_GetOriginalWord ( Word * word )
{
    Word * ow1, *ow0 ;
    for ( ow0 = word, ow1 = ow0->W_OriginalWord ; ow1 && ( ow1 != ow1->W_OriginalWord ) ; ow0 = ow1, ow1 = ow0->W_OriginalWord ) ;
    if ( ! ow0 ) ow0 = word ;
    return ow0 ;
}


