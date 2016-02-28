
#include "../includes/cfrtil.h"

void
_CfrTil_Run ( CfrTil * cfrTil, int32 restartCondition )
{
    while ( 1 )
    {
        Linux_SetupSignals ( 1 ) ; 
        OVT_MemListFree_Session ( ) ;
        cfrTil = _CfrTil_New ( cfrTil ) ;
        if ( cfrTil )
        {
            if ( ! setjmp ( cfrTil->JmpBuf0 ) )
            {
                System_RunInit ( _Q_->OVT_Context->System0 ) ;
                _CfrTil_Restart ( cfrTil, restartCondition ) ;
                // check if reset is ok ...
                if ( cfrTil && _Q_->OVT_Context && _Q_->OVT_Context->System0 )
                {
                    Ovt_RunInit ( _Q_ ) ;
                    System_Time ( cfrTil->Context0->System0, 0, ( char* ) "Startup", 1 ) ; //_Q_->StartedTimes == 1 ) ;
                    _CfrTil_Version ( 0 ) ;
                    CfrTil_InterpreterRun ( ) ;
                }
            }
        }
    }
}

void
_CfrTil_Restart ( CfrTil * cfrTil, int32 restartCondition )
{
    switch ( restartCondition )
    {
        case 0:
        case INITIAL_START:
        case FULL_RESTART:
        case RESTART:
        case RESET_ALL: CfrTil_ResetAll_Init ( cfrTil ) ;
        case ABORT: CfrTil_SyncStackPointerFromDsp ( cfrTil ) ;
        default:
        case QUIT:
        case STOP: ;
    }
}

void
CfrTil_CpuState_Show ( )
{
    _Q_->OVT_CfrTil->SaveCpuState ( ) ;
    _CpuState_Show ( _Q_->OVT_CfrTil->cs_CpuState ) ;
}

void
_CfrTil_NamespacesInit ( CfrTil * cfrTil )
{
    Namespace * ns = _DataObject_New ( NAMESPACE, 0, ( byte* ) "Namespaces", 0, 0, 0, 0, 0 ) ;
    ns->State = USING ; // nb. _Namespace_SetState ( ns, USING ) ; // !! can't be used with "Namespaces"
    cfrTil->Namespaces = ns ;
    CfrTil_AddCPrimitives ( ) ;
}

void
_CfrTil_SetStackPointerFromDsp ( CfrTil * cfrTil )
{
    cfrTil->DataStack->StackPointer = Dsp ;
}

void
CfrTil_SyncStackPointerFromDsp ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack ) _CfrTil_SetStackPointerFromDsp ( cfrTil ) ;
}

void
_CfrTil_SetDspFromStackPointer ( CfrTil * cfrTil )
{
    Dsp = cfrTil->DataStack->StackPointer ;
}

void
CfrTil_SetDspFromStackPointer ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack ) _CfrTil_SetDspFromStackPointer ( cfrTil ) ;
}

void
CfrTil_DataStack_InitEssential ( CfrTil * cfrTil )
{
    Stack * stk = cfrTil->DataStack ;
    _Stack_Init ( stk, _Q_->DataStackSize ) ;
    _CfrTil_SetDspFromStackPointer ( cfrTil ) ;
    cfrTil->SaveDsp = Dsp ;
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
    _CfrTil_DataStack_Init ( _Q_->OVT_CfrTil ) ;
}

void
_CfrTil_Init ( CfrTil * cfrTil, Namespace * nss )
{
    uint32 type = CFRTIL ;
    _Q_->OVT_CfrTil = cfrTil ;
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
    CfrTil_SetState ( cfrTil, CFRTIL_RUN | OPTIMIZE_ON | INLINE_ON, true ) ;
    if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\nSystem Memory is being reallocated.  " ) ;
    cfrTil->ContextStack = Stack_New ( 256, type ) ;
    cfrTil->ObjectStack = Stack_New ( 1 * K, type ) ;
    cfrTil->TokenList = _DLList_New ( type ) ;
    _Q_->OVT_Context = cfrTil->Context0 = _Context_New ( cfrTil, type ) ;
    cfrTil->Debugger0 = _Debugger_New ( type ) ; // nb : must be after System_NamespacesInit
    if ( nss ) // && ( _Q_->Signal <= ABORT ) )
    {
        cfrTil->Namespaces = nss ;
    }
    else
    {
        _CfrTil_NamespacesInit ( cfrTil ) ;
        CfrTil_MachineCodePrimitive_AddWords ( ) ;
        cfrTil->StoreWord = _Word_FindAny ( ( byte* ) "store" ) ;
        cfrTil->PokeWord = _Word_FindAny ( ( byte* ) "poke" ) ;
        cfrTil->LispNamespace = Namespace_Find ( ( byte* ) "Lisp" ) ;
    }
    if ( cfrTil->SaveDsp && cfrTil->DataStack )// with _Q_->RestartCondition = STOP from Debugger_Stop
    {
        Dsp = cfrTil->SaveDsp ;
    }
    else
    {
        cfrTil->DataStack = Stack_New ( _Q_->DataStackSize, CFRTIL ) ; // type ) ;
        _CfrTil_DataStack_Init ( cfrTil ) ;
    }
    CfrTil_ReadTables_Setup ( cfrTil ) ;
    CfrTil_LexerTables_Setup ( cfrTil ) ;
    cfrTil->LC = 0 ; //LC_New ( ) ;
    cfrTil->cs_CpuState = CpuState_New ( type ) ;
}

void
CfrTil_ResetMemory ( )
{
    OVT_MemListFree_CfrTilInternal ( ) ;
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
    return cfrTil ;
}

void
CfrTil_Lexer_SourceCodeOn ( )
{
    Lexer_SourceCodeOn ( _Q_->OVT_Context->Lexer0 ) ;
}

void
_CfrTil_AddStringToSourceCode ( byte * str )
{
    strcat ( ( char* ) _Q_->OVT_CfrTil->SourceCodeScratchPad, ( char* ) str ) ;
    strcat ( ( CString ) _Q_->OVT_CfrTil->SourceCodeScratchPad, ( CString ) " " ) ;
}

void
SC_ScratchPadIndex_Init ( )
{
    _Q_->OVT_CfrTil->SC_ScratchPadIndex = strlen ( ( char* ) _Q_->OVT_CfrTil->SourceCodeScratchPad ) ;
}

void
__CfrTil_InitSourceCode ( )
{
    _Q_->OVT_CfrTil->SourceCodeScratchPad [ 0 ] = 0 ;
    _Q_->OVT_CfrTil->SC_ScratchPadIndex = 0 ;
    SetState ( _Q_->OVT_CfrTil, SOURCE_CODE_INITIALIZED, true ) ;
}

void
_InitSourceCode ( int32 force )
{
    if ( force || ( ! GetState ( _Q_->OVT_CfrTil, SOURCE_CODE_INITIALIZED ) ) )
    {
        Lexer_SourceCodeOn ( _Q_->OVT_Context->Lexer0 ) ;
        __CfrTil_InitSourceCode ( ) ;
    }
}

void
_CfrTil_InitSourceCode ( )
{
    _InitSourceCode ( 1 ) ;
    SC_ScratchPadIndex_Init ( ) ;
}

void
_CfrTil_InitSourceCode_WithName ( byte * name )
{
    _InitSourceCode ( 1 ) ;
    _CfrTil_AddStringToSourceCode ( name ) ;
    SC_ScratchPadIndex_Init ( ) ;
}

void
CfrTil_InitSourceCode_WithCurrentInputChar ( )
{
    Lexer * lexer = _Q_->OVT_Context->Lexer0 ;
    _InitSourceCode ( 1 ) ;
    _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter ) ;
}

void
_CfrTil_FinishSourceCode ( Word * word )
{
    // keep a LambdaCalculus LO_Define0 created SourceCode value
    if ( ! word->SourceCode ) word->SourceCode = String_New ( _Q_->OVT_CfrTil->SourceCodeScratchPad, DICTIONARY ) ;
    Lexer_SourceCodeOff ( _Q_->OVT_Context->Lexer0 ) ;
    SetState ( _Q_->OVT_CfrTil, SOURCE_CODE_INITIALIZED, false ) ;
}

void
_CfrTil_UnAppendFromSourceCode ( int nchars )
{
    int plen = strlen ( ( CString ) _Q_->OVT_CfrTil->SourceCodeScratchPad ) ;
    if ( plen >= nchars )
    {
        _Q_->OVT_CfrTil->SourceCodeScratchPad [ strlen ( ( CString ) _Q_->OVT_CfrTil->SourceCodeScratchPad ) - nchars ] = 0 ;
    }
    SC_ScratchPadIndex_Init ( ) ;
}

void
_CfrTil_UnAppendTokenFromSourceCode ( byte * tkn )
{
    _CfrTil_UnAppendFromSourceCode ( strlen ( ( CString ) tkn ) + 1 ) ;
}

void
_CfrTil_AppendCharToSourceCode ( byte c )
{
    if ( _Q_->OVT_CfrTil->SC_ScratchPadIndex < ( SOURCE_CODE_BUFFER_SIZE - 1 ) )
    {
        _Q_->OVT_CfrTil->SourceCodeScratchPad [ _Q_->OVT_CfrTil->SC_ScratchPadIndex ++ ] = c ;
        _Q_->OVT_CfrTil->SourceCodeScratchPad [ _Q_->OVT_CfrTil->SC_ScratchPadIndex ] = 0 ;
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
    DLList_AddNodeToHead ( _Q_->OVT_CfrTil->TokenList, ( DLNode* ) tknSym ) ;
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
    if ( tknSym = ( Symbol* ) _DLList_First ( _Q_->OVT_CfrTil->TokenList ) )
    {
        DLNode_Remove ( ( DLNode* ) tknSym ) ;
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
    tknSym->S_Value = _Q_->OVT_Context->Lexer0->TokenStart_ReadLineIndex ;
    tknSym->S_Value2 = _Q_->OVT_Context->Lexer0->TokenEnd_ReadLineIndex ;
    DLList_AddNodeToTail ( _Q_->OVT_CfrTil->TokenList, ( DLNode* ) tknSym ) ;
}

void
_CfrTil_AddTokenToHeadOfTokenList ( byte * token )
{
    Symbol * tknSym = _Symbol_New ( token, TEMPORARY ) ;
    tknSym->S_Value = _Q_->OVT_Context->Lexer0->TokenStart_ReadLineIndex ;
    tknSym->S_Value2 = _Q_->OVT_Context->Lexer0->TokenEnd_ReadLineIndex ;
    DLList_AddNodeToHead ( _Q_->OVT_CfrTil->TokenList, ( DLNode* ) tknSym ) ;
}

void
CfrTil_OptimizeOn ( )
{
    SetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON, true ) ;
}

void
CfrTil_OptimizeOff ( )
{
    SetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON, false ) ;
}

void
CfrTil_StringMacrosOn ( )
{
    SetState ( _Q_->OVT_CfrTil, STRING_MACROS_ON, true ) ;
    _CfrTil_StringMacros_Init ( ) ;
}

void
CfrTil_StringMacrosOff ( )
{
    SetState ( _Q_->OVT_CfrTil, STRING_MACROS_ON, false ) ;
    SetState ( &_Q_->OVT_CfrTil->Sti, STI_INITIALIZED, false ) ;
}

void
CfrTil_InlineOn ( )
{
    SetState ( _Q_->OVT_CfrTil, INLINE_ON, true ) ;
}

void
CfrTil_InlineOff ( )
{
    SetState ( _Q_->OVT_CfrTil, INLINE_ON, false ) ;
}

// "d:"

void
CfrTil_DebugOn ( )
{
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, true ) ;
    SetState ( _Q_->OVT_CfrTil->Debugger0, DBG_PRE_DONE | DBG_INTERPRET_LOOP_DONE | DBG_AUTO_MODE, false ) ;
    _Q_->OVT_CfrTil->Debugger0->StartHere = 0 ;
}

void
CfrTil_DebugOff ( )
{
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, false ) ;
}

