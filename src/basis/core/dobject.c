
#include "../../include/cfrtil.h"

/* from macros.h
#define dobject_Get_M_Slot( dobj, m ) (((dobject*) dobj)->do_iData [m]) 
#define dobject_Set_M_Slot( dobj, m, value ) (((dobject*) dobj)->do_iData [m] = ((int64)value) ) 
#define List_Set_N_Node_M_Slot( list, n, m, value ) _dllist_Set_N_Node_M_Slot ( list, 0, 0, value ) 
#define List_Get_N_Node_M_Slot( list, n, m ) _dllist_Get_N_Node_M_Slot ( (dllist * )list, (int64) n, (int64) m )
 * from dllist.c
int64 _dllist_Get_N_Node_M_Slot ( dllist * list, int64 n, int64 m )
void _dllist_Set_N_Node_M_Slot ( dllist * list, int64 n, int64 m, int64 value )
 */

byte *
_object_Allocate ( int64 size, int64 allocType )
{
    return Mem_Allocate ( size, allocType ) ;
}

dobject *
_dobject_Allocate ( int64 doType, int64 slots, uint64 allocType )
{
    int64 size = sizeof ( dobject ) + ( slots * sizeof ( int64 ) ) ;
    dobject * dobj = ( dobject * ) _object_Allocate ( size, allocType ) ;
    dobj->do_iData = ( int64* ) ( ( dobject* ) dobj + 1 ) ;
    dobj->do_Slots = ( int16 ) slots ;
    dobj->do_int32_Size = ( int16 ) size ;
    dobj->do_Type = ( int16 ) doType ;
    return dobj ;
}

dobject *
_dobject_New_M_Slot_Node ( int64 allocType, int64 dobjType, int64 m_slots, ... )
{
    dobject *dobj ;
    va_list args ;
    int64 i ;
    va_start ( args, m_slots ) ;
    dobj = _dobject_Allocate ( dobjType, m_slots, allocType ) ;
    for ( i = 0 ; i < m_slots ; i ++ ) dobj->do_iData[i] = va_arg ( args, int64 ) ;
    va_end ( args ) ;
    return dobj ;
}

void
_dobject_Print ( dobject * dobj )
{
    int64 i ;
    _Printf ( ( byte* ) "\n\ndobject  = 0x%08x : word Name = %s", dobj, dobj->do_iData[2] ? ( ( Word* ) dobj->do_iData[2] )->Name : ( byte* ) "" ) ;
    _Printf ( ( byte* ) "\nType     = %d", dobj->do_Type ) ;
    _Printf ( ( byte* ) "\nSlots    = %d", dobj->do_Slots ) ;
    _Printf ( ( byte* ) "\nSize     = %d", dobj->do_int32_Size ) ;
    for ( i = 0 ; i < dobj->do_Slots ; i ++ )
    {
        _Printf ( ( byte* ) "\nSlot [%d] = 0x%08x", i, dobj->do_iData[i] ) ;
    }
    //_Printf ( ( byte* ) "\n" ) ;
}
// remember : Word = DynamicObject = DObject = Namespace

void
_DObject_C_StartupCompiledWords_DefInit ( byte * function, int64 arg )
{
    if ( arg == - 1 )
    {
        ( ( void (* )( ) )( function ) ) ( ) ;
    }
    else
    {
        ( ( void (* ) ( int64 ) )( function ) ) ( arg ) ;
    }
}

void
_DObject_ValueDefinition_Init ( Word * word, uint64 value, uint64 funcType, byte * function, int64 arg )
// using a variable that is a type or a function 
{    

    word->W_PtrToValue = & word->W_Value ;
    word->W_Value = value ;
    if ( GetState ( _Context_->Compiler0, LC_ARG_PARSING | PREFIX_ARG_PARSING ) ) word->W_StartCharRlIndex = _Context_->Lexer0->TokenStart_ReadLineIndex ;

    if ( funcType & BLOCK )
    {
        word->Definition = ( block ) ( function ? function : ( byte* ) value ) ; //_OptimizeJumps ( ( byte* ) value ) ; // this comes to play (only(?)) with unoptimized code
        word->CodeStart = ( byte* ) word->Definition ;
        if ( NamedByteArray_CheckAddress ( _Q_CodeSpace, word->CodeStart ) ) word->S_CodeSize = Here - word->CodeStart ;
        else word->S_CodeSize = 0 ; 
    }
    else
    {
        ByteArray * svcs = _Q_CodeByteArray ;
        int64 sscm = GetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE ) ;
        SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, false ) ;
        //Compiler_SetCompilingSpace_MakeSureOfRoom ( "ObjectSpace" ) ; 
        word->Coding = Here ;
        word->CodeStart = Here ;
        word->Definition = ( block ) Here ;
        if ( arg ) _DObject_C_StartupCompiledWords_DefInit ( function, arg ) ;
        //else Compile_Call_With32BitDisp ( ( byte* ) DataObject_Run ) ;
        else Compile_Call ( ( byte* ) DataObject_Run ) ; //Compile_Call_ToAddressThruReg ( ( byte* ) DataObject_Run, R8 ) ;
        _Compile_Return ( ) ;
        //d1 ( _Debugger_Disassemble ( _Debugger_, (byte*) word->Definition, 32, 1 ) ) ;
        word->S_CodeSize = Here - word->CodeStart ; // for use by inline
        Set_CompilerSpace ( svcs ) ;
        SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, sscm ) ;
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
        //if ( IsSourceCodeOn ) word->State |= W_SOURCE_CODE_MODE ;
    }
    if ( GetState ( _Context_, INFIX_MODE ) ) word->CProperty |= INFIX_WORD ;
    word->W_NumberOfArgs = _Context_->Compiler0->NumberOfArgs ;
    word->W_NumberOfLocals = _Context_->Compiler0->NumberOfLocals ;
    _CfrTil_->LastFinishedWord = word ;
}

Word *
_DObject_Init ( Word * word, uint64 value, uint64 ftype, byte * function, int64 arg, int64 addToInNs, Namespace * addToNs )
{
    // remember : Word = Namespace = DObject : each have an s_Symbol
    _DObject_ValueDefinition_Init ( word, value, ftype, function, arg ) ;
    _DObject_Finish ( word ) ;
    word->RunType = ftype ;
    _Word_Add ( word, addToInNs, addToNs ) ;
    return word ;
}

// DObject : dynamic object
// remember : Word = Namespace = DObject has a s_Symbol

Word *
_DObject_New ( byte * name, uint64 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int64 arg, int64 addToInNs, Namespace * addToNs, uint64 allocType )
{
    Word * word = _Word_New ( name, ctype, ltype, allocType ) ; //( addToInNs || addToNs ) ? DICTIONARY : allocType ) ;
    _DObject_Init ( word, value, ftype, function, arg, addToInNs, addToNs ) ;
    _CfrTil_->DObjectCreateCount ++ ;
    return word ;
}

DObject *
_DObject_FindSlot_BottomUp ( DObject * dobject, byte * name )
{
    Word * word ;
    do
    {
        if ( ( word = Finder_FindWord_InOneNamespace ( _Finder_, dobject, name ) ) ) break ;
        dobject = dobject->ContainingNamespace ;
    }
    while ( dobject ) ;

    return ( DObject * ) word ;
}

DObject *
_DObject_SetSlot ( DObject * dobject, byte * name, int64 value )
{
    DObject * ndobject = _DObject_FindSlot_BottomUp ( dobject, name ) ;
    if ( ! ndobject ) return _DObject_NewSlot ( dobject, name, value ) ;

    else return ndobject ;
}

void
DObject_SubObjectInit ( DObject * dobject, Word * parent )
{
    if ( ! parent ) parent = _CfrTil_Namespace_InNamespaceGet ( ) ;
    else if ( ! ( parent->CProperty & NAMESPACE ) )
    {

        parent->W_List = dllist_New ( ) ;
        parent->CProperty |= NAMESPACE ;
        _Namespace_AddToNamespacesTail ( parent ) ;
    }
    if ( parent->S_WAllocType == WORD_COPY_MEM ) parent = Word_Copy ( (Word*) parent, DICTIONARY ) ; // nb! : this allows us to
    Namespace_DoAddWord ( parent, dobject ) ; 
    dobject->CProperty |= parent->CProperty ;
    dobject->Slots = parent->Slots ;
    _Namespace_SetState ( parent, USING ) ;
}

DObject *
DObject_Sub_New ( DObject * proto, byte * name, uint64 category )
{
    DObject * dobject = _DObject_New ( name, 0, ( category | DOBJECT | IMMEDIATE ), 0, DOBJECT, ( byte* ) _DataObject_Run, 0, 0, 0, DICTIONARY ) ;
    DObject_SubObjectInit ( dobject, proto ) ;
    return dobject ;
}

// types an object as a DOBJECT
// but all variables and objects are created as DOBJECTs so this is not necessary

void
CfrTil_SetPropertiesAsDObject ( )
{
    DObject * o = ( DObject * ) _DataStack_Pop ( ) ;
    o->CProperty |= DOBJECT ;
}

DObject *
_DObject_NewSlot ( DObject * proto, byte * name, int64 value )
{
    DObject * dobject = DObject_Sub_New ( proto, name, DOBJECT ) ;
    dobject->W_DObjectValue = value ;
    dobject->W_PtrToValue = & dobject->W_DObjectValue ;
    proto->Slots ++ ;

    return dobject ;
}

void
CfrTil_DObject_Clone ( )
{
    DObject * proto = ( DObject * ) _DataStack_Pop ( ) ;
    byte * name = ( byte * ) _DataStack_Pop ( ) ;
    if ( ! ( proto->CProperty & DOBJECT ) ) Error2 ( ( byte* ) "Cloning Error : \'%s\' is not a dynamic object.", proto->Name, 1 ) ;
    DObject_Sub_New ( proto, name, DOBJECT ) ;
}

void
DObject_NewClone ( DObject * proto )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    DObject_Sub_New ( proto, name, DOBJECT ) ;
}

void
DObject_New ( )
{
    DObject_NewClone ( 0 ) ;
}

