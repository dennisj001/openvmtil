
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
        _Debugger_Disassemble ( _Debugger_, start, word->S_CodeSize ? word->S_CodeSize : 128, 0 ) ;
    }
}

void
CfrTil_Word_Disassemble ( )
{
    //Word * word = Word_GetFromCodeAddress_NoAlias ( address ) ;
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    //byte * address = ( byte* ) word->Definition ;
    if ( word )
    {
        Printf ( ( byte* ) "\nWord : %s : disassembly :> \n", c_dd ( word->Name ) ) ;
        _CfrTil_Word_Disassemble ( word ) ;
        Printf ( ( byte* ) "\n" ) ;
    }
    else
    {
        Printf ( ( byte* ) "\n%s : WordDisassemble : Can't find word code at this (0x%x) address.\n", c_dd ( Context_Location ( ) ), ( uint ) word->Definition ) ;
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
    _Debugger_Disassemble ( _Debugger_, address, number, 0 ) ;
    Printf ( ( byte* ) "\n" ) ;
}


