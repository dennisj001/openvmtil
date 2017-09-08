
#include "../include/cfrtil.h"

#if 0 // not used yet -- maybe replaced by better DObject functionality
Property *
_Type_Create ( )
{
    Property * property ;
#if 0    
    if ( category & ( LOCAL_VARIABLE | PARAMETER_VARIABLE ) )
    {
        property = ( Property* ) Mem_Allocate_AddToList ( sizeof (Property ), COMPILER_TEMP, 0 ) ;
    }
    else
#endif        
    {
        property = ( Property * ) Mem_Allocate ( sizeof (Property ), DICTIONARY ) ;
    }
    return property ;
}

Property *
_Property_Init ( Property * property, byte * name, int64 value, uint64 category )
{
    //property->Symbol = ( Symbol * ) Mem_Allocate ( sizeof ( Symbol ), DICTIONARY ) ;
    _Symbol_Init_AllocName ( (Symbol*) property, name, DICTIONARY ) ;
    property->W_Value = value ;
    return property ;
}

Property *
_Property_New ( byte * name, int64 value )
{
    Property * property = _Type_Create ( ) ;
    _Property_Init ( property, name, value, PROPERTY | NAMESPACE_VARIABLE ) ;
    return property ;
}

void
_Property_SetValue ( Property * property, int64 value )
{
    if ( property ) property->W_Value = value ;
}

int64
_Property_GetValue ( Property * property )
{
    if ( property ) return property->W_Value ;
    else return 0 ;
}

void
CfrTil_Property_New ( )
{
    byte * name = ( byte * ) _DataStack_Pop ( ) ;
    int64 value = ( int64 ) _DataStack_Pop ( ) ;
    Property * property = _Property_New ( name, value ) ;
    _CfrTil_AddSymbol ( (Symbol*) property ) ;
}

Property *
_DObject_FindProperty ( DObject * dobject, byte * name )
{
    Word * word = Finder_FindWord_InOneNamespace ( _Finder_, dobject, name ) ;
    if ( word ) return ( (Property*) word ) ;
    else return 0 ;
}

Property *
_DObject_Property_New ( DObject * dobject, byte * name, int64 value )
{
    Property * property = _Property_New ( name, value ) ;
    Namespace_DoAddSymbol ( dobject, (Symbol*) property ) ;
    dobject->Slots ++ ;
    return property ;
}

int64
_DObject_GetPropertyValue ( DObject * dobject, byte * name )
{
    Property * property = _DObject_FindProperty ( dobject, name ) ;
    return _Property_GetValue ( property ) ;
}

Property *
_DObject_SetPropertyValue ( DObject * dobject, byte * name, int64 value )
{
    Property * property = _DObject_FindProperty ( dobject, name ) ;
    if ( property ) _Property_SetValue ( property, value ) ;
    else property = _DObject_Property_New ( dobject, name, value ) ;
    return property ;
}

Property *
_DObject_FindProperty_BottomUp ( DObject * dobject, byte * name )
{
    Word * word = 0 ;
    do
    {
        if ( ( word = Finder_FindWord_InOneNamespace ( _Finder_, dobject, name ) ) ) break ;
    }
    while ( ( dobject = dobject->ContainingNamespace ) ) ;
    if ( word ) return (Property*) ( word ) ;
    else return 0 ;
}
#endif


