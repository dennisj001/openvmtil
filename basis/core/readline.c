
#include "../../includes/cfrtil.h"

#define ReadLine_AppendPoint( rl ) &rl->InputBuffer [ rl->EndPosition ]

void
_ReadLine_NullDelimitInputBuffer ( ReadLiner * rl )
{
    rl->InputLine [ rl->EndPosition ] = 0 ;
}

void
_ReadLine_QuickAppendCharacter ( ReadLiner * rl, byte chr )
{
    rl->InputLine [ rl->EndPosition ++ ] = chr ;
    rl->InputLine [ rl->EndPosition ] = 0 ;
}

void
_ReadLine_SetOutputLineCharacterNumber ( ReadLiner * rl )
{
    rl->OutputLineCharacterNumber = strlen ( ( char* ) rl->Prompt ) + rl->EndPosition ;
}

void
__ReadLine_AppendCharacter ( ReadLiner * rl, byte chr )
{
    _ReadLine_QuickAppendCharacter ( rl, chr ) ;
    rl->EndPosition ++ ;
    _ReadLine_SetOutputLineCharacterNumber ( rl ) ;
    _ReadLine_SetMaxEndPosition ( rl ) ;
}

void
__ReadLine_AppendCharacter_Actual ( ReadLiner * rl, byte chr )
{
    _ReadLine_QuickAppendCharacter ( rl, chr ) ;
    _ReadLine_SetOutputLineCharacterNumber ( rl ) ;
    _ReadLine_SetMaxEndPosition ( rl ) ;
}

void
_ReadLine_AppendCharacter ( ReadLiner * rl )
{
    __ReadLine_AppendCharacter ( rl, rl->InputKeyedCharacter ) ;
}

void
_ReadLine_AppendCharacter_Actual ( ReadLiner * rl )
{
    __ReadLine_AppendCharacter_Actual ( rl, rl->InputKeyedCharacter ) ;
}

void
ReadLine_DoCursorMoveInput ( ReadLiner * rl, int32 newCursorPosition )
{
    ReadLine_ClearCurrentTerminalLine ( rl, rl->CursorPosition ) ;
    ReadLine_SetCursorPosition ( rl, newCursorPosition ) ;
    ReadLine_ShowPad ( rl ) ;
    ReadLine_ShowCursor ( rl ) ;
}

#if 0

int32
ReadLine_PositionCursor ( ReadLiner * rl )
{
    int32 pos = rl->i32_CursorPosition ;
    while ( ( pos >= 0 ) && ( rl->InputLine [ pos ] == 0 ) )
        rl->InputLine [ -- pos ] = ' ' ;
    return rl->i32_CursorPosition ; //= pos >= 0 ? pos : 0 ;
}
#endif

void
ReadLine_SetCursorPosition ( ReadLiner * rl, int32 pos )
{
    if ( pos < 0 ) pos = 0 ; // prompt width 
    rl->CursorPosition = pos ;
}

void
ReadLiner_CommentToEndOfLine ( ReadLiner * rl )
{
    rl->ReadIndex = BUFFER_SIZE ; // cf. _ReadLine_GetNextChar
    ReadLiner_Done ( rl ) ;
}

void
ReadLiner_Done ( ReadLiner * rl )
{
    SetState ( rl, READLINER_DONE, true ) ;
}

Boolean
ReadLiner_IsDone ( ReadLiner * rl )
{
    return ( ( GetState ( rl, READLINER_DONE ) ) || ( rl->EndPosition >= BUFFER_SIZE ) || ( rl->ReadIndex >= BUFFER_SIZE ) ) ;
}

#define Cursor_Up( n ) _Printf ( (byte*) "\r%c[%dA", ESC, n )

void
_ReadLine_MoveInputStartToLineStart ( int32 fromPosition )
{
    // nb. this is *necessary* when user scrolls up with scrollbar in eg. konsole and then hits up/down arrow
    int32 n, columns = GetTerminalWidth ( ) ;
    if ( fromPosition && columns )
    {
        n = ( fromPosition ) / ( columns ) ;
        if ( ( fromPosition % columns ) < 2 ) n -- ; // nb : ?? -- i don't understand this but it works
        if ( n ) Cursor_Up ( n ) ; //_Printf ( "\r%c[%dA", ESC, n ) ; // move n lines up 
    }
    else _Printf ( ( byte* ) "\r" ) ; // nb -- a workaround : ?? second sequence ( clear 2 eol ) not necessary but seems to reset things to work -- ??
    //_Printf ( "\r%c[2K", ESC ) ; // nb -- a workaround : ?? second sequence ( clear 2 eol ) not necessary but seems to reset things to work -- ??
}

void
_ReadLine_PrintfClearTerminalLine ( )
{
    _Printf ( ( byte* ) "\r%c[J", ESC ) ; // clear from cursor to end of screen -- important if we have (mistakenly) gone up an extra line
}

void
ReadLine_ClearCurrentTerminalLine ( ReadLiner * rl, int32 fromPosition )
{
    _ReadLine_MoveInputStartToLineStart ( fromPosition + PROMPT_LENGTH + 1 ) ; // 1 : zero array indexing
    _ReadLine_PrintfClearTerminalLine ( ) ;
}

void
ReadLine_SetInputLine ( ReadLiner * rl, byte * buffer )
{
    strcpy ( ( char* ) rl->InputLine, ( char* ) buffer ) ;
}

void
ReadLine_InputLine_Clear ( ReadLiner * rl )
{
    Mem_Clear ( rl->InputLine, BUFFER_SIZE ) ;
}

void
ReadLine_InputLine_Init ( ReadLiner * rl )
{
    ReadLine_InputLine_Clear ( rl ) ;
    rl->InputLineCharacterNumber = 0 ;
    rl->ReadIndex = 0 ;
    rl->InputLineString = rl->InputLine ;
    SetState ( rl, READLINER_DONE, false ) ;
}

void
ReadLine_RunInit ( ReadLiner * rl )
{
    rl->HistoryNode = 0 ;
    rl->EscapeModeFlag = 0 ;
    _ReadLine_CursorToStart ( rl ) ;
    rl->EndPosition = 0 ;
    rl->MaxEndPosition = 0 ;
    SetState ( rl, END_OF_INPUT | END_OF_LINE | TAB_WORD_COMPLETION, false ) ; // ?? here ??
    ReadLine_InputLine_Init ( rl ) ;
}

void
ReadLine_Init ( ReadLiner * rl, ReadLiner_KeyFunction ipf, uint32 type )
{
    ReadLine_RunInit ( rl ) ;
    SetState ( rl, CHAR_ECHO, true ) ; // this is how we see our input at the command line!
    rl->LineNumber = 0 ;
    rl->InputFile = stdin ;
    rl->OutputFile = stdout ;
    rl->Filename = 0 ;
    rl->FileCharacterNumber = 0 ;
    rl->NormalPrompt = ( byte* ) "<: " ;
    rl->AltPrompt = ( byte* ) ":> " ;
    rl->DebugPrompt = ( byte* ) "d:==> " ;
    rl->DebugAltPrompt = ( byte* ) "<== " ;
    rl->Prompt = rl->NormalPrompt ;
    rl->InputStringOriginal = 0 ;
    if ( ipf ) ReadLine_SetRawInputFunction ( rl, ipf ) ;
}

ReadLiner *
ReadLine_New ( uint32 type )
{
    ReadLiner * rl = ( ReadLiner * ) Mem_Allocate ( sizeof (ReadLiner ), type ) ;
    rl->TabCompletionInfo0 = TabCompletionInfo_New ( type ) ;
    rl->TciNamespaceStack = Stack_New ( 64, type ) ;
    ReadLine_Init ( rl, _CfrTil_GetC, type ) ;
    return rl ;
}

void
_ReadLine_Copy ( ReadLiner * rl, ReadLiner * rl0, uint32 type )
{
    memcpy ( rl, rl0, sizeof (ReadLiner ) ) ;
    rl->TabCompletionInfo0 = TabCompletionInfo_New ( type ) ;
    rl->TciNamespaceStack = Stack_New ( 64, SESSION ) ;
    //rl->TciDownStack = Stack_New ( 32, SESSION ) ;
    //ReadLine_Init ( rl, rl0->Key, type ) ; //_CfrTil_GetC ) ;
    //strcpy ( ( char* ) rl->InputLine, ( char* ) rl0->InputLine ) ;
    //rl->InputStringOriginal = rl0->InputStringOriginal ;
    //rl->State = rl0->State ;
}

ReadLiner *
ReadLine_Copy ( ReadLiner * rl0, uint32 type )
{
    ReadLiner * rl = ( ReadLiner * ) Mem_Allocate ( sizeof (ReadLiner ), type ) ;
    _ReadLine_Copy ( rl, rl0, type ) ;
    return rl ;
}

void
ReadLine_TabWordCompletion ( ReadLiner * rl )
{
    if ( ! GetState ( rl, TAB_WORD_COMPLETION ) )
    {
        RL_TabCompletionInfo_Init ( rl ) ;
    }
    RL_TabCompletion_Run ( rl, rl->TabCompletionInfo0->NextWord ? rl->TabCompletionInfo0->NextWord : rl->TabCompletionInfo0->RunWord ) ; // the main workhorse here
}

void
__ReadLine_AppendCharacterAndCursoRight ( ReadLiner * rl, byte c )
{
    __ReadLine_AppendCharacter ( rl, c ) ;
    _ReadLine_CursorRight ( rl ) ;
    _ReadLine_SetEndPosition ( rl ) ;
}

void
_ReadLine_AppendCharacterAndCursoRight ( ReadLiner * rl )
{
    __ReadLine_AppendCharacterAndCursoRight ( rl, rl->InputKeyedCharacter ) ;
}

void
_ReadLine_AppendAndShowCharacter ( ReadLiner * rl, byte c )
{
    __ReadLine_AppendCharacterAndCursoRight ( rl, c ) ;
    _ReadLine_ShowCharacter ( rl, c ) ;
}

void
ReadLine_AppendAndShowCharacter ( ReadLiner * rl )
{
    _ReadLine_AppendAndShowCharacter ( rl, rl->InputKeyedCharacter ) ;
}

byte *
ReadLine_GetPrompt ( ReadLiner * rl )
{
    if ( rl->CursorPosition < rl->EndPosition ) rl->Prompt = ReadLine_GetAltPrompt ( rl ) ;
    else rl->Prompt = ReadLine_GetNormalPrompt ( rl ) ;
    return rl->Prompt ;
}

void
ReadLine_SetPrompt ( ReadLiner * rl, byte * newPrompt )
{
    rl->NormalPrompt = newPrompt ;
}

byte *
ReadLine_GetAltPrompt ( ReadLiner * rl )
{
    return (GetState ( _Debugger_, DBG_ACTIVE ) ? rl->DebugPrompt : rl->AltPrompt ) ;
}

byte *
ReadLine_GetNormalPrompt ( ReadLiner * rl )
{
    return (GetState ( _Debugger_, DBG_ACTIVE ) ? rl->DebugPrompt : rl->NormalPrompt ) ;
}

void
_ReadLine_Show ( ReadLiner * rl, byte * prompt )
{
    _Printf ( ( byte* ) "\r%s%s", prompt, rl->InputLine ) ;
}

void
_ReadLine_ShowPad ( ReadLiner * rl, byte * prompt )
{
    _ReadLine_Show ( rl, prompt ) ;
    _ReadLine_SetEndPosition ( rl ) ;
}

void
ReadLine_ShowPad ( ReadLiner * rl )
{
    _ReadLine_ShowPad ( rl, ReadLine_GetPrompt ( rl ) ) ;
}

void
_ReadLine_ClearAndShowPad ( ReadLiner * rl, byte * prompt )
{
    ReadLine_ClearCurrentTerminalLine ( rl, rl->CursorPosition ) ;
    _ReadLine_ShowPad ( rl, prompt ) ;
}

void
__ReadLine_DoStringInput ( ReadLiner * rl, byte * string, byte * prompt )
{
    ReadLine_ClearCurrentTerminalLine ( rl, rl->CursorPosition ) ;
    ReadLine_InputLine_Clear ( rl ) ;
    strcpy ( ( char* ) rl->InputLine, ( char* ) string ) ;
    _ReadLine_ShowPad ( rl, prompt ) ;
}

void
ReadLine_ClearAndShowPad ( ReadLiner * rl )
{
    ReadLine_ClearCurrentTerminalLine ( rl, rl->CursorPosition ) ;
    ReadLine_ShowPad ( rl ) ;
}

void
_ReadLine_ShowCursor ( ReadLiner * rl, byte * prompt )
{
    _ReadLine_MoveInputStartToLineStart ( rl->EndPosition + PROMPT_LENGTH + 1 ) ;
    byte saveChar = rl->InputLine [ rl->CursorPosition ] ; // set up to show cursor at end of new word
    rl->InputLine [ rl->CursorPosition ] = 0 ; // set up to show cursor at end of new word
    _ReadLine_Show ( rl, prompt ) ;
    rl->InputLine [ rl->CursorPosition ] = saveChar ; // set up to show cursor at end of new word
}

void
ReadLine_ShowCursor ( ReadLiner * rl )
{
    _ReadLine_ShowCursor ( rl, ReadLine_GetPrompt ( rl ) ) ;
}

void
_ReadLine_DoStringInput ( ReadLiner * rl, byte * string, byte * prompt )
{
    __ReadLine_DoStringInput ( rl, string, prompt ) ;
    _ReadLine_ShowCursor ( rl, prompt ) ;
}

void
ReadLine_ShowStringWithCursor ( ReadLiner * rl, byte * string )
{
    _ReadLine_DoStringInput ( rl, string, ReadLine_GetPrompt ( rl ) ) ;
}

void
ReadLine_ClearAndShowPadWithCursor ( ReadLiner * rl )
{
    ReadLine_ClearAndShowPad ( rl ) ;
    ReadLine_ShowCursor ( rl ) ;
}

void
ReadLine_ShowNormalPrompt ( ReadLiner * rl )
{
    //_ReadLine_ShowStringWithCursor ( rl, ( byte* ) "", rl->NormalPrompt ) ;
    _ReadLine_PrintfClearTerminalLine ( ) ;
    _Printf ( ( byte* ) "\r%s", rl->NormalPrompt ) ;
    rl->EndPosition = 0 ;
    rl->InputLine [ 0 ] = 0 ;
}

void
ReadLine_InsertCharacter ( ReadLiner * rl )
// insert rl->InputCharacter at cursorPostion
{
    String_InsertCharacter ( ( CString ) rl->InputLine, rl->CursorPosition, rl->InputKeyedCharacter ) ;
    ReadLine_ClearAndShowPadWithCursor ( rl ) ;
}

void
ReadLine_SaveCharacter ( ReadLiner * rl )
{
    if ( rl->CursorPosition < rl->EndPosition )
    {
        ReadLine_InsertCharacter ( rl ) ;
        _ReadLine_CursorRight ( rl ) ;
        ReadLine_ClearAndShowPadWithCursor ( rl ) ;
        return ;
    }
    ReadLine_AppendAndShowCharacter ( rl ) ;
}

void
_ReadLine_InsertStringIntoInputLineSlotAndShow ( ReadLiner * rl, int32 startOfSlot, int32 endOfSlot, byte * data )
{
    String_InsertDataIntoStringSlot ( rl->InputLine, startOfSlot, endOfSlot, data ) ; // size in bytes
    ReadLine_ClearAndShowPadWithCursor ( rl ) ;
}

void
ReadLiner_InsertTextMacro ( ReadLiner * rl, Word * word )
{
    int nlen = ( strlen ( ( char* ) word->Name ) + 1 ) ;
    String_InsertDataIntoStringSlot ( rl->InputLine, rl->ReadIndex - nlen, rl->ReadIndex, ( byte* ) word->W_Value ) ; // size in bytes
    rl->ReadIndex -= nlen ;
    _CfrTil_UnAppendFromSourceCode ( nlen ) ;
}

void
ReadLine_DeleteChar ( ReadLiner * rl )
{
    Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
    byte * b = Buffer_Data ( buffer ) ;
    if ( -- rl->EndPosition < 0 ) rl->EndPosition = 0 ;
    if ( rl->CursorPosition > rl->EndPosition )// shouldn't ever be greater but this will be more robust
    {
        if ( -- rl->CursorPosition < 0 ) _ReadLine_CursorToStart ( rl ) ;
    }
    rl->InputLine [ rl->CursorPosition ] = 0 ;
    // prevent string overwriting itself while coping ...
    strcpy ( ( char* ) b, ( char* ) & rl->InputLine [ rl->CursorPosition + 1 ] ) ;
    if ( rl->CursorPosition < rl->EndPosition ) strcat ( ( char* ) rl->InputLine, ( char* ) b ) ;
    ReadLine_ClearAndShowPadWithCursor ( rl ) ;
    Buffer_SetAsUnused ( buffer ) ;
}

int32
ReadLine_IsLastCharADot ( ReadLiner * rl, int32 pos )
{
    return String_IsLastCharADot ( rl->InputLine, pos ) ;
}

int32
ReadLine_FirstCharOfToken_FromLastChar ( ReadLiner * rl, int32 pos )
{
    return String_FirstCharOfToken_FromPosOfLastChar ( rl->InputLine, pos ) ;
}

int32
ReadLine_IsThereADotSeparator ( ReadLiner * rl, int32 pos )
{
    String_IsThereADotSeparatorBackFromPosToLastNonDelmiter ( rl->InputLine, pos ) ;
}

int32
ReadLine_LastCharOfLastToken_FromPos ( ReadLiner * rl, int32 pos )
{
    return String_LastCharOfLastToken_FromPos ( rl->InputLine, pos ) ;
}

int32
ReadLine_EndOfLastToken ( ReadLiner * rl )
{
    return ReadLine_LastCharOfLastToken_FromPos ( rl, rl->CursorPosition ) ;
}

int32
ReadLine_BeginningOfLastToken ( ReadLiner * rl )
{
    return ReadLine_FirstCharOfToken_FromLastChar ( rl, ReadLine_EndOfLastToken ( rl ) ) ;
}

Boolean
ReadLine_IsReverseTokenQualifiedID ( ReadLiner * rl )
{
    String_IsReverseTokenQualifiedID ( rl->InputLine, rl->ReadIndex ) ; //int32 pos ) ;
}

byte
ReadLine_Key ( ReadLiner * rl )
{
    rl->InputKeyedCharacter = rl->Key ( rl ) ;
    rl->InputLineCharacterNumber ++ ;
    rl->FileCharacterNumber ++ ;
    return rl->InputKeyedCharacter ;
}

byte
ReadLine_GetNextCharFromString ( ReadLiner * rl )
{
    rl->InputStringIndex ++ ;
    if ( rl->InputStringIndex <= rl->InputStringLength ) return * rl->InputStringCurrent ++ ;
    else return 0 ;
}

void
ReadLine_SetRawInputFunction ( ReadLiner * rl, ReadLiner_KeyFunction ripf )
{
    rl->Key = ripf ;
}

void
ReadLine_ReadFileToString ( ReadLiner * rl, FILE * file )
{
    int32 size, result ;
    size = _File_Size ( file ) ;
    byte * fstr = Mem_Allocate ( size, SESSION ) ; // 2 : an extra so readline doesn't read into another area of allocated mem
    result = fread ( fstr, 1, size, file ) ;
    if ( result != size )
    {
        fstr = 0 ;
        size = 0 ;
    }
    rl->InputStringOriginal = fstr ;
    rl->InputStringCurrent = rl->InputStringOriginal ;
    rl->InputStringIndex = 0 ;
    rl->InputStringLength = size ;
}

void
ReadLine_SetInputString ( ReadLiner * rl, byte * string )
{
    rl->InputStringOriginal = string ;
    rl->InputStringCurrent = rl->InputStringOriginal ;
    rl->InputStringIndex = 0 ;
    rl->InputStringLength = strlen ( ( char* ) string ) ;
}

void
_ReadLine_TabCompletion_Check ( ReadLiner * rl )
{
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    if ( rl->InputKeyedCharacter != '\t' )
    {
        if ( GetState ( rl, TAB_WORD_COMPLETION ) )
        {
            SetState ( rl, TAB_WORD_COMPLETION, false ) ;
            if ( ( rl->InputKeyedCharacter == ' ' ) && ( tci->TrialWord ) )
            {
                TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
                RL_TC_StringInsert_AtCursor ( rl, tci->TrialWord->Name ) ;
            }
            else if ( rl->InputKeyedCharacter == '\r' ) rl->InputKeyedCharacter = ' ' ; // leave line as is and append a space instead of '\r'
        }
    }
}

void
ReadLine_GetLine ( ReadLiner * rl )
// we're here until we get a newline char ( '\n' or '\r' ), a eof or a buffer overflow
// note : ReadLinePad [ 0 ] starts after the prompt ( "-: " | "> " ) and doesn't include them
{
    ReadLine_RunInit ( rl ) ;
    rl->LineStartFileIndex = rl->InputStringIndex ;
    while ( ! ReadLiner_IsDone ( rl ) )
    {
        ReadLine_Key ( rl ) ;
        if ( AtCommandLine ( rl ) ) _ReadLine_TabCompletion_Check ( rl ) ;
        _Q_->OVT_CfrTil->ReadLine_FunctionTable [ _Q_->OVT_CfrTil->ReadLine_CharacterTable [ rl->InputKeyedCharacter ] ] ( rl ) ;
        SetState ( rl, ANSI_ESCAPE, false ) ;
    }
}

byte
ReadLine_NextChar ( ReadLiner * rl )
{
    byte nchar = _ReadLine_GetNextChar ( rl ) ;
    if ( nchar ) return nchar ;
    else
    {
        if ( GetState ( rl, STRING_MODE ) )
        {
            SetState ( rl, STRING_MODE, false ) ; // only good once
            return nchar ;
        }
        else ReadLine_GetLine ( rl ) ; // get a line of characters
    }
    nchar = _ReadLine_GetNextChar ( rl ) ;
    return nchar ;
}

void
Readline_Setup_OneStringInterpret ( ReadLiner * rl, byte * str )
{
    rl->ReadIndex = 0 ;
    SetState ( rl, STRING_MODE, true ) ;
    ReadLine_SetInputLine ( rl, str ) ;
}

void
Readline_SaveInputLine ( ReadLiner * rl )
{
    byte * svLine = Buffer_Data ( _Q_->OVT_CfrTil->InputLineB ) ;
    strcpy ( ( char* ) svLine, ( char* ) rl->InputLine ) ;
}

void
Readline_RestoreInputLine ( ReadLiner * rl )
{
    byte * svLine = Buffer_Data ( _Q_->OVT_CfrTil->InputLineB ) ;
    strcpy ( ( char* ) rl->InputLine, ( char* ) svLine ) ;
}

int32
_Readline_CheckArrayDimensionForVariables ( ReadLiner * rl )
{
    byte *p, * ilri = & rl->InputLine [ rl->ReadIndex ], * prb = ( byte* ) strchr ( ( char* ) &rl->InputLine [ rl->ReadIndex ], ']' ) ;
    if ( prb )
    {
        for ( p = ilri ; p != prb ; p ++ ) if ( isalpha ( * p ) ) return true ;
    }
    return false ;
}

int32
_Readline_Is_AtEndOfBlock ( ReadLiner * rl0 )
{
    ReadLiner * rl = ReadLine_Copy ( rl0, COMPILER_TEMP ) ;
    Word * word = Compiler_WordList ( 0 ) ;
    int32 iz, ib, index = word->W_StartCharRlIndex + strlen ( word->Name ), sd = _Stack_Depth ( _Context_->Compiler0->BlockStack ) ;
    byte c ;
    if ( GetState ( _Context_, C_SYNTAX ) )
    {
        for ( ib = false, iz = false ; 1 ; iz = false )
        {
            c = rl->InputLine [ index ++ ] ;
            if ( ! c )
            {
                if ( iz ) return false ; // two '0' chars in a row returns false 
                ReadLine_GetLine ( rl ) ;
                index = 0 ;
                iz = true ; // z : zero
                continue ;
            }
            if ( c == '}' )
            {
                if ( --sd <= 1 ) return true ;
                ib = 1 ; // b : bracket
                continue ;
            }
            
            if ( ( c == '/' ) && ( rl->InputLine [ index ] == '/' ) ) CfrTil_CommentToEndOfLine ( ) ;
            else if ( ib && ( c > ' ' ) && ( c != ';' ) ) return false ;
        }
    }
    return false ;
}


