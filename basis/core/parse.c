#include "../../includes/cfrtil.h"

// assuming we are using "Class" namespace
// syntax : ':{' ( classId identifer ( '[' integer ']' )* ';' ? )* '};'

void
_CfrTil_Parse_ClassStructure ( int32 cloneFlag )
{
    int32 size = 0, offset = 0, sizeOf = 0, i, arrayDimensionSize ;
    Namespace *ns, *inNs = _CfrTil_Namespace_InNamespaceGet ( ), *arrayBaseObject ;
    byte * token ;
    int32 arrayDimensions [ 32 ] ;
    memset ( arrayDimensions, 0, sizeof (arrayDimensions ) ) ;
    if ( cloneFlag )
    {
        offset = _Namespace_VariableValueGet ( inNs, ( byte* ) "size" ) ; // allows for cloning - prototyping
        sizeOf = offset ;
    }
    while ( 1 )
    {
        // each name/word is an increasing offset from object address on stack
        // first name is at 0 offset
        // token = Lexer_NextToken ( _Context_->Lexer0 ) ;
        _CfrTil_Namespace_InNamespaceSet ( inNs ) ; // parsing arrays changes namespace so reset it here
        token = _Lexer_ReadToken ( _Context_->Lexer0, ( byte* ) " ,\n\r\t" ) ;
gotNextToken:
        if ( String_Equal ( ( char* ) token, "};" ) ) break ;
        if ( ( String_Equal ( ( char* ) token, "}" ) ) && GetState ( _Context_, C_SYNTAX ) )
        {
            CfrTil_PropertydefStructEnd ( ) ;
            break ;
        }
        if ( String_Equal ( ( char* ) token, ";" ) ) continue ;
        if ( String_Equal ( ( char* ) token, "//" ) )
        {
            ReadLiner_CommentToEndOfLine ( _Context_->ReadLiner0 ) ;
            continue ;
        }
        ns = _Namespace_Find ( token, 0, 0 ) ;
        if ( ! ns ) CfrTil_Exception ( NAMESPACE_ERROR, 1 ) ;
        size = _Namespace_VariableValueGet ( ns, ( byte* ) "size" ) ;
        if ( ns && size )
        {
            token = Lexer_ReadToken ( _Context_->Lexer0 ) ;
            arrayBaseObject = _CfrTil_ClassField_New ( token, ns, size, offset ) ; // nb! : in case there is an array so it will be there for ArrayDimensions
            token = Lexer_PeekNextNonDebugTokenWord ( _Context_->Lexer0 ) ;
            if ( token [0] != '[' )
            {
                offset += size ;
                sizeOf += size ;
                continue ;
            }
        }
        else CfrTil_Exception ( NAMESPACE_ERROR, 1 ) ; // else structure component size error
        for ( i = 0 ; 1 ; )
        {
            token = Lexer_ReadToken ( _Context_->Lexer0 ) ;
            if ( token && String_Equal ( ( char* ) token, "[" ) )
            {
                CfrTil_InterpretNextToken ( ) ; // next token must be an integer for the array dimension size
                arrayDimensionSize = _DataStack_Pop ( ) ;
                size = size * arrayDimensionSize ;
                offset += size ;
                sizeOf += size ;
                token = Lexer_ReadToken ( _Context_->Lexer0 ) ;
                arrayDimensions [ i ] = arrayDimensionSize ;
                if ( ! String_Equal ( ( char* ) token, "]" ) ) CfrTil_Exception ( SYNTAX_ERROR, 1 ) ;
                i ++ ;
            }
            else
            {
                if ( i )
                {
                    //arrayBaseObject->CProperty |= VARIABLE ;
                    arrayBaseObject->ArrayDimensions = ( int32 * ) Mem_Allocate ( i * sizeof (int32 ), DICTIONARY ) ;
                    memcpy ( arrayBaseObject->ArrayDimensions, arrayDimensions, i * sizeof (int32 ) ) ;
                }
                if ( token ) goto gotNextToken ;
                else break ;
            }
        }
    }
    _Namespace_VariableValueSet ( inNs, ( byte* ) "size", sizeOf ) ;
}

void
Compile_InitRegisterVariables ( Compiler * compiler )
{
    int32 initVars = compiler->NumberOfRegisterVariables - compiler->NumberOfLocals ; // we only initialize, from the incoming stack, the stack variables
    int32 regOrder [ 4 ] = { EBX, ECX, EDX, EAX }, fpIndex = - 1, regIndex = 0 ; // -1 : cf locals.c 
    for ( ; initVars -- > 0 ; regIndex ++, fpIndex -- )
    {
        if ( GetState ( compiler, RETURN_TOS | RETURN_EAX ) ) _Compile_Move_StackN_To_Reg ( regOrder [ regIndex ], DSP, 0 ) ;
        else _Compile_Move_StackN_To_Reg ( regOrder [ regIndex ], FP, fpIndex ) ; //

    }
}
// old docs :
// parse local variable notation to a temporary "_locals_" namespace
// calculate necessary frame size
// the stack frame (Fsp) will be just above TOS -- at higer addresses
// save entry Dsp in a CfrTil variable (or at Fsp [ 0 ]). Dsp will be reset to just
// above the framestack during duration of the function and at the end of the function
// will be reset to its original value on entry stored in the CfrTil variable (Fsp [ 0 ])
// so that DataStack pushes and pops during the function will be accessing above the top of the new Fsp
// initialize the words to access a slot in the framesize so that the
// compiler can use the slot number in the function being compiled
// compile a local variable such that when used at runtime it pushes
// the slot address on the DataStack

Namespace *
_CfrTil_Parse_LocalsAndStackVariables ( int32 svf, int32 lispMode, ListObject * args ) // stack variables flag
{
    // number of stack variables, number of locals, stack variable flag
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    Lexer * lexer = cntx->Lexer0 ;
    Finder * finder = cntx->Finder0 ;
    byte * svDelimiters = lexer->TokenDelimiters ;
    Word * word ;
    int64 ctype ;
    int32 svff = 0, addWords, getReturn = 0, getReturnFlag = 0, regToUse = 0 ;
    Boolean regFlag = false ;
    int32 regOrder [ 4 ] = { EBX, ECX, EDX, EAX }, regIndex = 0 ;
    byte *token, *returnVariable = 0 ;
    Namespace *typeNamespace = 0, *saveInNs = _Q_->OVT_CfrTil->InNamespace, *localsNs = Namespace_FindOrNew_Local ( ) ;

    if ( svf ) svff = 1 ;
    addWords = 1 ;
    if ( lispMode ) args = ( ListObject * ) args->Lo_List->head ;
    uint64 ltype = lispMode ? T_LISP_SYMBOL : 0 ;
    while ( ( lispMode ? ( int32 ) _LO_Next ( args ) : 1 ) )
    {
        if ( lispMode )
        {
            args = _LO_Next ( args ) ;
            if ( args->LProperty & ( LIST | LIST_NODE ) ) args = _LO_First ( args ) ;
            token = ( byte* ) args->Lo_Name ;
        }
        else token = _Lexer_ReadToken ( lexer, ( byte* ) " ,\n\r\t" ) ;
        if ( String_Equal ( token, "(" ) ) continue ;
        word = Finder_Word_FindUsing ( finder, token, 1 ) ; // ?? find after Literal - eliminate making strings or numbers words ??
        if ( word && ( word->CProperty & ( NAMESPACE | CLASS ) ) && ( CharTable_IsCharType ( ReadLine_PeekNextChar ( lexer->ReadLiner0 ), CHAR_ALPHA ) ) )
        {
            typeNamespace = word ;
            continue ;
        }
        if ( strcmp ( ( char* ) token, "|" ) == 0 )
        {
            svff = 0 ; // set stack variable flag to off -- no more stack variables ; begin local variables
            continue ; // don't add a node to our temporary list for this token
        }
        if ( strcmp ( ( char* ) token, "--" ) == 0 ) // || ( strcmp ( ( char* ) token, "|-" ) == 0 ) || ( strcmp ( ( char* ) token, "|--" ) == 0 ) )
        {
            if ( ! svf ) break ;
            else
            {
                addWords = 0 ;
                getReturnFlag = 1 ;
                continue ;
            }
        }
        if ( strcmp ( ( char* ) token, ")" ) == 0 )
        {
            break ;
        }
        if ( strcmp ( ( char* ) token, "REG:" ) == 0 )
        {
            regFlag = true ;
            continue ;
        }
        if ( strcmp ( ( char* ) token, "EAX:" ) == 0 )
        {
            regFlag = true ;
            regToUse = 3 ;
            continue ;
        }
        else if ( strcmp ( ( char* ) token, "ECX:" ) == 0 )
        {
            regFlag = true ;
            regToUse = 1 ;
            continue ;
        }
        else if ( strcmp ( ( char* ) token, "EDX:" ) == 0 )
        {
            regFlag = true ;
            regToUse = 2 ;
            continue ;
        }
        else if ( strcmp ( ( char* ) token, "EBX:" ) == 0 )
        {
            regFlag = true ;
            regToUse = 0 ;
            continue ;
        }
        if ( ( strcmp ( ( char* ) token, "{" ) == 0 ) || ( strcmp ( ( char* ) token, ";" ) == 0 ) )
        {
            Printf ( ( byte* ) "\nLocal variables syntax error : no close parenthesis ')' found" ) ;
            CfrTil_Exception ( SYNTAX_ERROR, 1 ) ;
        }
        if ( getReturnFlag )
        {
            addWords = 0 ;
            if ( stricmp ( token, ( byte* ) "EAX" ) == 0 ) getReturn = RETURN_EAX ;
            else if ( stricmp ( token, ( byte* ) "TOS" ) == 0 ) getReturn = RETURN_TOS ;
            else if ( stricmp ( token, ( byte* ) "0" ) == 0 ) getReturn = DONT_REMOVE_STACK_VARIABLES ;
            else returnVariable = token ;
            continue ;
        }
        if ( addWords )
        {
            if ( svff )
            {
                compiler->NumberOfParameterVariables ++ ;
                ctype = PARAMETER_VARIABLE ;
                if ( lispMode ) ctype |= T_LISP_SYMBOL ;
            }
            else
            {
                compiler->NumberOfLocals ++ ;
                ctype = LOCAL_VARIABLE ;
                if ( lispMode ) ctype |= T_LISP_SYMBOL ;
            }
            if ( regFlag == true )
            {
                ctype |= REGISTER_VARIABLE ;
                compiler->NumberOfRegisterVariables ++ ;
            }
            //DebugShow_OFF ;
            word = _DataObject_New ( ctype, 0, token, ctype, ltype, ( ctype & LOCAL_VARIABLE ) ? compiler->NumberOfLocals : compiler->NumberOfParameterVariables, 0, cntx->Lexer0->TokenStart_ReadLineIndex ) ;
            //DebugShow_ON ;
            if ( regFlag == true )
            {
                if ( regToUse )
                {
                    word->RegToUse = regOrder [ regToUse ] ;
                    regToUse = 0 ;
                }
                else
                {
                    word->RegToUse = regOrder [ regIndex ++ ] ;
                    if ( regIndex == 3 ) regIndex = 0 ;
                }
            }
            regFlag = false ;
            if ( typeNamespace )
            {
                word->TypeNamespace = typeNamespace ;
                word->CProperty |= OBJECT ;
            }
            typeNamespace = 0 ;
            if ( String_Equal ( token, "this" ) ) word->CProperty |= THIS ;
        }
    }
    compiler->State |= getReturn ;

    // we support nested locals and may have locals in other blocks so the indexes are cumulative
    if ( compiler->NumberOfRegisterVariables ) Compile_InitRegisterVariables ( compiler ) ;
    if ( returnVariable ) compiler->ReturnVariableWord = Word_FindInOneNamespace ( localsNs, returnVariable ) ;

    _Q_->OVT_CfrTil->InNamespace = saveInNs ;
    List_Init ( compiler->WordList ) ;
    finder->w_Word = 0 ;
    Lexer_SetTokenDelimiters ( lexer, svDelimiters, SESSION ) ;
    SetState ( compiler, VARIABLE_FRAME, true ) ;
    cntx->CurrentRunWord->NumberOfArgs = compiler->NumberOfLocals ;
    return localsNs ;
}

void
_Lexer_ParseAsAString ( Lexer * lexer, uint32 allocType )
{
    byte *s0 ;
    if ( ! ( s0 = _String_UnBox ( lexer->OriginalToken, allocType ) ) )
    {
        SetState ( lexer, KNOWN_OBJECT, false ) ;
        return ;
    }
    lexer->LiteralString = s0 ;
    if ( lexer->OriginalToken [ 0 ] == '"' ) lexer->TokenType = T_STRING ;
    else lexer->TokenType = T_RAW_STRING ;
    SetState ( lexer, KNOWN_OBJECT, true ) ;
}

void
Lexer_ParseString ( Lexer * lexer )
{
    _Lexer_ParseAsAString ( lexer, 0 ) ;
}

void
_Lexer_ParseBinary ( Lexer * lexer, int offset )
{
    byte * token = & lexer->OriginalToken [offset] ;
    int32 cc = 0, i, l = strlen ( ( char* ) token ) ; // 8 bits/byte
    byte current ;
    for ( i = 0 ; i < l ; i ++ )
    {
        current = token [ l - i - 1 ] ; // 1 : remember zero based array indexing
        if ( current == '1' )
            cc += ( 1 << i ) ;
        else if ( current == '0' )
            continue ;
        else if ( current == ' ' )
            continue ;
        else
        {
            SetState ( lexer, KNOWN_OBJECT, false ) ;
        }
    }
    lexer->Literal = cc ;
    SetState ( lexer, KNOWN_OBJECT, true ) ;
}

void
Lexer_ParseBinary ( Lexer * lexer, byte * token, uint32 allocType, int32 offset )
{
    _Lexer_ParseBinary ( lexer, offset ) ;
    if ( GetState ( lexer, KNOWN_OBJECT ) )
    {
        lexer->TokenType = T_INT ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else _Lexer_ParseAsAString ( lexer, allocType ) ;
}

void
Lexer_ParseBigNum ( Lexer * lexer, byte * token )
{
    _CfrTil_Namespace_InNamespaceGet ( ) ;
    {
        byte * name = _CfrTil_InNamespace ( )->Name ;
        if ( String_Equal ( ( char* ) name, "BigInt" ) )
        {
            mpz_t *bi = ( mpz_t* ) _BigInt_New ( 0 ) ;
            if ( token )
            {
                gmp_sscanf ( ( char* ) token, "%Zd", *bi ) ;
            }
            lexer->Literal = ( int32 ) bi ;
            lexer->TokenType = T_BIG_INT ;
            SetState ( lexer, KNOWN_OBJECT, true ) ;
        }
        else if ( String_Equal ( ( char* ) name, "BigFloat" ) )
        {
            mpf_t *bf = ( mpf_t* ) _BigFloat_New ( 0 ) ;
            if ( token )
            {

                gmp_sscanf ( ( char* ) token, "%Fd", *bf ) ;
            }
            lexer->Literal = ( int32 ) bf ;
            lexer->TokenType = T_BIG_FLOAT ;
            SetState ( lexer, KNOWN_OBJECT, true ) ;
        }
    }
}
// return boolean 0 or 1 if lexer->Literal value is pushed

void
_Lexer_ParseHex ( Lexer * lexer, byte * token, uint32 allocType )
{
#if 0    
    if ( sscanf ( ( char* ) token, "%llx", ( unsigned long long int* ) &lexer->Literal ) )
    {
        lexer->TokenType = T_INT ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    //else 
#endif    
    if ( sscanf ( ( char* ) token, HEX_INT_FRMT, ( unsigned int* ) &lexer->Literal ) )
    {
        lexer->TokenType = T_INT ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( sscanf ( ( char* ) token, HEX_UINT_FRMT, ( unsigned int* ) &lexer->Literal ) )
    {
        lexer->TokenType = T_INT ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( sscanf ( ( char* ) token, LISP_HEX_FRMT, ( unsigned int* ) &lexer->Literal ) )
    {
        lexer->TokenType = T_INT ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else _Lexer_ParseAsAString ( lexer, allocType ) ;
}

void
_Lexer_ParseDecimal ( Lexer * lexer, byte * token, uint32 allocType )
{
    float f ;
    if ( sscanf ( ( char* ) token, INT_FRMT, ( int* ) &lexer->Literal ) )
    {
        lexer->TokenType = T_INT ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( sscanf ( ( char* ) token, LISP_DECIMAL_FRMT, ( int* ) &lexer->Literal ) )
    {
        lexer->TokenType = T_INT ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( sscanf ( ( char* ) token, "%f", &f ) )
    {
        lexer->TokenType = T_FLOAT ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        return Lexer_ParseBigNum ( lexer, token ) ;
    }
    else _Lexer_ParseAsAString ( lexer, allocType ) ;
}

void
_Lexer_Parse ( Lexer * lexer, byte * token, uint32 allocType )
{
    Context * cntx = _Context_ ;
    int32 offset = 0 ;
    lexer->OriginalToken = token ;
    lexer->Literal = 0 ;
    if ( token )
    {
        if ( ( token [0] == '0' ) || ( token [0] == '#' ) ) // following scheme notation
        {
            char c ;
            if ( ( c = tolower ( token [1] ) ) == 'x' )
            {
                token [1] = c ;
                if ( token [0] == '#' ) token [0] = '0' ; // Scheme format to C format
                _Lexer_ParseHex ( lexer, token[0] == '#' ? &token[1] : token, allocType ) ; // #x
                return ;
            }
            else if ( ( c = tolower ( token [1] ) ) == 'b' )
            {
                if ( token [0] == '#' ) // following scheme notation
                {
                    offset = 2 ;
                    Lexer_ParseBinary ( lexer, token, offset, allocType ) ; // #b
                    return ;
                }
            }
            else if ( tolower ( token [1] ) == 'd' )
            {
                _Lexer_ParseDecimal ( lexer, token, allocType ) ; // #d
                return ;
            }
            //else if ( tolower ( token [1] ) == 'o' ) goto doOctal ; // #o
        }
        if ( cntx->System0->NumberBase == 10 ) _Lexer_ParseDecimal ( lexer, token, allocType ) ;
        else if ( cntx->System0->NumberBase == 2 ) Lexer_ParseBinary ( lexer, token, allocType, 0 ) ;
        else if ( cntx->System0->NumberBase == 16 ) _Lexer_ParseHex ( lexer, token, allocType ) ;
    }
}

void
Lexer_ParseObject ( Lexer * lexer, byte * token )
{
    _Lexer_Parse ( lexer, token, SESSION ) ;
}

byte *
Parse_Macro ( int64 type )
{
    byte * value ;
    Lexer * lexer = _Context_->Lexer0 ;
    if ( type == STRING_MACRO )
    {
        value = Lexer_ReadToken ( lexer ) ;
        while ( strcmp ( ";", ( char* ) Lexer_ReadToken ( lexer ) ) ) ; // nb. we take only the first string all else ignored
    }
    else if ( type == TEXT_MACRO )
    {
        int n = 0 ;
        Buffer * b = Buffer_New ( BUFFER_SIZE ) ;
        byte nc, *buffer = Buffer_Data ( b ) ;
        buffer [0] = 0 ;
        do
        {
            nc = _ReadLine_GetNextChar ( _Context_->ReadLiner0 ) ;
            //_Lexer_AppendCharToSourceCode ( lexer, nc ) ;
            if ( nc == ';' )
            {
                buffer [ n ] = 0 ;
                break ;
            }
            buffer [ n ++ ] = nc ;
        }
        while ( nc ) ;
        value = String_New ( ( byte* ) buffer, TEMPORARY ) ;
        Buffer_SetAsUnused ( b ) ;
    }
    return value ;
}
