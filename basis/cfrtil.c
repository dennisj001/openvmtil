
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
                    d1 ( Pause () ; ) ; // shouldn't reach here
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
    Printf ( "\n\n" ) ;
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
    cfrTil->Scratch1B = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
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
    cfrTil->DebugWordList = _dllist_New ( type ) ;
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
    cfrtil->SC_ScratchPadIndex += ( strlen ( ( char* ) str ) + 1 ) ; // 1 : add " " (above)
}

void
_CfrTil_SC_ScratchPadIndex_Init ( CfrTil * cfrtil )
{
    cfrtil->SC_ScratchPadIndex = strlen ( ( char* ) _CfrTil_->SourceCodeScratchPad ) ;
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
    _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter ) ;
}

void
CfrTil_SourceCode_Init ( )
{
    Word * word = Compiler_WordList ( 0 ) ;
    if ( word ) _CfrTil_InitSourceCode_WithName ( _CfrTil_, word->Name ) ;
    //d1 ( else Printf ( "\nwhoa\n" ) ) ;
}

void
_CfrTil_SourceCodeCompileOn ( )
{
    SetState ( _CfrTil_, SOURCE_CODE_MODE, true ) ;
}

void
_CfrTil_SourceCodeCompileOff ( )
{
    SetState ( _CfrTil_, SOURCE_CODE_MODE, false ) ;
}

void
CfrTil_SourceCodeCompileOn ( )
{
    _CfrTil_SourceCodeCompileOn ( ) ;
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
    if ( GetState ( cfrtil, SOURCE_CODE_MODE ) )
    {
        //word->DebugWordList = _CfrTil_->DebugWordList ;
        _CfrTil_->DebugWordList = 0 ; //_dllist_New ( CFRTIL ) ;
    }
}

void
_CfrTil_UnAppendFromSourceCode ( CfrTil * cfrtil, int nchars )
{
    int plen = strlen ( ( CString ) cfrtil->SourceCodeScratchPad ) ;
    if ( plen >= nchars )
    {
        cfrtil->SourceCodeScratchPad [ strlen ( ( CString ) cfrtil->SourceCodeScratchPad ) - nchars ] = 0 ;
    }
    _CfrTil_SC_ScratchPadIndex_Init ( cfrtil ) ;
}

void
_CfrTil_UnAppendTokenFromSourceCode ( CfrTil * cfrtil, byte * tkn )
{
    _CfrTil_UnAppendFromSourceCode ( cfrtil, strlen ( ( CString ) tkn ) + 1 ) ;
}

void
_CfrTil_AppendCharToSourceCode ( CfrTil * cfrtil, byte c )
{
    if ( cfrtil->SC_ScratchPadIndex < ( SOURCE_CODE_BUFFER_SIZE - 1 ) )
    {
        cfrtil->SourceCodeScratchPad [ cfrtil->SC_ScratchPadIndex ++ ] = c ;
        cfrtil->SourceCodeScratchPad [ cfrtil->SC_ScratchPadIndex ] = 0 ;
    }
}

//----------------------------------------------------------------------------------------|
//              get from/ add to head  |              | get from head      add to tail    |      
// TokenList Tail <--> TokenList Head  |<interpreter> | PeekList Head <--> PeekList Tail  |
// token token token token token token | currentToken | token token token token token ... |
//----------------------------------------------------------------------------------------|

#if 0

byte *
_CfrTil_AddSymbolToHeadOfTokenList ( Symbol * tknSym )
{
    dllist_AddNodeToHead ( _CfrTil_->TokenList, ( dlnode* ) tknSym ) ;
}

byte *
_CfrTil_AddNewTokenSymbolToHeadOfTokenList ( byte * token )
{
    Symbol * tknSym = _Symbol_New ( token, TEMPORARY ) ;
    _CfrTil_AddSymbolToHeadOfTokenList ( tknSym ) ;
}
#endif

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

dobject *
_CfrTil_FindSourceCodeNode_AtAddress ( Word * word, byte * address )
{
    byte * caddress ;
    dllist * list = word ? word->DebugWordList : 0 ;
    dlnode * node ;
    if ( list && address )
    {
        for ( node = dllist_First ( ( dllist* ) list ) ; node ; node = dlnode_Next ( node ) )
        {
            caddress = ( byte* ) dobject_Get_M_Slot ( node, DOBJ_SC_CADDRESS ) ;
            if ( address == caddress )
            {
                d0 ( //if ( Is_DebugOn )
                {
                    Word * word0 = ( Word* ) dobject_Get_M_Slot ( node, DOBJ_SC_WORD ) ;
                    Printf ( "\nFound node     = 0x%08x : word Name = \'%-12s\'\t : at address  = 0x%08x\n", node, word0->Name, address ) ;
                    //Printf ( "\nAdjusting node = 0x%08x : word Name = \'%-12s\'\t : old address = 0x%08x : new address = 0x%08x\n", node, word0->Name, dobject_Get_M_Slot ( node, 0 ), newAddress ) ;
                    //_dobject_Print ( ( dobject * ) node ) ;
                } ) ;
                return ( dobject * ) node ;
            }
        }
    }
    return 0 ;
}

byte *
PrepareSourceCodeString ( Word * scWord, Word * word, int32 wi )
{
    byte * sc = scWord->SourceCode, *name ;
    byte * buffer = Buffer_Data ( _CfrTil_->DebugB2 ) ;
    memset ( buffer, 0, BUFFER_SIZE ) ;
    int32 i, j, k, n, nd = 0, tp = 34, wl, wl0, cl = strlen ( sc ), tw = GetTerminalWidth ( ), svWi ; //, tabs = _String_CountTabs ( sc, &sc[wi] ), extraCharsPerTab = 1 ;
    if ( tw > 160 ) tw -= 60 ; // 60 : adjust for possible tabs on the line //( tw > 80 ) ? 80 : tw ;
    name = word->Name ;
    wl0 = strlen ( name ) ; // nb! : wl0 is strlen before c_dd transform below
    if ( strncmp ( name, & sc [wi], strlen ( name ) ) )
    {
        for ( svWi = wi ; wi > 0 ; wi -- )
        {
            if ( sc [wi] == name [0] )
            {
                if ( strlen ( name ) > 1 )
                {
                    if ( sc [wi + 1] == name [1] ) break ;
                }
                else break ;
            }
        }
        if ( wi <= 0 ) wi = svWi ;
    }
    name = c_dd ( name ) ;
    wl = strlen ( name ) ;
    if ( wi < tp )
    {
        for ( i = 0, n = tp - wi - 1 ; n -- ; i ++ ) buffer [i] = ' ' ;
        strncat ( buffer, sc, wi ) ;
    }
    else if ( wi >= tp )
    {
        j = wi - tp ;
        if ( j >= 4 ) nd = 3 ;
        if ( nd ) for ( i = 0, k = nd ++ ; k -- ; i ++ ) buffer [i] = '.', strncat ( buffer, " ", 1 ) ; // nd++ : count the space here
        strncat ( buffer, &sc [ nd + j + 1 ], tp - nd - 1 ) ; // 1 : 0 indexed array adjust
    }
    strncat ( buffer, name, wl ) ;
    strncat ( buffer, &sc [ wi + wl0 ], ( cl - wi - wl0 ) >= tw ? tw : ( cl - wi - wl0 ) ) ; // wi + wl : after the wi word which we concated above
    return buffer ;
}

void
_CfrTil_AdjustSourceCodeAddress ( byte * address, byte * newAddress )
{
    dobject * node = _CfrTil_FindSourceCodeNode_AtAddress ( _Context_->Compiler0->CurrentWord, address ) ;
    if ( node )
    {
        d0
            (
            Word * word0 = ( Word* ) dobject_Get_M_Slot ( node, DOBJ_SC_WORD ) ;
            Printf ( "\nAdjusting node = 0x%08x : word Name = \'%-12s\'\t : old address = 0x%08x : new address = 0x%08x\n", node, word0->Name, dobject_Get_M_Slot ( node, 0 ), newAddress ) ;
            //if ( Is_DebugOn ) _dobject_Print ( ( dobject * ) node ) ;
            ) ;
        dobject_Set_M_Slot ( node, DOBJ_SC_CADDRESS, newAddress ) ;
        d0
            (
            //if ( Is_DebugOn ) _dobject_Print ( ( dobject * ) node ) ;
            ) ;
    }
}

void
_Debugger_ShowSourceCodeAtAddress ( Debugger * debugger )
{
    // ...source code source code TP source code source code ... EOL
    Word * scWord = debugger->w_Word, *word ;
    int32 wordIndex ;
    dobject * dobj = _CfrTil_FindSourceCodeNode_AtAddress ( scWord, debugger->DebugAddress ) ;
    if ( dobj )
    {
        wordIndex = dobject_Get_M_Slot ( dobj, DOBJ_SC_WORD_INDEX ) ;
        word = ( Word* ) dobject_Get_M_Slot ( dobj, DOBJ_SC_WORD ) ;
        //DebugColors ;
        byte * buffer = PrepareSourceCodeString ( scWord, word, wordIndex ) ; //if ( wordIndex < TP ) 
        _Printf ( ( byte* ) "%s\n", buffer ) ; //&word->SourceCode [wordIndex] ) ;
        //DefaultColors ;
    }
}

void
_SC_SetSourceCodeAddress ( int32 index )
{
    dobject * dobj = ( dobject* ) _dllist_Get_N_Node_M_Slot ( _Context_->Compiler0->WordList, index, DOBJ_SC_WORD_INDEX ) ;
    if ( dobj )
    {
        dobject_Set_M_Slot ( dobj, DOBJ_SC_CADDRESS, Here ) ; // notice : we are setting the slot in the obj that was in slot 1 of the 
        // WordList node not in the WordList node which will be recycled soon 
        d0
            (
            Word * word0 = ( Word* ) dobject_Get_M_Slot ( dobj, DOBJ_SC_WORD ) ;
            Printf ( "\nSetting Source Code Address : dobject = 0x%08x : word Name = \'%-12s\'\t : sca = 0x%08x\n", dobj, word0 ? word0->Name : ( byte* ) "", Here ) ;
            //if ( Is_DebugOn ) _dobject_Print ( dobj ) ;
            ) ;
    }
}

void
_CfrTil_SetSourceCodeAddress ( int32 index )
{
    if ( GetState ( _CfrTil_, SOURCE_CODE_MODE ) ) //&& ( _CfrTil_->SCA_BlockedIndex != index ) )
    {
        if ( GetState ( _CfrTil_, SCA_ON ) || ( GetState ( _CfrTil_, OPTIMIZE_ON ) && ( ! GetState ( _CfrTil_, IN_OPTIMIZER ) ) ) ) return ;
        _SC_SetSourceCodeAddress ( index ) ;
    }
}

void
_CfrTil_WordLists_PushWord ( Word * word )
{
    if ( ! ( word->CProperty & ( DEBUG_WORD ) ) )
    {
        dobject * dobj = 0 ;
        if ( GetState ( _CfrTil_, SOURCE_CODE_MODE ) && Compiling )
        {
            dobj = DebugWordList_NewNode ( _CfrTil_->SC_ScratchPadIndex - strlen ( word->Name ) - 1, word ) ;
            DbgWL_Push ( dobj ) ;
        }
        CompilerWordList_Push ( word, dobj ) ;
    }
}

