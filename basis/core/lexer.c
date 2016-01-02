#include "../../includes/cfrtil.h"
// lexer.c has been strongly influenced by the ideas in the lisp reader algorithm 
// "http://www.ai.mit.edu/projects/iiip/doc/CommonLISP/HyperSpec/Body/sec_2-2.html"
// although it doesn't fully conform to them yet the intention is to be eventually somewhat of a superset of them
/*
//    !, ", #, $, %, &, ', (, ), *, +, ,, -, ., /, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, :, ;, <, =, >, ?,
// @, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, _CfrTil, R, S, T, U, V, W, X, Y, Z, [, \, ], ^, _,
// `, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z, {, |, }, ~,
 */


#define TokenBuffer_AppendPoint( lexer ) &lexer->TokenBuffer [ lexer->TokenWriteIndex ]
#define _AppendCharacterToTokenBuffer( lex, character ) lexer->TokenBuffer [ lex->TokenWriteIndex ] = character
#define SourceCode_AppendPoint &_Q_->OVT_CfrTil->SourceCodeScratchPad [ strlen ( ( CString ) _Q_->OVT_CfrTil->SourceCodeScratchPad ) ]

void
CfrTil_LexerTables_Setup ( CfrTil * cfrtl )
{
    int32 i ;
    for ( i = 0 ; i < 256 ; i ++ ) cfrtl->LexerCharacterTypeTable [ i ].CharInfo = 0 ;
    cfrtl->LexerCharacterTypeTable [ 0 ].CharFunctionTableIndex = 1 ;
    cfrtl->LexerCharacterTypeTable [ '-' ].CharFunctionTableIndex = 2 ;
    cfrtl->LexerCharacterTypeTable [ '>' ].CharFunctionTableIndex = 3 ;
    cfrtl->LexerCharacterTypeTable [ '.' ].CharFunctionTableIndex = 4 ;
    cfrtl->LexerCharacterTypeTable [ '\n' ].CharFunctionTableIndex = 6 ;
    cfrtl->LexerCharacterTypeTable [ '\\' ].CharFunctionTableIndex = 7 ;
    cfrtl->LexerCharacterTypeTable [ eof ].CharFunctionTableIndex = 8 ;
    cfrtl->LexerCharacterTypeTable [ '\r' ].CharFunctionTableIndex = 9 ;
    cfrtl->LexerCharacterTypeTable [ ',' ].CharFunctionTableIndex = 10 ;

    cfrtl->LexerCharacterTypeTable [ '"' ].CharFunctionTableIndex = 5 ;
    cfrtl->LexerCharacterTypeTable [ '[' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ ']' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ '\'' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ '`' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ '(' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ ')' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ '%' ].CharFunctionTableIndex = 11 ;
    //cfrtl->LexerCharacterTypeTable [ '&' ].CharFunctionTableIndex = 11 ;
    //cfrtl->LexerCharacterTypeTable [ ',' ].CharFunctionTableIndex = 11 ;

    cfrtl->LexerCharacterTypeTable [ '$' ].CharFunctionTableIndex = 12 ;
    cfrtl->LexerCharacterTypeTable [ '#' ].CharFunctionTableIndex = 12 ;
    cfrtl->LexerCharacterTypeTable [ '/' ].CharFunctionTableIndex = 14 ;
    cfrtl->LexerCharacterTypeTable [ ';' ].CharFunctionTableIndex = 15 ;
    cfrtl->LexerCharacterTypeTable [ '&' ].CharFunctionTableIndex = 16 ;
    cfrtl->LexerCharacterTypeTable [ '@' ].CharFunctionTableIndex = 17 ;

    cfrtl->LexerCharacterFunctionTable [ 0 ] = Lexer_Default ;
    cfrtl->LexerCharacterFunctionTable [ 1 ] = _Zero ;
    cfrtl->LexerCharacterFunctionTable [ 2 ] = Minus ;
    cfrtl->LexerCharacterFunctionTable [ 3 ] = GreaterThan ;
    cfrtl->LexerCharacterFunctionTable [ 4 ] = Dot ;
    cfrtl->LexerCharacterFunctionTable [ 5 ] = DoubleQuote ;
    cfrtl->LexerCharacterFunctionTable [ 6 ] = NewLine ;
    cfrtl->LexerCharacterFunctionTable [ 7 ] = BackSlash ;
    cfrtl->LexerCharacterFunctionTable [ 8 ] = _EOF ;
    cfrtl->LexerCharacterFunctionTable [ 9 ] = CarriageReturn ;
    cfrtl->LexerCharacterFunctionTable [ 10 ] = Comma ;

    cfrtl->LexerCharacterFunctionTable [ 11 ] = TerminatingMacro ;
    cfrtl->LexerCharacterFunctionTable [ 12 ] = NonTerminatingMacro ;
    cfrtl->LexerCharacterFunctionTable [ 13 ] = SingleEscape ;
    cfrtl->LexerCharacterFunctionTable [ 14 ] = ForwardSlash ;
    cfrtl->LexerCharacterFunctionTable [ 15 ] = Semi ;
    cfrtl->LexerCharacterFunctionTable [ 16 ] = AddressOf ;
    cfrtl->LexerCharacterFunctionTable [ 17 ] = AtFetch ;
}

byte
Lexer_NextNonDelimiterChar ( Lexer * lexer )
{
    return _String_NextNonDelimiterChar ( _ReadLine_pb_NextChar ( lexer->ReadLiner0 ), lexer->DelimiterCharSet ) ;
}

byte *
Lexer_StrTok ( Lexer * lexer )
{
    byte * buffer = 0 ;
    byte * nextChar = _ReadLine_pb_NextChar ( lexer->ReadLiner0 ) ;
    if ( nextChar )
    {
        buffer = Buffer_Data ( _Q_->OVT_CfrTil->StringB ) ;
        _StrTok ( _ReadLine_pb_NextChar ( lexer->ReadLiner0 ), buffer, lexer->DelimiterCharSet ) ;
    }
    return buffer ;
}

Word *
Lexer_Do_ObjectToken_New ( Lexer * lexer, byte * token, int32 parseFlag )
{
    Word * word = 0 ;
    if ( token )
    {
        if ( parseFlag ) Lexer_ParseObject ( lexer, token ) ;
        if ( lexer->TokenType & T_RAW_STRING )
        {
            if ( GetState ( _Q_, AUTO_VAR ) ) // make it a 'variable' right here 
            {
                word = _CfrTil_Variable ( token, 0 ) ;
                _Interpret_MorphismWord_Default ( _Q_->OVT_Context->Interpreter0, word ) ;
                goto next ;
            }
            else
            {
                ClearLine ;
                Printf ( ( byte* ) "\n%s ?\n", ( char* ) token ) ;
                CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
            }
        }
        word = Literal_New ( lexer, lexer->Literal ) ;
next:
        lexer->TokenWord = word ;
        _Q_->OVT_Context->CurrentRunWord = word ;
        word->W_StartCharRlIndex = _Q_->OVT_Context->Lexer0->TokenStart_ReadLineIndex ;

        _Compiler_WordStack_PushWord ( _Q_->OVT_Context->Compiler0, word ) ;

        if ( ! Lexer_GetState ( lexer, KNOWN_OBJECT ) )
        {
            ClearLine ;
            Printf ( ( byte* ) "\n%s ?\n", ( char* ) token ) ;
            CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
        }
    }
    return word ;
}

byte *
_Lexer_NextNonDebugTokenWord ( Lexer * lexer )
{
    byte * token ;
    Word * word ;
    while ( 1 )
    {
        token = _Lexer_LexNextToken_WithDelimiters ( lexer, 0, 0, 0 ) ;
        word = Finder_Word_FindUsing ( lexer->OurInterpreter->Finder0, token, 1 ) ;
        if ( word && ( word->CType & DEBUG_WORD ) )
        {
            _CfrTil_AddTokenToTailOfTokenList ( token ) ;
        }
        else break ;
    }
    return token ;
}

byte *
Lexer_PeekNextNonDebugTokenWord ( Lexer * lexer )
{
    byte * token ;
    if ( _AtCommandLine ( ) && Lexer_CheckIfDone ( lexer, LEXER_DONE ) ) return 0 ;
    token = _Lexer_NextNonDebugTokenWord ( lexer ) ;
    _CfrTil_AddTokenToTailOfTokenList ( token ) ; // TODO ; list should instead be a stack
    return token ;
}

byte *
_Lexer_LexNextToken_WithDelimiters ( Lexer * lexer, byte * delimiters, int32 checkListFlag, uint64 state )
{
    ReadLiner * rl = lexer->ReadLiner0 ;
    if ( ( ! checkListFlag ) || ( ! ( lexer->OriginalToken = _CfrTil_GetTokenFromTokenList ( ) ) ) ) // ( ! checkListFlag ) : allows us to peek multiple tokens ahead if we already have peeked tokens
    {
        Lexer_Init ( lexer, delimiters, lexer->State, SESSION ) ;
        lexer->State |= state ;
        while ( ( ! Lexer_CheckIfDone ( lexer, LEXER_DONE ) ) ) //&& ( ! ReadLiner_IsDone ( rl ) ) )
        {
            lexer->TokenInputCharacter = lexer->NextChar ( lexer->ReadLiner0 ) ;
            lexer->CurrentReadIndex = rl->ReadIndex ;
            //if ( ! startFlag++ ) lexer->TokenStart_ReadLineIndex = rl->ReadIndex ; 
            Lexer_DoChar ( lexer ) ;
        }
        lexer->CurrentTokenDelimiter = lexer->TokenInputCharacter ;
        lexer->Token_Length = lexer->TokenEnd_ReadLineIndex - lexer->TokenStart_ReadLineIndex + 1 ;
        if ( lexer->TokenWriteIndex && ( ! GetState ( lexer, LEXER_RETURN_NULL_TOKEN ) ) )
        {
            lexer->TokenEnd_ReadLineIndex = rl->ReadIndex - 1 ;
            _AppendCharacterToTokenBuffer ( lexer, 0 ) ; // null terminate TokenBuffer
            lexer->OriginalToken = TemporaryString_New ( lexer->TokenBuffer ) ; // SessionObjectsAllocate
            //_CfrTil_AddNewTokenSymbolToHeadOfTokenList ( lexer->OriginalToken ) ;
        }
        else
        {
            lexer->OriginalToken = ( byte * ) 0 ; // why not goto restartToken ? -- to allow user to hit newline and get response
        }
    }
    //lexer->OriginalToken = token ;
    return lexer->OriginalToken ;
}

void
Lexer_LexNextToken_WithDelimiters ( Lexer * lexer, byte * delimiters )
{
    _Lexer_LexNextToken_WithDelimiters ( lexer, delimiters, 1, 0 ) ;
}

byte *
_Lexer_ReadToken ( Lexer * lexer, byte * delimiters )
{
    Lexer_LexNextToken_WithDelimiters ( lexer, delimiters ) ;
    return lexer->OriginalToken ;
}

byte *
Lexer_ReadToken ( Lexer * lexer )
{
    return _Lexer_ReadToken ( lexer, 0 ) ;
}

void
_Lexer_AppendCharacterToTokenBuffer ( Lexer * lexer )
{
    if ( lexer->TokenWriteIndex == 0 ) lexer->TokenStart_ReadLineIndex = lexer->ReadLiner0->ReadIndex - 1 ;
    lexer->TokenBuffer [ lexer->TokenWriteIndex ++ ] = lexer->TokenInputCharacter ;
    lexer->TokenBuffer [ lexer->TokenWriteIndex ] = 0 ;
}

void
Lexer_Append_ConvertedCharacterToTokenBuffer ( Lexer * lexer )
{
    _String_AppendConvertCharToBackSlash ( TokenBuffer_AppendPoint ( lexer ), lexer->TokenInputCharacter ) ;
    _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter ) ;
    lexer->TokenWriteIndex ++ ;
}

void
Lexer_AppendCharacterToTokenBuffer ( Lexer * lexer )
{
    _Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
    _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter ) ;
}

byte
Lexer_UnAppendCharacterToTokenBuffer ( Lexer * lexer )
{
    return lexer->TokenBuffer [ -- lexer->TokenWriteIndex ] ;
}

byte
Lexer_LastChar ( Lexer * lexer )
{
    return lexer->TokenBuffer [ lexer->TokenWriteIndex - 1 ] ;
}

void
Lexer_SetTokenDelimiters ( Lexer * lexer, byte * delimiters, uint32 allocType )
{
    if ( lexer->DelimiterCharSet ) CharSet_Init ( lexer->DelimiterCharSet, 128, delimiters ) ;
    else lexer->DelimiterCharSet = CharSet_New ( delimiters, allocType ) ;
    lexer->TokenDelimiters = delimiters ;
}

void
Lexer_SetBasicTokenDelimiters ( Lexer * lexer, byte * delimiters, uint32 allocType )
{
    lexer->BasicDelimiterCharSet = CharSet_New ( delimiters, allocType ) ;
    lexer->BasicTokenDelimiters = delimiters ;
}

void
Lexer_Init ( Lexer * lexer, byte * delimiters, uint64 state, uint32 allocType )
{
    lexer->TokenBuffer = _Q_->OVT_CfrTil->TokenBuffer ;
    Mem_Clear ( lexer->TokenBuffer, BUFFER_SIZE ) ;
    lexer->OriginalToken = 0 ;
    lexer->Literal = 0 ;
    if ( delimiters ) Lexer_SetTokenDelimiters ( lexer, delimiters, allocType ) ;
    else
    {
        lexer->DelimiterCharSet = lexer->BasicDelimiterCharSet ; //Lexer_SetTokenDelimiters ( lexer, " \n\r\t", allocType ) ;
        lexer->TokenDelimiters = lexer->BasicTokenDelimiters ;
    }
    lexer->State = state & ( ~ LEXER_RETURN_NULL_TOKEN ) ;
    Lexer_SetState ( lexer, KNOWN_OBJECT | LEXER_DONE | END_OF_FILE | END_OF_STRING | LEXER_END_OF_LINE, false ) ;
    lexer->TokenDelimitersAndDot = ( byte* ) " .\n\r\t" ;
    RestartToken ( lexer ) ;
}

Lexer *
Lexer_New ( uint32 allocType )
{
    Lexer * lexer = ( Lexer * ) Mem_Allocate ( sizeof (Lexer ), allocType ) ;
    Lexer_Init ( lexer, 0, 0, allocType ) ;
    lexer->DelimiterOrDotCharSet = CharSet_New ( lexer->TokenDelimitersAndDot, allocType ) ;
    Lexer_SetBasicTokenDelimiters ( lexer, (byte*) " \n\r\t", allocType ) ;
    lexer->ReadLiner0 = ReadLine_New ( allocType ) ;
    lexer->NextChar = _Lexer_NextChar ;
    return lexer ;
}

void
_Lexer_Copy ( Lexer * lexer, Lexer * lexer0, uint32 allocType )
{
    memcpy ( lexer, lexer0, sizeof (Lexer ) ) ;
    Lexer_Init ( lexer, 0, 0, allocType ) ;
    lexer->NextChar = _Lexer_NextChar ;
}

Lexer *
Lexer_Copy ( Lexer * lexer0, uint32 allocType )
{
    Lexer * lexer = ( Lexer * ) Mem_Allocate ( sizeof (Lexer ), allocType ) ;
    _Lexer_Copy ( lexer, lexer0, allocType ) ;
    return lexer ;
}

void
RestartToken ( Lexer * lexer )
{
    lexer->TokenWriteIndex = 0 ;
}

// special case here is quoted Strings - "String literals"
// use lexer->ReadLinePosition = 0 to cause a new Token read
// or lexer->Token_ReadLineIndex = BUFFER_SIZE

void
Lexer_SourceCodeOn ( Lexer * lexer )
{
    Lexer_SetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ), true ) ;
}

void
Lexer_SourceCodeOff ( Lexer * lexer )
{
    Lexer_SetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ), false ) ;
}

void
_Lexer_AppendCharToSourceCode ( Lexer * lexer, byte c )
{
    if ( Lexer_GetState ( lexer, ADD_CHAR_TO_SOURCE ) )
    {
        _CfrTil_AppendCharToSourceCode ( c ) ;
    }
}

void
Lexer_DoDelimiter ( Lexer * lexer )
{
    _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter == '\n' ? ' ' : lexer->TokenInputCharacter ) ;
    // must have at least one non-delimiter to make a token
    // else keep going we just have multiple delimiters ( maybe just spaces ) in a row
    if ( lexer->TokenWriteIndex )
    {
        Lexer_SetState ( lexer, LEXER_DONE, true ) ;
        return ;
    }
    else
    {
        RestartToken ( lexer ) ; //prevent null token which complicates lexers
        return ;
    }
}

Boolean
Lexer_IsCurrentInputCharADelimiter ( Lexer * lexer )
{
    return ( Boolean ) _Lexer_IsCharDelimiter ( lexer, lexer->TokenInputCharacter ) ;
}

void
Lexer_Default ( Lexer * lexer )
{
    if ( Lexer_IsCurrentInputCharADelimiter ( lexer ) ) //_IsChar_Delimiter ( lexer->TokenDelimiters, lexer->TokenInputCharacter ) )
    {
        _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter == '\n' ? ' ' : lexer->TokenInputCharacter ) ;
        //_Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter ) ; //== '\n' ? ' ' : lexer->TokenInputCharacter ) ;
        // must have at least one non-delimiter to make a token
        // else keep going we just have multiple delimiters ( maybe just spaces ) in a row
        if ( lexer->TokenWriteIndex )
        {
            Lexer_SetState ( lexer, LEXER_DONE, true ) ;
            return ;
        }
        else
        {
            RestartToken ( lexer ) ; //prevent null token which complicates lexers
            return ;
        }
    }
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}

void
TerminatingMacro ( Lexer * lexer )
{
    if ( ( ! lexer->TokenWriteIndex ) || ( lexer->TokenBuffer [ lexer->TokenWriteIndex - 1 ] == '_' ) ) Lexer_Default ( lexer ) ; // allow for "_(" token 
    else ReadLine_UnGetChar ( lexer->ReadLiner0 ) ; // so NextChar will have this TokenInputCharacter for the next token
    Lexer_FinishTokenHere ( lexer ) ;
    return ;
}

void
NonTerminatingMacro ( Lexer * lexer )
{
    Lexer_Default ( lexer ) ;
    if ( lexer->TokenWriteIndex == 1 )
    {
        byte chr = ReadLine_PeekNextChar ( lexer->ReadLiner0 ) ;
        if ( ( chr != 'x' ) && ( chr != 'X' ) && ( chr != 'b' ) && ( chr != 'o' ) && ( chr != 'd' ) ) Lexer_FinishTokenHere ( lexer ) ; // x/X : check for hexidecimal marker
    }
    return ;
}

int32
_Lexer_MacroChar_NamespaceCheck ( Lexer * lexer, byte * nameSpace )
{
    byte buffer [2] ;
    buffer [0] = lexer->TokenInputCharacter ;
    buffer [1] = 0 ;
    return _CfrTil_IsContainingNamespace ( buffer, nameSpace ) ;
}

void
Lexer_FinishTokenHere ( Lexer * lexer )
{
    _AppendCharacterToTokenBuffer ( lexer, 0 ) ;
    Lexer_SetState ( lexer, LEXER_DONE, true ) ;
    return ;
}

void
SingleEscape ( Lexer * lexer )
{
    lexer->TokenInputCharacter = ReadLine_NextChar ( lexer->ReadLiner0 ) ;
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}

void
_MultipleEscape ( Lexer * lexer )
{
    byte multipleEscapeChar = lexer->TokenInputCharacter ;
    while ( 1 )
    {
        lexer->TokenInputCharacter = ReadLine_NextChar ( lexer->ReadLiner0 ) ;
        if ( lexer->TokenInputCharacter == multipleEscapeChar ) break ;
        Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
    }
    Lexer_SetState ( lexer, LEXER_DONE, true ) ;
}

// '"'

void
DoubleQuote ( Lexer * lexer )
{
    TerminatingMacro ( lexer ) ;
}


// '->' for pointers within a string and without surrounding spaces 

void
Minus ( Lexer * lexer ) // '-':
{
    byte nextChar ;
    if ( lexer->TokenWriteIndex )
    {
        nextChar = ReadLine_PeekNextChar ( lexer->ReadLiner0 ) ;
        if ( ( nextChar == '-' ) || ( nextChar == '>' ) )
        {
            ReadLine_UnGetChar ( lexer->ReadLiner0 ) ; // allow to read '[' or ']' as next token
            Lexer_SetState ( lexer, LEXER_DONE, true ) ;
            return ;
        }
    }
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}

void
ForwardSlash ( Lexer * lexer ) // '/':
{
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
    if ( ReadLine_PeekNextChar ( lexer->ReadLiner0 ) == '/' )
    {
        lexer->NextChar ( lexer->ReadLiner0 ) ;
        Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
        Lexer_SetState ( lexer, LEXER_DONE, true ) ;
    }
}

void
AddressOf ( Lexer * lexer ) // ';':
{
    //if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) && ( ReadLine_PeekNextChar ( lexer->ReadLiner ) != '&' ) ) TerminatingMacro ( lexer ) ;
    //if ( ( CharTable_IsCharType ( ReadLine_PeekNextChar ( lexer->ReadLiner ), CHAR_ALPHA ) && ( ReadLine_LastChar ( lexer->ReadLiner ) != '&' ) ) ) TerminatingMacro ( lexer ) ;
    if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) && CharTable_IsCharType ( ReadLine_PeekNextChar ( lexer->ReadLiner0 ), CHAR_ALPHA ) ) TerminatingMacro ( lexer ) ;
    else Lexer_Default ( lexer ) ;
}

void
AtFetch ( Lexer * lexer ) // ';':
{
    Lexer_Default ( lexer ) ;
    if ( _Q_->OVT_LC && GetState ( _Q_->OVT_LC, LC_READ ) ) Lexer_FinishTokenHere ( lexer ) ;
}

void
Semi ( Lexer * lexer ) // ';':
{
    //if ( _CfrTil_AreWeInThisNamespace ( "C_Syntax" ) ) 
    if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) ) TerminatingMacro ( lexer ) ;
    else Lexer_Default ( lexer ) ;
}

void
GreaterThan ( Lexer * lexer ) // '>':
{
    if ( lexer->TokenWriteIndex )
    {
        if ( Lexer_LastChar ( lexer ) == '-' )
        {
            Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
            Lexer_SetState ( lexer, LEXER_DONE, true ) ;

            return ;
        }
    }
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}

// package the dot to be lexed as a token

void
Dot ( Lexer * lexer ) //  '.':
{
    if ( ( Lexer_LastChar ( lexer ) != '/' ) && ( ! GetState ( lexer, LEXER_ALLOW_DOT ) ) ) //allow for lisp special char sequence "/." as a substitution for lambda
    {
        int32 i ;
        if ( ( ! Lexer_GetState ( lexer, PARSING_STRING ) ) ) //&& ( ! GetState ( _Q_->OVT_Context, CONTEXT_PARSING_QUALIFIED_ID ) ) ) // if we are not parsing a String ?
        {
            if ( lexer->TokenWriteIndex )
            {
                for ( i = lexer->TokenWriteIndex - 1 ; i >= 0 ; i -- ) // go back into previous chars read, check if it is a number
                {
                    if ( ! isdigit ( lexer->TokenBuffer [ i ] ) )
                    {
                        ReadLine_UnGetChar ( lexer->ReadLiner0 ) ; // allow to read '.' as next token
                        //_CfrTil_UnAppendFromSourceCode ( 1 ) ;
                        Lexer_SetState ( lexer, LEXER_DONE, true ) ;
                        return ;
                    }
                }
            }
            else if ( ! isdigit ( ReadLine_PeekNextChar ( lexer->ReadLiner0 ) ) )
            {
                Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
                Lexer_SetState ( lexer, LEXER_DONE, true ) ;
                return ;
            }
        }
    }
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}

void
Lexer_DoReplMacro ( Lexer * lexer )
{
    ReadLine_UnGetChar ( lexer->ReadLiner0 ) ; // let the repl re-get the char 
    Lexer_FinishTokenHere ( lexer ) ;
    LO_ReadEvalPrint ( ) ;
    SetState ( lexer, LEXER_RETURN_NULL_TOKEN, true ) ;
    return ;
}

void
Lexer_CheckMacroRepl ( Lexer * lexer )
{
    byte nextChar = ReadLine_PeekNextNonWhitespaceChar ( lexer->ReadLiner0 ) ;
    if ( ( nextChar == '(' ) ) //|| ( nextChar == ',' ) )
    {
        Lexer_DoReplMacro ( lexer ) ;
        return ;
    }
}

void
Comma ( Lexer * lexer )
{
    if ( ! GetState ( _Q_->OVT_Context->Compiler0, LC_ARG_PARSING ) )
    {
        if ( _Lexer_MacroChar_NamespaceCheck ( lexer, (byte*) "Lisp" ) )
        {
            if ( _Q_->OVT_LC )
            {
                byte nextChar = ReadLine_PeekNextNonWhitespaceChar ( lexer->ReadLiner0 ) ;
                if ( nextChar == '@' )
                {
                    Lexer_AppendCharacterToTokenBuffer ( lexer ) ; // the comma
                    byte chr = _ReadLine_GetNextChar ( lexer->ReadLiner0 ) ; // the '@'
                    lexer->TokenInputCharacter = chr ;
                }
                Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
                Lexer_FinishTokenHere ( lexer ) ;

                return ;
            }
        }
    }
    Lexer_Default ( lexer ) ;
}

void
_BackSlash ( Lexer * lexer, int32 flag )
{
    ReadLiner * rl = lexer->ReadLiner0 ;
    byte nextChar = rl->InputLine [ rl->ReadIndex ], lastChar = rl->InputLine [ rl->ReadIndex - 2 ] ;
    if ( nextChar == 'n' )
    {
        _ReadLine_GetNextChar ( lexer->ReadLiner0 ) ;
        lexer->TokenInputCharacter = '\n' ;
        Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
    }
    else if ( nextChar == 'r' )
    {
        _ReadLine_GetNextChar ( lexer->ReadLiner0 ) ;
        lexer->TokenInputCharacter = '\r' ;
        Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
    }
    else if ( nextChar == 't' )
    {
        _ReadLine_GetNextChar ( lexer->ReadLiner0 ) ;
        lexer->TokenInputCharacter = '\t' ;
        Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
    }
    else if ( lastChar == '/' ) // current lisp lambda abbreviation "/\"
    {
        Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
    }
    else if ( flag ) SingleEscape ( lexer ) ;
    else if ( ! flag ) Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}

void
BackSlash ( Lexer * lexer )
{
    _BackSlash ( lexer, 1 ) ;
}

void
CarriageReturn ( Lexer * lexer )
{
    NewLine ( lexer ) ;
}

void
NewLine ( Lexer * lexer )
{
    if ( ( ! _Q_->OVT_Context->System0->IncludeFileStackNumber ) || GetState ( _Q_->OVT_CfrTil->Debugger0, DBG_COMMAND_LINE ) )
    {
        Lexer_SetState ( lexer, LEXER_DONE | LEXER_END_OF_LINE, true ) ;
        if ( lexer->OurInterpreter ) Interpreter_SetState ( lexer->OurInterpreter, INTERPRETER_DONE | END_OF_LINE, true ) ;
    }
    else
    {
        Lexer_SetState ( lexer, LEXER_END_OF_LINE, true ) ;
        Lexer_Default ( lexer ) ;
    }
}

void
_EOF ( Lexer * lexer ) // case eof:
{

    if ( lexer->OurInterpreter ) Interpreter_SetState ( lexer->OurInterpreter, END_OF_FILE, true ) ;
    Lexer_SetState ( lexer, LEXER_DONE | END_OF_FILE, true ) ;
}

void
_Zero ( Lexer * lexer ) // case 0
{

    if ( lexer->OurInterpreter ) Interpreter_SetState ( lexer->OurInterpreter, END_OF_STRING, true ) ;
    Lexer_SetState ( lexer, LEXER_DONE | END_OF_STRING, true ) ;
}

int32
Lexer_CheckIfDone ( Lexer * lexer, int32 flags )
{
    return lexer->State & flags ;
}

// the non-string lexer char input function

byte
_Lexer_NextChar ( ReadLiner * rl )
{
    return ReadLine_NextChar ( rl ) ;
}

void
Lexer_SetInputFunction ( Lexer * lexer, byte ( *lipf ) ( ReadLiner *) )
{
    lexer->NextChar = lipf ;
}

void
Lexer_DoChar ( Lexer * lexer )
{
    _Q_->OVT_CfrTil->LexerCharacterFunctionTable [ _Q_->OVT_CfrTil->LexerCharacterTypeTable [ lexer->TokenInputCharacter ].CharInfo ] ( lexer ) ;
}

Boolean
Lexer_IsTokenQualifiedID ( Lexer * lexer )
{
    if ( Lexer_IsTokenReverseDotted ( lexer ) ) return true ;
    else return Lexer_IsTokenForwardDotted ( lexer ) ;
}

Boolean
Lexer_IsTokenReverseDotted ( Lexer * lexer )
{
    ReadLiner * rl = lexer->ReadLiner0 ;
    int32 i, start = lexer->TokenStart_ReadLineIndex - 1 ;
    for ( i = start ; i >= 0 ; i -- )
    {
        if ( rl->InputLine [ i ] == '.' ) return true ;
        if ( rl->InputLine [ i ] == ']' ) return true ;
        if ( isgraph ( rl->InputLine [ i ] ) ) break ;
    }
    return false ;
}

Boolean
Lexer_IsTokenForwardDotted ( Lexer * lexer )
{
    ReadLiner * rl = lexer->ReadLiner0 ;
    int32 i, end = lexer->TokenEnd_ReadLineIndex + 1 ;
    for ( i = end ; i < rl->MaxEndPosition ; i ++ )
    {
        if ( rl->InputLine [ i ] == '.' ) return true ;
        if ( rl->InputLine [ i ] == '[' ) return true ;

        if ( isgraph ( rl->InputLine [ i ] ) ) break ;
    }
    return false ;
}
