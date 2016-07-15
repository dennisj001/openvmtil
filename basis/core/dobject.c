
#include "../../include/cfrtil.h"

dobject *
_dobject_Allocate ( int32 doType, int32 slots, uint32 allocType )
{
    int32 size = sizeof ( dobject ) + ( slots * sizeof ( int32 ) ) ;
    dobject * dobj = ( dobject * ) _object_Allocate ( size, allocType ) ;
    dobj->do_iData = ( int* ) ( ( dobject* ) dobj + 1 ) ;
    dobj->do_Slots = (int16) slots ;
    dobj->do_Size = (int16) size ;
    dobj->do_Type = (int16) doType ;
    return dobj ;
}

dobject *
_dobject_New_M_Slot_Node ( int32 allocType, int32 dobjType, int m_slots, ... )
{
    dobject *dobj ;
    va_list args ;
    int i ;
    va_start ( args, m_slots ) ;
    dobj = _dobject_Allocate ( dobjType, m_slots, allocType ) ;
    for ( i = 0 ; i < m_slots ; i ++ ) dobj->do_iData[i] = va_arg ( args, int32 ) ;
    va_end ( args ) ;
    return dobj ;
}

void
_dobject_Print ( dobject * dobj )
{
    int32 i ;
    Printf ( "\n\ndobject  = 0x%08x : word Name = %s", dobj, dobj->do_iData[2] ? ((Word*)dobj->do_iData[2])->Name : (byte*) "" ) ;
    Printf ( "\nType     = %d", dobj->do_Type ) ;
    Printf ( "\nSlots    = %d", dobj->do_Slots ) ;
    Printf ( "\nSize     = %d", dobj->do_Size ) ;
    for ( i = 0 ; i < dobj->do_Slots ; i ++ )
    {
        Printf ( "\nSlot [%d] = 0x%08x", i, dobj->do_iData[i] ) ;
    }
    Printf ( "\n" ) ;
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
    DObject * dobject = _DObject_New ( name, 0, ( category | DOBJECT | IMMEDIATE ), 0, DOBJECT, ( byte* ) _DataObject_Run, 0, 0, 0, DICTIONARY ) ;
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


