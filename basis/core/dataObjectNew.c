
#include "../../include/cfrtil.h"

// Dynamic Object New : Word = Namespace = DObject : have a s_Symbol

void
_DObject_C_StartupCompiledWords_DefInit ( byte * function, int32 arg )
{
    if ( arg == - 1 )
    {
        ( ( void (* )( ) )( function ) ) ( ) ;
    }
    else
    {
        ( ( void (* ) ( int ) )( function ) ) ( arg ) ;
    }
}

void
_DObject_ValueDefinition_Init ( Word * word, uint32 value, uint64 funcType, byte * function, int arg )
// using a variable that is a type or a function 
{
    word->W_PtrToValue = & word->W_Value ;
    word->W_Value = value ;
    if ( GetState ( _Context_->Compiler0, LC_ARG_PARSING | PREFIX_ARG_PARSING ) ) word->W_StartCharRlIndex = _Context_->Lexer0->TokenStart_ReadLineIndex ;

    if ( funcType & BLOCK )
    {
        word->Definition = ( block ) ( function ? function : ( byte* ) value ) ; //_OptimizeJumps ( ( byte* ) value ) ; // this comes to play (only(?)) with unoptimized code
        word->CodeStart = ( byte* ) word->Definition ;
        if ( ( word->CodeStart < ( byte* ) _Q_CodeByteArray->BA_Data ) || ( word->CodeStart > ( byte* ) _Q_CodeByteArray->bp_Last ) ) word->S_CodeSize = 0 ; // ?!? not quite accurate
        else word->S_CodeSize = Here - word->CodeStart ; // for use by inline
    }
    else
    {
        ByteArray * svcs = _Q_CodeByteArray ;
        _Compiler_SetCompilingSpace ( ( byte* ) "ObjectSpace" ) ; // same problem as namespace ; this can be called in the middle of compiling another word 
        word->Coding = Here ;
        word->CodeStart = Here ;
        word->Definition = ( block ) Here ;
        if ( arg ) _DObject_C_StartupCompiledWords_DefInit ( function, arg ) ;
        else Compile_Call ( ( byte* ) DataObject_Run ) ;
        _Compile_Return ( ) ;
        word->S_CodeSize = Here - word->CodeStart ; // for use by inline
        Set_CompilerSpace ( svcs ) ;
    }
}

void
_DObject_Finish ( Word * word )
{
    uint64 ctype = word->CProperty ;
    if ( ! ( ctype & CPRIMITIVE ) )
    {
        if ( GetState ( _CfrTil_, OPTIMIZE_ON ) ) word->State |= COMPILED_OPTIMIZED ;
        if ( GetState ( _CfrTil_, INLINE_ON ) ) word->State |= COMPILED_INLINE ;
        if ( GetState ( _Context_, INFIX_MODE ) ) word->State |= W_INFIX_MODE ;
        if ( GetState ( _Context_, C_SYNTAX ) ) word->State |= W_C_SYNTAX ;
        if ( GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) ) word->State |= W_SOURCE_CODE_MODE ;
    }
    if ( GetState ( _Context_, INFIX_MODE ) ) word->CProperty |= INFIX_WORD ;
    word->NumberOfArgs = _Context_->Compiler0->NumberOfParameterVariables ;
    _CfrTil_->LastFinishedWord = word ;
}

Word *
_DObject_Init ( Word * word, uint32 value, uint64 ctype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs )
{
    // remember : Word = Namespace = DObject : each have an s_Symbol
    _DObject_ValueDefinition_Init ( word, value, ftype, function, arg ) ;
    _Word_Add ( word, addToInNs, addToNs ) ;
    _DObject_Finish ( word ) ;
    word->RunType = ftype ;
    CfrTil_Set_DebugSourceCodeIndex ( word, 0 ) ;
    return word ;
}

// DObject : dynamic object
// remember : Word = Namespace = DObject has a s_Symbol

Word *
_DObject_New ( byte * name, uint32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, uint32 allocType )
{
    Word * word = _Word_Create ( name, ctype, ltype, allocType ) ;
    _DObject_Init ( word, value, ctype, ftype, function, arg, addToInNs, addToNs ) ;
    return word ;
}

byte *
_CfrTil_NamelessObjectNew ( int32 size )
{
    byte * obj = 0 ;
    if ( size )
    {
        obj = Mem_Allocate ( size, OBJECT_MEMORY ) ;
    }
    return obj ;
}

void
Class_Object_Init ( Word * word, Namespace * ns )
{
    DebugShow_Off ;
    Stack * nsstack = _Context_->Compiler0->NamespacesStack ;
    Stack_Init ( nsstack ) ; // !! ?? put this in Compiler ?? !!
    // init needs to be done by the most super class first successively down to the current class 
    do
    {
        Word * initWord ;
        if ( ( initWord = Word_FindInOneNamespace ( ns, ( byte* ) "init" ) ) )
        {
            _Stack_Push ( nsstack, ( int32 ) initWord ) ;
        }
        ns = ns->ContainingNamespace ;
    }
    while ( ns ) ;
    int32 i, * svDsp = Dsp ;
    //DebugShow_Off ;
    SetState ( _Debugger_, DEBUG_SHTL_OFF, true ) ;
    for ( i = Stack_Depth ( nsstack ) ; i > 0 ; i -- )
    {
        DSP_Push ( ( int32 ) * word->W_PtrToValue ) ;
        Word * initWord = ( Word* ) _Stack_Pop ( nsstack ) ;
        _Word_Eval ( initWord ) ;
    }
    Dsp = svDsp ; // this seems a little too presumptive -- a finer tuned stack adjust maybe be more correct
    SetState ( _Debugger_, DEBUG_SHTL_OFF, false ) ;
    //DebugShow_StateRestore ;
}

// class object new

Word *
_Class_Object_New ( byte * name, uint64 category )
{
    int32 size ;
    byte * object ;
    Word * word ;
    Namespace * ns = _CfrTil_Namespace_InNamespaceGet ( ) ;
    size = _Namespace_VariableValueGet ( ns, ( byte* ) "size" ) ;
    object = _CfrTil_NamelessObjectNew ( size ) ;
    // _DObject_New ( byte * name, uint32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, uint32 allocType )
    word = _DObject_New ( name, ( int32 ) object, ( OBJECT | IMMEDIATE | category ), 0, OBJECT, ( byte* ) _DataObject_Run, 0, 1, 0, DICTIONARY ) ;
    word->Size = size ;
    Class_Object_Init ( word, ns ) ;
    _Namespace_VariableValueSet ( ns, ( byte* ) "this", ( int32 ) object ) ;
    return word ;
}

Namespace *
_Class_New ( byte * name, uint64 type, int32 cloneFlag )
{
    Namespace * ns = _Namespace_Find ( name, 0, 0 ), * sns ;
    int32 size = 0 ;
    if ( ! ns )
    {
        //if ( type == C_TYPE ) sns = _Namespace_Find ( "C_Syntax", 0, 1 ) ;
        //else 
        sns = _CfrTil_Namespace_InNamespaceGet ( ) ;
        if ( cloneFlag )
        {
            size = _Namespace_VariableValueGet ( sns, ( byte* ) "size" ) ;
        }
        ns = _DObject_New ( name, 0, CPRIMITIVE | CLASS | IMMEDIATE | type, 0, type, ( byte* ) _DataObject_Run, 0, 0, sns, DICTIONARY ) ;
        _Namespace_DoNamespace ( ns, 1 ) ; // before "size", "this"
        Word *ws = _CfrTil_Variable_New ( ( byte* ) "size", size ) ; // start with size of the prototype for clone
        ws->CProperty |= NAMESPACE_VARIABLE ;
        _Context_->Interpreter0->ThisNamespace = ns ;
        //_Class_Object_New ( ( byte* ) "this", THIS | NAMESPACE_VARIABLE ) ;
        Word *wt = _CfrTil_Variable_New ( ( byte* ) "this", size ) ; // start with size of the prototype for clone
        wt->CProperty |= THIS | NAMESPACE_VARIABLE ;
    }
    else
    {
        Printf ( ( byte* ) "\nNamespace Error ? : class \"%s\" already exists!\n", ns->Name ) ;
        _Namespace_DoNamespace ( ns, 1 ) ;
    }
    //Stack_Init ( _Context_->Compiler0->WordStack ) ; // try to keep WordStack to a minimum
    List_Init ( _Context_->Compiler0->WordList ) ; // try to keep WordStack to a minimum
    return ns ;
}

Word *
_CfrTil_ClassField_New ( byte * token, Class * aclass, int32 size, int32 offset )
{
    Word * word = _DObject_New ( token, 0, IMMEDIATE | OBJECT_FIELD, 0, OBJECT_FIELD, ( byte* ) _DataObject_Run, 0, 1, 0, DICTIONARY ) ;
    word->ClassFieldTypeNamespace = aclass ;
    word->Size = size ;
    word->Offset = offset ;
    return word ;
}

// ( <name> value -- )

// remember : this word runs at compile/interpret time ; nothing is compiled yet

void
Class_Value_New ( byte * name )
{
    _DataObject_New ( OBJECT, 0, name, 0, 0, 0, 0, 0 ) ;
}

void
DObject_New ( )
{
    //DObject * proto = Namespace_Find ( ( byte* ) "DObject" ) ;
    DObject_NewClone ( 0 ) ;
}

// this maybe should be in primitives/dobject.c

Word *
_CfrTil_Variable_New ( byte * name, int32 value )
{
    Word * word ;
    if ( CompileMode && ( ! GetState ( _Context_, C_SYNTAX ) ) ) // we're not using this yet but it may be useful to some
    {
        word = _DObject_New ( name, value, ( LOCAL_VARIABLE | IMMEDIATE ), 0, LOCAL_VARIABLE, ( byte* ) _DataObject_Run, 0, 0, 0, DICTIONARY ) ;
        word->Index = ++ _Context_->Compiler0->NumberOfLocals ;
    }
    else word = _DObject_New ( name, value, NAMESPACE_VARIABLE | IMMEDIATE, 0, NAMESPACE_VARIABLE, ( byte* ) _DataObject_Run, 0, 1, 0, DICTIONARY ) ;
    return word ;
}

void
_CfrTil_Label ( byte * lname )
{
    Namespace * ns = Namespace_FindOrNew_SetUsing ( ( byte* ) "__labels__", _CfrTil_->Namespaces, 1 ) ;
    _DObject_New ( lname, ( int32 ) Here, CONSTANT | IMMEDIATE, 0, CONSTANT, ( byte* ) _DataObject_Run, 0, 0, ns, DICTIONARY ) ;
}

Word *
_CfrTil_LocalWord ( byte * name, int32 index, int64 ctype, uint64 ltype ) // svf : flag - whether stack variables are in the frame
{
    Word * word = _DObject_New ( name, 0, ( ctype | IMMEDIATE ), ltype, LOCAL_VARIABLE | PARAMETER_VARIABLE, ( byte* ) _DataObject_Run, 0, 1, 0, SESSION ) ;
    word->Index = index ;
    return word ;
}

Word *
Literal_New ( Lexer * lexer, uint32 uliteral )
{
    // nb.! : remember the compiler optimizes with the WordStack so everything has to be converted to a Word
    // _DObject_New : calls _Do_Literal which pushes the literal on the data stack or compiles a push ...
    Word * word ;
    byte _name [ 256 ], *name ;
    if ( ! GetState ( lexer, T_STRING | T_RAW_STRING | KNOWN_OBJECT ) )
    {
        snprintf ( ( char* ) _name, 256, "<unknown object type> : %x", ( uint ) uliteral ) ;
        name = TemporaryString_New ( _name ) ;
    }
    else
    {
        name = lexer->OriginalToken ;
    }
    word = _DObject_New ( name, ( uint32 ) uliteral, LITERAL | CONSTANT | IMMEDIATE, 0, LITERAL, ( byte* ) _DataObject_Run, 0, 0, 0, DICTIONARY ) ;
    //if ( GetState ( lexer, T_STRING | T_RAW_STRING ) ) word->W_PtrToValue = (uint32*) word->W_PtrValue ;
    return word ;
}

Namespace *
_Namespace_New ( byte * name, Namespace * containingNs )
{
    Namespace * ns = _DObject_New ( name, 0, ( CPRIMITIVE | NAMESPACE ), 0, NAMESPACE, ( byte* ) _DataObject_Run, 0, 0, containingNs, DICTIONARY ) ;
    return ns ;
}

Word *
_DataObject_New ( uint64 type, Word * word, byte * name, uint64 ctype, uint64 ltype, int32 index, int32 value, int32 startCharRlIndex )
{
    if ( startCharRlIndex ) _Context_->Lexer0->TokenStart_ReadLineIndex = startCharRlIndex ;
    switch ( type )
    {
        case T_LC_NEW:
        {
            word = _LO_New ( ltype, ctype, ( byte* ) value, word, LISP_TEMP ) ; // all words are symbols
            break ;
        }
        case T_LC_LITERAL:
        {
            word = _LO_New_RawStringOrLiteral ( _Context_->Lexer0, name, index ) ;
            break ;
        }
        case CFRTIL_WORD:
        {
            word = _DObject_New ( name, value, ctype | BLOCK, ltype | BLOCK, BLOCK, 0, 0, 1, 0, DICTIONARY ) ;
            break ;
        }
        case NAMESPACE:
        {
            word = _Namespace_New ( name, ( Namespace * ) value ) ;
            break ;
        }
        case NAMESPACE_VARIABLE:
        {
            word = _CfrTil_Variable_New ( name, value ) ;
            break ;
        }
        case LITERAL:
        {
            word = Literal_New ( _Context_->Lexer0, value ) ;
            break ;
        }
        case CONSTANT:
        {
            word = _DObject_New ( name, value, CONSTANT | IMMEDIATE, 0, CONSTANT, ( byte* ) _DataObject_Run, 0, 1, 0, DICTIONARY ) ;
            break ;
        }
        case OBJECT:
        {
            word = _Class_Object_New ( name, type ) ;
            break ;
        }
        case DOBJECT:
        {
            DObject_New ( ) ;
            break ;
        }
        case CLASS:
        {
            word = _Class_New ( name, CLASS, 0 ) ;
            break ;
        }
        case CLASS_CLONE:
        {
            word = _Class_New ( name, CLASS_CLONE, 1 ) ;
            break ;
        }
        case C_CLASS:
        {
            word = _Class_New ( name, C_CLASS, 0 ) ;
            break ;
        }
        case C_TYPE:
        {
            word = _Class_New ( name, C_TYPE, 0 ) ;
            _Property_Create ( ) ;
            break ;
        }
        case C_TYPEDEF:
        {
            _CfrTil_PropertyDef ( ) ;
            break ;
        }
        case PARAMETER_VARIABLE: case LOCAL_VARIABLE: case T_LISP_SYMBOL | PARAMETER_VARIABLE: case T_LISP_SYMBOL | LOCAL_VARIABLE:
        default: // REGISTER_VARIABLE combinations with others in this case
        {
            word = _CfrTil_LocalWord ( name, index, ctype, ltype ) ; // svf : flag - whether stack variables are in the frame
            break ;
        }
    }
    //if ( word && ( ! word->W_StartCharRlIndex ) ) word->W_StartCharRlIndex = startCharRlIndex ;
    return word ;
}

void
_CfrTil_MachineCodePrimitive_NewAdd ( const char * name, uint64 cType, block * callHook, byte * function, int32 functionArg, const char *nameSpace, const char * superNamespace )
{
    //_DObject_New ( byte * name, uint32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, uint32 allocType )
    Word * word = _DObject_New ( ( byte* ) name, ( uint32 ) function, cType, 0, 0, function, functionArg, 0, 0, DICTIONARY ) ;
    if ( callHook ) *callHook = word->Definition ;
    _CfrTil_InitialAddWordToNamespace ( word, ( byte* ) nameSpace, ( byte* ) superNamespace ) ;
}
