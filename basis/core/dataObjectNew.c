
#include "../../includes/cfrtil.h"

// this function is very central and should be worked on
// ?!? i tried to bring alot of things together here but now it needs simplification at least to be more understandable ?!?
// Dynamic Object New : Word = Namespace = DObject : have a s_Symbol
// it seems alot here could be just unnecessary in at least some cases

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
_DObject_Definition_EvalStore ( Word * word, uint32 value, uint64 ctype, uint64 funcType, byte * function, int arg )
// using a variable that is a type or a function 
{
    byte * csName ;
    // remember : Word = Namespace = DObject 
    byte * token = word->Name ; 
    DEBUG_START ;
    if ( dm )
    {
        DebugColors ;
        Printf ( ( byte* ) "\n_DObject_Definition_EvalStore : entering : word = %s : value = 0x%08x...", word->Name, value ) ;
        Stack ( ) ;
    }
    word->W_PtrToValue = & word->W_Value ;
    word->W_Value = value ; // this could be reset below
    if ( ( ( funcType != 0 ) || ( function != 0 ) ) )
    {
        if ( dm && ( ! GetState ( debugger, DBG_DONE ) ) )
        {
            token = String_ConvertToBackSlash ( token ) ;
            csName = Get_CompilerSpace ( )->OurNBA->NBA_Name ;
            if ( DebugLevel ( 2 ) ) Printf ( c_dd ( "\n_DObject_Definition_EvalStore : %s : Compiling to %s by _DObject_Definition_EvalStore as a new literal ..." ), token, csName ) ;
        }
        if ( funcType & BLOCK )
        {
            word->Definition = ( block ) ( function ? function : ( byte* ) value ) ; //_OptimizeJumps ( ( byte* ) value ) ; // this comes to play (only(?)) with unoptimized code
            word->CodeStart = ( byte* ) word->Definition ;
            if ( ( word->CodeStart < ( byte* ) CompilerMemByteArray->BA_Data ) || ( word->CodeStart > ( byte* ) CompilerMemByteArray->bp_Last ) ) word->S_CodeSize = 0 ; // ?!? not quite accurate
            else word->S_CodeSize = Here - word->CodeStart ; // for use by inline
            word->W_Value = value ;
        }
        else if ( ( ! ( _Q_->OVT_LC && ( GetState ( _Q_->OVT_LC, ( LC_READ | LC_PRINT | LC_OBJECT_NEW_OFF ) ) ) ) ) || ( GetState ( _Q_->OVT_Context->Compiler0, ( LC_ARG_PARSING ) ) ) )
        {
            ByteArray * scs ;
            if ( ! word->W_Value ) word->W_Value = value ; //or maybe : if ( ! Is_NamespaceType ( word ) )
            scs = CompilerMemByteArray ;
            if ( funcType != LITERAL ) // only strict literals are compiled into CodeSpace
            {
                _Compiler_SetCompilingSpace ( ( byte* ) "ObjectSpace" ) ; // same problem as namespace ; this can be called in the middle of compiling another word 
            }
            word->Coding = Here ;
            word->CodeStart = Here ;
            word->Definition = ( block ) Here ;
            if ( funcType & ( LITERAL ) )
            {
                DataObject_Run ( word ) ;
                // nb : no RET insn is or should be compiled for literals : cf. below
            }
            else if ( funcType & ( CONSTANT | VARIABLE | LOCAL_VARIABLE | PARAMETER_VARIABLE | NAMESPACE | CLASS | OBJECT_FIELD | OBJECT | DOBJECT | C_TYPE | C_CLASS | CLASS_CLONE ) )
            {
                _Compile_C_Call_1_Arg ( ( byte* ) DataObject_Run, ( int32 ) word ) ; // this make every object a function => fully functional language
            }
            else if ( arg ) _DObject_C_StartupCompiledWords_DefInit ( function, arg ) ;
            else if ( ctype & C_PREFIX_RTL_ARGS )
            {
                _Compile_Stack_Push ( DSP, ( int32 ) word ) ;
                Compile_Call ( ( byte* ) function ) ;
            }
            else Compile_Call ( function ) ; //
            if ( funcType != LITERAL )
            {
                _Compile_Return ( ) ;
            }
            word->S_CodeSize = Here - word->CodeStart ; // for use by inline
            Set_CompilerSpace ( scs ) ;
        }
    }
    DEBUG_SHOW ;
    if ( dm ) // 'dm' and 'debugger' are initialized in the DEBUG_START macro above
    {
        SetState ( debugger, DBG_FORCE_SHOW_WRITTEN_CODE, false ) ;
        if ( ( funcType & ( LITERAL ) ) && ( ! GetState ( debugger, DBG_DONE ) ) )
        {
            Printf ( c_dd ( "\nLiteral : %s : Compiled to %s by _DObject_Definition_EvalStore as a new literal ..." ), token, csName ) ;
        }
        Printf ( ( byte* ) "\n_DObject_Definition_EvalStore : exiting ..." ) ;
        Stack ( ) ;
        DefaultColors ;
    }
}

void
_DObject_Finish ( Word * word )
{
    uint64 ctype = word->CType ;
    ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
    if ( ! ( ctype & CPRIMITIVE ) )
    {
        if ( GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) ) word->State |= COMPILED_OPTIMIZED ;
        if ( CfrTil_GetState ( _Q_->OVT_CfrTil, INLINE_ON ) ) word->State |= COMPILED_INLINE ;
    }
    if ( GetState ( _Q_->OVT_Context, INFIX_MODE ) ) word->CType |= INFIX_WORD ;
    if ( rl->InputStringOriginal && ( ! word->S_WordData->Filename ) ) // this is now done first in Word_Create
    {
        word->S_WordData->Filename = rl->Filename ;
        word->S_WordData->LineNumber = rl->LineNumber ;
        word->W_CursorPosition = rl->CursorPosition ;
    }
    word->NumberOfArgs = _Q_->OVT_Context->Compiler0->NumberOfStackVariables ;
    _Q_->OVT_CfrTil->LastFinishedWord = word ;
}

Word *
_DObject_Init ( Word * word, uint32 value, uint64 ctype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs )
{
    // remember : Word = Namespace = DObject : each have an s_Symbol
    _DObject_Definition_EvalStore ( word, value, ctype, ftype, function, arg ) ;
    _Word_Add ( word, addToInNs, addToNs ) ;
    _DObject_Finish ( word ) ;
    word->RunType = ftype ;
    return word ;
}

// DObject : dynamic object

Word *
_DObject_New ( byte * name, uint32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, uint32 allocType )
{
    // remember : Word = Namespace = DObject has a s_Symbol
    Word * word = _Word_New ( name, ctype, ltype, allocType ) ;
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
    Stack * stack = _Q_->OVT_Context->Compiler0->NamespacesStack ;
    Stack_Init ( stack ) ; // !! ?? put this in Compiler ?? !!
    // init needs to be done by the most super class first successively down to the current class 
    do
    {
        Word * initWord ;
        if ( ( initWord = Word_FindInOneNamespace ( ns, ( byte* ) "init" ) ) )
        {
            _Stack_Push ( stack, ( int32 ) initWord ) ;
        }
        ns = ns->ContainingNamespace ;
    }
    while ( ns ) ;
    int32 i ;
    for ( i = Stack_Depth ( stack ) ; i > 0 ; i -- )
    {
        _Push ( ( int32 ) * word->W_PtrToValue ) ;
        Word * initWord = ( Word* ) _Stack_Pop ( stack ) ;
        _Word_Eval ( initWord ) ;
    }
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
    word = _DObject_New ( name, ( int32 ) object, ( OBJECT | IMMEDIATE | category ), 0, OBJECT, ( byte* ) DataObject_Run, - 1, 1, 0, DICTIONARY ) ;
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
        ns = _DObject_New ( name, 0, CPRIMITIVE | CLASS | IMMEDIATE | type, 0, type, ( byte* ) DataObject_Run, - 1, 0, sns, DICTIONARY ) ;
        _Namespace_DoNamespace ( ns, 1 ) ; // before "size", "this"
        _CfrTil_Variable ( ( byte* ) "size", size ) ; // start with size of the prototype for clone
        _Class_Object_New ( ( byte* ) "this", THIS | VARIABLE ) ;
    }
    else
    {
        Printf ( ( byte* ) "\nNamespace Error ? : class \"%s\" already exists!\n", ns->Name ) ;
        _Namespace_DoNamespace ( ns, 1 ) ;
    }
    Stack_Init ( _Q_->OVT_Context->Compiler0->WordStack ) ; // try to keep WordStack to a minimum
    return ns ;
}

void
_CfrTil_ClassField_New ( byte * token, Class * aclass, int32 size, int32 offset )
{
    Word * word = _DObject_New ( token, 0, IMMEDIATE | OBJECT_FIELD, 0, OBJECT_FIELD, ( byte* ) DataObject_Run, - 1, 1, 0, DICTIONARY ) ;
    word->ClassFieldTypeNamespace = aclass ;
    word->Size = size ;
    word->Offset = offset ;
}

void
CfrTil_Class_New ( void )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    //_Class_New ( name, CLASS, 0, ( byte* ) _Namespace_DoNamespace ) ;
    _DataObject_New ( CLASS, name, 0, 0, 0, 0 ) ;
}

void
CfrTil_Class_Clone ( void )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    //_Class_New ( name, CLASS_CLONE, 1, ( byte* ) _Namespace_DoNamespace ) ;
    _DataObject_New ( CLASS_CLONE, name, 0, 0, 0, 0 ) ;
}
// ( <name> value -- )

// remember : this word runs at compile/interpret time ; nothing is compiled yet

void
Class_Value_New ( byte * name )
{
    //_Class_Value_New ( name, 0 ) ;
    _DataObject_New ( OBJECT, name, 0, 0, 0, 0 ) ;
}

void
CfrTil_Class_Value_New ( )
{
    //Class_Value_New ( ( byte* ) _DataStack_Pop ( ) ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataObject_New ( OBJECT, name, 0, 0, 0, 0 ) ;
}

void
DObject_NewClone ( DObject * proto )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    DObject_Sub_New ( proto, name, DOBJECT ) ;
}

void
CfrTil_DObject_Clone ( )
{
    DObject * proto = ( DObject * ) _DataStack_Pop ( ) ; //, * one = WordStack ( - 1 ) ; //remember : _CfrTil_Do_DObject pushes &dobject->W_Object
    byte * name = ( byte * ) _DataStack_Pop ( ) ;
    if ( ! ( proto->CType & DOBJECT ) ) Error2 ( ( byte* ) "Cloning Error : %s is not an object.", proto->Name, 1 ) ;
    DObject_Sub_New ( proto, name, DOBJECT ) ;
}

void
DObject_New ( )
{
    DObject * proto = Namespace_Find ( ( byte* ) "DObject" ) ;
    DObject_NewClone ( proto ) ;
}

// this maybe should be in primitives/dobject.c

void
CfrTil_DObject_New ( )
{
    // clone DObject -- create an object with DObject as it's prototype (but not necessarily as it's namespace)
    //DObject_New ( ) ;
    _DataObject_New ( DOBJECT, 0, 0, 0, 0, 0 ) ;

}

Word *
_CfrTil_Variable ( byte * name, int32 value )
{
    Namespace * addToNamespace ;
    Word * word ;
    if ( CompileMode )
    {
        BlockInfo * bi = ( BlockInfo * ) _Stack_Top ( _Q_->OVT_Context->Compiler0->BlockStack ) ;
        if ( bi->LocalsNamespace ) addToNamespace = bi->LocalsNamespace ;
        else
        {
            addToNamespace = Namespace_FindOrNew_Local ( ) ;
        }
        word = _DObject_New ( name, value, ( LOCAL_VARIABLE | IMMEDIATE ), 0, LOCAL_VARIABLE, ( byte* ) DataObject_Run, - 1, ( ( int32 ) addToNamespace ) ? 0 : 1, addToNamespace, SESSION ) ;
        word->Index = _Q_->OVT_Context->Compiler0->NumberOfLocals ++ ;
    }
        //else word = _DObject_New ( name, value, VARIABLE | IMMEDIATE, 0, VARIABLE, ( byte* ) Do_VariableOrLiteral, 0, 1, 0, DICTIONARY ) ;
    else word = _DObject_New ( name, value, VARIABLE | IMMEDIATE, 0, VARIABLE, ( byte* ) DataObject_Run, 0, 1, 0, DICTIONARY ) ;
    return word ;
}

void
CfrTil_Constant ( )
{
    int32 value = _DataStack_Pop ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DObject_New ( name, value, CONSTANT | IMMEDIATE, 0, CONSTANT, ( byte* ) DataObject_Run, 0, 1, 0, DICTIONARY ) ;
}

void
_CfrTil_Label ( byte * lname )
{
    Namespace * ns = Namespace_FindOrNew_SetUsing ( ( byte* ) "__labels__", _Q_->OVT_CfrTil->Namespaces, 1 ) ;
    _DObject_New ( lname, ( int32 ) Here, CONSTANT | IMMEDIATE, 0, CONSTANT, ( byte* ) DataObject_Run, 0, 0, ns, DICTIONARY ) ;
}

Word *
_CfrTil_LocalWord ( byte * name, int32 index, int64 ctype, uint64 ltype ) // svf : flag - whether stack variables are in the frame
{
    Word * word = _DObject_New ( name, 0, ( ctype | VARIABLE | IMMEDIATE ), ltype, LOCAL_VARIABLE, ( byte* ) DataObject_Run, - 1, 1, 0, SESSION ) ;
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
    if ( ! Lexer_GetState ( lexer, KNOWN_OBJECT ) )
    {
        snprintf ( ( char* ) _name, 256, "<unknown object type> : %x", ( uint ) uliteral ) ;
        name = _name ;
    }
    else //if ( ! Lexer_GetState ( lexer, KNOWN_OBJECT ) )
    {
        snprintf ( ( char* ) _name, 256, "<literal> : %s", lexer->OriginalToken ) ;
        name = _name ;
    }
    //_DObject_New ( byte * name, uint32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, uint32 allocType )
    word = _DObject_New ( name, uliteral, LITERAL | CONSTANT, 0, LITERAL, ( byte* ) DataObject_Run, 0, 0, 0, ( CompileMode ? DICTIONARY : SESSION ) ) ;
    return word ;
}

Namespace *
_Namespace_New ( byte * name, Namespace * containingNs )
{
    Namespace * ns = _DObject_New ( name, 0, ( CPRIMITIVE | NAMESPACE ), 0, NAMESPACE, ( byte* ) DataObject_Run, - 1, 0, containingNs, DICTIONARY ) ;
    return ns ;
}

Namespace *
CfrTil_Type_New ( )
{
    CfrTil_Token ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    return _DataObject_New ( C_TYPE, name, 0, 0, 0, 0 ) ;
}

void
CfrTil_Typedef ( )
{
    //_CfrTil_Typedef ( ) ;
    _DataObject_New ( C_TYPEDEF, 0, 0, 0, 0, 0 ) ;
}

//_LO_New ( uint64 ltype, uint64 ctype, byte * value, Word * word, int32 addFlag, byte * name, uint32 allocType )

Word *
_DataObject_New ( uint64 type, byte * name, uint64 ctype, uint64 ltype, int32 index, int32 value )
{
    Word * word = 0 ;
    switch ( type )
    {
        case NAMESPACE:
        {
            word = _Namespace_New ( name, ( Namespace * ) value ) ;
            break ;
        }
        case VARIABLE:
        {
            word = _CfrTil_Variable ( name, value ) ;
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
            _Type_Create ( ) ;
            break ;
        }
        case C_TYPEDEF:
        {
            _CfrTil_Typedef ( ) ;
            break ;
        }
        case PARAMETER_VARIABLE:
        case LOCAL_VARIABLE:
        case T_LISP_SYMBOL | PARAMETER_VARIABLE:
        case T_LISP_SYMBOL | LOCAL_VARIABLE:
        default: // REGISTER_VARIABLE combinations with others in this case
        {
            word = _CfrTil_LocalWord ( name, index, ctype, ltype ) ; // svf : flag - whether stack variables are in the frame
            break ;
        }
    }
    return word ;
}

#if 1

void
_CfrTil_MachineCodePrimitive_NewAdd ( const char * name, uint64 cType, block * callHook, byte * function, int32 functionArg, const char *nameSpace, const char * superNamespace )
{
    //_DObject_New ( byte * name, uint32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, uint32 allocType )
    Word * word = _DObject_New ( ( byte* ) name, ( uint32 ) function, cType, 0, 0, function, functionArg, 0, 0, DICTIONARY ) ;
    if ( callHook ) *callHook = word->Definition ;
    _CfrTil_InitialAddWordToNamespace ( word, ( byte* ) nameSpace, ( byte* ) superNamespace ) ;
}
#else

void
_CfrTil_MachineCodePrimitive_NewAdd ( const char * name, uint64 cType, block * callHook, byte * function, int32 functionArg, const char *nameSpace, const char * superNamespace )
{
    Word * word = _DObject_New ( ( byte* ) name, 0, cType, 0, 0, function, functionArg, 0, 0, DICTIONARY ) ;
    if ( callHook ) *callHook = word->Definition ;
    _CfrTil_InitialAddWordToNamespace ( word, ( byte* ) nameSpace, ( byte* ) superNamespace ) ;
}

#endif