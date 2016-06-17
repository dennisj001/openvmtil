
#include "../../includes/cfrtil.h"

void
_Udis_PrintInstruction ( ud_t * ud, byte * address, byte * prefix, byte * postfix, byte * debugAddress )
{
    //                                      //prefix <addr> <code hex> <code disassembly> <call/jmp naming>
    byte buffer [ 128 ], *format = ( byte* ) "%s0x%-12x\t% -17s%-15s\t%s\n" ;
    postfix = GetPostfix ( address, postfix, buffer ) ; // buffer is returned as postfix by GetPostfix
    if ( address != debugAddress ) format = ( byte* ) c_ud ( format ) ;
    Printf ( format, prefix, ( int32 ) ud_insn_off ( ud ), ud_insn_hex ( ud ), ud_insn_asm ( ud ), postfix ) ;
}

int32
_Udis_GetInstructionSize ( ud_t * ud, byte * address )
{
    ud_set_input_buffer ( ud, address, 16 ) ;
    ud_set_pc ( ud, ( int32 ) address ) ;
    int32 isize = ud_disassemble ( ud ) ;
    return isize ;
}

ud_t *
_Udis_Init ( ud_t * ud )
{
#if 1 
    ud_init ( ud ) ;
    ud_set_mode ( ud, 32 ) ;
    ud_set_syntax ( ud, UD_SYN_INTEL ) ;
#else  
    //pud_init = ( void(* ) ( ud_t * ) ) _Dlsym ( "libudis86", "ud_init" ) ;
    //pud_init ( ud ) ;
    ( ( void(* ) ( ud_t * ) ) _Dlsym ( "ud_init", "/usr/local/lib/libudis86.so" ) )( ud ) ;
    ( ( void (* )( struct ud*, uint8_t ) ) _Dlsym ( "ud_set_mode", "/usr/local/lib/libudis86.so" ) ) ( ud, 32 ) ;
    ( ( void (* )( struct ud*, void (* )( struct ud* ) ) ) _Dlsym ( "ud_set_syntax", "/usr/local/lib/libudis86.so" ) ) ( ud, UD_SYN_INTEL ) ;
#endif   
    return ud ;
}

int32
_Udis_OneInstruction ( ud_t * ud, byte * address, byte * prefix, byte * postfix )
{
    if ( address )
    {
        int32 isize ;
        ud_set_input_buffer ( ud, address, 16 ) ;
        ud_set_pc ( ud, ( int32 ) address ) ;
        isize = ud_disassemble ( ud ) ;
        _Udis_PrintInstruction ( ud, address, prefix, postfix, _Debugger_->DebugAddress ) ;
        return isize ;
    }
    return 0 ;
}

void
_Udis_Disassemble ( ud_t *ud, byte* address, int32 number, int32 cflag, byte * debugAddress )
{
    if ( address )
    {
        char * iasm ;
        int32 isize = 0 ;
        //if ( number > 1024 ) number = 1024 ;
        ud_set_input_buffer ( ud, ( byte* ) address, number ) ;
        ud_set_pc ( ud, ( int32 ) address ) ;
        while ( ( number -= isize ) > 0 )
        {
            isize = ud_disassemble ( ud ) ;
            iasm = ( char* ) ud_insn_asm ( ud ) ;
            address = ( byte* ) ( int32 ) ud_insn_off ( ud ) ;
            _Udis_PrintInstruction ( ud, address, ( byte* ) "", ( byte* ) "", debugAddress ) ;
            if ( cflag && ( ! ( stricmp ( ( byte* ) "ret", ( byte* ) iasm ) ) ) ) break ; //isize = 1024 ; // cause return after next print insn
        }
    }
    Printf ( ( byte* ) "\n" ) ;
}

