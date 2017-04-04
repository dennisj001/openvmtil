
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

int32
_CheckForString ( byte * address, int32 maxLength )
{
    int32 i, flag ;
    for ( i = 0, flag = 0 ; i < maxLength ; i ++ )
    {
        if ( ( address [i] > 128 ) ) return false ;
        else flag = 1 ;
        if ( flag && address [i] ) return true ;
    }
    return true ;
}

byte *
CheckForString ( byte * address )
{
    if ( _CheckForString ( address, 64 ) ) return address ;
    else return 0 ;
}

byte *
GetPostfix ( byte * address, byte* postfix, byte * buffer )
{
    byte * iaddress = 0, *str;
    Word * word = 0, *dbgWord = _Debugger_->w_Word ;
    char * prePostfix = ( char* ) "  \t" ;
    if ( iaddress = Calculate_Address_FromOffset_ForCallOrJump ( address ) )
    {
        if ( dbgWord && ( Is_NamespaceType ( dbgWord ) ) )
        {
            word = Finder_Address_FindInOneNamespace ( _Finder_, dbgWord->S_ContainingNamespace, iaddress ) ;
        }
        if ( ! word ) word = Word_GetFromCodeAddress ( iaddress ) ;
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
                    word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "", name, iaddress - ( byte* ) word->CodeStart, postfix ) ;
            }
        }
        else snprintf ( ( char* ) buffer, 128, "%s< %s >", prePostfix, ( char * ) "C compiler code" ) ;
        postfix = buffer ;
    }
#if 1    
    //else if ( NamedByteArray_CheckAddress ( _Q_->MemorySpace0->StringSpace, (byte*) *( (uint32*) (address + 2) ) ) && CheckForString ( (byte*) *( (uint32*) (address + 2) ) ) ) 
    else if ( str = String_CheckForAtAdddress ( (byte*) *( (uint32*) (address + 2) ) ) ) 
    {
        snprintf ( ( char* ) buffer, 128, "%s%s", prePostfix, str ) ;
        postfix = buffer ;
    }
#endif    
    return postfix ;
}

void
Compile_Debug_GetESP ( ) // where we want the acquired pointer
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
    Compile_Debug_GetESP ( ) ;
    //Compile_Call ( ( byte* ) CfrTil_Debugger_SaveCpuState ) ; //_Debugger_->SaveCpuState ) ;
    //Compile_Call ( ( byte* ) _CfrTil_->SaveCpuState ) ;
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