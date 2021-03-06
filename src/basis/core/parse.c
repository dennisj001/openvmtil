#include "../../include/cfrtil32.h"

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
            CfrTil_TypedefStructEnd ( ) ;
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
            token = Lexer_PeekNextNonDebugTokenWord ( _Context_->Lexer0, 1 ) ;
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
Compile_InitRegisterParamenterVariables ( Compiler * compiler )
{
    int32 regIndex, nRVars = compiler->NumberOfRegisterVariables, nPVars = compiler->NumberOfArgs ;
    for ( regIndex = 0 ; nRVars -- > 0 && nPVars -- > 0 ; regIndex ++ )
    {
        //if ( GetState ( compiler, RETURN_TOS | RETURN_EAX ) ) 
        _Compile_Move_StackN_To_Reg ( compiler->RegOrder [ regIndex ], DSP, regIndex * CELL ) ;
        //else _Compile_Move_StackN_To_Reg ( regOrder [ regIndex ], FP, fpIndex ) ; //
    }
}

void
Compiler_TypedObjectInit ( Namespace * typeNamespace, Word * word )
{
    word->TypeNamespace = typeNamespace ;
    word->CProperty |= typeNamespace->CProperty ;
    if ( typeNamespace->CProperty & CLASS ) word->CProperty |= OBJECT ;
    word->LProperty |= LOCAL_OBJECT ;
    //_DObject_Init ( Word * word, uint32 value, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs )
    _DObject_Init ( word, ( int32 ) 0, LOCAL_OBJECT, ( byte* ) _DataObject_Run, 0, 1, 0 ) ;
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
_CfrTil_Parse_LocalsAndStackVariables ( int32 svf, int32 lispMode, ListObject * args, Stack * nsStack, Namespace * localsNs ) // stack variables flag
{
    // number of stack variables, number of locals, stack variable flag
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    Lexer * lexer = cntx->Lexer0 ;
    Finder * finder = cntx->Finder0 ;
    int32 scm = IsSourceCodeOn ;
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, false ) ;
    byte * svDelimiters = lexer->TokenDelimiters ;
    Word * word ;
    int64 ctype ;
    int32 svff = 0, addWords, getReturn = 0, getReturnFlag = 0, regToUseIndex = 0 ;
    Boolean regFlag = false ;
    byte *token, *returnVariable = 0 ;
    Namespace *typeNamespace = 0, *saveInNs = _CfrTil_->InNamespace ;
    if ( ! localsNs ) localsNs = _Namespace_FindOrNew_Local ( nsStack ? nsStack : compiler->LocalsNamespacesStack ) ;
    //Namespace *localsNs = forceNewLocalsFlag ? _DataObject_New ( NAMESPACE, 0, ( byte* ) "tmpLocals", 0, 0, 0, ( int32 ) 0, 0 ) : Namespace_FindOrNew_Local ( ) ;
    //if ( forceNewLocalsFlag ) _Namespace_ActivateAsPrimary ( localsNs ) ;

    if ( svf ) svff = 1 ;
    addWords = 1 ;
    if ( lispMode ) args = ( ListObject * ) args->Lo_List->head ;

    while ( ( lispMode ? ( int32 ) _LO_Next ( args ) : 1 ) )
    {
        if ( lispMode )
        {
            args = _LO_Next ( args ) ;
            if ( args->LProperty & ( LIST | LIST_NODE ) ) args = _LO_First ( args ) ;
            token = ( byte* ) args->Lo_Name ;
            CfrTil_AddStringToSourceCode ( _CfrTil_, token ) ;
        }
        else token = _Lexer_ReadToken ( lexer, ( byte* ) " ,\n\r\t" ) ;
        if ( token )
        {
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
                regToUseIndex = 3 ;
                continue ;
            }
            else if ( strcmp ( ( char* ) token, "ECX:" ) == 0 )
            {
                regFlag = true ;
                regToUseIndex = 1 ;
                continue ;
            }
            else if ( strcmp ( ( char* ) token, "EDX:" ) == 0 )
            {
                regFlag = true ;
                regToUseIndex = 2 ;
                continue ;
            }
            else if ( strcmp ( ( char* ) token, "EBX:" ) == 0 )
            {
                regFlag = true ;
                regToUseIndex = 0 ;
                continue ;
            }
            if ( ( strcmp ( ( char* ) token, "{" ) == 0 ) || ( strcmp ( ( char* ) token, ";" ) == 0 ) )
            {
                _Printf ( ( byte* ) "\nLocal variables syntax error : no close parenthesis ')' found" ) ;
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
                    //compiler->NumberOfArgs ++ ;
                    ctype = PARAMETER_VARIABLE ;
                    if ( lispMode ) ctype |= T_LISP_SYMBOL ;
                }
                else
                {
                    //compiler->NumberOfLocals ++ ;
                    ctype = LOCAL_VARIABLE ;
                    if ( lispMode ) ctype |= T_LISP_SYMBOL ;
                }
                if ( regFlag == true )
                {
                    ctype |= REGISTER_VARIABLE ;
                    compiler->NumberOfRegisterVariables ++ ;
                }
                word = _CfrTil_LocalWord ( token, ( ctype & LOCAL_VARIABLE ) ? ++ compiler->NumberOfLocals : ++ compiler->NumberOfArgs, ctype ) ; // svf : flag - whether stack variables are in the frame
                if ( regFlag == true )
                {
                    word->RegToUse = compiler->RegOrder [ regToUseIndex ++ ] ;
                }
                regFlag = false ;
                if ( typeNamespace )
                {
                    Compiler_TypedObjectInit ( typeNamespace, word ) ;
                    //SetState ( word, W_INITIALIZED, false ) ; // shouldn't be necessary
                }
                typeNamespace = 0 ;
                if ( String_Equal ( token, "this" ) ) word->CProperty |= THIS ;
            }
        }
        else return 0 ; // Syntax Error or no local or parameter variables
    }
    compiler->State |= getReturn ;

    // we support nested locals and may have locals in other blocks so the indexes are cumulative
    if ( compiler->NumberOfRegisterVariables ) Compile_InitRegisterParamenterVariables ( compiler ) ;
    if ( returnVariable ) compiler->ReturnVariableWord = Finder_FindWord_InOneNamespace ( _Finder_, localsNs, returnVariable ) ;

    _CfrTil_->InNamespace = saveInNs ;
    //Compiler_WordList_RecycleInit ( compiler ) ;
    finder->FoundWord = 0 ;
    Lexer_SetTokenDelimiters ( lexer, svDelimiters, COMPILER_TEMP ) ;
    SetState ( compiler, VARIABLE_FRAME, true ) ;
    //cntx->CurrentlyRunningWord->W_NumberOfArgs = compiler->NumberOfArgs ;
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, scm ) ;
    return localsNs ;
}

void
Lexer_ParseAsAString ( Lexer * lexer )
{
    if ( lexer->OriginalToken [ 0 ] == '"' )
    {
        lexer->TokenType = ( T_STRING | KNOWN_OBJECT ) ;
        lexer->LiteralString = _String_UnBox ( lexer->OriginalToken ) ;
    }
    else if ( lexer->OriginalToken [ 0 ] == '\'' )
    {
        //char buffer [4] ; buffer[0]= '\'' ; buffer[1]= lexer->OriginalToken [ 1 ] ; buffer[2]= '\'' ; buffer[3]= 0 ;
        lexer->TokenType = ( T_CHAR | KNOWN_OBJECT ) ;
        lexer->Literal = ( int32 ) lexer->OriginalToken [ 1 ] ; //buffer  ;
    }
    else
    {
        lexer->TokenType = ( T_RAW_STRING | KNOWN_OBJECT ) ;
        lexer->LiteralString = lexer->OriginalToken ;
    }
    SetState ( lexer, KNOWN_OBJECT, true ) ;
}

void
_Lexer_ParseBinary ( Lexer * lexer, int offset )
{
    byte * token = & lexer->OriginalToken [offset] ;
    int32 cc = 0, i, l = Strlen ( ( char* ) token ) ; // 8 bits/byte
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
Lexer_ParseBinary ( Lexer * lexer, byte * token, int32 offset )
{
    _Lexer_ParseBinary ( lexer, offset ) ;
    if ( GetState ( lexer, KNOWN_OBJECT ) )
    {
        lexer->TokenType = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else Lexer_ParseAsAString ( lexer ) ;
}

void
Lexer_ParseBigNum ( Lexer * lexer, byte * token )
{
    if ( Namespace_IsUsing ( "BigNum" ) ) //String_Equal ( ( char* ) name, "BigNum" ) )
    {
        mpfr_t *bfr = ( mpfr_t* ) _BigNum_New ( token ) ;
        lexer->Literal = ( int32 ) bfr ;
        lexer->TokenType = ( T_BIG_NUM | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
    }
}
// return boolean 0 or 1 if lexer->Literal value is pushed

void
_Lexer_ParseHex ( Lexer * lexer, byte * token )
{
#if 0    
    if ( sscanf ( ( char* ) token, "%llx", ( unsigned long long int* ) &lexer->Literal ) )
    {
        lexer->TokenType = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    //else 
#endif    
    if ( sscanf ( ( char* ) token, HEX_INT_FRMT, ( unsigned int* ) &lexer->Literal ) )
    {
        lexer->TokenType = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( sscanf ( ( char* ) token, HEX_UINT_FRMT, ( unsigned int* ) &lexer->Literal ) )
    {
        lexer->TokenType = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( sscanf ( ( char* ) token, LISP_HEX_FRMT, ( unsigned int* ) &lexer->Literal ) )
    {
        lexer->TokenType = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else Lexer_ParseAsAString ( lexer ) ;
}

void
_Lexer_ParseDecimal ( Lexer * lexer, byte * token )
{
    float f ;
    if ( sscanf ( ( char* ) token, INT_FRMT, ( int* ) &lexer->Literal ) )
    {
        lexer->TokenType = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( sscanf ( ( char* ) token, LISP_DECIMAL_FRMT, ( int* ) &lexer->Literal ) )
    {
        lexer->TokenType = ( T_INT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        Lexer_ParseBigNum ( lexer, token ) ;
    }
    else if ( sscanf ( ( char* ) token, "%f", &f ) )
    {
        lexer->TokenType = ( T_FLOAT | KNOWN_OBJECT ) ;
        SetState ( lexer, KNOWN_OBJECT, true ) ;
        return Lexer_ParseBigNum ( lexer, token ) ;
    }
    else Lexer_ParseAsAString ( lexer ) ;
}

void
Lexer_ParseObject ( Lexer * lexer, byte * token )
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
                _Lexer_ParseHex ( lexer, token[0] == '#' ? &token[1] : token ) ; // #x
                return ;
            }
            else if ( ( c = tolower ( token [1] ) ) == 'b' )
            {
                if ( token [0] == '#' ) // following scheme notation
                {
                    offset = 2 ;
                    Lexer_ParseBinary ( lexer, token, offset ) ; // #b
                    return ;
                }
            }
            else if ( tolower ( token [1] ) == 'd' )
            {
                _Lexer_ParseDecimal ( lexer, token ) ; // #d
                return ;
            }
            //else if ( tolower ( token [1] ) == 'o' ) goto doOctal ; // #o
        }
        if ( cntx->System0->NumberBase == 10 ) _Lexer_ParseDecimal ( lexer, token ) ;
        else if ( cntx->System0->NumberBase == 2 ) Lexer_ParseBinary ( lexer, token, 0 ) ;
        else if ( cntx->System0->NumberBase == 16 ) _Lexer_ParseHex ( lexer, token ) ;
    }
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
        //Buffer * b = Buffer_New ( BUFFER_SIZE ) ;
        byte nc, *buffer = Buffer_Data ( _CfrTil_->ScratchB1 ) ;
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
        //Buffer_SetAsUnused ( b ) ;
    }
    return value ;
}



