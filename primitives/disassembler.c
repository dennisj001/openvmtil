
#include "../includes/cfrtil.h"

void
_CfrTil_Word_Disassemble ( Word * word )
{
    byte * start ;
    if ( word )
    {
        if ( GetState ( _Q_->OVT_CfrTil, INLINE_ON ) )
        {
            start = ( byte* ) word->Definition ;
        }
        else start = word->CodeStart ;
        _Debugger_Disassemble ( DEBUGGER, start, word->S_CodeSize ? word->S_CodeSize : 128, 0 ) ;
    }
}

void
CfrTil_Word_Disassemble ( )
{
    byte * address = ( byte* ) _DataStack_Pop ( ) ;
    Word * word = Word_GetFromCodeAddress_NoAlias ( address ) ;
    if ( word )
    {
        Printf ( ( byte* ) "\nWord : %s : disassembly :> \n", c_dd ( word->Name ) ) ;
        _CfrTil_Word_Disassemble ( word ) ;
        Printf ( ( byte* ) "\n" ) ;
    }
    else
    {
        Printf ( ( byte* ) "\n%s : WordDisassemble : Can't find word code at this (0x%x) address.\n", c_dd ( Context_Location ( ) ), ( uint ) address ) ;
    }
}

void
Debugger_WDis ( Debugger * debugger )
{
    Printf ( ( byte* ) "\n" ) ;
    Word * word = debugger->w_Word ;
    if ( ! word ) word = _Q_->OVT_Interpreter->w_Word ;
    if ( word )
    {
        Printf ( ( byte* ) "\nWord : %s : disassembly :> \n", word->Name ) ;
        _CfrTil_Word_Disassemble ( word ) ;
    }
    Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_Disassemble ( )
{
    uint number = _DataStack_Pop ( ) ;
    byte * address = ( byte* ) _DataStack_Pop ( ) ;
    _Debugger_Disassemble ( DEBUGGER, address, number, 0 ) ;
    Printf ( ( byte* ) "\n" ) ;
}


