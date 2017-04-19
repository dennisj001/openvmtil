
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
    //if ( debugger->w_Word && debugger->w_Word->DebugWordList ) //debugger->w_Word && debugger->w_Word->DebugWordList )
    if ( debugger->DebugWordList ) //debugger->DebugWordListWord && debugger->DebugWordList ) 
    {
        _Printf ( ( byte* ) "%s", prefix ) ;
        _Debugger_ShowSourceCodeAtAddress ( debugger ) ;
        prefix = ( byte* ) "" ;
    }
    return _Debugger_Udis_OneInstruction ( debugger, address, prefix, postfix ) ;
}

int32
_Debugger_Disassemble ( Debugger * debugger, byte* address, int32 number, int32 cflag )
{
    int32 size = _Udis_Disassemble ( Debugger_UdisInit ( debugger ), address, ( ( number > 2 * K ) ? 2 * K : number ), 1 ) ;
    debugger->LastDisStart = address ;
    return size ;
}

void
Debugger_Disassemble ( Debugger * debugger, byte * format, byte * address )
{
    _Printf ( ( byte* ) format, address ) ;
    _Debugger_Disassemble ( debugger, address, Here - address, 1 ) ;
}

void
Debugger_Dis ( Debugger * debugger )
{
    Debugger_GetWordFromAddress ( debugger ) ;
    Word * word = debugger->w_Word ;
    if ( ( word ) && ( word->S_CodeSize ) )
    {
        _Printf ( ( byte* ) "\rDisassembly of : %s.%s", c_ud ( word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "" ), c_dd ( word->Name ) ) ;
        int32 codeSize = word->S_CodeSize ;
        _Debugger_Disassemble ( debugger, ( byte* ) word->CodeStart, codeSize ? codeSize : 64, word->CProperty & ( CPRIMITIVE | DLSYM_WORD ) ? 1 : 0 ) ;
        if ( debugger->DebugAddress )
        {
            _Printf ( ( byte* ) "\nNext instruction ..." ) ;
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\n", ( byte* ) "" ) ; // the next instruction
        }
    }
    else
    {
        word = _Context_->CurrentlyRunningWord ;
        if ( word ) _Printf ( ( byte* ) "\rDisassembly of : %s.%s : has no code size! Disassembling accumulated ...", c_ud ( word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "" ), c_dd ( word->Name ) ) ;
        Debugger_DisassembleAccumulated ( debugger ) ;
    }
}

// a function of PreHere, OptimizedCodeAffected FirstDisAddress

void
_Debugger_DisassembleWrittenCode ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    byte * optimizedCode = debugger->OptimizedCodeAffected ;
    if ( ( optimizedCode ) && ( optimizedCode < debugger->PreHere ) ) debugger->PreHere = optimizedCode ;
    int32 codeSize = Here - debugger->PreHere ;
    if ( codeSize )
    {
        if ( word && ( codeSize > 0 ) )
        {
            //ConserveNewlines ;
            byte * csName = ( byte * ) c_dd ( Get_CompilerSpace ( )->OurNBA->NBA_Name ) ;
            _Printf ( ( byte* ) "\nCode compiled to %s for word :> %s <: ...", csName, c_dd ( String_CB ( word->Name ) ) ) ;
            _Debugger_Disassemble ( debugger, debugger->PreHere, codeSize, word->CProperty & ( CPRIMITIVE | DLSYM_WORD ) ? 1 : 0 ) ;
            //debugger->PreHere = Here ;
        }
    }
}

// this needs work -- some of these options are not necessary ?! or useful at all

void
Debugger_DisassembleAccumulated ( Debugger * debugger )
{
    //if ( Compiling )
    {
        byte * spformat ;
        byte buffer [256] ;
        spformat = ( byte* ) "\nDisassembling %d bytes of code accumulated since start with word \'%s\' at : 0x%08x ..." ;
        if ( debugger->EntryWord ) snprintf ( ( char* ) buffer, 256, ( char* ) spformat, Here - debugger->StartHere, ( char* ) debugger->EntryWord->Name, debugger->StartHere ) ;
        Debugger_Disassemble ( debugger, buffer, debugger->StartHere ) ;
    }
    //else _Printf ( ( byte* ) "\nDisassemble Accumulated is for only when compiling." ) ;
}

void
Debugger_DisassembleTotalAccumulated ( Debugger * debugger )
{
    _Printf ( ( byte* ) "\nDisassembling the current word : \'%s\' : total accumulated code ...", _Context_->Compiler0->CurrentWord ? _Context_->Compiler0->CurrentWord->Name : ( byte* ) "" ) ;
    byte * address = _Context_->Compiler0->InitHere ;
    int32 size = Here - address ;
    _Debugger_Disassemble ( debugger, address, size, 0 ) ;
    //_Printf ( ( byte* ) "\n" ) ;
}

