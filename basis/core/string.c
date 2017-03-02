
#include "../../include/cfrtil.h"

Boolean
IsChar_Dot ( byte character )
{
    return character == '.' ;
}

Boolean
IsChar_Whitespace ( byte character )
{
    return character <= ' ' ;
}

Boolean
IsChar_DelimiterOrDot ( byte character )
{
    return _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, character ) ;
}

Boolean
IsChar_ADotAndNotANonDelimiter ( byte character )
{
    return _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, character ) ;
}

// backward parsing

void
Mem_Clear ( byte * buffer, int32 size )
{
    memset ( ( char* ) buffer, 0, size ) ;
}

//|-----------------------------
//| REVERSE PARSING ...
//|-----------------------------

int32
String_IsPreviousCharA_ ( byte * s, int32 pos, byte c )
{
    int32 i ;
    for ( i = pos ; i >= 0 ; i -- )
    {
        if ( s [ i ] == c ) return i ;
        else if ( _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s [ i ] ) ) continue ;
        else break ;
    }
    return false ;
}

int32
String_IsLastCharADot ( byte * s, int32 pos )
{
    return String_IsPreviousCharA_ ( s, pos, ( byte ) '.' ) ;
}

int32
String_FirstCharOfToken_FromPosOfLastChar ( byte * s, int32 pos )
{
    int32 i ;
    for ( i = pos ; i ; i -- )
    {
        if ( _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s [i] ) ) break ;
    }
    return _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s [i] ) ? i + 1 : i ; // nb. we could have 'break' becuase i == 0 - beginning of line
}

int32
String_IsThereADotSeparatorBackFromPosToLastNonDelmiter ( byte * s, int32 pos )
{
    int32 i ;
    for ( i = pos ; i > 0 ; i -- )
    {
        if ( _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s [i] ) )
        {
            if ( s [i] == '.' )
            {
                if ( s [i - 1] == '.' ) return i - 1 ; // deal with the unique case of the dot, '.', token in the Class namespace 
                else return i ;
            }
        }
        else return 0 ;
    }
    return 0 ;
}
// reverse parsing

int32
String_LastCharOfLastToken_FromPos ( byte * s, int32 pos )
{
    int32 i, spaces = 0, dotFlag = 0 ;
    for ( i = pos ; i ; i -- )
    {
        if ( ! _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s[ i ] ) ) break ;
        if ( ( i != _ReadLine_CursorPosition ( _Context_->ReadLiner0 ) ) && ( s [ i ] == ' ' ) ) spaces ++ ;
        if ( s[ i ] == '.' ) dotFlag ++ ;
        // a space with no dot is an end of token
    }
    if ( ( spaces && ( ! dotFlag ) ) || ( dotFlag > 1 ) ) return pos ;
    return i ;
}

int32
String_FirstTokenDelimiter_FromPos ( byte * s, int32 pos )
{
    int32 i, flag = 0 ;
    for ( i = pos ; 1 ; i ++ )
    {
        if ( ! _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s[ i ] ) ) flag = 1 ;
        if ( flag && _Lexer_IsCharDelimiterOrDot ( _Context_->Lexer0, s[ i ] ) ) break ;
    }
    return i ;
}

Boolean
String_IsReverseTokenQualifiedID ( byte * s, int32 pos )
{
    //int32 lastChar = ReadLine_LastCharOfLastToken_FromPos ( rl, rl->ReadIndex ) ;
    int32 lastChar = String_LastCharOfLastToken_FromPos ( s, pos ) ;
    //int32 firstChar = ReadLine_FirstCharOfToken_FromLastChar ( rl, lastChar ) ;
    int32 firstChar = String_FirstCharOfToken_FromPosOfLastChar ( s, lastChar ) ;
    //return ReadLine_IsThereADotSeparator ( rl, firstChar - 1 ) ;
    return String_IsThereADotSeparatorBackFromPosToLastNonDelmiter ( s, firstChar ) ;
}

/*
|-----------------------------|
| ... REVERSE PARSING         |
|-----------------------------|
 */

// unbox string 'in place'

byte *
_String_UnBox ( byte * token )
{
    byte * start ;
    if ( token [ 0 ] == '"' )
    {
        char * s = Buffer_Data ( _CfrTil_->TokenB ) ;
        strcpy ( ( char* ) s, ( char* ) token ) ; // preserve token - this string is used by the Interpreter for SourceCode
        int32 length = Strlen ( ( char* ) s ) ;
        if ( s [ length - 1 ] == '"' )
        {
            s [ length - 1 ] = 0 ;
        }
        s = & s [ 1 ] ;
        start = String_New ( ( byte* ) s, TEMPORARY ) ;
    }
    else start = token ;
    return start ;
}

byte *
_String_InsertColors ( byte * s, Colors * c )
{
    if ( s )
    {
        Colors * current = _Q_->Current ;
        byte * tbuffer = Buffer_Data ( _CfrTil_->StringInsertB ) ;
        String_ShowColors ( tbuffer, c ) ; // new foreground, new background
        strcat ( ( char* ) tbuffer, ( char* ) s ) ;
        String_ShowColors ( &tbuffer[Strlen ( ( char* ) tbuffer )], current ) ; // old foreground, old background
        tbuffer = String_New ( tbuffer, TEMPORARY ) ;
        return tbuffer ;
    }
    else return ( byte* ) "" ;
}

byte *
_String_Insert_AtIndexWithColors ( byte * token, int ndx, Colors * c )
{
    int preTokenLen ; // Lexer reads char finds it is delimiter : reading index auto increments index 
    if ( strncmp ( ( char* ) token, ( char* ) &_Context_->ReadLiner0->InputLine [ ndx ], Strlen ( ( char* ) token ) ) )
        return String_RemoveFinalNewline ( String_New ( ( byte* ) _Context_->ReadLiner0->InputLine, TEMPORARY ) ) ;
    byte * buffer = Buffer_Data ( _CfrTil_->StringInsertB2 ) ;
    byte * tbuffer = Buffer_Data ( _CfrTil_->StringInsertB3 ) ;

    strcpy ( ( char* ) buffer, ( char* ) _Context_->ReadLiner0->InputLine ) ;
    String_RemoveFinalNewline ( ( byte* ) buffer ) ;
    if ( ! _Lexer_IsCharDelimiter ( _Context_->Lexer0, buffer [ ndx ] ) ) ndx ++ ; // Lexer index auto increments index at token end ; dot doesn't incrment index therefore it is a dot at index
    preTokenLen = ndx - Strlen ( ( char* ) token ) ;
    if ( preTokenLen < 0 ) preTokenLen = 0 ;

    Strncpy ( ( char* ) tbuffer, ( char* ) buffer, preTokenLen ) ; // copy upto start of token
    tbuffer [ preTokenLen ] = 0 ; // Strncpy does not necessarily null delimit
    String_ShowColors ( &tbuffer [ Strlen ( ( char* ) tbuffer ) ], c ) ; // new foreground, new background
    strcat ( ( char* ) tbuffer, ( char* ) token ) ;
    String_ShowColors ( &tbuffer [ Strlen ( ( char* ) tbuffer ) ], &_Q_->Default ) ; // old foreground, old background
    strcat ( ( char* ) tbuffer, ( char* ) &buffer [ ndx ] ) ; // copy the rest of the buffer after the token : -1 : get the delimiter; 0 based array
    tbuffer = String_New ( tbuffer, TEMPORARY ) ;
    return tbuffer ;
}

byte *
String_ReadLineToken_HighLight ( byte * token )
{
    return _String_Insert_AtIndexWithColors ( token, _Context_->ReadLiner0->ReadIndex - 1, &_Q_->User ) ;
}

// ?? use pointers with these string functions ??

byte *
_String_AppendConvertCharToBackSlashAtIndex ( byte * dst, byte c, int32 * index, int32 quoteMode )
{
    int32 i = * index ;
    if ( c < ' ' )
    {
        if ( quoteMode )
        {
            if ( c == '\n' )
            {
                dst [ i ++ ] = '\\' ;
                dst [ i ++ ] = 'n' ;
            }
            else if ( c == '\r' )
            {
                dst [ i ++ ] = '\\' ;
                dst [ i ++ ] = 'r' ;
            }
            else if ( c == '\t' )
            {
                dst [ i ++ ] = '\\' ;
                dst [ i ++ ] = 't' ;
            }
        }
        else dst [ i ++ ] = ' ' ;
    }
    else
    {
        if ( ! ( ( c == ' ' ) && ( dst [i - 1] == ' ' ) ) ) dst [ i ++ ] = c ;
    }
    dst [ i ] = 0 ;
    *index = i ;
    return &dst [ i ] ;
}

byte *
_String_AppendConvertCharToBackSlash ( byte * dst, byte c )
{
    int i = 0 ;
    if ( ( c < ' ' ) )
    {
        if ( c == '\n' )
        {
            dst [ i ++ ] = '\\' ;
            dst [ i ++ ] = 'n' ;
        }
        else if ( c == '\r' )
        {
            dst [ i ++ ] = '\\' ;
            dst [ i ++ ] = 'r' ;
        }
        else if ( c == '\t' )
        {
            dst [ i ++ ] = '\\' ;
            dst [ i ++ ] = 't' ;
        }
    }
    else dst [ i ++ ] = c ;
    dst [ i ] = 0 ;
    return &dst [ i ] ;
}

byte *
_String_ConvertStringFromBackSlash ( byte * dst, byte * src )
{
    int i, j, len = Strlen ( ( char* ) src ), quoteMode = 0 ;
    for ( i = 0, j = 0 ; i < len ; i ++ )
    {
        byte c = src [ i ] ;
        if ( c == '"' )
        {
            if ( quoteMode ) quoteMode = 0 ;
            else quoteMode = 1 ;
        }
        if ( ! quoteMode )
        {
            if ( c == '\\' )
            {
                byte d = src [ ++ i ] ;
                if ( d == 'n' )
                {
                    dst [ j ++ ] = ' ' ;
                    continue ;
                }
                else if ( d == 'r' )
                {
                    dst [ j ++ ] = ' ' ;
                    continue ;
                }
                else if ( c == '\t' )
                {
                    dst [ j ++ ] = '\\' ;
                    dst [ j ++ ] = 't' ;
                }
            }
        }
        dst [ j ++ ] = c ;
    }
    dst [ j ] = 0 ;
    return dst ;
}

byte
String_ConvertEscapeCharToSpace ( byte c )
{
    if ( ( c == '\n' ) || ( c == '\t' ) || ( c == '\r' ) ) c = ' ' ;
    return c ;
}

byte *
_String_ConvertString_EscapeCharToSpace ( byte * dst, byte * src )
{
    int i, j, len = Strlen ( ( char* ) src ), quoteMode = 0 ;
    for ( i = 0, j = 0 ; i < len ; i ++ )
    {
        byte c = src [ i ] ;
        if ( c == '"' )
        {
            if ( quoteMode ) quoteMode = 0 ;
            else quoteMode = 1 ;
        }
        else if ( c == ' ' )
        {
            quoteMode = 0 ;
            while ( src [ i + 1 ] == ' ' ) i ++ ;
            dst [ j ++ ] = src [ i ] ;
            continue ;
        }
        else if ( ! quoteMode )
        {
            if ( ( c == '\n' ) || ( c == '\r' ) || ( c == '\t' ) ) c = ' ' ;
            else if ( c == '\\' )
            {
                byte d = src [ ++ i ] ;
                if ( ( d == 'n' ) || ( d == 'r' ) || ( d == 't' ) ) c = ' ' ;
            }
        }
        dst [ j ++ ] = c ;
    }
    dst [ j ] = 0 ;
    return dst ;
}

byte *
String_ConvertString_EscapeCharToSpace ( byte * istring )
{
    byte * nstring = Buffer_Data ( _CfrTil_->StringInsertB3 ) ;
    _String_ConvertString_EscapeCharToSpace ( nstring, istring ) ;
    nstring = String_New ( ( byte* ) nstring, TEMPORARY ) ;
    return nstring ;
}

byte *
_String_ConvertStringToBackSlash ( byte * dst, byte * src )
{

    int i, j, len = src ? Strlen ( ( char* ) src ) : 0, quote = 1 ;
    for ( i = 0, j = 0 ; i < len ; i ++ )
    {
        byte c = src [ i ] ;

        if ( c == '"' )
        {
            if ( ! quote ) quote = 1 ;
            else quote = 0 ;
        }
        if ( c < ' ' )
        {
            if ( quote )
            {
                if ( c == '\n' )
                {
                    dst [ j ++ ] = '\\' ;
                    dst [ j ++ ] = 'n' ;
                }
                else if ( c == '\r' )
                {
                    dst [ j ++ ] = '\\' ;
                    dst [ j ++ ] = 'r' ;
                }
                else if ( c == '\t' )
                {
                    dst [ j ++ ] = '\\' ;
                    dst [ j ++ ] = 't' ;
                }
            }
        }
        else dst [ j ++ ] = c ;
    }
    dst [ j ] = 0 ;
    return dst ;
}

byte *
String_ConvertToBackSlash ( byte * str )
{
    //byte * dst = Buffer_Data ( _CfrTil_->Scratch1B ) ;
    Buffer * b = Buffer_New ( BUFFER_SIZE ) ;
    byte * buffer = Buffer_Data ( b ) ;
    _String_ConvertStringToBackSlash ( buffer, str ) ;
    return buffer ;
}

int32
stricmp ( byte * str0, byte * str1 )
{
    int32 i, result = 0 ;
    for ( i = 0 ; str0 [ i ] && ( ! result ) ; i ++ )
    {
        result = tolower ( ( int ) str0 [ i ] ) - tolower ( ( int ) str1 [ i ] ) ;
    }
    return result ;
}

int32
StrnCmp ( byte * str0, byte * str1, int32 n )
{
    int32 i, result = 0 ;
    for ( i = 0 ; str0 [ i ] && str1 [ i ] && n && ( ! result ) ; i ++, n -- )
    {
        result = ( int ) str0 [ i ] - ( int ) str1 [ i ] ;
    }
    if ( ! n ) return result ;
    else return ( - 1 ) ;
}

int32
StrnICmp ( byte * str0, byte * str1, int32 n )
{
    int32 i, result = 0 ;
    for ( i = 0 ; str0 [ i ] && str1 [ i ] && n && ( ! result ) ; i ++, n -- )
    {
        result = tolower ( ( int ) str0 [ i ] ) - tolower ( ( int ) str1 [ i ] ) ;
    }
    if ( ! n ) return result ;
    else return - 1 ;
}

byte *
strToLower ( byte * dest, byte * str )
{
    int i ;
    for ( i = 0 ; str [ i ] ; i ++ )
    {
        dest [ i ] = tolower ( str [ i ] ) ;
    }
    dest [i] = 0 ;
    return dest ;
}

void
String_RemoveEndWhitespaceAndAddNewline ( byte * string )
{
    byte * ptr = string + Strlen ( ( char* ) string ) ;
    for ( ; *ptr <= ' ' ; ptr -- ) ;
    *++ ptr = '\n' ;
    *++ ptr = 0 ;
}

byte *
String_FilterMultipleSpaces ( byte * istring, int32 allocType )
{
    int32 i, j ;
    byte * nstring = Buffer_Data ( _CfrTil_->StringInsertB3 ) ;
    for ( i = 0, j = 0 ; istring [ i ] ; i ++ )
    {
        if ( ( istring [ i ] == ' ' ) && ( istring [ i + 1 ] == ' ' ) ) continue ;
        nstring [ j ++ ] = istring [ i ] ;
    }
    nstring [ j ] = 0 ;
    nstring = String_New ( ( byte* ) nstring, allocType ) ;
    return nstring ;
}

void
String_InsertCharacter ( CString into, int32 position, byte character )
{
    char * b = ( char* ) Buffer_Data ( _CfrTil_->StringInsertB2 ) ;
    strcpy ( ( char* ) b, into ) ;
    b [ position ] = character ;
    b [ position + 1 ] = 0 ;
    strcat ( ( char* ) b, &into [ position ] ) ;
    strcpy ( into, ( CString ) b ) ;
}

#if 0

CString
String_Wrap ( CString in, CString s, CString pre, CString post )
{
    //if ( Strlen ( s ) + Strlen (pre) + Strlen (post) < BUFFER_SIZE )
    {
        strcpy ( in, pre ) ;
        strcat ( in, s ) ;
        strcat ( in, post ) ;
        return in ;
    }
    //else CfrTil_Exception ( BUFFER_OVERFLOW, 1 ) ;
}
#endif
// insert data into slot ( startOfSlot, endOfSlot ) in buffer

void
String_InsertDataIntoStringSlot ( byte * str, int32 startOfSlot, int32 endOfSlot, byte * data ) // size in bytes
{
    byte * b = Buffer_DataCleared ( _CfrTil_->StringInsertB2 ) ;
    if ( ( Strlen ( ( char* ) str ) + Strlen ( ( char* ) data ) ) < BUFFER_SIZE )
    {
        if ( Strlen ( ( char* ) str ) > startOfSlot ) //( endOfSlot - startOfSlot ) )
        {
            strcpy ( ( char* ) b, ( char* ) str ) ;
            strcpy ( ( char* ) & b [ startOfSlot ], ( char* ) data ) ; // watch for overlapping ??
            strcat ( ( char* ) b, ( char* ) &str [ endOfSlot ] ) ;
            strcpy ( ( char* ) str, ( char* ) b ) ;
        }
        else strcat ( ( char* ) str, ( char* ) data ) ;
    }
    else CfrTil_Exception ( BUFFER_OVERFLOW, 1 ) ;
}

byte *
String_RemoveFinalNewline ( byte * astring )
{
    byte character = astring [ Strlen ( ( char* ) astring ) - 1 ] ;
    if ( character == '\n' || character == '\r' || character == eof ) astring [ Strlen ( ( char* ) astring ) - 1 ] = 0 ;
    return astring ;
}

// necessary for strings with '"' in them

byte *
String_N_New ( byte * string, int32 n, uint32 allocType )
{
    byte * newString ;
    if ( string )
    {
        newString = Mem_Allocate ( n + 1, allocType ) ;
        Strncpy ( ( char* ) newString, ( char* ) string, n ) ;
        return newString ;
    }
    return 0 ;
}

byte *
String_New ( byte * string, uint32 allocType )
{
    byte * newString ;
    if ( string )
    {
        newString = Mem_Allocate ( Strlen ( ( char* ) string ) + 1, allocType ) ;
        d0 ( if ( newString == ( byte* ) 0xf7c3a6fa )
        {
            Printf ( "\nstring = 0x%8x", newString ) ;
        } )
        strcpy ( ( char* ) newString, ( char* ) string ) ;
        return newString ;
    }
    return 0 ;
}

byte
_String_NextNonDelimiterChar ( byte * str0, byte * cset )
{
    if ( str0 )
    {
        for ( ; *str0 ; str0 ++ )
        {
            if ( ! _CharSet_IsDelimiter ( cset, *str0 ) ) break ;
        }
    }
    return str0 ? *str0 : 0 ;
}

int32
_CfrTil_StrTok ( byte * inBuffer )
{
    StrTokInfo * sti = & _CfrTil_->Sti ;
    int i, start, end ;
    byte * str0 = sti->In = inBuffer, * buffer = sti->Out, *cset = sti->CharSet0, *str1, *str2 ;
    // find start of non-delimiter text
    // str0 is the original string
    for ( i = 0, str1 = str0 ; *str1 ; str1 ++, i ++ )
    {
        if ( ! _CharSet_IsDelimiter ( cset, *str1 ) ) break ;
    }
    start = i ;
    // str1 is the start of our search string - to find the first "delimiter"
    // find first string <== the first delimiter after start of non-delimiter text
    for ( i = 0, str2 = str1 ; *str2 ; str2 ++, i ++ )
    {
        if ( _CharSet_IsDelimiter ( cset, *str2 ) ) break ;
        buffer [ i ] = * str2 ;
    }
    // str2 is either a delimiter or a null, which is also a delimiter. Add 1 for start of a possible next token ...
    if ( *str2 ) // check if it's just a null, the end of string - 0
    {
        buffer [ i ] = 0 ; // remember buffer is sti->Out, the arg to the macro function, the pre-expanded string, if that macro function exists in SMNamespace
        end = start + i ;
        sti->StartIndex = start ;
        sti->EndIndex = end ;
    }
    else end = 0 ;
    return end ;
}

byte *
StringMacro_Run ( byte * pb_namespaceName, byte * str )
{
    byte *nstr ;
    Word * sword ;
    Namespace * ns ;
    if ( pb_namespaceName )
    {
        ns = Namespace_Find ( pb_namespaceName ) ;
        if ( ns ) sword = Word_FindInOneNamespace ( ns, str ) ;
    }
    else sword = _Word_FindAny ( str ) ;
    if ( sword )
    {
        _Word_Run ( sword ) ;
        nstr = ( byte* ) _DataStack_Pop ( ) ;
        return nstr ;
    }
    return 0 ;
}

byte *
_CfrTil_StringMacros_Init ( )
{
    StrTokInfo * sti = & _CfrTil_->Sti ;
    //byte * pb_nsn = StringMacro_Run ( "Root", "_SMN_" ) ; // _SMN_ StringMacrosNamespace
    byte * pb_nsn = StringMacro_Run ( 0, ( byte* ) "_SMN_" ) ; // _SMN_ StringMacrosNamespace
    if ( pb_nsn )
    {
        sti->SMNamespace = pb_nsn ;
        byte * delimiters = StringMacro_Run ( pb_nsn, ( byte* ) "Delimiters" ) ;
        if ( ! delimiters ) delimiters = _Context_->Lexer0->TokenDelimiters ;
        //memset ( sti, 0, sizeof (StrTokInfo ) ) ;
        // sti->In will be set in _CfrTil_StrTok
        sti->Delimiters = delimiters ;
        sti->CharSet0 = CharSet_New ( delimiters, TEMPORARY ) ;
        CharSet_SetChar ( sti->CharSet0, '"' ) ; // always add a '"' as a delimiter
        sti->Out = Buffer_Data ( _CfrTil_->StringMacroB ) ;
        SetState ( sti, STI_INITIALIZED, true ) ;
    }
    else SetState ( sti, STI_INITIALIZED, false ) ;
}

// _CfrTil_StringMacros_Do ::
// get first string delimited by the initialized Delimiters variable in 'buffer' variable, find its macro 
// in the initialized Namespace and substitute/insert it into the original string - 'buffer' : (in place - 
// string must have room for expansion) 

byte *
_CfrTil_StringMacros_Do ( byte * buffer ) // buffer :: the string to which we apply any set macros also cf. .init.cft beginning for how to initialize 
{
    StrTokInfo * sti = & _CfrTil_->Sti ;
    if ( _CfrTil_StrTok ( buffer ) ) // ==> sti->Out :: get first string delimited by the initialized Delimiters variable, find its macro and substitute/insert it in the string
    {
        byte * nstr = StringMacro_Run ( sti->SMNamespace, sti->Out ) ; // sti->Out is the macro pre-expanded string, the arg to the macro function if it exists in SMNamespace
        if ( nstr ) String_InsertDataIntoStringSlot ( buffer, sti->StartIndex, sti->EndIndex, nstr ) ; // use the original buffer for the total result of the macro
    }
}

byte *
_String_GetStringToEndOfLine ( )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    byte * str = String_New ( & rl->InputLine [rl->ReadIndex], TEMPORARY ) ;
    ReadLiner_CommentToEndOfLine ( rl ) ;
    SetState ( _Context_->Lexer0, LEXER_DONE, true ) ;
    return str ;
}

#if 0 // some future possibly useful string functions
// returns end : an offset from 0 from which a strtok for a possible next token can be undertaken
// token found is in caller's buffer arg

int32
_StrTok ( byte * str0, byte * buffer, byte * cset )
{
    int i, start, end ;
    byte *str1, *str2 ;
    // find start of non-delimiter text
    // str0 is the original string
    for ( i = 0, str1 = str0 ; *str1 ; str1 ++, i ++ )
    {
        if ( ! _CharSet_IsDelimiter ( cset, *str1 ) ) break ;
    }
    start = i ;
    // str1 is the start of our search string - to find the first "delimiter"
    // find first string <== the first delimiter after start of non-delimiter text
    for ( i = 0, str2 = str1 ; *str2 ; str2 ++, i ++ )
    {
        if ( _CharSet_IsDelimiter ( cset, *str2 ) ) break ;
        buffer [ i ] = * str2 ;
    }
    buffer [ i ] = 0 ;
    // str2 is either a delimiter or a null, which is also a delimiter. Add 1 for start of a possible next token ...
    if ( *str2 ) // check if it's just a null, the end of string - 0
    {
        // remember buffer is sti->Out, the arg to the macro function, the pre-expanded string, if that macro function exists in SMNamespace
        end = start + i ; // end is an offset from 0 from which a strtok for a possible next token can be undertaken
    }
    else end = 0 ;
    return end ;
}

byte *
String_GetDelimitedString ( byte * str0, byte delimiter )
{
    int32 i ;
    byte * str = String_New ( str0, TEMPORARY ) ;
    for ( i = 0 ; str [i] ; i ++ )
    {
        if ( str [i] == delimiter )
        {
            str [i] = 0 ;
            return str ;
        }
    }
}

int32
_String_CountTabs ( byte * start, byte * end )
{
    int32 n ;
    for ( n = 0 ; start != end ; start ++ )
    {
        if ( *start == '\t' ) n ++ ;
    }
    return n ;
}
#endif

void
_Buffer_Clear ( Buffer * b )
{
    Mem_Clear ( b->B_Data, b->B_Size ) ;
}

byte *
Buffer_Clear ( Buffer * b )
{
    Mem_Clear ( b->B_Data, b->B_Size ) ;
    return Buffer_Data ( b ) ; //b->B_Data ;
}

Buffer *
_Buffer_New ( int32 size, int32 flag )
{
    dlnode * node, * nextNode ;
    Buffer * b ;
    if ( _Q_ && _Q_->MemorySpace0 )
    {
        for ( node = dllist_First ( ( dllist* ) _Q_->MemorySpace0->BufferList ) ; node ; node = nextNode )
        {
            nextNode = dlnode_Next ( node ) ;
            b = ( Buffer* ) node ;
            //printf ( "Recycled buffer = 0x%08x\n", (uint) b ) ; fflush ( stdout ) ;
            if ( ( b->InUseFlag == false ) && ( b->B_Size >= size ) ) goto done ;
            if ( b->InUseFlag == B_PERMANENT ) break ;
        }
    }
    b = ( Buffer * ) Mem_Allocate ( sizeof ( Buffer ) + size + 1, BUFFER ) ;
    //printf ( "Allocated buffer = 0x%08x\n", (uint) b ) ; fflush ( stdout ) ;
    b->B_CProperty = BUFFER ;
    b->B_Size = size ;
    b->B_Data = ( byte* ) b + sizeof (Buffer ) ;
    if ( flag == B_PERMANENT ) dllist_AddNodeToTail ( _Q_->MemorySpace0->BufferList, ( dlnode* ) b ) ;
    else dllist_AddNodeToHead ( _Q_->MemorySpace0->BufferList, ( dlnode* ) b ) ;
done:
    Mem_Clear ( b->B_Data, size ) ;
    b->InUseFlag = flag ;
    return b ;
}

// set all non-permanent buffers as unused - available

void
Buffer_SetAsUnused ( Buffer * b )
{
    if ( b->InUseFlag == true )
    {
        _Buffer_SetAsUnused ( b ) ; // must check ; others may be permanent or locked ( true + 1, true + 2) .
    }
}

void
Buffers_SetAsUnused ( )
{
    dlnode * node, * nextNode ;
    if ( _Q_ && _Q_->MemorySpace0 )
    {
        for ( node = dllist_First ( ( dllist* ) _Q_->MemorySpace0->BufferList ) ; node ; node = nextNode )
        {
            nextNode = dlnode_Next ( node ) ;
            Buffer_SetAsUnused ( ( Buffer* ) node ) ;
        }
    }
}

Buffer *
Buffer_New ( int32 size )
{
    return _Buffer_New ( size, true ) ;
}

Buffer *
Buffer_NewLocked ( int32 size )
{
    return _Buffer_New ( size, true + 1 ) ;
}

Buffer *
_Buffer_NewPermanent ( int32 size )
{
    return _Buffer_New ( size, true + 2 ) ;
}

byte *
Buffer_New_pbyte ( int32 size )
{
    Buffer *b = Buffer_New ( size ) ;
    return Buffer_Data ( b ) ;
}


