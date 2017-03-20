
#include "../include/cfrtil.h"

void
_CfrTil_Run ( CfrTil * cfrTil, int32 restartCondition )
{
    while ( 1 )
    {
        cfrTil = _CfrTil_New ( cfrTil ) ;
        if ( ! sigsetjmp ( cfrTil->JmpBuf0, 0 ) )
        {
            System_RunInit ( _Context_->System0 ) ;
            _CfrTil_ReStart ( cfrTil, restartCondition ) ;
            // check if reset is ok ...
            if ( cfrTil && _Context_ && _Context_->System0 )
            {
                DebugOff ;
                CfrTil_C_Syntax_Off ( ) ;
                Ovt_RunInit ( _Q_ ) ;
                CfrTil_InterpreterRun ( ) ;
                d0 ( Pause ( "\n_CfrTil_Run : ??shouldn't reach here??" ) ; ) ; // shouldn't reach here
            }
        }
    }
}

void
_CfrTil_ReStart ( CfrTil * cfrTil, int32 restartCondition )
{
    switch ( restartCondition )
    {
        case 0:
        case INITIAL_START:
        case FULL_RESTART:
        case RESTART:
        case RESET_ALL:
        {
            CfrTil_ResetAll_Init ( cfrTil ) ;
        }
        case ABORT: CfrTil_SetStackPointerFromDsp ( cfrTil ) ;
        default:
        case QUIT:
        case STOP: ;
    }
}

void
_CfrTil_CpuState_Show ( )
{
    _CpuState_Show ( _CfrTil_->cs_CpuState ) ;
    _Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_CpuState_Show ( )
{
    _CfrTil_->SaveCpuState ( ) ;
    _CfrTil_CpuState_Show ( ) ;
}

void
_CfrTil_NamespacesInit ( CfrTil * cfrTil )
{
    Namespace * ns = _DataObject_New ( NAMESPACE, 0, ( byte* ) "Namespaces", 0, 0, 0, 0, 0 ) ;
    ns->State |= USING ; // nb. _Namespace_SetState ( ns, USING ) ; // !! can't be used with "Namespaces"
    cfrTil->Namespaces = ns ;
    CfrTil_AddCPrimitives ( ) ;
}

void
_CfrTil_DataStack_Init ( CfrTil * cfrTil )
{
    CfrTil_DataStack_InitEssential ( cfrTil ) ;
    if ( _Q_->Verbosity > 2 ) _Printf ( ( byte* ) "\nData Stack reset." ) ;
}

void
CfrTil_DataStack_Init ( )
{
    _CfrTil_DataStack_Init ( _CfrTil_ ) ;
}

void
_CfrTil_Init ( CfrTil * cfrTil, Namespace * nss )
{
    uint32 allocType = CFRTIL ;
    _CfrTil_ = cfrTil ;
    // TODO : organize these buffers and their use 
    cfrTil->OriginalInputLineB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->InputLineB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->SourceCodeSPB = _Buffer_NewPermanent ( SOURCE_CODE_BUFFER_SIZE ) ;
    cfrTil->LC_PrintB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->LC_DefineB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->TokenB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    //cfrTil->PrintfB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->ScratchB1 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->ScratchB2 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->DebugB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->DebugB1 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->DebugB2 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringInsertB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringInsertB2 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringInsertB3 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->TabCompletionBuf = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringMacroB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StrCatBuffer = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->OriginalInputLine = Buffer_Data ( cfrTil->OriginalInputLineB ) ;
    cfrTil->SourceCodeScratchPad = Buffer_Data ( cfrTil->SourceCodeSPB ) ;
    cfrTil->LispPrintBuffer = Buffer_Data ( cfrTil->LC_PrintB ) ;
    //cfrTil->LispDefineBuffer = Buffer_Data ( cfrTil->LC_DefineB ) ;
    cfrTil->TokenBuffer = Buffer_Data ( cfrTil->TokenB ) ;
    SetState ( cfrTil, CFRTIL_RUN | OPTIMIZE_ON | INLINE_ON, true ) ;

    if ( _Q_->Verbosity > 2 ) _Printf ( ( byte* ) "\nSystem Memory is being reallocated.  " ) ;

    cfrTil->ContextStack = Stack_New ( 256, allocType ) ;
    cfrTil->ObjectStack = Stack_New ( 1 * K, allocType ) ;
    //cfrTil->DebugStateStack = Stack_New ( 1 * K, allocType ) ;
    //_Stack_Push ( cfrTil->DebugStateStack, 0 ) ;
    cfrTil->TokenList = _dllist_New ( allocType ) ;
    _Context_ = cfrTil->Context0 = _Context_New ( cfrTil ) ;

    cfrTil->Debugger0 = _Debugger_New ( allocType ) ; // nb : must be after System_NamespacesInit
    cfrTil->cs_CpuState = CpuState_New ( allocType ) ;
    if ( cfrTil->SaveDsp && cfrTil->DataStack )// with _Q_->RestartCondition = STOP from Debugger_Stop
    {
        Dsp = cfrTil->SaveDsp ;
    }
    else
    {
        cfrTil->DataStack = Stack_New ( _Q_->DataStackSize, CFRTIL ) ;
        _CfrTil_DataStack_Init ( cfrTil ) ;
    }
    if ( nss )
    {
        cfrTil->Namespaces = nss ;
    }
    else
    {
        _CfrTil_NamespacesInit ( cfrTil ) ;
    }
    cfrTil->StoreWord = _Word_FindAny ( ( byte* ) "store" ) ;
    cfrTil->PokeWord = _Word_FindAny ( ( byte* ) "poke" ) ;
    cfrTil->LispNamespace = Namespace_Find ( ( byte* ) "Lisp" ) ;
    CfrTil_MachineCodePrimitive_AddWords ( ) ; // in any case we need to reinit these for eg. debugger->SaveCpuState (), etc.
    CfrTil_ReadTables_Setup ( cfrTil ) ;
    CfrTil_LexerTables_Setup ( cfrTil ) ;
    cfrTil->LC = 0 ;
    cfrTil->SC_QuoteMode = 0 ;
}

void
CfrTil_ResetMemory ( CfrTil * cfrTil )
{
    if ( cfrTil->ContextStack )
    {
        while ( Stack_Depth ( cfrTil->ContextStack ) )
        {
            Context * cntx = ( Context* ) _Stack_Pop ( cfrTil->ContextStack ) ;
            NamedByteArray_Delete ( cntx->ContextNba ) ;
        }
        if ( cfrTil->Context0 ) NamedByteArray_Delete ( cfrTil->Context0->ContextNba ) ;
    }
    //OVT_MemListFree_ContextMemory ( ) ;
    _OVT_MemListFree_CfrTilInternal ( ) ;
    OVT_MemListFree_Session ( ) ;
    OVT_MemListFree_ContextMemory ( ) ;
    OVT_MemListFree_LispTemp ( ) ;
    OVT_MemListFree_TempObjects ( ) ;
    OVT_MemListFree_Buffers ( ) ;
    OVT_MemListFree_CompilerTempObjects ( ) ;
    if ( _Q_->Verbosity > 1 ) OVT_ShowMemoryAllocated ( ) ;
}

CfrTil *
_CfrTil_New ( CfrTil * cfrTil )
{
    // nb. not all of this logic has really been needed or used or tested; it should be reworked according to need
    Namespace * nss = 0 ;
    if ( cfrTil )
    {
        if ( _Q_->RestartCondition < RESET_ALL )
        {
            nss = cfrTil->Namespaces ; // in this case (see also below) only preserve Namespaces, all else is recycled and reinitialized
            if ( cfrTil->LogFILE )
            {
                CfrTil_LogOff ( ) ;
            }
        }
        CfrTil_ResetMemory ( cfrTil ) ;
    }
    else
    {
        nss = 0 ;
    }
    _Context_ = 0 ;
    cfrTil = ( CfrTil* ) Mem_Allocate ( sizeof ( CfrTil ), OPENVMTIL ) ;
    _CfrTil_Init ( cfrTil, nss ) ;
    Linux_SetupSignals ( &cfrTil->JmpBuf0, 1 ) ;
    return cfrTil ;
}

void
CfrTil_Lexer_SourceCodeOn ( )
{
    Lexer_SourceCodeOn ( _Context_->Lexer0 ) ;
}

void
_CfrTil_AddStringToSourceCode ( CfrTil * cfrtil, byte * str )
{
    strcat ( ( char* ) cfrtil->SourceCodeScratchPad, ( char* ) str ) ;
    strcat ( ( CString ) cfrtil->SourceCodeScratchPad, ( CString ) " " ) ;
}

void
CfrTil_AddStringToSourceCode ( CfrTil * cfrtil, byte * str )
{
    strcat ( ( char* ) cfrtil->SourceCodeScratchPad, ( char* ) str ) ;
    strcat ( ( CString ) cfrtil->SourceCodeScratchPad, ( CString ) " " ) ;
    cfrtil->SC_ScratchPadIndex += ( Strlen ( ( char* ) str ) + 1 ) ; // 1 : add " " (above)
}

void
_CfrTil_SC_ScratchPadIndex_Init ( CfrTil * cfrtil )
{
    cfrtil->SC_ScratchPadIndex = Strlen ( ( char* ) _CfrTil_->SourceCodeScratchPad ) ;
}

void
_CfrTil_SourceCode_Init ( CfrTil * cfrtil )
{
    cfrtil->SourceCodeScratchPad [ 0 ] = 0 ;
    cfrtil->SC_ScratchPadIndex = 0 ;
    SetState ( cfrtil, SOURCE_CODE_INITIALIZED, true ) ;
}

void
_CfrTil_InitSourceCode ( CfrTil * cfrtil )
{
    //if ( force || ( ! GetState ( _CfrTil_, SOURCE_CODE_INITIALIZED ) ) ) {
    Lexer_SourceCodeOn ( _Context_->Lexer0 ) ;
    _CfrTil_SourceCode_Init ( cfrtil ) ;
}

void
CfrTil_InitSourceCode ( CfrTil * cfrtil )
{
    _CfrTil_InitSourceCode ( cfrtil ) ;
    _CfrTil_SC_ScratchPadIndex_Init ( cfrtil ) ;
}

void
_CfrTil_InitSourceCode_WithName ( CfrTil * cfrtil, byte * name )
{
    _CfrTil_InitSourceCode ( cfrtil ) ;
    _CfrTil_AddStringToSourceCode ( cfrtil, name ) ;
    _CfrTil_SC_ScratchPadIndex_Init ( cfrtil ) ;
}

void
CfrTil_InitSourceCode_WithCurrentInputChar ( CfrTil * cfrtil )
{
    Lexer * lexer = _Context_->Lexer0 ;
    _CfrTil_InitSourceCode ( cfrtil ) ;
    _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter, 0 ) ;
}

void
CfrTil_SourceCode_Init ( )
{
    Word * word = Compiler_WordList ( 0 ) ;
    if ( word ) _CfrTil_InitSourceCode_WithName ( _CfrTil_, word->Name ) ;
    d0 ( else _Printf ( ( byte* ) "\nwhoa\n" ) ) ;
}

void
_CfrTil_DebugSourceCodeCompileOn ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, true ) ;
}

void
_CfrTil_DebugSourceCodeCompileOff ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, false ) ;
}

void
CfrTil_SourceCodeCompileOn ( )
{
    _CfrTil_DebugSourceCodeCompileOn ( ) ;
    CfrTil_SourceCode_Init ( ) ;
    if ( ! GetState ( _Context_, C_SYNTAX ) ) CfrTil_Colon ( ) ;
}

void
_CfrTil_FinishSourceCode ( CfrTil * cfrtil, Word * word )
{
    // keep a LambdaCalculus LO_Define0 created SourceCode value
    if ( ! word->SourceCode ) word->SourceCode = String_New ( cfrtil->SourceCodeScratchPad, STRING_MEM ) ;
    Lexer_SourceCodeOff ( _Context_->Lexer0 ) ;
    SetState ( cfrtil, SOURCE_CODE_INITIALIZED, false ) ;
}

void
_CfrTil_UnAppendFromSourceCode ( CfrTil * cfrtil, int nchars )
{
    int plen = Strlen ( ( CString ) cfrtil->SourceCodeScratchPad ) ;
    if ( plen >= nchars )
    {
        cfrtil->SourceCodeScratchPad [ Strlen ( ( CString ) cfrtil->SourceCodeScratchPad ) - nchars ] = 0 ;
    }
    _CfrTil_SC_ScratchPadIndex_Init ( cfrtil ) ;
}

void
_CfrTil_UnAppendTokenFromSourceCode ( CfrTil * cfrtil, byte * tkn )
{
    _CfrTil_UnAppendFromSourceCode ( cfrtil, Strlen ( ( CString ) tkn ) + 1 ) ;
}

void
_CfrTil_AppendCharToSourceCode ( CfrTil * cfrtil, byte c )
{
    cfrtil->SourceCodeScratchPad [ cfrtil->SC_ScratchPadIndex ++ ] = c ;
    cfrtil->SourceCodeScratchPad [ cfrtil->SC_ScratchPadIndex ] = 0 ;
}

void
CfrTil_AppendCharToSourceCode ( CfrTil * cfrtil, byte c, int32 convertToSpaceFlag )
{
    if ( cfrtil->SC_ScratchPadIndex < ( SOURCE_CODE_BUFFER_SIZE - 1 ) )
    {
        if ( c == '"' )
        {
            if ( cfrtil->SC_QuoteMode ) cfrtil->SC_QuoteMode = 0 ;
            else cfrtil->SC_QuoteMode = 1 ;
            _CfrTil_AppendCharToSourceCode ( cfrtil, c ) ;
        }
#if 0        
        else if ( convertToSpaceFlag )
        {
            c = String_ConvertEscapeCharToSpace ( c ) ;
            if ( ! ( ( c == ' ' ) && ( cfrtil->SourceCodeScratchPad [ cfrtil->SC_ScratchPadIndex - 1 ] == ' ' ) ) )
            {
                _CfrTil_AppendCharToSourceCode ( cfrtil, c ) ;
            }
        }
#endif        
        else
        {
            _String_AppendConvertCharToBackSlashAtIndex ( cfrtil->SourceCodeScratchPad, c, &cfrtil->SC_ScratchPadIndex, cfrtil->SC_QuoteMode ) ;
        }
    }
}


//----------------------------------------------------------------------------------------|
//              get from/ add to head  |              | get from head      add to tail    |      
// TokenList Tail <--> TokenList Head  |<interpreter> | PeekList Head <--> PeekList Tail  |
// token token token token token token | currentToken | token token token token token ... |
//----------------------------------------------------------------------------------------|

byte *
_CfrTil_GetTokenFromTokenList ( Lexer * lexer )
{
    Symbol * tknSym ;
    if ( tknSym = ( Symbol* ) _dllist_First ( ( dllist* ) _CfrTil_->TokenList ) )
    {
        dlnode_Remove ( ( dlnode* ) tknSym ) ;
        lexer->TokenStart_ReadLineIndex = tknSym->S_Value ;
        lexer->TokenEnd_ReadLineIndex = tknSym->S_Value2 ;
        return tknSym->S_Name ;
    }
    return 0 ;
}

void
_CfrTil_AddTokenToTailOfTokenList ( byte * token )
{
    Symbol * tknSym = _Symbol_New ( token, TEMPORARY ) ;
    tknSym->S_Value = _Context_->Lexer0->TokenStart_ReadLineIndex ;
    tknSym->S_Value2 = _Context_->Lexer0->TokenEnd_ReadLineIndex ;
    dllist_AddNodeToTail ( _CfrTil_->TokenList, ( dlnode* ) tknSym ) ;
}

void
_CfrTil_AddTokenToHeadOfTokenList ( byte * token )
{
    Symbol * tknSym = _Symbol_New ( token, TEMPORARY ) ;
    tknSym->S_Value = _Context_->Lexer0->TokenStart_ReadLineIndex ;
    tknSym->S_Value2 = _Context_->Lexer0->TokenEnd_ReadLineIndex ;
    dllist_AddNodeToHead ( _CfrTil_->TokenList, ( dlnode* ) tknSym ) ;
}

void
CfrTil_ClearTokenList ( )
{
    _dllist_Init ( _CfrTil_->TokenList ) ;
}

void
CfrTil_OptimizeOn ( )
{
    SetState ( _CfrTil_, OPTIMIZE_ON, true ) ;
}

void
CfrTil_OptimizeOff ( )
{
    SetState ( _CfrTil_, OPTIMIZE_ON, false ) ;
}

void
CfrTil_StringMacrosOn ( )
{
    SetState ( _CfrTil_, STRING_MACROS_ON, true ) ;
    _CfrTil_StringMacros_Init ( ) ;
}

void
CfrTil_StringMacrosOff ( )
{
    SetState ( _CfrTil_, STRING_MACROS_ON, false ) ;
    SetState ( &_CfrTil_->Sti, STI_INITIALIZED, false ) ;
}

void
CfrTil_InlineOn ( )
{
    SetState ( _CfrTil_, INLINE_ON, true ) ;
}

void
CfrTil_InlineOff ( )
{
    SetState ( _CfrTil_, INLINE_ON, false ) ;
}

void
_CfrTil_DebugOff ( )
{
    DebugOff ;
}

void
_CfrTil_DebugOn ( )
{
    DebugOn ;
}

/*
 * 
 * Compiler Word List has nodes (CWLNs) with 2 slots one for the *word and one for a pointer to a Source Code Node (SCN) which has source code index info.
 * CWLN : slot 0 word, slot 1 SCN
 * Source code nodes (SCNs) have three slots for source code byte index, the code address, and a pointer to the word, they are on the _CfrTil_->DebugWordList.
 * SCN : slot 0 : SCN_SC_CADDRESS, slot 1 : SCN_WORD_SC_INDEX, slot 2 : SCN_SC_WORD
 * So, they each have pointers to each other.
 * 
 */

void
DWL_ShowNode ( dlnode * node, byte * insert )
{
    if ( node )
    {
        int32 sc_index = dobject_Get_M_Slot ( node, SCN_WORD_SC_INDEX ) ;
        byte * address = ( byte* ) dobject_Get_M_Slot ( node, SCN_SC_CADDRESS ) ;
        Word * word = ( Word* ) dobject_Get_M_Slot ( node, SCN_SC_WORD ) ;
        _Printf ( ( byte* ) "\n\tDWL_ShowNode :: %s :: node :: = 0x%08x : word Name = \'%-12s\'\t : at address  = 0x%08x : with sc_index = %d",
            insert, node, word->Name, address, sc_index ) ;
        d0 ( else _Printf ( ( byte* ) "\nDWL_ShowNode : node :: = 0x%08x : word Name = \'%-12s\'\t : at address  = 0x%08x : with index = %d", node, word->Name, address, sc_index ) ) ;
    }
}

void
DebugWordList_Show ( )
{
    dllist * list ;
    if ( Is_DebugOn && ( list = _CfrTil_->DebugWordList ) ) //GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) )
    {
        int32 index ;
        dlnode * node, *nextNode ;
        for ( index = 0, node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode, index ++ )
        {
            nextNode = dlnode_Next ( node ) ;
            dobject * dobj = ( dobject * ) node ;
            byte * address = ( byte* ) dobject_Get_M_Slot ( dobj, SCN_SC_CADDRESS ) ; // notice : we are setting the slot in the obj that was in the SCN_SC_WORD_INDEX slot (1) of the 
            int32 scwi = dobject_Get_M_Slot ( dobj, SCN_WORD_SC_INDEX ) ;
            Word * word = ( Word * ) dobject_Get_M_Slot ( dobj, SCN_SC_WORD ) ;
            _Printf ( "\nDebugWordList : index %d : word = \'%-12s\' : address = 0x%8x : scwi = %d", index, word->Name, address, scwi ) ;
        }
        //_Pause ( ) ;
    }
}

// ...source code source code TP source code source code ... EOL

byte *
PrepareSourceCodeString ( dobject * dobj, Word * scWord, Word * word, int32 scwi0 ) // scwi : source code word index
{
    byte * sc = scWord->SourceCode, *name0, *name ;
    byte * buffer = Buffer_Data ( _CfrTil_->DebugB1 ) ;
    memset ( buffer, 0, BUFFER_SIZE ) ;
    int32 scwi, i, j, k, n, tp, wl, wl0, tw = GetTerminalWidth ( ), space, inc = 20 ; // tp: text point 
    dobject * dobj2 ;
    //if ( tw > 160 ) tw = 120 ; // 60 : adjust for possible tabs on the line //( tw > 80 ) ? 80 : tw ;
    tp = 34 ; // text point aligned with disassembly
start:
    name0 = String_ConvertToBackSlash ( word->Name ) ;
    wl0 = Strlen ( name0 ) ; // nb! : wl0 is Strlen before c_dd transform below
    scwi = scwi0 ; // source code word index
    scwi0 -= wl0 ; /// word length
    d0 ( byte * scspp = & sc [ scwi0 ] ) ;
    i = 0, n = wl0 + inc ;
    scwi = String_FindStrnCmpIndex ( sc, name0, &i, scwi0, wl0, 20 ) ;
    d0 ( byte * scspp2 = & sc [ scwi ] ) ;
    if ( i > n )
    {
        dlnode_Remove ( ( node* ) dobj ) ; // so we don't find the same one again
        dobj2 = ( dobject* ) DWL_Find ( 0, _Debugger_->DebugAddress, 0, 0, 0, 0, 0 ) ;
        if ( dobj2 )
        {
            dlnode_InsertThisBeforeANode ( ( node* ) dobj, ( node* ) dobj2 ) ; // so we can find it next time thru
            word = ( Word* ) dobject_Get_M_Slot ( dobj, SCN_SC_WORD ) ;
            scwi = dobject_Get_M_Slot ( dobj, SCN_WORD_SC_INDEX ) ;
            dobj = dobj2 ;
            goto start ;
        }
    }
    name = c_dd ( name0 ) ;
    wl = Strlen ( name ) ;
    if ( scwi < tp ) // tp: text point 
    {
        for ( i = 0, n = tp - scwi - 3 ; n -- ; i ++ ) buffer [i] = ' ' ;
        Strncat ( buffer, &sc [0], scwi ) ;
    }
    else
    {
        j = scwi - tp ; // tp: text point 
        if ( j >= 4 )
        {
            for ( i = 0, k = 3 ; k -- ; i ++ ) buffer [i] = '.' ;
            Strncat ( buffer, " ", 1 ) ;
            space = 4 ;
        }
        else
        {
            space = 0 ;
        }
        Strncat ( buffer, &sc [ j + space + 3 ], tp - space - 3 ) ;
    }
    Strncat ( buffer, name, wl ) ;

    byte * buffer2 = Buffer_Data ( _CfrTil_->DebugB2 ) ;
    Strncpy ( buffer2, &sc [ scwi + wl0 ], 256 ) ;
    byte * scp = String_FilterMultipleSpaces ( String_ConvertToBackSlash ( buffer2 ), TEMPORARY ) ;
    Strncat ( buffer, scp, tw - tp - wl ) ; // wi + wl : after the wi word which we concated above
    return buffer ;
}

#define SC_WINDOW 50

node *
DWL_Find ( Word * word, byte * address, byte* name, int32 showAll, int32 fromFirst, int32 takeFirstFind, byte * newAddress )
{
    byte * caddress ;
    Word * wordn ;
    dllist * list = _CfrTil_->DebugWordList ;
    dlnode * node = 0, *foundNode = 0 ;
    int32 numFound = 0 ;
    uint32 adiff = SC_WINDOW, diff1, scwi ; //, scwi0 = - 1 ;
    if ( list && ( word || name || address ) )
    {
        for ( node = fromFirst ? dllist_First ( ( dllist* ) list ) : dllist_Last ( ( dllist* ) list ) ; node ;
            node = fromFirst ? dlnode_Next ( node ) : dlnode_Previous ( node ) )
        {
            caddress = ( byte* ) dobject_Get_M_Slot ( node, SCN_SC_CADDRESS ) ;
            wordn = ( Word* ) dobject_Get_M_Slot ( node, SCN_SC_WORD ) ;
            if ( ( word && ( word == wordn ) ) || ( name && wordn && ( String_Equal ( wordn->Name, name ) ) ) ) foundNode = node ;
            else if ( address && ( address == caddress ) )
            {
                scwi = dobject_Get_M_Slot ( ( dobject * ) node, SCN_WORD_SC_INDEX ) ;
                if ( newAddress && ( address == caddress ) )
                {
                    dobject_Set_M_Slot ( ( dobject * ) node, SCN_SC_CADDRESS, newAddress ) ;
                    if ( showAll ) _Printf ( "\nDWL_Find : word = \'%-12s\' : address = 0x%8x : scwi = %d : newAddress = 0x%8x", wordn->Name, address, scwi, newAddress ) ;
                }
                else if ( takeFirstFind ) break ;
                else if ( showAll )
                {
                    numFound ++ ;
                    DWL_ShowNode ( node, "FOUND" ) ;
                }
#if 0                
                if ( wordn->CProperty & COMBINATOR ) 
                {
                    foundNode = node ;
                    //break ;
                }
#endif                
                //else
                {
                    if ( scwi >= _Debugger_->LastSourceCodeIndex ) diff1 = scwi - _Debugger_->LastSourceCodeIndex ;
                    else diff1 = _Debugger_->LastSourceCodeIndex - scwi ;
                    if ( diff1 < adiff )
                    {
                        foundNode = node ;
                        adiff = diff1 ;
                        //continue ;
                    }
                    else if ( ! foundNode )
                    {
                        if ( _Debugger_->LastSourceCodeAddress < address )
                        {
                            if ( diff1 < SC_WINDOW ) foundNode = node ;
                        }
                        else foundNode = node ;
                    }
                }
            }
        }
    }
    if ( showAll && foundNode && ( numFound > 1 ) )
    {
        _Printf ( ( byte* ) "\nNumber Found = %d :: Choosen node :", numFound ) ;
        DWL_ShowNode ( foundNode, "FOUND" ) ;
    }
    if ( address ) _Debugger_->LastSourceCodeAddress = address ;
    return foundNode ;
}

void
_Debugger_ShowSourceCodeAtAddress ( Debugger * debugger )
{
    // ...source code source code TP source code source code ... EOL
    Word * scWord = debugger->w_Word, *word ;
    if ( scWord->DebugWordList ) //GetState ( scWord, W_SOURCE_CODE_MODE ) )
    {
        int32 scwi, fixed = 0 ;
        dobject * dobj ;
        //DebugWordList_Show ( ) ;
        dobj = ( dobject* ) DWL_Find ( 0, debugger->DebugAddress, 0, 0, 0, 0, 0 ) ;
        if ( dobj )
        {
            word = ( Word* ) dobject_Get_M_Slot ( dobj, SCN_SC_WORD ) ;
            if ( GetState ( scWord, W_C_SYNTAX ) && String_Equal ( word->Name, "store" ) )
            {
                word->Name = ( byte* ) "=" ;
                fixed = 1 ;
            }
            scwi = dobject_Get_M_Slot ( dobj, SCN_WORD_SC_INDEX ) ;
            byte * buffer = PrepareSourceCodeString ( dobj, scWord, word, scwi ) ;
            _Printf ( ( byte* ) "\n%s", buffer ) ;
            debugger->LastSourceCodeIndex = scwi ;
            if ( fixed )
            {
                word->Name = ( byte* ) "store" ;
            }
        }
        d0 ( else DebugWordList_Show ( ) ) ;
    }
}

// DWL - DebugWordList : _CfrTil_->DebugWordList 

void
_CfrTil_AdjustSourceCodeAddress ( byte * address, byte * newAddress )
{
    if ( IsSourceCodeOn )
    {
        DWL_Find ( 0, address, 0, 0, 0, 0, newAddress ) ;
    }
}

void
DWL_CheckPush_Word ( Word * word )
{
    if ( word && ( GetState ( _CfrTil_, SC_FORCE_PUSH ) || ( IsSourceCodeOn && ( _CfrTil_->ScoOcCrw != word ) ) ) )
    {
        if ( ! ( word->LProperty & ( W_COMMENT | W_PREPROCESSOR ) ) )
        {
            _CfrTil_->ScoOcCrw = word ;
            DebugWordList_PushWord ( _CfrTil_->ScoOcCrw ) ;
        }
        SC_SetForcePush ( false ) ; // one shot only
    }
}

void
SC_DWL_PushCWL_Index ( int32 index )
{
    DWL_CheckPush_Word ( Compiler_WordList ( index ) ) ;
}

dobject *
DebugWordList_PushWord ( Word * word )
{
    dobject * dobj = 0 ;
    if ( word )
    {
        int32 scindex ;
        scindex = ( GetState ( _Compiler_, LC_ARG_PARSING ) || ( word->CProperty & COMBINATOR )) ? ( word->W_SC_ScratchPadIndex ? word->W_SC_ScratchPadIndex : _CfrTil_->SC_ScratchPadIndex ) : _CfrTil_->SC_ScratchPadIndex ;
        dobj = Node_New_ForDebugWordList ( TEMPORARY, scindex, word ) ; // _dobject_New_M_Slot_Node ( DICTIONARY, WORD_LOCATION, 3, 0, scindex, word ) 
        dobject_Set_M_Slot ( ( dobject* ) dobj, SCN_SC_CADDRESS, Here ) ;
        DbgWL_Push ( dobj ) ; // _dllist_AddNodeToHead ( _CfrTil_->DebugWordList, ( dlnode* ) dobj )
        d0 ( DWL_ShowNode ( ( node* ) dobj, "PUSHED" ) ) ;
    }
    return dobj ;
}

void
CfrTil_DebugWordList_Pop ( )
{
    node * first = _dllist_First ( _CfrTil_->DebugWordList ) ;

    if ( first ) dlnode_Remove ( first ) ;
}

void
CfrTil_WordList_Pop ( )
{
    node * first = _dllist_First ( _Compiler_->WordList ) ;

    if ( first ) dlnode_Remove ( first ) ;
}

/*
 * Compiler Word List : has nodes (CWLNs) with 2 slots one for the *word and one for a pointer to a Source Code Node (SCN) which has source code index info.
 * CWLN : slot 0 word, slot 1 SCN
 * _CfrTil_->DebugWordList : has Source code nodes (SCNs) have three slots for source code byte index, the code address, and a pointer to the word, they are on the _CfrTil_->DebugWordList.
 * SCN : slot 0 : SCN_SC_CADDRESS, slot 1 : SCN_WORD_SC_INDEX, slot 2 : SCN_SC_WORD
 * So, they each have pointers to each other.
 * 
 */
void
_CfrTil_WordLists_PushWord ( Word * word )
{

    dobject * dobj = 0 ;
    //if ( IsSourceCodeOn ) dobj = DebugWordList_PushWord ( word ) ;
    CompilerWordList_Push ( word, dobj ) ; // _dllist_Push_M_Slot_Node ( _Compiler_->WordList, WORD, COMPILER_TEMP, 2, ((int32) word), ((int32) dnode) )
}

void
CfrTil_WordLists_PopWord ( )
{

    CfrTil_WordList_Pop ( ) ;
    CfrTil_DebugWordList_Pop ( ) ;
}

void
CfrTil_Compile_SaveIncomingCpuState ( CfrTil * cfrtil )
{
    // save the incoming current C cpu state

    Compile_Call ( ( byte* ) cfrtil->SaveCpuState ) ; // save incoming current C cpu state
    _Compile_MoveReg_To_Mem ( EBP, ( byte * ) & cfrtil->cs_CpuState->Ebp, EBX, CELL ) ; // EBX : scratch reg
    _Compile_MoveReg_To_Mem ( ESP, ( byte * ) & cfrtil->cs_CpuState->Esp, EBX, CELL ) ;

}

void
CfrTil_Compile_RestoreIncomingCpuState ( CfrTil * cfrtil )
{
    // restore the incoming current C cpu state
    Compile_Call ( ( byte* ) _CfrTil_->RestoreCpuState ) ;
    _Compile_MoveMem_To_Reg ( EBP, ( byte * ) & cfrtil->cs_CpuState->Ebp, EBX, CELL ) ;
    _Compile_MoveMem_To_Reg ( ESP, ( byte * ) & cfrtil->cs_CpuState->Esp, EBX, CELL ) ;
}

