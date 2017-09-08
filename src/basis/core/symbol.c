
#include "../../include/cfrtil.h"

void
_Symbol_NameInit ( Symbol * symbol, byte * name )
{
    symbol->S_Name = name ;
}

void
_Symbol_Init_AllocName ( Symbol * symbol, byte * name, uint64 allocType )
{
    if ( name )
    {
        byte* sname = String_New ( name, allocType ) ;
        _Symbol_NameInit ( symbol, sname ) ;
    }
}

// doesn't allocate name

Symbol *
__Symbol_New ( uint64 allocType )
{
    return ( Symbol * ) Mem_Allocate ( sizeof (Symbol ), allocType ) ;
}

Symbol *
_Symbol_New ( byte * name, uint64 allocType )
{
    Symbol * symbol = __Symbol_New ( allocType ) ;
    _Symbol_Init_AllocName ( symbol, name, allocType ) ;
    return symbol ;
}

// doesn't allocate name

Symbol *
Symbol_New ( byte * name )
{
    return _Symbol_New ( name, DICTIONARY ) ;
}

Symbol *
Symbol_NewValue ( int64 value, uint64 allocType )
{
    Symbol * sym = __Symbol_New ( allocType ) ;
    sym->W_Value = value ;
    return sym ;
}

Symbol *
_Symbol_CompareName ( Symbol * symbol, byte * name )
{
    d0 ( if ( _Q_->Verbosity > 3 ) _Printf ( (byte*) "\n symbol name = %s : name = %s", symbol->S_Name, name ) ) ;
    if ( name && symbol && symbol->S_Name && ( String_Equal ( symbol->S_Name, name ) ) ) return symbol ;
    else return 0 ;
}

Symbol *
Symbol_CompareName ( Symbol * symbol, byte * name )
{
    if ( symbol = _Symbol_CompareName ( symbol, name ) )
    {
        return symbol ;
    }
    return 0 ;
}
