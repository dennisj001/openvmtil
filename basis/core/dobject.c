
#include "../../includes/cfrtil.h"

#if 0 // i like this idea -- use this!? -- Do not delete! -- from sl5.c

Object *
obMake ( enum otype type32, int count, ... )
{
    Object *ob, *arg ;
    va_list args ;
    int i ;
    va_start ( args, count ) ;
    ob = ( byte* ) _Allocate ( sizeof (Object ) + ( count - 1 ) * sizeof (Object * ), Pnba_SL5 ) ;

    ob->type = type32 ;
    for ( i = 0 ; i < count ; i ++ ) ob->p[i] = va_arg ( args, Object * ) ;
    va_end ( args ) ;
    return ob ;
}
#endif

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
    else if ( ! ( parent->CType & NAMESPACE ) )
    {
        parent->W_List = DLList_New ( ) ;
        parent->CType |= NAMESPACE ;
        _Namespace_AddToNamespacesTail ( parent ) ;
    }
    _Namespace_DoAddWord ( parent, dobject ) ;
    dobject->CType |= parent->CType ;
    dobject->Slots = parent->Slots ;
    parent->State |= USING ;
}

DObject *
DObject_Sub_New ( DObject * proto, byte * name, uint64 category )
{
    DObject * dobject = _DObject_New ( name, 0, ( category | DOBJECT | IMMEDIATE ), 0, DOBJECT, ( byte* ) _DataObject_Run, - 1, 0, 0, DICTIONARY ) ;
    DObject_SubObjectInit ( dobject, proto ) ;
    return dobject ;
}

DObject *
_DObject_NewSlot ( DObject * proto, byte * name, int32 value )
{
    DObject * dobject = DObject_Sub_New ( proto, name, DOBJECT ) ;
    dobject->W_DObjectValue = value ;
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


