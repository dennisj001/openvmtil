
#include "../include/cfrtil.h"

void
_CfrTil_Word_Disassemble ( Word * word )
{
    byte * start ;
    if ( word )
    {
        start = word->CodeStart ;
        int64 size = _Debugger_Disassemble ( _Debugger_, start, word->S_CodeSize ? word->S_CodeSize : 128, 1 ) ;
        if ( ( ! word->S_CodeSize ) && ( size > 0 ) )
        {
            word->S_CodeSize = size ;
            _Printf ("\n_CfrTil_Word_Disassemble : word - \'%s\' :: codeSize = %d", word->Name, size ) ; 
        }
    }
}

void
_Word_Disassemble ( Word * word )
{
    if ( word )
    {
        _Printf ( ( byte* ) "\nWord : %s : disassembly at %s :", c_dd ( word->Name ), Context_Location () ) ;
        _CfrTil_Word_Disassemble ( word ) ;
        //_Printf ( ( byte* ) "\n" ) ;
    }
    else
    {
        _Printf ( ( byte* ) "\n%s : WordDisassemble : Can't find word code at this (0x%x) address.\n", c_dd ( Context_Location ( ) ), ( uint64 ) word->Definition ) ;
    }
}

void
CfrTil_Word_Disassemble ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Word_Disassemble ( word ) ;
}

void
Debugger_WDis ( Debugger * debugger )
{
    //_Printf ( ( byte* ) "\n" ) ;
    Word * word = debugger->w_Word ;
    if ( ! word ) word = _Interpreter_->w_Word ;
    _Word_Disassemble ( word ) ;
    //_Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_Disassemble ( )
{
    uint64 number = _DataStack_Pop ( ) ;
    byte * address = ( byte* ) _DataStack_Pop ( ) ;
    _Debugger_Disassemble ( _Debugger_, address, number, 0 ) ;
    //_Printf ( ( byte* ) "\n" ) ;
}


