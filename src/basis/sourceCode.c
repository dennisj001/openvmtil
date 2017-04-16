
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
            _Printf ( "\nDebugWordList : index %2d : word = \'%-12s\' : address = 0x%8x : scwi = %d", index, word->Name, address, scwi ) ;
        }
        //_Pause ( ) ;
    }
}

// ...source code source code TP source code source code ... EOL
#if 0

byte *
PrepareSourceCodeString ( dobject * dobj, Word * scWord, Word * word, int32 scwi0 ) // scwi : source code word index
{
    byte * sc = scWord->SourceCode, *name0, *name = word->Name ;
    byte * buffer = Buffer_Data ( _CfrTil_->DebugB1 ) ;
    memset ( buffer, 0, BUFFER_SIZE ) ;
    int32 scwi, i, j, k, n, tp, wl, wl0, tw = GetTerminalWidth ( ), space, inc = 20 ; // tp: text point 
    dobject * dobj2 ;
    //if ( tw > 160 ) tw = 120 ; // 60 : adjust for possible tabs on the line //( tw > 80 ) ? 80 : tw ;
    tp = 34 ; // text point aligned with disassembly
start:
    name0 = String_ConvertToBackSlash ( name ) ;
    wl0 = Strlen ( name ) ; // nb! : wl0 is Strlen before c_dd transform below
    scwi = scwi0 ; // source code word index
    scwi0 -= wl0 ; /// word length
    d1 ( byte * scspp = & sc [ scwi0 ] ) ;
    i = 0, n = wl0 + inc ;
    scwi = String_FindStrnCmpIndex ( sc, name, &i, scwi0, wl0, 20 ) ;
    d1 ( byte * scspp2 = & sc [ scwi ] ) ;
#if 1    
    if ( i > n )
    {
        dlnode_Remove ( ( node* ) dobj ) ; // so we don't find the same one again
        dobj2 = ( dobject* ) DWL_Find ( 0, _Debugger_->DebugAddress, 0, 0, 0, 0 ) ;
        if ( dobj2 )
        {
            dlnode_InsertThisBeforeANode ( ( node* ) dobj, ( node* ) dobj2 ) ; // so we can find it next time thru
            word = ( Word* ) dobject_Get_M_Slot ( dobj, SCN_SC_WORD ) ;
            scwi0 = dobject_Get_M_Slot ( dobj, SCN_WORD_SC_INDEX ) ;
            dobj = dobj2 ;
            goto start ;
        }
    }
#endif    
    name = c_dd ( name0 ) ;
    wl = Strlen ( name ) ;
    if ( scwi < tp ) // tp: text point 
    {
        for ( i = 0, n = tp - scwi - 3 ; ( -- n ) >= 0 ; i ++ ) buffer [i] = ' ' ;
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
#else

byte *
PrepareSourceCodeString ( dobject * dobj, Word * scWord, Word * word, int32 scwi0 ) // scwi : source code word index
{
    //byte * cc_line = Debugger_ShowSourceCodeLine ( debugger, word, token0, ( int32 ) Strlen ( obuffer ) ) ;
    byte * sc = scWord->SourceCode ;
    byte *nvw, * token0 = word->Name, *token1 ;
    int32 scwi, i = 0, tp = 34, lef, leftBorder, ts, rightBorder, ref; // tp : text point - where we want to start source code text to align with disassembly
start:
    token1 = String_ConvertToBackSlash ( token0 ) ;
    int32 tw = Debugger_TerminalLineWidth ( _Debugger_ ), slt0 = Strlen ( token0 ), slt1 = Strlen ( token1 ) ; // 3 : 0,1,2,3 ; ts : tokenStart
    int32 dl = slt1 - slt0, inc = 20 ; //n = slt0 + 20 ;
    dl = dl > 0 ? dl : 0 ;
    scwi = scwi0 ; // source code word index
    scwi0 -= slt0 ;
    byte * scspp2, *scspp ;
    d1 ( scspp = & sc [ scwi0 ] ) ;
    scwi = String_FindStrnCmpIndex ( sc, token0, &i, scwi0, slt0, inc ) ;
    d1 ( scspp2 = & sc [ scwi ] ) ;
#if 0    
    if ( i > n )
    {
        dlnode_Remove ( ( node* ) dobj ) ; // so we don't find the same one again
        dobject * dobj2 = ( dobject* ) DWL_Find ( 0, _Debugger_->DebugAddress, 0, 0, 0, 0 ) ;
        if ( dobj2 )
        {
            // a little tricky here
            dlnode_InsertThisBeforeANode ( ( node* ) dobj, ( node* ) dobj2 ) ; // so we can find it next time thru
            word = ( Word* ) dobject_Get_M_Slot ( dobj, SCN_SC_WORD ) ;
            scwi0 = dobject_Get_M_Slot ( dobj, SCN_WORD_SC_INDEX ) ;
            dobj = dobj2 ;
            goto start ;
        }
    }
#endif      
    if ( (scwi + 3) > tp )
    {
        tp -= 3 ; // 3 : account for lef ellipsis [0,1,2,3]
        nvw = & sc [scwi - tp] ;
        leftBorder = ts = tp ; // 3 : 0, 1, 2, 3
        rightBorder = tw - ( ts + slt0 + dl ) ;
        lef = 1 ; ref = strlen ( nvw - 4 ) > tw ? 1 : 0 ; 
    }
    else //if ( (scwi + 3) <= tp ) 
    {
        nvw = ( char* ) Buffer_New_pbyte ( BUFFER_SIZE ) ;
        tp -= 3 ; // ?? this works, i don't exactly know why yet but i think it's because _String_HighlightTokenInputLine is geared to a left ellipsis
        int32 pad = tp - scwi ; //=
        for ( i = 0 ; i < pad ; i++ ) strcat ( nvw, " " ) ;
        strcat ( nvw, sc ) ;
        leftBorder = ts = tp ; // 3 : 0, 1, 2, 3
        rightBorder = tw - (scwi + slt0 + dl) ;
        lef = 0 ; ref = strlen ( nvw - 4 ) > tw ? 1 : 0 ; // ref ?? not always
    }
    rightBorder = rightBorder > 0 ? rightBorder : 0 ;    
    byte * cc_line = ( word ? _String_HighlightTokenInputLine ( nvw, lef, leftBorder, ts, token1, rightBorder, ref, dl ) : ( byte* ) "" ) ; // nts : new token start is a index into b - the nwv buffer
    return cc_line ;
}
#endif

void
_Debugger_ShowSourceCodeAtAddress ( Debugger * debugger )
{
    // ...source code source code TP source code source code ... EOL
    Word * scWord = debugger->DebugWordListWord, *word ;
    if ( debugger->DebugWordList ) //scWord->DebugWordList ) //GetState ( scWord, W_SOURCE_CODE_MODE ) )
    {
        int32 scwi, fixed = 0 ;
        dobject * dobj ;
        if ( _Q_->Verbosity > 4 ) DebugWordList_Show ( ) ;
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
            byte * buffer = PrepareSourceCodeString ( dobj, scWord, word, scwi ) ;
            _Printf ( ( byte* ) "\n%s", buffer ) ;
            debugger->LastSourceCodeIndex = scwi ;
            if ( fixed )
            {
                word->Name = ( byte* ) "store" ;
            }
        }
    }
}

#define SC_WINDOW 60

node *
DWL_Check_SCI ( dlnode * node, byte * address, int32 scwi, Word * wordn, dlnode ** maybeFoundNode, int32 * diff1 )
{
    dlnode *foundNode = 0 ;
    int32 adiff, _diff1 = * diff1 ;
    if ( ! _Debugger_->LastSourceCodeIndex ) _Debugger_->LastSourceCodeIndex = scwi ;
    else if ( scwi >= _Debugger_->LastSourceCodeIndex ) _diff1 = scwi - _Debugger_->LastSourceCodeIndex ;
    else _diff1 = _Debugger_->LastSourceCodeIndex - scwi ;
    if ( _diff1 <= adiff )
    {
        foundNode = node ;
        //if ( diff1 < 10 ) break ;
        if ( _diff1 ) adiff = _diff1 ;
        //continue ;
    }
        //else if ( GetState ( _Debugger_->DebugWordListWord, W_C_SYNTAX ) && GetState ( _Context_, C_SYNTAX ) && ( wordn->CProperty & COMBINATOR ) )
    else if ( GetState ( _Debugger_->DebugWordListWord, W_C_SYNTAX ) && ( wordn->CProperty & COMBINATOR ) )
    {
        foundNode = node ;
        if ( _diff1 ) adiff = _diff1 ;
    }
    else if ( ! foundNode )
    {
        if ( _Debugger_->LastSourceCodeAddress < address )
        {
            if ( _diff1 < SC_WINDOW ) foundNode = node ;
            else *maybeFoundNode = node ;
        }
        else foundNode = node ;
    }
    if ( ( _Q_->Verbosity > 3 ) && foundNode )
    {
        DWL_ShowNode ( foundNode, "FOUND" ) ;
    }
    *diff1 = _diff1 ;
    return foundNode ;
}

node *
DWL_Find ( Word * word, byte * address, byte* name, int32 fromFirst, int32 takeFirstFind, byte * newAddress )
{
    byte * caddress ;
    Word * wordn ;
    dllist * list = _CfrTil_->DebugWordList ; // must use _CfrTil_ because _CfrTil_AdjustSourceCodeAddress must be use DWL_Find
    dlnode * node = 0, *foundNode = 0, *maybeFoundNode = 0 ;
    int32 numFound = 0 ;
    uint32 adiff = SC_WINDOW, diff1 = 0, scwi ; //, scwi0 = - 1 ;

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
                numFound ++ ;
                scwi = dobject_Get_M_Slot ( ( dobject * ) node, SCN_WORD_SC_INDEX ) ;
                if ( newAddress && ( address == caddress ) )
                {
                    dobject_Set_M_Slot ( ( dobject * ) node, SCN_SC_CADDRESS, newAddress ) ;
                    if ( _Q_->Verbosity > 2 ) _Printf ( "\nDWL_Find : ADDRESS ADJUST : word = \'%-12s\' : address = 0x%8x : scwi = %d : newAddress = 0x%8x", wordn->Name, address, scwi, newAddress ) ;
                    continue ;
                }
                else if ( takeFirstFind ) break ;
                foundNode = DWL_Check_SCI ( node, address, scwi, wordn, &maybeFoundNode, &diff1 ) ;
            }
        }
    }
    if ( ( ! foundNode ) && maybeFoundNode ) foundNode = maybeFoundNode ;
    if ( ( ! newAddress ) && ( _Q_->Verbosity > 3 ) && ( numFound ) )
    {
        _Printf ( ( byte* ) "\nNumber Found = %d :: diff1 = %d : window = %d : Choosen node = 0x%8x :", numFound, diff1, adiff, foundNode ) ;
        if ( foundNode ) DWL_ShowNode ( foundNode, "CHOSEN" ) ;
    }
    if ( address ) _Debugger_->LastSourceCodeAddress = address ;
    return foundNode ;
}

// DWL - DebugWordList : _CfrTil_->DebugWordList 

void
_CfrTil_AdjustSourceCodeAddress ( byte * address, byte * newAddress )
{
    if ( IsSourceCodeOn ) //_CfrTil_->DebugWordList )
    {
        DWL_Find ( 0, address, 0, 0, 0, newAddress ) ;
    }
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
        SC_SetForcePush ( false ) ; // one shot only
    }
}

void
DWL_CheckPush_Word ( )
{
    _DWL_CheckPush_Word ( _Context_->CurrentlyRunningWord ) ;
}

void
SC_DWL_PushCWL_Index ( int32 index )
{
    _DWL_CheckPush_Word ( Compiler_WordList ( index ) ) ;
}

dobject *
DebugWordList_PushWord ( Word * word )
{
    dobject * dobj = 0 ;
    if ( word && IsSourceCodeOn )
    {
        int32 scindex ;
        scindex = ( GetState ( _Compiler_, ( LC_ARG_PARSING | DOING_A_PREFIX_WORD ) ) || ( word->CProperty & COMBINATOR ) ) ?
            ( word->W_SC_ScratchPadIndex ? word->W_SC_ScratchPadIndex : _CfrTil_->SC_ScratchPadIndex ) : _CfrTil_->SC_ScratchPadIndex ;
        dobj = Node_New_ForDebugWordList ( TEMPORARY, scindex, word ) ; // _dobject_New_M_Slot_Node ( TEMPORARY, WORD_LOCATION, 3, 0, scindex, word ) 
        dobject_Set_M_Slot ( ( dobject* ) dobj, SCN_SC_CADDRESS, Here ) ;
        DbgWL_Push ( dobj ) ; // _dllist_AddNodeToHead ( _CfrTil_->DebugWordList, ( dlnode* ) dobj )
        if ( _Q_->Verbosity > 2 ) _Printf ( ( byte* ) "\n\tDebugWordList_PushWord :: PUSHED :: word->Name = \'%-10s\' : code address  = 0x%08x : sc_index = %4d",
            word->Name, Here, scindex ) ;
    }
    return dobj ;
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
CfrTil_SourceCodeBeginBlock ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, true ) ;
    if ( ! GetState ( _Context_, C_SYNTAX ) ) CfrTil_BeginBlock ( ) ;
}

void
CfrTil_SourceCodeEndBlock ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, false ) ;
    if ( ! GetState ( _Context_, C_SYNTAX ) ) CfrTil_EndBlock ( ) ;
}

void
CfrTil_SourceCode_Begin_C_Block ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, true ) ;
    Word * word = _Context_->Compiler0->CurrentWord ;
    //if ( ! word->DebugWordList ) word->DebugWordList = _dllist_New ( DICTIONARY ) ;
    if ( ! word->DebugWordList ) word->DebugWordList = _dllist_New ( TEMPORARY ) ;
}

void
CfrTil_SourceCode_End_C_Block ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, false ) ;
    CfrTil_End_C_Block ( ) ;
}


