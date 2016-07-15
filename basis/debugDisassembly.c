
#include "../include/cfrtil.h"

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
    if ( debugger->w_Word && debugger->w_Word->DebugWordList ) 
    //if ( GetState ( _Q_->OVT_CfrTil, SOURCE_CODE_MODE ) )
    {
        Printf ( "%s", prefix ) ;
        prefix = "" ;
        _Debugger_ShowSourceCodeAtAddress ( debugger ) ;
    }
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
            Printf ( ( byte* ) "\rDisassembly of : %s.%s\n", c_ud ( word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "" ), c_dd ( word->Name ) ) ;
            int32 codeSize = word->S_CodeSize ;
            _Debugger_Disassemble ( debugger, ( byte* ) word->CodeStart, codeSize ? codeSize : 64, word->CProperty & ( CPRIMITIVE | DLSYM_WORD ) ? 1 : 0 ) ;
            if ( debugger->DebugAddress )
            {
                Printf ( ( byte* ) "\nNext instruction ..." ) ;
                Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\n", ( byte* ) "" ) ; // the next instruction
            }
        }
    }
    else
    {
        word = _Context_->CurrentlyRunningWord ;
        if ( word ) Printf ( ( byte* ) "\rDisassembly of : %s.%s : has no code size! Disassembling accumulated ...\n", c_ud ( word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "" ), c_dd ( word->Name ) ) ;
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
        byte * csName = ( byte * ) c_dd ( Get_CompilerSpace ( )->OurNBA->NBA_Name ) ;
        Printf ( ( byte* ) "\nCode compiled to %s for word :> %s <: ...\n", csName, c_dd ( String_CB ( word->Name ) ) ) ;
        _Debugger_Disassemble ( debugger, debugger->PreHere, codeSize, word->CProperty & ( CPRIMITIVE | DLSYM_WORD ) ? 1 : 0 ) ;
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
                    //Printf ( ( byte* ) "\nNo accumulated code. Key 'A' for code accumulated since start of this compile" ) ;
                    Debugger_DisassembleTotalAccumulated ( debugger ) ;
                    return ;
                }
            }
        }
    }
    spformat = ( byte* ) "\nDisassembling %d bytes of code accumulated since start at word \'%s\' at : 0x%08x ...\n" ;
    if ( debugger->EntryWord ) snprintf ( ( char* ) buffer, 256, ( char* ) spformat, size, ( char* ) debugger->EntryWord->Name ) ;
    Debugger_Disassemble ( debugger, buffer, address ) ;
    Printf ( ( byte* ) "\n" ) ;
}

void
Debugger_DisassembleTotalAccumulated ( Debugger * debugger )
{
    Printf ( ( byte* ) "\nDisassembling the current word : \'%s\' : total accumulated code ...\n", _Context_->Compiler0->CurrentWord ? _Context_->Compiler0->CurrentWord->Name : ( byte* ) "" ) ;
    byte * address = _Context_->Compiler0->InitHere ;
    int32 size = Here - address ;
    _Debugger_Disassemble ( debugger, address, size, 0 ) ;
    Printf ( ( byte* ) "\n" ) ;
}

