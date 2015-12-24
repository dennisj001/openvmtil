
#include "../includes/cfrtil.h"

ud_t *
Debugger_UdisInit ( Debugger * debugger )
{
    if ( ! debugger->Udis ) debugger->Udis = ( ud_t* ) Mem_Allocate ( sizeof (ud_t ), CFRTIL ) ;
    return _Udis_Init ( debugger->Udis ) ;
}

int32
Debugger_Udis_GetInstructionSize ( Debugger * debugger )
{
    return _Udis_GetInstructionSize ( debugger->Udis, debugger->DebugAddress ) ;
}

int32
Debugger_UdisOneInstruction ( Debugger * debugger, byte * address, byte * prefix, byte * postfix )
{
    return _Udis_OneInstruction ( debugger->Udis, address, prefix, postfix ) ;
}

void
_Debugger_Disassemble ( Debugger * debugger, byte* address, int32 number, int32 cflag )
{
    _Udis_Disassemble ( Debugger_UdisInit ( debugger ), address, number, cflag, debugger->DebugAddress ) ;
    debugger->LastDisHere = address ;
}

void
Debugger_Disassemble ( Debugger * debugger, byte * format, byte * address )
{
    Printf ( ( byte* ) format, address ) ;
    _Debugger_Disassemble ( debugger, address, Here - address, 0 ) ;
}

void
Debugger_Dis ( Debugger * debugger )
{
    Printf ( ( byte* ) "\n" ) ;
    Debugger_GetWordFromAddress ( debugger ) ;
    Word * word = debugger->w_Word ;
    if ( word )
    {
        if ( word->S_CodeSize )
        {
            Printf ( ( byte* ) "\rDisassembly of : %s.%s\n", word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "", c_dd ( word->Name ) ) ;
            int32 codeSize = word->S_CodeSize ;
            _Debugger_Disassemble ( debugger, ( byte* ) word->CodeStart, codeSize ? codeSize : 64, word->CType & ( CPRIMITIVE | DLSYM_WORD ) ? 1 : 0 ) ;
            if ( debugger->DebugAddress )
            {
                Printf ( ( byte* ) "\nNext instruction ..." ) ;
                Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\n", ( byte* ) "" ) ; // the next instruction
            }
        }
    }
    else
    {
        word = _Q_->OVT_Context->CurrentRunWord ;
        if ( word ) Printf ( ( byte* ) "\rDisassembly of : %s.%s : has no code size! Disassembling accumulated ...\n", word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "", c_dd ( word->Name ) ) ;
        Debugger_DisassembleAccumulated ( debugger ) ;
    }
    Printf ( ( byte* ) "\n" ) ;
}

// a function of PreHere, OptimizedCodeAffected FirstDisAddress

void
_Debugger_DisassembleWrittenCode ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    int32 codeSize ;
    byte * optimizedCode ;
    if ( ( debugger->LastShowWord == word ) || ( debugger->LastDisHere == debugger->PreHere ) )return ;
    optimizedCode = debugger->OptimizedCodeAffected ;
    if ( optimizedCode )
    {
        if ( optimizedCode < debugger->PreHere ) debugger->PreHere = optimizedCode ;
    }
    codeSize = Here - debugger->PreHere ;
    if ( word && ( codeSize > 0 ) )
    {
        ConserveNewlines ;
        //if ( ! debugger->FirstDisAddress ) debugger->FirstDisAddress = debugger->PreHere ;
        byte * csName = (byte *) cc ( Get_CompilerSpace ( )->OurNBA->NBA_Name, &_Q_->Default ) ;
        Printf ( ( byte* ) "\nCode compiled to %s for word :> %s <: ...\n", csName, cc ( String_CB ( word->Name ), &_Q_->Default ) ) ;
        _Debugger_Disassemble ( debugger, debugger->PreHere, codeSize, word->CType & ( CPRIMITIVE | DLSYM_WORD ) ? 1 : 0 ) ;
    }
}

// this needs work -- some of these options are not necessary ?! or useful at all

void
Debugger_DisassembleAccumulated ( Debugger * debugger )
{
    int32 size ;
    byte * spformat, *address ;
    byte buffer [256] ;
    address = debugger->PreHere ;
    if ( ( size = Here - address ) <= 0 )
    {
        address = debugger->StartHere ;
        if ( ( size = Here - address ) <= 0 )
        {
            if ( size == 0 ) goto done ;
            address = debugger->DebugAddress ;
            if ( ( size = Here - address ) <= 0 )
            {
                address = debugger->LastDisHere ;
                if ( ( size = Here - address ) <= 0 )
                {
done:
                    Printf ( ( byte* ) "\nNo accumulated code. Key 'A' for code accumulated since start of this compile" ) ;
                    return ;
                }
            }
        }
    }
    spformat = (byte*) "\nDisassembling %d bytes of code accumulated since start of %s at : 0x%08x ...\n" ;
    if ( debugger->w_Word ) snprintf ( (char*) buffer, 256, (char*) spformat, size, (char*) debugger->w_Word->Name ) ;
    Debugger_Disassemble ( debugger, buffer, address ) ;
    Printf ( ( byte* ) "\n" ) ;
}

void
Debugger_DisassembleTotalAccumulated ( Debugger * debugger )
{
    Printf ( ( byte* ) "\nDisassembling the word's current total accumulated code ...\n" ) ;
    byte * address = _Q_->OVT_Context->Compiler0->InitHere ;
    int32 size = Here - address ;
    _Debugger_Disassemble ( debugger, address, size, 0 ) ;
    Printf ( ( byte* ) "\n" ) ;
}

