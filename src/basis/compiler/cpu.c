#include "../../include/cfrtil.h"

void
_CpuState_Show ( CpuState * cpu )
{
    _Printf ( ( byte* ) "\nEAX 0x%08x", cpu->Eax ) ;
    _Printf ( ( byte* ) " ECX 0x%08x", cpu->Ecx ) ;
    _Printf ( ( byte* ) " EDX 0x%08x", cpu->Edx ) ;
    _Printf ( ( byte* ) " EBX 0x%08x", cpu->Ebx ) ;
    _Printf ( ( byte* ) "\nESP 0x%08x", cpu->Esp ) ;
    _Printf ( ( byte* ) " EBP 0x%08x", cpu->Ebp ) ;
    _Printf ( ( byte* ) " ESI 0x%08x", cpu->Esi ) ;
    _Printf ( ( byte* ) " EDI 0x%08x", cpu->Edi ) ;
    if ( _Debugger_->DebugAddress ) _Printf ( ( byte* ) "\nEIP 0x%08x", _Debugger_->DebugAddress ) ;
    else _Printf ( ( byte* ) "\nEIP 0x%08x", cpu->Eip ) ;
    _Printf ( ( byte* ) " EFlags :: 0x%-8x :: ", cpu->EFlags ) ;
    Print_Binary ( (uint32) cpu->EFlags, 14, 14 ) ;
    _Printf ( ( byte* ) " :: of:11 %d sf:7:%d, zf:6:%d, af:4:%d, pf:2:%d, cf:0:%d :: flag:bit-position:value",
        (uint32) cpu->EFlags & OVERFLOW_FLAG ? 1 : 0, (uint32) cpu->EFlags & SIGN_FLAG ? 1 : 0, (uint32) cpu->EFlags & ZERO_FLAG ? 1 : 0,
        (uint32) cpu->EFlags & AUX_FLAG ? 1 : 0, (uint32) cpu->EFlags & PARITY_FLAG ? 1 : 0, (uint32) cpu->EFlags & CARRY_FLAG ? 1 : 0
        ) ;
}
#if 0
void
_Compile_CpuState_Save ( CpuState * cpu )
{
    // push order for pushad
    // nb : intel stacks grow down toward lesser memory by subtracting from ESP to push and adding to pop
    // SoftwareDevelopersManual-253665.pdf, section 6.2
    // registers are pushed in this order ...
    // eax, ecx, edx, ebx, esp, ebp, esi, edi
    
    _Compile_PushAD ( ) ; // save all regs
    _Compile_PushFD ( ) ; // save flags
    
    // now store them in the cpu struct
    _Compile_MoveImm_To_Reg ( EBX, 0, CELL ) ; // clear for clean take of flags 
    _Compile_PopToReg ( EBX ) ; //
    //_Compile_Lahf ( ) ; // doesn't work with core 2 duo
    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->EFlags, CELL ) ;
    _Compile_Move_Reg_To_Rm ( EAX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Edi, CELL ) ;
    _Compile_PopToReg ( EBX ) ;
    _Compile_Move_Reg_To_Rm ( EAX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Esi, CELL ) ;
    _Compile_PopToReg ( EBX ) ;
    _Compile_Move_Reg_To_Rm ( EAX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Ebp, CELL ) ;
    _Compile_PopToReg ( EBX ) ;
    _Compile_Move_Reg_To_Rm ( EAX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Esp, CELL ) ;
    _Compile_PopToReg ( EBX ) ;
    _Compile_Move_Reg_To_Rm ( EAX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Ebx, CELL ) ; // must be done here - in order :: edi, esi, ebp, esp, ebx, edx, ecx, eax - reversed from how they are pushed
    _Compile_PopToReg ( EBX ) ;
    _Compile_Move_Reg_To_Rm ( EAX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Edx, CELL ) ;
    _Compile_PopToReg ( EBX ) ;
    _Compile_Move_Reg_To_Rm ( EAX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Ecx, CELL ) ;
    _Compile_PopToReg ( EBX ) ;
    _Compile_Move_Reg_To_Rm ( EAX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Eax, CELL ) ;
    _Compile_PopToReg ( EBX ) ;
    _Compile_Move_Reg_To_Rm ( EAX, EBX, 0 ) ;

    _Compile_SetAddress_ThruReg ( (int32) & cpu->State, 1, EBX ) ;
   
    _Compile_Return ( ) ; 
}
#else
void
_Compile_CpuState_Save ( CpuState * cpu )
{
    // push order for pushad
    // nb : intel stacks grow down toward lesser memory by subtracting from ESP to push and adding to pop
    // SoftwareDevelopersManual-253665.pdf, section 6.2
    // registers are pushed in this order ...
    // eax, ecx, edx, ebx, esp, ebp, esi, edi

    _Compile_PushReg ( EAX ) ; // scratch reg2b
    _Compile_PushReg ( EBX ) ; // scratch reg0
    _Compile_PushReg ( EBX ) ; // scratch reg1
    _Compile_MoveRegToAddress_ThruReg ( ( byte* ) & cpu->Eax, EAX, EBX ) ;

    _Compile_PushFD ( ) ; // save flags
    // now store them in the cpu struct
    _Compile_MoveImm_To_Reg ( EBX, 0, CELL ) ; // clear for clean take of flags 
    _Compile_PopToReg ( EBX ) ; // now has eflags
    //_Compile_Lahf ( ) ; // doesn't work with core 2 duo
    _Compile_PushReg ( EAX ) ; // scratch reg2a
    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->EFlags, CELL ) ;
    _Compile_Move_Reg_To_Rm ( EAX, EBX, 0 ) ; // ebx has eflags
    _Compile_PopToReg ( EAX ) ; // restore scratch reg2a

    _Compile_MoveRegToAddress_ThruReg ( ( byte* ) & cpu->Edi, EDI, EBX ) ;
    _Compile_MoveRegToAddress_ThruReg ( ( byte* ) & cpu->Esi, ESI, EBX ) ;
    _Compile_MoveRegToAddress_ThruReg ( ( byte* ) & cpu->Edx, EDX, EBX ) ;
    _Compile_MoveRegToAddress_ThruReg ( ( byte* ) & cpu->Ecx, ECX, EBX ) ;
    _Compile_MoveRegToAddress_ThruReg ( ( byte* ) & cpu->Ebp, EBP, EBX ) ;
    _Compile_MoveRegToAddress_ThruReg ( ( byte* ) & cpu->Esp, ESP, EBX ) ; // this won't be accurate for the runtime because it is called from C 
    
    _Compile_SetAddress_ThruReg ( ( int32 ) & cpu->State, 1, EBX ) ; // mark this CpuState as having been saved

    _Compile_PopToReg ( EBX ) ; // restore scratch reg1
    _Compile_MoveRegToAddress_ThruReg ( ( byte* ) & cpu->Ebx, EBX, EAX ) ;
    _Compile_PopToReg ( EBX ) ; // restore scratch reg0
    _Compile_PopToReg ( EAX ) ; // restore scratch reg2b

    //_Compile_Return ( ) ;
}
#endif

void
_Compile_CpuState_Restore ( CpuState * cpu )
{
    //_Compile_PushReg ( EBX ) ; // scratch reg0 // no need to push it - it is restored last
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->State, CELL ) ; // check to see if the registers have actually been saved by _Compile_CpuState_Save
    _Compile_Move_Rm_To_Reg ( EBX, EBX, 0 ) ;
    _Compile_TEST_Reg_To_Reg ( EBX, EBX ) ;
    Compile_JCC ( NZ, ZERO_TTT, Here + 6 ) ;
    //_Compile_PopToReg ( EBX ) ; // restore scratch reg
    _Compile_Return ( ) ; // x86 - return opcode

    _Compile_MoveAddressValueToReg_ThruReg ( EDI, (byte*) & cpu->Edi, EBX ) ;
    _Compile_MoveAddressValueToReg_ThruReg ( ESI, (byte*) & cpu->Esi, EBX ) ;

#if 0 // ebp & esp can't be restored here or a ret insn will return to the wrong place
    _Compile_MoveAddressValueToReg_ThruReg ( EBP, (byte*) & cpu->Ebp, EBX ) ;
    _Compile_MoveAddressValueToReg_ThruReg ( ESP, (byte*) & cpu->Esp, EBX ) ;
#endif

    _Compile_MoveAddressValueToReg_ThruReg ( EDX, (byte*) & cpu->Edx, EBX ) ;
    _Compile_MoveAddressValueToReg_ThruReg ( ECX, (byte*) & cpu->Ecx, EBX ) ;
    _Compile_MoveAddressValueToReg_ThruReg ( EAX, (byte*) & cpu->Eax, EBX ) ;
    
    //_Compile_MoveImm_To_Reg ( EBX, 0, CELL ) ; // clear for clean take of flags 
    _Compile_MoveAddressValueToReg_ThruReg ( EBX, (byte*) & cpu->EFlags, EBX ) ;
    _Compile_PushReg ( EBX ) ; // the flags
    _Compile_PopFD ( ) ; // pops the pushed flags in ebx to flags reg
    
    _Compile_SetAddress_ThruReg ( ( int32 ) & cpu->State, 0, EBX ) ;
    _Compile_MoveAddressValueToReg_ThruReg ( EBX, (byte*) & cpu->Ebx, EBX ) ; // finally restore the scratch reg EBX
    
    //_Compile_Return ( ) ;
}

CpuState *
_CpuState_Copy ( CpuState *dst, CpuState * src )
{
    memcpy ( dst, src, sizeof ( CpuState ) ) ;
    return dst ;
}

CpuState *
CpuState_Copy ( CpuState * cpu0, uint32 type )
{
    CpuState * cpu = CpuState_New ( type ) ;
    //memcpy ( cpu, cpu0, sizeof ( CpuState ) ) ;
    _CpuState_Copy ( cpu, cpu0 ) ;
    return cpu ;
}

CpuState *
CpuState_New ( uint32 type )
{
    CpuState * cpu ;
    cpu = ( CpuState * ) Mem_Allocate ( sizeof (CpuState ), type ) ;
    return cpu ;
}

CpuState *
_CpuState_Save ()
{
    CpuState *svCpu = _CfrTil_->cs_CpuState, * newCpu ;
    _CfrTil_->cs_CpuState = newCpu = CpuState_New ( TEMPORARY ) ;
    _CfrTil_->SaveCpuState () ;
    _CfrTil_->cs_CpuState = svCpu ;
    return newCpu ;
}   

void
_CpuState_Restore ( CpuState * cpu )
{
    CpuState *svCpu = CpuState_Copy ( _CfrTil_->cs_CpuState, TEMPORARY ) ;
    _CfrTil_->cs_CpuState = cpu ;
    _CfrTil_->RestoreCpuState () ;
    _CfrTil_->cs_CpuState = svCpu ;
}   

void
CpuState_Save ()
{
    CpuState *newCpu = _CpuState_Save () ;
    _DataStack_Push ( (int32) newCpu ) ;
}

void
CpuState_Restore ( )
{
   CpuState *cpu = (CpuState *) _DataStack_Pop () ;
   _CpuState_Restore ( cpu ) ;
}