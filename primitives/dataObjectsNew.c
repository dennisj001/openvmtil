
#include "../includes/cfrtil.h"

void
CfrTil_Class_New ( void )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    //_Class_New ( name, CLASS, 0, ( byte* ) _Namespace_DoNamespace ) ;
    _DataObject_New ( CLASS, 0, name, 0, 0, 0, 0, 0 ) ;
}

void
CfrTil_Class_Value_New ( )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataObject_New ( OBJECT, 0, name, 0, 0, 0, 0, 0 ) ;
}

void
CfrTil_Class_Clone ( void )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataObject_New ( CLASS_CLONE, 0, name, 0, 0, 0, 0, 0 ) ;
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
CfrTil_DObject_New ( )
{
    // clone DObject -- create an object with DObject as it's prototype (but not necessarily as it's namespace)
    _DataObject_New ( DOBJECT, 0, 0, 0, 0, 0, 0, 0 ) ;
}

// types an object as a DOBJECT
// but all variables and objects are created as DOBJECTs so this is not necessary
void
CfrTil_DObject ()
{
    DObject * o = ( DObject * ) _DataStack_Pop ( ) ;
    o->CProperty |= DOBJECT ;
}

Namespace *
CfrTil_Property_New ( )
{
    CfrTil_Token ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    return _DataObject_New ( C_TYPE, 0, name, 0, 0, 0, 0, 0 ) ;
}

void
CfrTil_Propertydef ( )
{
    //_CfrTil_Propertydef ( ) ;
    _DataObject_New ( C_TYPEDEF, 0, 0, 0, 0, 0, 0, 0 ) ;
}


