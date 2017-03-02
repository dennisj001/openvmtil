
#include "../include/cfrtil.h"

void
_CfrTil_Run ( CfrTil * cfrTil, int32 restartCondition )
{
    while ( 1 )
    {
        cfrTil = _CfrTil_New ( cfrTil ) ;
        if ( cfrTil )
        {
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
    Printf ( (byte*) "\n\n" ) ;
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
    if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\nData Stack reset." ) ;
}

void
CfrTil_DataStack_Init ( )
{
    _CfrTil_DataStack_Init ( _CfrTil_ ) ;
}

void
_CfrTil_Init ( CfrTil * cfrTil, Namespace * nss )
{
    uint32 type = CFRTIL ;
    _CfrTil_ = cfrTil ;
    // TODO : organize these buffers and their use 
    cfrTil->OriginalInputLineB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->InputLineB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->SourceCodeSPB = _Buffer_NewPermanent ( SOURCE_CODE_BUFFER_SIZE ) ;
    cfrTil->LambdaCalculusPB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->TokenB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->PrintfB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->ScratchB1 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->DebugB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->DebugB1 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->DebugB2 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringInsertB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringInsertB2 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringInsertB3 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->TabCompletionBuf = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringMacroB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->OriginalInputLine = Buffer_Data ( cfrTil->OriginalInputLineB ) ;
    cfrTil->SourceCodeScratchPad = Buffer_Data ( cfrTil->SourceCodeSPB ) ;
    cfrTil->LispPrintBuffer = Buffer_Data ( cfrTil->LambdaCalculusPB ) ;
    cfrTil->TokenBuffer = Buffer_Data ( cfrTil->TokenB ) ;
    SetState ( cfrTil, CFRTIL_RUN | OPTIMIZE_ON | INLINE_ON, true ) ;
    if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\nSystem Memory is being reallocated.  " ) ;
    cfrTil->ContextStack = Stack_New ( 256, type ) ;
    cfrTil->ObjectStack = Stack_New ( 1 * K, type ) ;
    cfrTil->DebugStateStack = Stack_New ( 1 * K, type ) ;
    _Stack_Push ( cfrTil->DebugStateStack, 0 ) ;
    cfrTil->TokenList = _dllist_New ( type ) ;
    //cfrTil->DebugWordList = _dllist_New ( type ) ;
    _Context_ = cfrTil->Context0 = _Context_New ( cfrTil, type ) ;
    cfrTil->Debugger0 = _Debugger_New ( type ) ; // nb : must be after System_NamespacesInit
    cfrTil->cs_CpuState = CpuState_New ( type ) ;
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
    cfrTil->LC = 0 ; //LC_New ( ) ;
    cfrTil->SC_QuoteMode = 0 ;
    //cfrTil->SCA_BlockedIndex = - 1 ;
}

void
CfrTil_ResetMemory ( )
{
    _OVT_MemListFree_CfrTilInternal ( ) ;
    OVT_MemListFree_Session ( ) ;
    OVT_MemListFree_ContextMemory ( ) ;
    OVT_MemListFree_LispTemp ( ) ;
    OVT_MemListFree_TempObjects ( ) ;
    OVT_MemListFree_Buffers ( ) ;
    OVT_MemListFree_CompilerTempObjects ( ) ;
}

CfrTil *
_CfrTil_New ( CfrTil * cfrTil )
{
    // nb. not all of this logic has really been needed or used or tested; it should be reworked according to need
    Namespace * nss = 0 ;
    if ( cfrTil )
    {
        if ( _Q_->RestartCondition < RESTART )
        {
            nss = cfrTil->Namespaces ; // in this case (see also below) only preserve Namespaces, all else is recycled and reinitialized
            if ( cfrTil->LogFILE )
            {
                CfrTil_LogOff ( ) ;
            }
        }
        CfrTil_ResetMemory ( ) ;
    }
    else
    {
        nss = 0 ;
    }
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
    d0 ( else Printf ( (byte*) "\nwhoa\n" ) ) ;
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
    if ( ! word->SourceCode ) word->SourceCode = String_New ( cfrtil->SourceCodeScratchPad, DICTIONARY ) ;
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
DWL_ShowNode ( dlnode * node )
{
    if ( node )
    {
        int32 sc_index = dobject_Get_M_Slot ( node, SCN_WORD_SC_INDEX ) ;
        byte * address = ( byte* ) dobject_Get_M_Slot ( node, SCN_SC_CADDRESS ) ;
        Word * word = ( Word* ) dobject_Get_M_Slot ( node, SCN_SC_WORD ) ;
        Printf ( (byte*) "\n\tDWL_Find :: FOUND :: node :: = 0x%08x : word Name = \'%-12s\'\t : at address  = 0x%08x : with sc_index = %d\n",
            node, word->Name, address, sc_index ) ;
        d0 ( else Printf ( (byte*) "\nDWL_Find : node :: = 0x%08x : word Name = \'%-12s\'\t : at address  = 0x%08x : with index = %d\n", node, word->Name, address, sc_index ) ) ;
    }
}

// ...source code source code TP source code source code ... EOL

byte *
PrepareSourceCodeString ( dobject * dobj, Word * scWord, Word * word, int32 scwi0 ) // scwi : source code word index
{
    byte * sc = scWord->SourceCode, *name0, *name ;
    byte * buffer = Buffer_Data ( _CfrTil_->DebugB1 ) ;
    memset ( buffer, 0, BUFFER_SIZE ) ;
    int32 scwi, i, j, k, n, tp, wl, wl0, tw = GetTerminalWidth ( ), space ; // tp: text point 
    dobject * dobj2 ;
    //if ( tw > 160 ) tw = 120 ; // 60 : adjust for possible tabs on the line //( tw > 80 ) ? 80 : tw ;
    tp = 34 ; // text point aligned with disassembly
start:
    name0 = String_ConvertToBackSlash ( word->Name ) ;
    wl0 = Strlen ( name0 ) ; // nb! : wl0 is Strlen before c_dd transform below
    scwi = scwi0 ;
    scwi0 -= wl0 ;
    d1 ( byte * scspp = & sc [ scwi0 ] ) ;
    int32 index = scwi0 ;
    for ( i = 0, n = wl0 + 20 ; i <= n ; i ++ ) // tokens are parsed in different order with parameter and c rtl args, etc. 
    {
        if ( ! StrnCmp ( & sc [ index - i ], name0, wl0 ) )
        {
            index -= i ;
            break ;
        }
        if ( ! StrnCmp ( & sc [ index + i ], name0, wl0 ) )
        {
            index += i ;
            break ;
        }
        d0 ( if ( (i > 12) && (i < 20 )) Printf ( (byte*) "\n&sc[index - i] = %20s :: name0 = %20s\n", & sc [ index - i ], name0 ) ) ;
    }
    scwi = index ;
    d1 ( byte * scspp2 = & sc [ scwi ] ) ;
    if ( i > n )
    {
        dlnode_Remove ( ( node* ) dobj ) ; // so we don't find the same one again
        dobj2 = ( dobject* ) DWL_Find ( 0, _Debugger_->DebugAddress, 0, 0, 0, 0 ) ;
        dlnode_InsertThisBeforeANode ( ( node* ) dobj, ( node* ) dobj2 ) ; // so we can find it next time thru
        if ( dobj2 )
        {
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
        Strncat ( buffer, &sc [0], index ) ;
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
    //Strncat ( buffer, &sc [ scwi + wl0 ], tw - tp - wl ) ; // wi + wl : after the wi word which we concated above
    Strncat ( buffer, scp, tw - tp - wl ) ; // wi + wl : after the wi word which we concated above
    return buffer ;
    //return String_FilterMultipleSpaces ( buffer, TEMPORARY ) ; // buffer ;
}

node *
DWL_Find ( Word * word, byte * address, byte* name, int32 showAll, int32 fromFirst, int32 takeFirstFind )
{
    byte * caddress ;
    Word * wordn ;
    dllist * list = _CfrTil_->DebugWordList ;
    dlnode * node = 0, *foundNode = 0 ;
    int32 numFound = 0 ;
    if ( list && ( word || name || address ) )
    {
        for ( node = fromFirst ? dllist_First ( ( dllist* ) list ) : dllist_Last ( ( dllist* ) list ) ; node ;
            node = fromFirst ? dlnode_Next ( node ) : dlnode_Previous ( node ) )
        {
            caddress = ( byte* ) dobject_Get_M_Slot ( node, SCN_SC_CADDRESS ) ;
            wordn = ( Word* ) dobject_Get_M_Slot ( node, SCN_SC_WORD ) ;
            if ( ( address && ( address == caddress ) ) || ( word && ( word == wordn ) ) || ( name && wordn && ( String_Equal ( wordn->Name, name ) ) ) )
            {
                foundNode = node ;
                if ( showAll && ( address == caddress ) )
                //d0 ( if ( DebugOn || ( showAll && ( address == caddress ) ) ) )
                {
                    numFound ++ ;
                    DWL_ShowNode ( node ) ;
                    continue ;
                }
                if ( takeFirstFind ) break ;
            }
        }
    }
    if ( showAll && foundNode && ( numFound > 1 ) )
    //d0 ( if ( DebugOn || (showAll && foundNode && ( numFound > 1 ) ) ) ) 
    {
        Printf ( (byte*) "\nNumber Found = %d :: Choosen node :\n", numFound ) ;
        DWL_ShowNode ( foundNode ) ;
        Printf ( (byte*) "\n\n" ) ;
    }
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
        dobj = ( dobject* ) DWL_Find ( 0, debugger->DebugAddress, 0, 0, 0, 0 ) ;
start:
        if ( dobj )
        {
            word = ( Word* ) dobject_Get_M_Slot ( dobj, SCN_SC_WORD ) ;
            if ( GetState ( scWord, W_C_SYNTAX ) && String_Equal ( word->Name, "store" ) )
            {
                word->Name = (byte*) "=" ;
                fixed = 1 ;
            }
            scwi = dobject_Get_M_Slot ( dobj, SCN_WORD_SC_INDEX ) ;
            byte * buffer = PrepareSourceCodeString ( dobj, scWord, word, scwi ) ;
            Printf ( ( byte* ) "%s\n", buffer ) ;
            if ( fixed )
            {
                word->Name = (byte*) "store" ;
            }
        }
    }
}

void
_CfrTil_AdjustSourceCodeAddress ( byte * address, byte * newAddress )
{
    if ( GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) )
    {
        dobject * dobj = ( dobject* ) DWL_Find ( 0, address, 0, 0, 0, 0 ) ;
        if ( dobj )
        {
            dobject_Set_M_Slot ( dobj, SCN_SC_CADDRESS, newAddress ) ;
            d0 ( Printf ( (byte*) "\n_CfrTil_AdjustSourceCodeAddress :: address = 0x%08x :: newAddress = 0x%08x", address, newAddress ) ) ;
        }
    }
}

void
CfrTil_Set_DebugSourceCodeIndex ( Word * word )
{
    if ( word && word->Name )
    {
        if ( _CfrTil_->DebugWordList && GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) ) 
        {
            word->W_SC_ScratchPadIndex = _CfrTil_->SC_ScratchPadIndex ;
            _CfrTil_DebugWordList_PushWord ( word ) ;
        }
    }
}

void
DWL_SC_Word_SetSourceCodeAddress ( Word * word, byte * address )
{
    if ( GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) ) //DebugSourceCodeOn && ( _CfrTil_->SCA_BlockedIndex != index ) )
    {
        dlnode * node = DWL_Find ( word, 0, 0, 0, 0, 0 ) ;
        if ( node ) dobject_Set_M_Slot ( ( dobject* ) node, SCN_SC_CADDRESS, address ) ; // notice : we are setting the slot in the obj that was in the SCN_SC_WORD_INDEX slot (1) of the 
    }
}

void
CWL_SC_SetSourceCodeAddress ( int32 index )
{
    if ( GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) ) 
    {
        dllist * list = _Compiler_->WordList ;
        dobject * dobj = ( dobject* ) _dllist_Get_N_Node_M_Slot ( list, index, CWLN_SCN ) ;
        if ( dobj ) dobject_Set_M_Slot ( dobj, SCN_SC_CADDRESS, Here ) ; // notice : we are setting the slot in the obj that was in the SCN_SC_WORD_INDEX slot (1) of the 
    }
}

dobject *
_CfrTil_DebugWordList_PushWord ( Word * word )
{
    int32 scindex ;
    dobject * dobj = 0 ;
    if ( ! ( word->CProperty & ( DEBUG_WORD ) ) )
    {
        if ( _CfrTil_->DebugWordList && GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) )
        {
            scindex = word->W_SC_ScratchPadIndex ? word->W_SC_ScratchPadIndex : _CfrTil_->SC_ScratchPadIndex ;
            dobj = Node_New_ForDebugWordList ( scindex, word ) ; // _dobject_New_M_Slot_Node ( DICTIONARY, WORD_LOCATION, 3, 0, scindex, word ) 
            DbgWL_Push ( dobj ) ; // _dllist_AddNodeToHead ( _CfrTil_->DebugWordList, ( dlnode* ) dobj )
        }
    }
    return ( dobj ) ;
}

/*
 * Compiler Word List has nodes (CWLNs) with 2 slots one for the *word and one for a pointer to a Source Code Node (SCN) which has source code index info.
 * CWLN : slot 0 word, slot 1 SCN
 * Source code nodes (SCNs) have three slots for source code byte index, the code address, and a pointer to the word, they are on the _CfrTil_->DebugWordList.
 * SCN : slot 0 : SCN_SC_CADDRESS, slot 1 : SCN_WORD_SC_INDEX, slot 2 : SCN_SC_WORD
 * So, they each have pointers to each other.
 * 
 */
void
_CfrTil_WordLists_PushWord ( Word * word )
{
    dobject * dobj = _CfrTil_DebugWordList_PushWord ( word ) ;
    CompilerWordList_Push ( word, dobj ) ; // _dllist_Push_M_Slot_Node ( _Compiler_->WordList, WORD, COMPILER_TEMP, 2, ((int32) word), ((int32) dnode) )
}

