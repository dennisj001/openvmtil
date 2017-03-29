
#include "../include/cfrtil.h"

// we have the address of a jcc insn 
// get the address it jccs to

byte *
JccInstructionAddress_2Byte ( byte * address )
{
    int32 offset = * ( int32* ) ( address + 2 ) ; // 2 : 2 byte opCode
    byte * jcAddress = address + offset + 6 ; // 6 : sizeof 0f jcc insn - 0x0f8x - includes 2 byte opCode
    return jcAddress ;
}

byte *
JccInstructionAddress_1Byte ( byte * address )
{
    int32 offset = ( int32 ) * ( byte* ) ( address + 1 ) ; // 1 : 1 byte opCode
    byte * jcAddress = address + offset + 2 ; // 2 : sizeof 0f jcc insn - 0x7x - includes 1 byte opCode
    return jcAddress ;
}

// we have the address of a jmp/call insn 
// get the address it jmp/calls to

byte *
JumpCallInstructionAddress ( byte * address )
{
    // calculate jmp or call address
    int offset = * ( int32* ) ( address + 1 ) ; // 1 : 1 byte opCode
    byte * jcAddress = address + offset + 5 ; // 5 : sizeof jmp insn - includes 1 byte opcode
    return jcAddress ;
}

void
_CfrTil_ACharacterDump ( char aChar )
{
    if ( isprint ( aChar ) )
    {
        _Printf ( ( byte* ) "%c", aChar ) ;
    }
    else _Printf ( ( byte* ) "." ) ;
}

void
CfrTil_CharacterDump ( byte * address, int32 number )
{
    int32 i ;
    for ( i = 0 ; i < number ; i ++ )
    {

        _CfrTil_ACharacterDump ( address [ i ] ) ;
    }
    _Printf ( ( byte* ) " " ) ;
}

void
_CfrTil_AByteDump ( byte aByte )
{

    _Printf ( ( byte* ) "%02x ", aByte ) ;
}

void
CfrTil_NByteDump ( byte * address, int32 number )
{
    int32 i ;
    for ( i = 0 ; i < number ; i ++ )
    {

        _CfrTil_AByteDump ( address [ i ] ) ;
    }
    _Printf ( ( byte* ) " " ) ;
}

byte *
GetPostfix ( byte * address, byte* postfix, byte * buffer )
{
    byte * iaddress ;
    char * prePostfix = ( char* ) "\t" ;
    if ( ( * address == JMPI32 ) || ( * address == CALLI32 ) )
    {
        int32 offset = * ( ( int32 * ) ( address + 1 ) ) ;
        iaddress = address + offset + 1 + CELL ;
    }
    else if ( ( ( * address == 0x0f ) && ( ( * ( address + 1 ) >> 4 ) == 0x8 ) ) )
    {
        int32 offset = * ( ( int32 * ) ( address + 2 ) ) ;
        iaddress = address + offset + 2 + CELL ;
    }
    else return postfix ;
    {
        Word * word = Word_GetFromCodeAddress ( iaddress ) ;
        if ( word )
        {
            byte * name = ( byte* ) c_dd ( word->Name ) ; //, &_Q_->Default ) ;
            if ( ( byte* ) word->CodeStart == iaddress )
            {
                snprintf ( ( char* ) buffer, 128, "%s< %s.%s >%s", prePostfix, word->ContainingNamespace->Name, name, postfix ) ;
            }
            else
            {
                snprintf ( ( char* ) buffer, 128, "%s< %s.%s+%d >%s", prePostfix,
                    word->ContainingNamespace->Name, name, iaddress - ( byte* ) word->CodeStart, postfix ) ;
            }
        }
        else snprintf ( ( char* ) buffer, 128, "%s< %s >", prePostfix, ( char * ) "C compiler code" ) ;
        postfix = buffer ;
    }
    return postfix ;
}

void
Compile_Debug_GetESP () // where we want the acquired pointer
{
#if 0    
    // ! nb : x86 cant do rm offset with ESP reg directly so use EAX
    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) where, CELL ) ;
    //_Compile_Move_Reg_To_Reg ( EAX, ESP ) ;
    _Compile_Move_Reg_To_Rm ( EAX, ESP, 0 ) ;
#endif    
    //_Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _CfrTil_->cs_CpuState->Ebp, EBP, EBX ) ; // ebp
    //_Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _CfrTil_->cs_CpuState->Esp, ESP, EBX ) ; // esp 
    _Compile_PushReg ( EBX ) ;
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _Debugger_->DebugESP, ESP, EBX ) ; // esp 
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _Debugger_->DebugEBP, EBP, EBX ) ; // esp 
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _Debugger_->DebugESI, ESI, EBX ) ; // esi
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _Debugger_->DebugEDI, EDI, EBX ) ; // edi
    _Compile_PopToReg ( EBX ) ;
}

void
_Compile_DebugRuntimeBreakpoint ( ) // where we want the acquired pointer
{
    Compile_Debug_GetESP () ;
    Compile_Call ( ( byte* ) _Debugger_->SaveCpuState ) ;
    Compile_Call ( ( byte* ) _CfrTil_->SaveCpuState ) ;
    Compile_Call ( ( byte* ) CfrTil_DebugRuntimeBreakpoint ) ;
}

#if 0
void
_Compile_Pause ( )
{
    _Compile_Debug_GetESP ( ( int* ) & _Debugger_->DebugESP ) ;
    Compile_Call ( ( byte* ) _Debugger_->SaveCpuState ) ;
    Compile_Call ( ( byte* ) _CfrTil_->SaveCpuState ) ;
    Compile_Call ( ( byte* ) OpenVmTil_Pause ) ;
}
#endif