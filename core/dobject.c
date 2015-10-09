
#include "../includes/cfrtil.h"

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
DObject_SubObjectInit ( DObject * dobject, Word * proto )
{
#if 0    
    if ( ! proto->S_SymbolList ) proto->S_SymbolList = DLList_New () ;
    _Namespace_DoAddWord ( proto, dobject ) ;
#endif    
    dobject->CType |= proto->CType ;
    dobject->Slots = proto->Slots ;
    proto->State |= USING ;
}

DObject *
DObject_Sub_New ( DObject * proto, byte * name, uint64 category )
{
    DObject * dobject = _DObject_New ( name, 0, ( category | DOBJECT ), 0, DOBJECT, ( byte* ) DataObject_Run, - 1, 1, 0, DICTIONARY ) ;
    DObject_SubObjectInit ( dobject, proto ) ;
    return dobject ;
}

DObject *
_DObject_NewSlot ( DObject * proto, byte * name, int32 value )
{
    DObject * dobject = DObject_Sub_New ( proto, name, DOBJECT ) ;
    dobject->WD_ObjectReference = ( byte* ) value ;
    proto->Slots ++ ;
    return dobject ;
}

void
_DObject_NewClone ( DObject * proto, byte * name )
{
    DObject_Sub_New ( proto, name, DOBJECT ) ;
}

void
CfrTil_DObject ()
{
    DObject * o = ( DObject * ) _DataStack_Pop ( ) ;
    o->CType |= DOBJECT ;
}