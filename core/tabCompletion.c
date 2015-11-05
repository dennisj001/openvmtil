
#include "../includes/cfrtil.h"

// we have to remember that namespaces are being moved to the beginning of the Namespaces list by _Namespace_AddToUsingList

Word *
_TC_NextWord ( TabCompletionInfo * tci, Word * runWord )
{
    Word * nextrw ;
    if ( ! runWord ) return _Q_->OVT_CfrTil->Namespaces ;
    //if ( runWord == _Q_->OVT_CfrTil->Namespaces ) return ( Word* ) DLList_First ( runWord->Lo_List ) ;
    if ( Is_ValueType ( tci->OriginalWord ) && tci->ObjectExtWord )
    {
        nextrw = ( Word* ) DLNode_Next ( ( Node* ) tci->ObjectExtWord ) ;
        tci->ObjectExtWord = nextrw ;
        return tci->OriginalWord ;
    }
    if ( Is_NamespaceType ( runWord ) && ! ( runWord->CType & ALIAS ) ) if ( nextrw = ( Word* ) DLList_First ( runWord->Lo_List ) ) return nextrw ;
    if ( nextrw = ( Word* ) DLNode_Next ( ( Node* ) runWord ) ) return nextrw ;
    return ( Word* ) DLNode_Next ( ( Node* ) runWord->ContainingNamespace ) ; // this doesn't finish a namespace started in the middle
}
// map starting from any word
// used now only with tab completion

Word *
_TC_Map ( TabCompletionInfo * tci, Word * first, MapFunction mf )
{
    Word * word = first ;
    do
    {
        tci->NextWord = _TC_NextWord ( tci, word ) ;
        if ( mf ( ( Symbol* ) word ) ) return tci->NextWord ;
        word = tci->NextWord ;
        if ( kbhit ( ) ) return tci->NextWord ; // allow to break search 
    }
    while ( word != first ) ;
    return 0 ;
}

void
RL_TabCompletion_Run ( ReadLiner * rl, Word * rword )
{
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    tci->NextWord = _TC_Map ( tci, rword, ( MapFunction ) _TabCompletion_Compare ) ;
}

TabCompletionInfo *
TabCompletionInfo_New ( int32 type )
{
    TabCompletionInfo *tci = ( TabCompletionInfo * ) Mem_Allocate ( sizeof (TabCompletionInfo ), type ) ;
    return tci ;
}

byte *
ReadLiner_GenerateFullNamespaceQualifiedName ( ReadLiner * rl, Word * w )
{
    //ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    Buffer_Clear ( _Q_->OVT_CfrTil->TabCompletionBuf ) ;
    byte * b0 = ( char* ) Buffer_Data ( _Q_->OVT_CfrTil->TabCompletionBuf ) ;
    Stack_Init ( rl->TciNamespaceStack ) ;
    Stack * nsStk = rl->TciNamespaceStack ;
    Namespace *ns ; Word * w1 ;
    int32 i, dot = 0 ; //, ow = 0 ;

    String_Init ( b0 ) ;
    for ( ns = Is_NamespaceType ( w ) ? w : w->ContainingNamespace ; ns ; ns = ns->ContainingNamespace ) // && ( tw->ContainingNamespace != _Q_->CfrTil->Namespaces ) )
    {
        _Stack_Push ( nsStk, ( int32 ) ns ) ;
    }
    for ( i = Stack_Depth ( nsStk ) ; i ; i -- )
    {
        w1 = ( Word* ) _Stack_Pop ( nsStk ) ;
        if ( w1 )
        {
            strcat ( ( CString ) b0, ( CString ) w1->Name ) ;
            if ( i > 1 )
            {
                strcat ( ( CString ) b0, "." ) ;
            }
        }
    }
    if ( w1 != w )
    {
        if ( ! dot ) strcat ( ( CString ) b0, "." ) ;
        strcat ( ( CString ) b0, ( CString ) w->Name ) ; // namespaces are all added above
    }
    if ( Is_ValueType ( w ) )
    {
        if ( Is_ValueType ( tci->OriginalWord ) && ( tci->EndDottedPos || tci->ObjectExtWord ) )
        {
            if ( ! tci->ObjectExtWord )
            {
                if ( ! Is_ValueType ( tci->NextWord ) || ( w != tci->NextWord ) )
                {
                    strcat ( ( CString ) b0, "." ) ;
                    strcat ( ( CString ) b0, ( CString ) tci->NextWord->Name ) ;
                    tci->ObjectExtWord = tci->NextWord ;
                    tci->NextWord = tci->OriginalWord ;
                }
            }
            else //if ( ! IS_ValueType ( tci->ObjectExtWord ) )
            {
                strcat ( ( CString ) b0, "." ) ;
                strcat ( ( CString ) b0, ( CString ) tci->ObjectExtWord->Name ) ;
                tci->NextWord = tci->OriginalWord ;
            }
        }
    }
    return b0 ;
}

Boolean
_TabCompletion_Compare ( Word * word )
{
    ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    byte * searchToken ;
    if ( word )
    {
        searchToken = tci->SearchToken ;
        Word * tw = tci->TrialWord = word ;
        byte * twn = tw->Name, *fqn ;
        if ( twn ) //&& tw->ContainingNamespace )
        {
            int32 strOpRes1, strOpRes2, strOpRes3 ;
            if ( ! strlen ( searchToken ) ) // we match anything when user ends with a dot ( '.' ) ...
            {
                // except .. We don't want to jump down into a lower namespace here.
                if ( ( tw->ContainingNamespace == tci->OriginalContainingNamespace ) ) // || ( tw->ContainingNamespace == _Q_->CfrTil->Namespaces ) )
                {
                    strOpRes1 = 1 ;
                }
                else return false ;
            }
            else
            {
                byte bufw [128], bufo[128] ;
                strOpRes1 = ! strnicmp ( ( CString ) twn, ( CString ) searchToken, strlen ( ( CString ) searchToken ) ) ; // strstr == token : the start of the dictionary entry
                if ( ! strOpRes1 ) strOpRes2 = ( int32 ) strstr ( ( CString ) twn, ( CString ) searchToken ) ; // == ( String ) twn ) ;// strstr == token : the start of the dictionary entry
                if ( ! ( strOpRes1 | strOpRes2 ) )
                {
                    strToLower ( bufw, twn ) ;
                    strToLower ( bufo, searchToken ) ;
                    strOpRes3 = ( int32 ) strstr ( ( CString ) bufw, ( CString ) bufo ) ; // == ( String ) twn ) ;// strstr == token : the start of the dictionary entry
                }
            }
            if ( strOpRes1 | strOpRes2 | strOpRes3 ) //|| ( word == tci->OriginalWord ? tci->OriginalWord->CType &  NAMESPACE_TYPES : 0 ))
            {
                if ( ! tci->MarkWord ) tci->MarkWord = word ;
                fqn = ReadLiner_GenerateFullNamespaceQualifiedName ( rl, tw ) ;
                RL_TC_StringInsert_AtCursor ( rl, ( CString ) fqn ) ;
                return true ;
            }
        }
    }
    tci->TrialWord = 0 ;
    return false ;
}

// added 0.756.541
// problem here is that with the Class word '.' (dot) it loops and doesn't return

int32
_TC_FindPrevious_NamespaceQualifiedIdentifierStart ( TabCompletionInfo * tci, CString s, int32 pos )
{
    int f, l, last = 0, dot ; // these refer to positions in the string s
    do
    {
        l = String_LastCharOfLastToken_FromPos ( s, pos ) ;
        if ( ! last ) tci->TokenLastChar = last = l ;
        if ( ( last == pos ) && ( s [last] <= ' ' ) && ( last != _ReadLine_CursorPosition ( _Q_->OVT_Context->ReadLiner0 ) ) ) return last ;
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
RL_TC_StringInsert_AtCursor ( ReadLiner * rl, CString strToInsert )
{
    //ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    int32 stiLen, newCursorPos, startCursorPos = _ReadLine_CursorPosition ( rl ) ;
    int32 slotStart = _TC_FindPrevious_NamespaceQualifiedIdentifierStart ( tci, ( CString ) rl->InputLine, startCursorPos ) ; //cursorPos0 ) ; //rl->i32_CursorPosition ) ;
    stiLen = strlen ( ( CString ) strToInsert ) ;
    newCursorPos = slotStart + stiLen ; //+ ( Is_NamespaceType (tci->TrialWord) && tci->EndDottedFlag ? 1 : 0 ) ;
    if ( newCursorPos < stiLen )
    {
        ReadLine_InputLine_Clear ( rl ) ;
        strcpy ( ( CString ) rl->InputLine, ( CString ) _Q_->OVT_CfrTil->OriginalInputLine ) ;
    }
    ReadLine_SetCursorPosition ( rl, newCursorPos ) ;
    _ReadLine_InsertStringIntoInputLineSlotAndShow ( rl, slotStart, startCursorPos, ( byte* ) strToInsert ) ; // 1 : TokenLastChar is the last char of the identifier
}

byte *
_TabCompletionInfo_GetAPreviousIdentifier ( ReadLiner *rl, int32 start )
{
    byte b [128] ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    tci->TokenLastChar = ReadLine_LastCharOfLastToken_FromPos ( rl, start ) ;
    tci->TokenFirstChar = ReadLine_FirstCharOfToken_FromLastChar ( rl, tci->TokenLastChar ) ;
    tci->TokenLength = tci->TokenLastChar - tci->TokenFirstChar + 1 ; // zero array start
    strncpy ( ( CString ) b, ( CString ) & rl->InputLine [ tci->TokenFirstChar ], tci->TokenLength ) ;
    b [ tci->TokenLength ] = 0 ;
    return TemporaryString_New ( b ) ;
}

// nb. the notation (function names) around parsing in tab completion is based in 'reverse parsing' - going back in the input line from the cursor position
#if 0

void
RL_TabCompletionInfo_Init ( )
{
    Namespace * piw ;
    ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    memset ( tci, 0, sizeof ( TabCompletionInfo ) ) ;
    ReadLiner_SetState ( rl, TAB_WORD_COMPLETION, true ) ;
    strcpy ( ( CString ) _Q_->OVT_CfrTil->OriginalInputLine, ( CString ) rl->InputLine ) ; // we use this extra buffer at ReadLine_TC_StringInsert_AtCursor
    tci->Identifier = _TabCompletionInfo_GetAPreviousIdentifier ( rl, _ReadLine_CursorPosition ( rl ) ) ;
    tci->DotSeparator = ReadLine_IsThereADotSeparator ( rl, tci->TokenFirstChar - 1 ) ;
    if ( tci->TokenFirstChar ) tci->PreviousIdentifier = _TabCompletionInfo_GetAPreviousIdentifier ( rl, tci->TokenFirstChar - 1 ) ; // TokenStart refers to start of 'Identifier'
    if ( ReadLine_IsLastCharADot ( rl, rl->i32_CursorPosition ) ) //ReadLine_IsDottedToken ( rl ) )
    {
        tci->EndDottedFlag = 1 ; // using a dot in a tabCompletion causes the search to remain in the same namespace and down tree otherwise the search is of the whole word tree
        tci->SearchToken = ( byte * ) "" ; // everything matches
        rl->InputLine [-- rl->i32_CursorPosition] = ' ' ; // discard the final '.' 
    }
    else tci->SearchToken = tci->Identifier ? tci->Identifier : ( byte* ) "" ;
    if ( ( tci->OriginalWord = _CfrTil_FindInAnyNamespace ( tci->Identifier ) ) )
    {
        if ( tci->EndDottedFlag && Is_NamespaceType ( tci->OriginalWord ) )
        {
            tci->RunWord = ( Word* ) DLList_First ( tci->OriginalWord->Lo_List ) ;
            tci->OriginalContainingNamespace = tci->OriginalWord ;
        }
        else
        {
            tci->EndDottedFlag = 0 ;
            tci->OriginalContainingNamespace = tci->OriginalWord->ContainingNamespace ? tci->OriginalWord->ContainingNamespace : _Q_->OVT_CfrTil->Namespaces ;
            tci->RunWord = tci->OriginalWord ;
        }
    }
    else
    {
        if ( ( tci->DotSeparator ) && ( piw = _CfrTil_FindInAnyNamespace ( tci->PreviousIdentifier ) ) )
        {
            if ( Is_NamespaceType ( piw ) )
            {
                tci->RunWord = ( Word* ) DLList_First ( piw->Lo_List ) ;
                tci->OriginalContainingNamespace = piw ;
            }
        }
    }
    if ( ! tci->RunWord ) tci->RunWord = _Q_->OVT_CfrTil->Namespaces ;
    if ( ! tci->OriginalContainingNamespace ) tci->OriginalContainingNamespace = _Q_->OVT_CfrTil->Namespaces ;
    tci->OriginalRunWord = tci->RunWord ;
    tci->MarkWord = 0 ;
}

#else

void
RL_TabCompletionInfo_Init ( ReadLiner * rl )
{
    Namespace * piw ;
    //ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    memset ( tci, 0, sizeof ( TabCompletionInfo ) ) ;
    ReadLiner_SetState ( rl, TAB_WORD_COMPLETION, true ) ;
    strcpy ( ( CString ) _Q_->OVT_CfrTil->OriginalInputLine, ( CString ) rl->InputLine ) ; // we use this extra buffer at ReadLine_TC_StringInsert_AtCursor
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
                if ( ( tci->OriginalWord = Word_FindInOneNamespace ( piw, tci->Identifier ) ) ) tci->RunWord = tci->OriginalWord ;
                else tci->RunWord = ( Word* ) DLList_First ( piw->Lo_List ) ;
                tci->OriginalContainingNamespace = piw ;
            }
        }
    }
    else if ( ( tci->OriginalWord = _CfrTil_FindInAnyNamespace ( tci->Identifier ) ) )
    {
        if ( Is_NamespaceType ( tci->OriginalWord ) && ( tci->EndDottedPos ) )
        {
            tci->RunWord = ( Word* ) DLList_First ( tci->OriginalWord->Lo_List ) ;
            tci->OriginalContainingNamespace = tci->OriginalWord ;
        }
        else
        {
            tci->EndDottedPos = 0 ;
            tci->OriginalContainingNamespace = tci->OriginalWord->ContainingNamespace ? tci->OriginalWord->ContainingNamespace : _Q_->OVT_CfrTil->Namespaces ;
            tci->RunWord = tci->OriginalWord ;
        }
    }
    if ( ! tci->RunWord ) tci->RunWord = _Q_->OVT_CfrTil->Namespaces ;
    if ( ! tci->OriginalContainingNamespace ) tci->OriginalContainingNamespace = _Q_->OVT_CfrTil->Namespaces ;
    tci->OriginalRunWord = tci->RunWord ;
    tci->MarkWord = 0 ;
}
#endif
