#include "../../includes/cfrtil.h"

void
Word_PrintOffset ( Word * word, int32 increment, int32 totalIncrement )
{
    Context * cntx = _Q_->OVT_Context ;
    if ( IsDebugOn ) NoticeColors ;
    byte * name = String_ConvertToBackSlash ( word->Name ) ;
    if ( String_Equal ( "]", name ) )
    {
        Printf ( ( byte* ) "\n\'%s\' = array end :: base object \'%s\' : increment = %d : total totalIncrement = %d", name,
            cntx->Interpreter0->BaseObject->Name, increment, totalIncrement ) ;
    }
    else
    {
        Printf ( ( byte* ) "\n\'%s\' = object field :: type = %s : size = %d : base object \'%s\' : offset = %d : total offset = %d", name,
            word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "",
            TypeNamespace_Get ( word ) ? ( int32 ) _CfrTil_VariableValueGet ( TypeNamespace_Get ( word )->Name, ( byte* ) "size" ) : 0,
            cntx->Interpreter0->BaseObject ? String_ConvertToBackSlash ( cntx->Interpreter0->BaseObject->Name ) : ( byte* ) "",
            word->Offset, cntx->Compiler0->AccumulatedOptimizeOffsetPointer ? *cntx->Compiler0->AccumulatedOptimizeOffsetPointer : - 1 ) ;
    }
    if ( IsDebugOn ) DefaultColors ;
}

void
_Word_Location_Printf ( Word * word )
{
    if ( word ) Printf ( ( byte* ) "\n%s.%s : %s %d.%d", word->ContainingNamespace->Name, word->Name, word->S_WordData->Filename, word->S_WordData->LineNumber, word->W_CursorPosition ) ;
}

byte *
_Word_Location_pbyte ( Word * word )
{
    Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
    byte * b = Buffer_Data ( buffer ) ;
    if ( word ) sprintf ( ( char* ) b, "%s.%s : %s %d.%d", word->ContainingNamespace->Name, word->Name, word->S_WordData->Filename, word->S_WordData->LineNumber, word->W_CursorPosition ) ;
    return b ;
}

void
Word_PrintName ( Word * word )
{
    if ( word ) Printf ( ( byte* ) "%s ", word->Name ) ;
}

void
_Word_Print ( Word * word )
{
    _Q_->OVT_Context->WordCount ++ ;
    Printf ( ( byte* ) c_ud ( " %s" ), word->Name ) ;
}

void
__Word_ShowSourceCode ( Word * word )
{
    if ( word && word->S_WordData && word->SourceCode ) //word->CType & ( CPRIMITIVE | BLOCK ) )
    {
#if 1        
        Buffer *dstb = Buffer_NewLocked ( BUFFER_SIZE ) ;
        byte * dst = dstb->B_Data ;
        _String_ConvertStringToBackSlash ( dst, word->SourceCode ) ;
        byte * name = c_dd ( word->Name ), *dest = c_dd ( String_FilterForHistory ( dst ) ) ;
        Printf ( ( byte* ) "\nSourceCode for ""%s"" :> \n%s", name, dest ) ;
        Buffer_Unlock ( dstb ) ;
        Buffer_SetAsUnused ( dstb ) ;
#else
        Printf ( ( byte* ) "\nSourceCode for ""%s"" :> \n%s", c_dd ( word->Name ), c_dd ( word->SourceCode ) ) ;
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
    return Finder_Address_FindAny ( _Q_->OVT_Context->Finder0, address ) ;
}

Word *
Word_GetFromCodeAddress_NoAlias ( byte * address )
{
    return Finder_Address_FindAny_NoAlias ( _Q_->OVT_Context->Finder0, address ) ;
}

void
_CfrTil_WordName_Run ( byte * name )
{
    _Block_Eval ( Finder_Word_FindUsing ( _Q_->OVT_Context->Finder0, name, 0 )->Definition ) ;
}

void
_Word_Run ( Word * word )
{
    _Q_->OVT_Context->CurrentRunWord = word ;
    if ( ! setjmp ( _Q_->OVT_Context->JmpBuf0 ) ) // for CfrTil_DebugRuntimeBreakpoint
    {
        word->Definition ( ) ;
    }
}

void
_Word_Eval ( Word * word )
{
    if ( word )
    {
        if ( word->CType & DEBUG_WORD ) DebugColors ;
        byte * token = word->Name ; // necessary declaration for DEBUG_START, DEBUG_SHOW.
        _Q_->OVT_Context->CurrentRunWord = word ;
        if ( ! GetState ( _Q_->OVT_Context->Compiler0, LC_ARG_PARSING | PREFIX_ARG_PARSING | PREFIX_PARSING ) ) word->W_StartCharRlIndex = _Q_->OVT_Context->Lexer0->TokenStart_ReadLineIndex ;
        word->StackPushRegisterCode = 0 ; // nb. used! by the rewriting optimizer
        // keep track in the word itself where the machine code is to go if this word is compiled or causes compiling code - used for optimization
        word->Coding = Here ;
        DEBUG_START ;
        if ( ! GetState ( debugger, DBG_STEPPED ) )
        {
            if ( ( word->CType & IMMEDIATE ) || ( ! CompileMode ) )
            {
                _Word_Run ( word ) ;
            }
            else
            {
                _CompileWord ( word ) ;
            }
        }
        else SetState ( debugger, DBG_STEPPED, false ) ;
        DEBUG_SHOW ;
        if ( word->CType & DEBUG_WORD ) DefaultColors ; // reset colors after a debug word
    }
}

Namespace *
_Word_Namespace ( Word * word )
{
    if ( word->CType & NAMESPACE ) return ( Namespace * ) word ;
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
    if ( allocType & SESSION ) allocType = SESSION ;
    else if ( allocType & LISP_TEMP ) allocType = LISP_TEMP ;
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

Word *
_Word_Init ( Word * word, uint64 ctype, uint64 ltype )
{
    word->CType = ctype ;
    word->LType = ltype ;
    if ( Is_NamespaceType ( word ) ) word->Lo_List = DLList_New ( ) ;
    return word ;
}

Word *
_Word_New ( byte * name, uint64 ctype, uint64 ltype, uint32 allocType )
{
    Word * word = _Word_Allocate ( allocType ? allocType : DICTIONARY ) ;
    if ( ! ( allocType & EXISTING ) ) _Symbol_Init_AllocName ( ( Symbol* ) word, name, DICTIONARY ) ;
    else _Symbol_NameInit ( ( Symbol * ) word, name ) ;
    _Word_Init ( word, ctype, ltype ) ;
    return word ;
}

void
_Word_Finish ( Word * word )
{
    _DObject_Finish ( word ) ;
    _CfrTil_FinishSourceCode ( word ) ;
    Compiler_Init ( _Q_->OVT_Context->Compiler0, 0 ) ;
}

void
_Word_Add ( Word * word, int32 addToInNs, Namespace * addToNs )
{
    uint64 ctype = word->CType ;
    Namespace * ins = addToInNs ? _CfrTil_Namespace_InNamespaceGet ( ) : 0 ;
    if ( ins ) _Namespace_DoAddWord ( ins, word ) ;
    else if ( addToNs ) _Namespace_DoAddWord ( addToNs, word ) ;
        //else if ( ctype & LITERAL ) _Namespace_DoAddWord ( Namespace_FindOrNew_SetUsing ( ( byte* ) "Literals", 0, 0 ), word ) ;
#if 0    
    else if ( Is_NamespaceType ( word ) && _Q_->OVT_CfrTil->Namespaces )
    {
        _Namespace_DoAddWord ( _Q_->OVT_CfrTil->Namespaces, word ) ; // nb! namespaces are all added to _Q_->OVT_CfrTil->Namespaces list but their ContainingNamespace is set accurately
        if ( addToNs ) word->ContainingNamespace = addToNs ;
        else if ( addToInNs ) word->ContainingNamespace = ins ;
    }
#endif    
    if ( addToInNs && ( ! CompileMode ) && ( _Q_->Verbosity > 2 ) && ( ! ( ctype & ( SESSION | LOCAL_VARIABLE | PARAMETER_VARIABLE ) ) ) )
    {
        if ( ctype & BLOCK ) Printf ( ( byte* ) "\nnew Word :: %s.%s\n", ins->Name, word->Name ) ;
        else Printf ( ( byte* ) "\nnew DObject :: %s.%s\n", ins->Name, word->Name ) ;
    }
}

void
_Word_DefinitionStore ( Word * word, block code )
{
    _DObject_ValueDefinition_Init ( word, ( int32 ) code, word->CType | BLOCK, word->LType | BLOCK, 0, 0 ) ;
}

void
_Word ( Word * word, byte * code )
{
    _Word_DefinitionStore ( word, ( block ) code ) ;
    if ( ! word->S_ContainingNamespace ) _Word_Add ( word, 1, 0 ) ; // don't re-add if it is a recursive word cf. CfrTil_BeginRecursiveWord
    _Word_Finish ( word ) ;
}

Word *
_Word_Create ( byte * name, uint64 ctype, uint64 ltype, uint32 allocType )
{
    ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
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
    //Compiler_Init ( _Q_->OVT_Context->Compiler0, 0 ) ;
    Word * word = _Word_Create ( name, CFRTIL_WORD | WORD_CREATE, 0, DICTIONARY ) ;
    _Word_Add ( word, 1, 0 ) ;
    return word ;
}

// alias : postfix

Word *
_CfrTil_Alias ( Word * word, byte * name )
{
    Word * alias = _Word_Create ( name, word->CType | ALIAS, word->LType, DICTIONARY ) ; // inherit type from original word
    while ( ( ! word->Definition ) && word->AliasOf ) word = word->AliasOf ;
    _Word ( alias, ( byte* ) word->Definition ) ;
    alias->S_CodeSize = word->S_CodeSize ;
    alias->AliasOf = word ;
    return alias ;
}

void
Do_TextMacro ( )
{
    Interpreter * interp = _Q_->OVT_Context->Interpreter0 ;
    ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
    ReadLiner_InsertTextMacro ( rl, interp->w_Word ) ;
    Interpreter_SetState ( interp, END_OF_LINE | END_OF_FILE | END_OF_STRING | DONE, false ) ; // reset a possible read newline
}

void
Do_StringMacro ( )
{
    Interpreter * interp = _Q_->OVT_Context->Interpreter0 ;
    ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
    String_InsertDataIntoStringSlot ( rl->InputLine, rl->ReadIndex, rl->ReadIndex, _String_UnBox ( ( byte* ) interp->w_Word->W_Value, 0 ) ) ; // size in bytes
    Interpreter_SetState ( interp, END_OF_LINE | END_OF_FILE | END_OF_STRING | DONE, false ) ; // reset a possible read newline
}

void
_CfrTil_Macro ( int64 mtype, byte * function )
{
    byte * name = _Word_Begin ( ), *macroString ;
    macroString = Parse_Macro ( mtype ) ;
    byte * code = String_New ( macroString, DICTIONARY ) ;
    // untested 
#if 0    
    Word * macro = _Word_Create ( name, mtype | IMMEDIATE, 0, DICTIONARY ) ;
    _DObject_ValueDefinition_Init ( macro, ( int32 ) code, IMMEDIATE, mtype, function, 0 ) ;
    _Word_Finish ( macro ) ;
#else
    //_DObject_New ( byte * name, uint32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, uint32 allocType )
    _DObject_New ( name, ( uint32 ) code, IMMEDIATE, 0, mtype, function, 0, 1, 0, DICTIONARY ) ;
#endif    
}

