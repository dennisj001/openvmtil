
#include "../include/cfrtil.h"

void
_CfrTil_Word_Disassemble ( Word * word )
{
    byte * start ;
    if ( word )
    {
#if 1       
        if ( GetState ( _CfrTil_, INLINE_ON ) )
        {
            start = ( byte* ) word->Definition ;
        }
        else
#endif        
            start = word->CodeStart ;
        _Debugger_Disassemble ( _Debugger_, start, word->S_CodeSize ? word->S_CodeSize : 128, 1 ) ;
    }
}

void
_Word_Disassemble ( Word * word )
{
    if ( word )
    {
        _Printf ( ( byte* ) "\nWord : %s : disassembly :>", c_dd ( word->Name ) ) ;
        _CfrTil_Word_Disassemble ( word ) ;
        _Printf ( ( byte* ) "\n" ) ;
    }
    else
    {
        _Printf ( ( byte* ) "\n%s : WordDisassemble : Can't find word code at this (0x%x) address.\n", c_dd ( Context_Location ( ) ), ( uint ) word->Definition ) ;
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
    _Printf ( ( byte* ) "\n" ) ;
    Word * word = debugger->w_Word ;
    if ( ! word ) word = _Q_->OVT_Interpreter->w_Word ;
    _Word_Disassemble ( word ) ;
    //_Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_Disassemble ( )
{
    uint number = _DataStack_Pop ( ) ;
    byte * address = ( byte* ) _DataStack_Pop ( ) ;
    _Debugger_Disassemble ( _Debugger_, address, number, 0 ) ;
    _Printf ( ( byte* ) "\n" ) ;
}

