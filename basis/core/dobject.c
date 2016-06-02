
#include "../../includes/cfrtil.h"


dobject *
_dobject_Allocate ( int32 doType, int32 slots, uint32 allocType )
{
    int32 size = sizeof ( dobject ) + ( slots * sizeof ( int32 ) ) ;
    dobject * dyno = ( dobject * ) _object_Allocate ( size, allocType ) ;
    dyno->do_iData = ( int* ) ( ( dobject* ) dyno + 1 ) ;
    dyno->do_Size = size ;
    dyno->do_Type = doType ;
    return dyno ;
}

dobject *
dobject_New ( int32 dynoType, int32 allocType, int slots, ... )
{
    dobject *dyno ;
    va_list args ;
    int i ;
    va_start ( args, slots ) ;
    dyno = _dobject_Allocate ( dynoType, slots, allocType ) ;
    for ( i = 0 ; i < slots ; i ++ ) dyno->do_iData[i] = va_arg ( args, int32 ) ;
    va_end ( args ) ;
    return dyno ;
}
// remember : Word = DynamicObject = DObject = Namespace

DObject *
_DObject_FindSlot_BottomUp ( DObject * dobject, byte * name )
{
    Word * word ;
    do
    {
        if ( ( word = Word_FindInOneNamespace ( dobject, name ) ) ) break ;
        dobject = dobject->ContainingNamespace ;
    }
    while ( dobject ) ;
    return ( DObject * ) word ;
}

DObject *
_DObject_SetSlot ( DObject * dobject, byte * name, int32 value )
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
    _Namespace_DoAddWord ( parent, dobject ) ;
    dobject->CProperty |= parent->CProperty ;
    dobject->Slots = parent->Slots ;
    //parent->State |= USING ;
    _Namespace_SetState ( parent, USING ) ;
}

DObject *
DObject_Sub_New ( DObject * proto, byte * name, uint64 category )
{
    DObject * dobject = _DObject_New ( name, 0, ( category | DOBJECT | IMMEDIATE ), 0, DOBJECT, ( byte* ) Interpreter_DataObject_Run, 0, 0, 0, DICTIONARY ) ;
    DObject_SubObjectInit ( dobject, proto ) ;
    return dobject ;
}

DObject *
_DObject_NewSlot ( DObject * proto, byte * name, int32 value )
{
    DObject * dobject = DObject_Sub_New ( proto, name, DOBJECT ) ;
    //dobject->W_DObjectValue = value ;
    dobject->W_PtrToValue = & dobject->W_DObjectValue ;
    proto->Slots ++ ;
    return dobject ;
}

void
DObject_NewClone ( DObject * proto )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    DObject_Sub_New ( proto, name, DOBJECT ) ;
}


