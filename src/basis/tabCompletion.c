
#include "../include/cfrtil.h"

void
RL_TabCompletion_Run ( ReadLiner * rl, Word * rword )
{
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    tci->StartFlag = 0 ;
    Word * nextWord ;
    //do
    {
        //nextWord = TC_Tree_Map_1 ( tci, _CfrTil_->Namespaces->W_List, ( MapFunction ) _TabCompletion_Compare, rword, &tci->StartFlag ) ; // working
        nextWord = TC_Tree_Map_2 ( _CfrTil_->Namespaces->W_List, ( MapFunction ) _TabCompletion_Compare, rword ) ; // new
        tci->NextWord = nextWord ; // wrap around
    }
    //while ( ( ( tci->WordWrapCount < 4 ) && ( ! tci->FoundCount ) ) ) ; //|| ( ! tci->FoundWrapCount )) ;
}

TabCompletionInfo *
TabCompletionInfo_New ( uint64 type )
{
    TabCompletionInfo *tci = ( TabCompletionInfo * ) Mem_Allocate ( sizeof (TabCompletionInfo ), type ) ;
    return tci ;
}

byte *
ReadLiner_GenerateFullNamespaceQualifiedName ( ReadLiner * rl, Word * w )
{
    byte * b0 = Buffer_Data_Cleared ( _CfrTil_->TabCompletionBuf ) ;
    Stack_Init ( rl->TciNamespaceStack ) ;
    Stack * nsStk = rl->TciNamespaceStack ;
    Namespace *ns ;
    byte * nsName, *c_udDot = 0 ;//( CString ) c_dd ( "." ) ;
    int64 i, dot = 0, notUsing = 0 ; //, ow = 0 ;

    String_Init ( b0 ) ;
    for ( ns = ( Is_NamespaceType ( w ) ? w : w->ContainingNamespace ) ; ns ; ns = ns->ContainingNamespace ) // && ( tw->ContainingNamespace != _Q_->CfrTil->Namespaces ) )
    {
        if ( ns->State & NOT_USING )
        {
            notUsing = 1 ;
        }
        _Stack_Push ( nsStk, ( int64 ) (( ns->State & NOT_USING ) ? c_ud ( ns->Name ) : ( ns->Name )) ) ;
    }
    if ( notUsing ) c_udDot = ( CString ) c_ud ( "." ) ;
    for ( i = Stack_Depth ( nsStk ) ; i ; i -- )
    { 
        nsName = ( byte* ) _Stack_Pop ( nsStk ) ;
        if ( nsName )
        {
            strcat ( ( CString ) b0, nsName ) ; //( CString ) notUsing ? c_dd ( nsName ) : nsName ) ;
            if ( i > 1 )
            {
                strcat ( ( CString ) b0, "." ) ; //( CString ) notUsing ? ( CString ) c_ddDot : ( CString ) "." ) ;
            }
        }
    }
    if ( ! String_Equal ( nsName, w->Name ) )
    {
        if ( ! dot ) strcat ( ( CString ) b0, ( CString ) notUsing ? ( CString ) c_udDot : ( CString ) "." ) ;
        strcat ( ( CString ) b0, ( CString ) notUsing ? c_ud ( w->Name ) : w->Name ) ; // namespaces are all added above
    }
    return b0 ;
}

void
TM_WrapWordCount ( TabCompletionInfo * tci, Word * word )
{
    if ( word == tci->OriginalRunWord ) //|| ( ! tci->OriginalRunWord ) )
    {
        //if ( tci->WordCount )
        tci->WordWrapCount ++ ;
        //else tci->OriginalRunWord = word ;
    }
    else if ( ! tci->OriginalRunWord ) tci->OriginalRunWord = word ;
}

Boolean
_TabCompletion_Compare ( Word * word )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    byte * searchToken ;
    int64 gotOne = 0, slst, sltwn, strOpRes = - 1 ; //, strOpRes1 = - 1, strOpRes2 = - 1, strOpRes3 = - 1 ;
    tci->WordCount ++ ;
    TM_WrapWordCount ( tci, word ) ;
    if ( word )
    {
        searchToken = tci->SearchToken ;
        Word * tw = tci->TrialWord = word ;
        byte * twn = tw->Name, *fqn ;
        if ( twn )
        {
            slst = Strlen ( ( CString ) searchToken ), sltwn = Strlen ( twn ) ;
            if ( ! slst ) // we match anything when user ends with a dot ( '.' ) ...
            {
                // except .. We don't want to jump down into a lower namespace here.
                if ( ( tw->ContainingNamespace == tci->OriginalContainingNamespace ) ) // || ( tw->ContainingNamespace == _Q_->CfrTil->Namespaces ) )
                {
                    gotOne = 1 ;
                }
                else return false ;
            }
            else
            {
                switch ( tci->WordWrapCount )
                {
                        // this arrangement allows us to see some word matches before others
                    case 0: //case 1:
                    {
                        strOpRes = strcmp ( twn, searchToken ) ;
                        if ( ! strOpRes )
                        {
                            strOpRes = stricmp ( twn, searchToken ) ;
                        }
                        if ( ( ! strOpRes ) ) //|| ( ! strOpRes1 ) )
                        {
                            if ( slst == sltwn )
                            {
                                gotOne = 1 ;
                            }
                        }
                        break ;
                    }
                    case 2: case 1:
                    {
                        strOpRes = StrnCmp ( twn, searchToken, slst ) ;
                        if ( ! strOpRes )
                        {
                            gotOne = tci->WordWrapCount ;
                        }
                        break ;
                    }
                    case 3:
                    {
                        strOpRes = StrnICmp ( twn, searchToken, slst ) ;
                        if ( ! strOpRes )
                        {
                            gotOne = tci->WordWrapCount ;
                        }
                        strOpRes = ( int64 ) strstr ( ( CString ) twn, ( CString ) searchToken ) ;
                        if ( strOpRes )
                        {
                            gotOne = 31 ;
                        }
                        break ;
                    }
                    case 4:
                    {
                        strOpRes = ( int64 ) strstr ( ( CString ) twn, ( CString ) searchToken ) ;
                        if ( strOpRes )
                        {
                            gotOne = tci->WordWrapCount ;
                        }
                        break ;
                    }
                    case 5: default:
                    {
                        byte bufw [128], bufo[128] ;
                        strToLower ( bufw, twn ) ;
                        strToLower ( bufo, searchToken ) ;
                        strOpRes = ( int64 ) strstr ( ( CString ) bufw, ( CString ) bufo ) ;
                        if ( strOpRes )
                        {
                            gotOne = tci->WordWrapCount ;
                        }
                        break ;
                    }
                }
            }
            if ( gotOne )
            {
                if ( word->W_FoundMarker == tci->FoundMarker )
                {
                    tci->FoundMarker = rand ( ) ;
                    tci->FoundWrapCount ++ ;
                }
                word->W_FoundMarker = tci->FoundMarker ;
                fqn = ReadLiner_GenerateFullNamespaceQualifiedName ( rl, tw ) ;
                RL_TC_StringInsert_AtCursor ( rl, fqn ) ;
                tci->FoundCount ++ ;
                if ( _Q_->Verbosity > 3 )
                {
                    if ( tci->FoundWrapCount )
                    {
                        _Printf ( ( byte* ) " [ FoundWrapCount = %d : WordWrapCount = %d : WordCount = %d ]", tci->FoundWrapCount, tci->WordWrapCount, tci->WordCount ) ;
                    }
                }
                return true ;
            }
        }
    }
    return false ;
}

// added 0.756.541
// problem here is that with the Class word '.' (dot) it loops and doesn't return

int64
_TC_FindPrevious_NamespaceQualifiedIdentifierStart ( TabCompletionInfo * tci, byte * s, int64 pos )
{
    int64 f, l, last = 0, dot ; // these refer to positions in the string s
    do
    {
        l = String_LastCharOfLastToken_FromPos ( s, pos ) ;
        if ( ! last ) tci->TokenLastChar = last = l ;
        if ( ( last == pos ) && ( s [last] <= ' ' ) && ( last != _ReadLine_CursorPosition ( _Context_->ReadLiner0 ) ) ) return last ;
        f = String_FirstCharOfToken_FromPosOfLastChar ( s, l ) ;
        if ( f > 0 )
        {
            dot = String_IsThereADotSeparatorBackFromPosToLastNonDelmiter ( s, f - 1 ) ;
        }
        else break ;
    }
    while ( pos = dot ) ;
    return f ;
}

void
RL_TC_StringInsert_AtCursor ( ReadLiner * rl, byte * strToInsert )
{
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    int64 stiLen, newCursorPos, startCursorPos = _ReadLine_CursorPosition ( rl ) ;
    int64 slotStart = _TC_FindPrevious_NamespaceQualifiedIdentifierStart ( tci, rl->InputLine, startCursorPos ) ;
    stiLen = Strlen ( ( CString ) strToInsert ) ;
    newCursorPos = slotStart + stiLen ;
    if ( newCursorPos < stiLen )
    {
        ReadLine_InputLine_Clear ( rl ) ;
        strcpy ( ( CString ) rl->InputLine, ( CString ) _CfrTil_->OriginalInputLine ) ;
    }
    ReadLine_SetCursorPosition ( rl, newCursorPos ) ;
    _ReadLine_InsertStringIntoInputLineSlotAndShow ( rl, slotStart, startCursorPos, ( byte* ) strToInsert ) ; // 1 : TokenLastChar is the last char of the identifier
}

byte *
_TabCompletionInfo_GetAPreviousIdentifier ( ReadLiner *rl, int64 start )
{
    byte b [128] ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    tci->TokenLastChar = ReadLine_LastCharOfLastToken_FromPos ( rl, start ) ;
    tci->TokenFirstChar = ReadLine_FirstCharOfToken_FromLastChar ( rl, tci->TokenLastChar ) ;
    tci->TokenLength = tci->TokenLastChar - tci->TokenFirstChar + 1 ; // zero array start
    Strncpy ( ( CString ) b, ( CString ) & rl->InputLine [ tci->TokenFirstChar ], tci->TokenLength ) ;
    b [ tci->TokenLength ] = 0 ;
    return TemporaryString_New ( b ) ;
}

// nb. the notation (function names) around parsing in tab completion is based in 'reverse parsing' - going back in the input line from the cursor position

void
RL_TabCompletionInfo_Init ( ReadLiner * rl )
{
    Namespace * piw ;
    Word * wf ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    memset ( tci, 0, sizeof ( TabCompletionInfo ) ) ;
    SetState ( rl, TAB_WORD_COMPLETION, true ) ;
    strcpy ( ( CString ) _CfrTil_->OriginalInputLine, ( CString ) rl->InputLineString ) ; // we use this extra buffer at ReadLine_TC_StringInsert_AtCursor
    tci->Identifier = _TabCompletionInfo_GetAPreviousIdentifier ( rl, _ReadLine_CursorPosition ( rl ) ) ;
    tci->DotSeparator = ReadLine_IsThereADotSeparator ( rl, tci->TokenFirstChar - 1 ) ;
    if ( tci->TokenFirstChar ) tci->PreviousIdentifier = _TabCompletionInfo_GetAPreviousIdentifier ( rl, tci->TokenFirstChar - 1 ) ; // TokenStart refers to start of 'Identifier'
    if ( ( tci->EndDottedPos = ReadLine_IsLastCharADot ( rl, _ReadLine_CursorPosition ( rl ) ) ) ) //ReadLine_IsDottedToken ( rl ) )
    {
        tci->SearchToken = ( byte * ) "" ; // everything matches
        rl->CursorPosition = tci->EndDottedPos ;
        rl->InputLine [ tci->EndDottedPos ] = ' ' ; // overwrite the final '.' with ' ' and move cursor pos back to that space 
    }
    else tci->SearchToken = tci->Identifier ? tci->Identifier : ( byte* ) "" ;
    if ( tci->DotSeparator )
    {
        tci->PreviousIdentifier = _TabCompletionInfo_GetAPreviousIdentifier ( rl, tci->DotSeparator - 1 ) ; // TokenStart refers to start of 'Identifier'
        if ( tci->PreviousIdentifier && ( piw = _CfrTil_FindInAnyNamespace ( tci->PreviousIdentifier ) ) )
        {
            if ( Is_NamespaceType ( piw ) )
            {
                if ( ( tci->OriginalWord = Finder_FindWord_InOneNamespace ( _Finder_, piw, tci->Identifier ) ) ) tci->RunWord = tci->OriginalWord ;
                else if ( wf = Finder_FindWord_AnyNamespace ( _Finder_, tci->Identifier ), ( wf && ( wf->ContainingNamespace == piw ) ) ) tci->RunWord = tci->OriginalWord = wf ;
                else tci->RunWord = ( Word* ) dllist_First ( ( dllist* ) piw->Lo_List ) ;
                tci->OriginalContainingNamespace = piw ;
            }
            if ( tci->OriginalWord && Is_NamespaceType ( tci->OriginalWord ) && ( tci->EndDottedPos ) )
            {
                tci->RunWord = ( Word* ) dllist_First ( ( dllist* ) tci->OriginalWord->Lo_List ) ;
                tci->OriginalContainingNamespace = tci->OriginalWord ;
            }
        }
    }
    else
    {
        if ( ( tci->OriginalWord = Finder_FindWord_InOneNamespace ( _Finder_, _CfrTil_Namespace_InNamespaceGet ( ), tci->Identifier ) ) ||
            ( tci->OriginalWord = _CfrTil_FindInAnyNamespace ( tci->Identifier ) ) )
        {
            if ( Is_NamespaceType ( tci->OriginalWord ) && ( tci->EndDottedPos ) )
            {
                tci->RunWord = ( Word* ) dllist_First ( ( dllist* ) tci->OriginalWord->Lo_List ) ;
                tci->OriginalContainingNamespace = tci->OriginalWord ;
            }
            else
            {
                tci->EndDottedPos = 0 ;
                tci->OriginalContainingNamespace = tci->OriginalWord->ContainingNamespace ? tci->OriginalWord->ContainingNamespace : _CfrTil_->Namespaces ;
                tci->RunWord = tci->OriginalWord ;
            }
        }
    }
    if ( ! tci->OriginalContainingNamespace ) tci->OriginalContainingNamespace = _CfrTil_->Namespaces ;
    tci->OriginalRunWord = tci->RunWord ;
    tci->SearchNumber = rand ( ) ;
    tci->WordWrapCount = 0 ;
    tci->WordCount = 0 ;
    tci->StartFlag = 0 ;
    tci->FoundMarker = rand ( ) ;
    tci->FoundWrapCount = 0 ;
    _Context_->NlsWord = 0 ;
}


