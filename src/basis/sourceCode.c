
#include "../include/cfrtil.h"

/*
 * 
 * Compiler Word List has nodes (CWLNs) with 2 slots one for the *word and one for a pointer to a Source Code Node (SCN) which has source code index info.
 * CWLN : slot 0 word, slot 1 SCN
 * Source code nodes (SCNs) have three slots for the source code byte index, the code address, and a pointer to the word, they are on the _CfrTil_->DebugWordList.
 * SCN : slot 0 : SCN_SC_CADDRESS, slot 1 : SCN_WORD_SC_INDEX, slot 2 : SCN_SC_WORD
 * So, they each have pointers to each other.
 * 
 */
const int64 SC_WINDOW = 60 ;
const int64 SCWI_MAX_DIFF = 60 ;
const int64 SCWI_MIN_DIFF = 2 ;

node *
DWL_Check_SCWI ( dlnode * node, byte * address, int64 scwi, Word * wordn, dlnode ** maybeFoundNode, int64 * diff1 )
{
    dlnode *foundNode = 0 ;
    int64 scwiDiff ;
    if ( ! _Debugger_->LastSourceCodeIndex ) _Debugger_->LastSourceCodeIndex = scwi ;
    scwiDiff = abs ( scwi - _Debugger_->LastSourceCodeIndex ) ;
    if ( ( scwiDiff < SCWI_MAX_DIFF ) ) foundNode = node ;
    else if ( GetState ( _CfrTil_->DebugWordListWord, W_C_SYNTAX ) && ( wordn->CProperty & COMBINATOR ) )
    {
        foundNode = node ;
    }
#if 0    
    else if ( ! foundNode )
    {
        if ( _Debugger_->LastSourceCodeAddress < address )
        {
            if ( scwiDiff < SC_WINDOW ) *maybeFoundNode = node ;
        }
        else foundNode = node ;
    }
#endif    
    if ( ( _Q_->Verbosity > 3 ) && foundNode )
    {
        DWL_ShowNode ( foundNode, "FOUND", scwiDiff ) ;
    }
    *diff1 = scwiDiff ;
    return foundNode ;
}

dlnode *
DWL_Find ( Word * word, byte * address, byte* name, int64 fromFirst, int64 takeFirstFind, byte * newAddress )
{
    byte * naddress ;
    Word * nword ;
    dllist * list = _CfrTil_->DebugWordList ; // must use _CfrTil_ because _CfrTil_AdjustSourceCodeAddress must be use DWL_Find
    dlnode * node = 0, *foundNode = 0, *afoundNode, *maybeFoundNode = 0 ;
    int64 numFound = 0 ;
    uint64 wdiff = SC_WINDOW, fDiff, minDiffFound = SCWI_MAX_DIFF, scwi ; //, scwi0 = - 1 ;

    if ( list && ( word || name || address ) )
    {
        for ( node = fromFirst ? dllist_First ( ( dllist* ) list ) : dllist_Last ( ( dllist* ) list ) ; node ;
            node = fromFirst ? dlnode_Next ( node ) : dlnode_Previous ( node ) )
        {
            naddress = ( byte* ) dobject_Get_M_Slot ( node, SCN_SC_CADDRESS ) ;
            nword = ( Word* ) dobject_Get_M_Slot ( node, SCN_SC_WORD ) ;
            //if ( ( word && ( word == nword ) ) || ( name && nword && ( String_Equal ( nword->Name, name ) ) ) ) foundNode = node ;
            if ( address == naddress )
            {
                numFound ++ ;
                scwi = dobject_Get_M_Slot ( ( dobject * ) node, SCN_WORD_SC_INDEX ) ;
                if ( newAddress ) //&& ( address == caddress ) )
                {
                    dobject_Set_M_Slot ( ( dobject * ) node, SCN_WORD_SC_INDEX, scwi ) ;
                    dobject_Set_M_Slot ( ( dobject * ) node, SCN_SC_CADDRESS, newAddress ) ;
                    if ( _Q_->Verbosity > 2 )
                    {
                        _Printf ( "\nDWL_Find : DebugWordListWord = %s : ADDRESS ADJUST : word = 0x%8x : word->Name = \'%-12s\' : address = 0x%8x : scwi = %4d : newAddress = 0x%8x", _CfrTil_->DebugWordListWord->Name, nword, nword->Name, address, scwi, newAddress ) ;
                    }
                    continue ;
                }
                //foundNode = node ;
                afoundNode = DWL_Check_SCWI ( node, address, scwi, nword, &maybeFoundNode, &fDiff ) ;
                if ( afoundNode && ( fDiff <= minDiffFound ) ) //&& ( nword->Name[0] != '}' ) )
                {
                    minDiffFound = fDiff ;
                    foundNode = afoundNode ;
                }
                if ( ( _Q_->Verbosity > 2 ) ) //&& afoundNode )
                {
                    DWL_ShowNode ( node, "FOUND", fDiff ) ;
                }
                if ( foundNode && takeFirstFind ) break ;
            }
        }
    }
    if ( ( ! foundNode ) && maybeFoundNode ) foundNode = maybeFoundNode ;
    if ( ( ! newAddress ) && ( _Q_->Verbosity > 2 ) && ( numFound ) )
    {
        _Printf ( ( byte* ) "\nNumber Found = %d :: minDiffFound = %d : window = %d : Choosen node = 0x%8x :", numFound, minDiffFound, wdiff, foundNode ) ;
        if ( foundNode ) DWL_ShowNode ( foundNode, "CHOSEN", minDiffFound ) ;
    }
    if ( address ) _Debugger_->LastSourceCodeAddress = address ;
    return foundNode ;
}

void
_Debugger_ShowSourceCodeAtAddress ( Debugger * debugger )
{
    // ...source code source code TP source code source code ... EOL
    Word * scWord = debugger->w_Word, *word ;
    if ( _CfrTil_->DebugWordList ) //scWord->DebugWordList ) //GetState ( scWord, W_SOURCE_CODE_MODE ) )
    {
        int64 scwi, fixed = 0 ;
        dobject * dobj ;
        //if ( _Q_->Verbosity > 2 ) DebugWordList_Show ( ) ;
        dobj = ( dobject* ) DWL_Find ( 0, debugger->DebugAddress, 0, 0, 0, 0 ) ;
        if ( dobj )
        {
            word = ( Word* ) dobject_Get_M_Slot ( dobj, SCN_SC_WORD ) ;
            if ( GetState ( scWord, W_C_SYNTAX ) && String_Equal ( word->Name, "store" ) )
            {
                word->Name = ( byte* ) "=" ;
                fixed = 1 ;
            }
            scwi = dobject_Get_M_Slot ( dobj, SCN_WORD_SC_INDEX ) ;
            byte * buffer = PrepareSourceCodeString ( scWord->W_SourceCode, word, scwi ) ;
            _Printf ( ( byte* ) "\n%s", buffer ) ;
            debugger->LastSourceCodeIndex = scwi ;
            if ( fixed )
            {
                word->Name = ( byte* ) "store" ;
            }
        }
    }
}

dobject *
DebugWordList_PushWord ( Word * word )
{
    dobject * dobj = 0 ;
    word = Word_GetOriginalWord ( word ) ; // copied words are recycled
    if ( word && IsSourceCodeOn && _CfrTil_->DebugWordList )
    {
        int64 scindex ;
        scindex = ( GetState ( _Compiler_, ( LC_ARG_PARSING | DOING_A_PREFIX_WORD ) ) || ( word->CProperty & COMBINATOR ) ) ?
            ( word->W_SC_ScratchPadIndex ? word->W_SC_ScratchPadIndex : _CfrTil_->SC_ScratchPadIndex ) : _CfrTil_->SC_ScratchPadIndex ;
        dobj = Node_New_ForDebugWordList ( TEMPORARY, scindex, word ) ; // _dobject_New_M_Slot_Node ( TEMPORARY, WORD_LOCATION, 3, 0, scindex, word ) 
        dobject_Set_M_Slot ( ( dobject* ) dobj, SCN_SC_CADDRESS, Here ) ;
        _dllist_AddNodeToHead ( _CfrTil_->DebugWordList, ( dlnode* ) dobj ) ;
        if ( _Q_->Verbosity > 2 ) _Printf ( ( byte* ) "\n\tDebugWordList_PushWord %s :: PUSHED :: word = 0x%08x : word->Name = \'%-10s\' : code address  = 0x%08x : sc_index = %4d",
            _CfrTil_->DebugWordListWord->Name, word, word->Name, Here, scindex ) ;
        word->State |= W_SOURCE_CODE_MODE ;
        d0 ( if ( String_Equal ( word->Name, "power" ) ) _Printf ( "\nPush : Got it! : %s\n", word->Name ) ) ;
    }
    return dobj ;
}

void
Debugger_SetDebugWordList ( Debugger * debugger )
{
    if ( debugger->w_Word && debugger->w_Word->DebugWordList )
    {
        _CfrTil_->DebugWordListWord = debugger->w_Word ;
        debugger->DebugWordList = debugger->w_Word->DebugWordList ;
        _CfrTil_->DebugWordList = debugger->DebugWordList ;
    }
}

void
CfrTil_SourceCodeBeginBlock ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, true ) ;
    _SC_Global_On ;
    if ( ! GetState ( _Context_, C_SYNTAX ) ) CfrTil_BeginBlock ( ) ;
}

void
CfrTil_SourceCodeEndBlock ( )
{
    if ( ! GetState ( _Context_, C_SYNTAX ) ) CfrTil_EndBlock ( ) ;
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, false ) ;
}

void
CfrTil_SourceCode_SetDebugWordList ( Word * word )
{
    if ( ( ! word->DebugWordList ) || ( ! _CfrTil_->DebugWordList ) )
    {
        word->DebugWordList = _dllist_New ( TEMPORARY ) ;
        _CfrTil_->DebugWordList = word->DebugWordList ;
        _CfrTil_->DebugWordListWord = word ;
    }
}

void
CfrTil_SourceCode_Begin_C_Block ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, true ) ;
    _SC_Global_On ;
    Word * word = _Context_->Compiler0->CurrentWord ;
    CfrTil_SourceCode_SetDebugWordList ( word ) ;
}

void
CfrTil_SourceCode_End_C_Block ( )
{
    CfrTil_End_C_Block ( ) ;
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, false ) ;
}

void
CfrTil_SourceCodeOff ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, false ) ;
    SetState ( _CfrTil_, GLOBAL_SOURCE_CODE_MODE, false ) ;
    Compiler_WordList_RecycleInit ( _Compiler_ ) ;
}

// ...source code source code TP source code source code ... EOL

byte *
PrepareSourceCodeString ( byte * sc, Word * word, int64 scwi0 ) // sc : source code ; scwi : source code word index
{
    //byte * cc_line = Debugger_ShowSourceCodeLine ( debugger, word, token0, ( int64 ) Strlen ( obuffer ) ) ;
    byte *nvw, * token0 = word->Name, *token1 ;
    int64 scwi, i = 0, tp = 34, lef, leftBorder, ts, rightBorder, ref ; // tp : text point - where we want to start source code text to align with disassembly
start:
    token1 = String_ConvertToBackSlash ( token0 ) ;
    int64 tw = Debugger_TerminalLineWidth ( _Debugger_ ), slt0 = Strlen ( token0 ), slt1 = Strlen ( token1 ) ; // 3 : 0,1,2,3 ; ts : tokenStart
    int64 dl = slt1 - slt0, inc = 20 ; //n = slt0 + 20 ;
    dl = dl > 0 ? dl : 0 ;
    scwi = scwi0 ; // source code word index
    scwi0 -= slt0 ;
    byte * scspp2, *scspp ;
    d1 ( scspp = & sc [ scwi0 ] ) ;
    scwi = String_FindStrnCmpIndex ( sc, token0, &i, scwi0, slt0, inc ) ;
    d1 ( scspp2 = & sc [ scwi ] ) ;
    if ( ( scwi + 3 ) > tp )
    {
        tp -= 3 ; // 3 : account for lef ellipsis [0,1,2,3]
        nvw = & sc [scwi - tp] ;
        leftBorder = ts = tp ; // 3 : 0, 1, 2, 3
        rightBorder = tw - ( ts + slt0 + dl ) ;
        lef = 1 ;
        ref = strlen ( nvw - 4 ) > tw ? 1 : 0 ;
    }
    else //if ( (scwi + 3) <= tp ) 
    {
        nvw = ( char* ) Buffer_New_pbyte ( BUFFER_SIZE ) ;
        tp -= 3 ; // ?? this works, i don't exactly know why yet but i think it's because _String_HighlightTokenInputLine is geared to a left ellipsis
        int64 pad = tp - scwi ; //=
        for ( i = 0 ; i < pad ; i ++ ) strcat ( nvw, " " ) ;
        strcat ( nvw, sc ) ;
        leftBorder = ts = tp ; // 3 : 0, 1, 2, 3
        rightBorder = tw - ( scwi + slt0 + dl ) ;
        lef = 0 ;
        ref = strlen ( nvw - 4 ) > tw ? 1 : 0 ; // ref ?? not always
    }
    rightBorder = rightBorder > 0 ? rightBorder : 0 ;
    byte * cc_line = ( word ? _String_HighlightTokenInputLine ( nvw, lef, leftBorder, ts, token1, rightBorder, ref, dl ) : ( byte* ) "" ) ; // nts : new token start is a index into b - the nwv buffer
    return cc_line ;
}

// DWL - DebugWordList : _CfrTil_->DebugWordList 

void
_CfrTil_AdjustSourceCodeAddress ( byte * address, byte * newAddress )
{
    if ( IsSourceCodeOn && _CfrTil_->DebugWordList )
    {
        DWL_Find ( 0, address, 0, 0, 0, newAddress ) ;
    }
}

void
_CfrTil_WordLists_PushWord ( Word * word )
{

    dobject * dobj = 0 ;
    //if ( _IsSourceCodeOn ) dobj = DebugWordList_PushWord ( word ) ;
    CompilerWordList_Push ( word, dobj ) ; // _dllist_Push_M_Slot_Node ( _Compiler_->WordList, WORD, COMPILER_TEMP, 2, ((int64) word), ((int64) dnode) )
}

void
_DWL_CheckPush_Word ( Word * word )
{
    if ( word && IsSourceCodeOn && ( GetState ( _CfrTil_, SC_FORCE_PUSH ) || ( _CfrTil_->ScoOcCrw != word ) ) )
    {
        if ( ! ( word->LProperty & ( W_COMMENT | W_PREPROCESSOR ) ) )
        {
            _CfrTil_->ScoOcCrw = word ;
            DebugWordList_PushWord ( _CfrTil_->ScoOcCrw ) ;
        }
        SC_SetForcePush ( false ) ; // one shot only flag
    }
}

void
SC_DWL_PushCWL_Index ( int64 index )
{
    _DWL_CheckPush_Word ( Compiler_WordList ( index ) ) ;
}

void
CfrTil_DebugWordList_Pop ( )
{
    if ( _CfrTil_->DebugWordList )
    {
        node * first = _dllist_First ( _CfrTil_->DebugWordList ) ;
        if ( first ) dlnode_Remove ( first ) ;
    }
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
_CfrTil_WordLists_PopWord ( int64 n )
{
    while ( n -- )
    {
        CfrTil_WordList_Pop ( ) ;
        //CfrTil_DebugWordList_Pop ( ) ;
    }
}

void
CfrTil_WordLists_PopWord ( )
{
    _CfrTil_WordLists_PopWord ( 1 ) ;
}

void
DWL_ShowNode ( dlnode * node, byte * insert, int64 scwiDiff )
{
    if ( node )
    {
        int64 sc_index = dobject_Get_M_Slot ( node, SCN_WORD_SC_INDEX ) ;
        byte * address = ( byte* ) dobject_Get_M_Slot ( node, SCN_SC_CADDRESS ) ;
        Word * word = ( Word* ) dobject_Get_M_Slot ( node, SCN_SC_WORD ) ;
        _Printf ( ( byte* ) "\n\tDWL_ShowNode :: %s :: node :: = 0x%08x : word Name = \'%-12s\'\t : at address  = 0x%08x : with sc_index = %d, scwiDiff = %d",
            insert, node, word->Name, address, sc_index, scwiDiff ) ;
        d0 ( else _Printf ( ( byte* ) "\nDWL_ShowNode : word = 0x%08x : word Name = \'%-12s\'\t : at address  = 0x%08x : with index = %d", node, word->Name, address, sc_index ) ) ;
    }
}

void
DebugWordList_Show ( )
{
    dllist * list ;
    if ( Is_DebugModeOn && ( list = _CfrTil_->DebugWordList ) ) //GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) )
    {
        if ( _CfrTil_->DebugWordListWord ) _Printf ( "\nSourceCode DebugWordList : for word = %s", _CfrTil_->DebugWordListWord->Name ) ;
        int64 index ;
        dlnode * node, *nextNode ;
        for ( index = 0, node = dllist_First ( ( dllist* ) list ) ; node ; node = nextNode, index ++ )
        {
            nextNode = dlnode_Next ( node ) ;
            dobject * dobj = ( dobject * ) node ;
            byte * address = ( byte* ) dobject_Get_M_Slot ( dobj, SCN_SC_CADDRESS ) ; // notice : we are setting the slot in the obj that was in the SCN_SC_WORD_INDEX slot (1) of the 
            int64 scwi = dobject_Get_M_Slot ( dobj, SCN_WORD_SC_INDEX ) ;
            Word * word = ( Word * ) dobject_Get_M_Slot ( dobj, SCN_SC_WORD ) ;
            _Printf ( "\nDebugWordList : DebugWordListWord %s : index %2d : word = 0x%8x : \'%-12s\' : address = 0x%8x : scwi = %3d", _CfrTil_->DebugWordListWord->Name, index, word, word->Name, address, scwi ) ;
        }
        //_Pause ( ) ;
    }
}

#if 0

void
DWL_CheckPush_Word ( )
{
    _DWL_CheckPush_Word ( _Context_->CurrentlyRunningWord ) ;
}
#endif





