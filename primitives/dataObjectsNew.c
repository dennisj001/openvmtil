
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
    DObject * proto = ( DObject * ) _DataStack_Pop ( ) ; //, * one = WordStack ( - 1 ) ; //remember : _CfrTil_Do_DObject pushes &dobject->W_Object
    byte * name = ( byte * ) _DataStack_Pop ( ) ;
    if ( ! ( proto->CType & DOBJECT ) ) Error2 ( ( byte* ) "Cloning Error : %s is not an object.", proto->Name, 1 ) ;
    DObject_Sub_New ( proto, name, DOBJECT ) ;
}

void
CfrTil_DObject_New ( )
{
    // clone DObject -- create an object with DObject as it's prototype (but not necessarily as it's namespace)
    //DObject_New ( ) ;
    _DataObject_New ( DOBJECT, 0, 0, 0, 0, 0, 0, 0 ) ;
}

Namespace *
CfrTil_Type_New ( )
{
    CfrTil_Token ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    return _DataObject_New ( C_TYPE, 0, name, 0, 0, 0, 0, 0 ) ;
}

void
CfrTil_Typedef ( )
{
    //_CfrTil_Typedef ( ) ;
    _DataObject_New ( C_TYPEDEF, 0, 0, 0, 0, 0, 0, 0 ) ;
}


