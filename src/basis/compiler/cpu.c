#include "../../include/cfrtil.h"

void
_CpuState_Show ( Cpu * cpu )
{
    byte * location = Context_Location ( ) ;
    if ( cpu == _Debugger_->cs_Cpu ) _Printf ( (byte*) "\nDebugger CpuState : at %s", location ) ;
    else _Printf ( (byte*) "\nC Runtime (_CfrTil_) CpuState :" ) ;
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
    Print_Binary ( ( uint64 ) cpu->EFlags, 14, 14 ) ;
    _Printf ( ( byte* ) " :: of:11 %d sf:7:%d, zf:6:%d, af:4:%d, pf:2:%d, cf:0:%d :: flag:bit:value",
        ( uint64 ) cpu->EFlags & OVERFLOW_FLAG ? 1 : 0, ( uint64 ) cpu->EFlags & SIGN_FLAG ? 1 : 0, ( uint64 ) cpu->EFlags & ZERO_FLAG ? 1 : 0,
        ( uint64 ) cpu->EFlags & AUX_FLAG ? 1 : 0, ( uint64 ) cpu->EFlags & PARITY_FLAG ? 1 : 0, ( uint64 ) cpu->EFlags & CARRY_FLAG ? 1 : 0
        ) ;
    d0 ( _PrintNStackWindow ( ( int64* ) cpu->Esp, "ReturnStack", "ESP", 8 ) ) ;
}

// save the incoming current cpu register state to the C struct
// we are mainly concerned here about what goes into the C struct
// but we have to be careful not to negatively affect the C compiler 
// runtime in the process

void
_Compile_CpuState_Save ( Cpu * cpu )
{
    // push order for pushad
    // nb : intel stacks grow down toward lesser memory by 
    // subtracting from ESP to push and adding to pop
    // SoftwareDevelopersManual-253665.pdf, section 6.2
    // registers are pushed in this order ...
    // eax, ecx, edx, ebx, esp, ebp, esi, edi

    // C compiler uses ebx alot so we use ecx for scatch
    _Compile_PushReg ( ECX ) ; // scratch reg1
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Eax, EAX, ECX ) ; // save eax
    _Compile_PopToReg ( ECX ) ; // restore scratch reg1 so we can save it
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Ecx, ECX, EAX ) ; // save ecx

    // eax and ebx are already save so we can use them freely
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Ebx, EBX, ECX ) ; // ebx
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Edx, EDX, ECX ) ; // edx

    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Esp, ESP, ECX ) ; // esp //this won't be accurate for the runtime because it is called from C 
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Ebp, EBP, ECX ) ; // ebp
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Esi, ESI, ECX ) ; // esi
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Edi, EDI, ECX ) ; // edi
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _Debugger_->DebugESP, ESP, ECX ) ; // esp //this won't be accurate for the runtime because it is called from C 
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _Debugger_->DebugEBP, EBP, ECX ) ; // ebp
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _Debugger_->DebugESI, ESI, ECX ) ; // esi
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & _Debugger_->DebugEDI, EDI, ECX ) ; // edi

    _Compile_PushFD ( ) ; // save flags
    _Compile_PopToReg ( ECX ) ; // ebx now has eflags
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->EFlags, ECX, EAX ) ; //flags

    _Compile_Set_C_LValue_WithImm_ThruReg ( ( int64 ) & cpu->State, CPU_STATE_SAVED, ECX ) ; // mark this CpuState as having been saved

    // restore our scratch regs so we leave things as we found them
    _Compile_Get_FromCAddress_ToReg ( EAX, ( byte* ) & cpu->Eax ) ; // ebx
    _Compile_Get_FromCAddress_ToReg ( ECX, ( byte* ) & cpu->Ecx ) ; // our scratch reg
    //_Compile_Return ( ) ; // ok but not necessary we add a return in DataObject_New
}

// we have the cpu register state stored in a C struct 
// accurately restore that struct into the cpu register state 
// so that the cpu register state is as saved in the C struct when we leave

void
_Compile_CpuState_Restore ( Cpu * cpu, int64 cStackRegFlag )
{
    // first check to see if the registers have actually been saved by _Compile_CpuState_Save

    _Compile_Get_FromCAddress_ToReg ( ECX, ( byte * ) & cpu->State ) ;
    _Compile_TEST_Reg_To_Reg ( ECX, ECX ) ;
    Compile_JCC ( NZ, ZERO_TTT, Here + 6 ) ;
    _Compile_Return ( ) ; // x86 - return opcode

    _Compile_Get_FromCAddress_ToReg ( EAX, ( byte* ) & cpu->Eax ) ; // eax
    _Compile_Get_FromCAddress_ToReg ( EBX, ( byte* ) & cpu->Ebx ) ; // ecx
    _Compile_Get_FromCAddress_ToReg ( EDX, ( byte* ) & cpu->Edx ) ; // edx

    _Compile_Get_FromCAddress_ToReg ( ESI, ( byte* ) & cpu->Esi ) ; // esi
    _Compile_Get_FromCAddress_ToReg ( EDI, ( byte* ) & cpu->Edi ) ; // edi
    if ( cStackRegFlag )
    {
        _Compile_Get_FromCAddress_ToReg_ThruReg ( ESP, ( byte * ) & cpu->Esp, ECX ) ;
        _Compile_Get_FromCAddress_ToReg_ThruReg ( EBP, ( byte * ) & cpu->Ebp, ECX ) ;
    }
    _Compile_Get_FromCAddress_ToReg ( ECX, ( byte* ) & cpu->EFlags ) ;
    _Compile_PushReg ( ECX ) ; // the flags
    _Compile_PopFD ( ) ; // pops the pushed flags in ebx to flags reg           // eflags

    _Compile_Get_FromCAddress_ToReg ( ECX, ( byte* ) & cpu->Ecx ) ; // ecx
    //_Compile_Return ( ) ;
}

void
Compile_CpuState_Restore ( Cpu * cpu )
{
    _Compile_CpuState_Restore ( cpu, 0 ) ;
}

Cpu *
_CpuState_Copy ( Cpu *dst, Cpu * src )
{
    memcpy ( dst, src, sizeof ( Cpu ) ) ;
    return dst ;
}

Cpu *
CpuState_Copy ( Cpu * cpu0, uint64 type )
{
    Cpu * cpu = CpuState_New ( type ) ;
    //memcpy ( cpu, cpu0, sizeof ( CpuState ) ) ;
    _CpuState_Copy ( cpu, cpu0 ) ;
    return cpu ;
}

Cpu *
CpuState_New ( uint64 type )
{
    Cpu * cpu ;
    cpu = ( Cpu * ) Mem_Allocate ( sizeof (Cpu ), type ) ;
    return cpu ;
}

Cpu *
_CpuState_Save ( )
{
    Cpu *svCpu = _CfrTil_->cs_Cpu, * newCpu ;
    _CfrTil_->cs_Cpu = newCpu = CpuState_New ( TEMPORARY ) ;
    _CfrTil_->SaveCpuState ( ) ;
    _CfrTil_->cs_Cpu = svCpu ;
    return newCpu ;
}

void
_CpuState_Restore ( Cpu * cpu )
{
    Cpu *svCpu = CpuState_Copy ( _CfrTil_->cs_Cpu, TEMPORARY ) ;
    _CfrTil_->cs_Cpu = cpu ;
    _CfrTil_->RestoreCpuState ( ) ;
    _CfrTil_->cs_Cpu = svCpu ;
}

void
CpuState_Save ( )
{
    Cpu *newCpu = _CpuState_Save ( ) ;
    _DataStack_Push ( ( int64 ) newCpu ) ;
}

void
CpuState_Restore ( )
{
    Cpu *cpu = ( Cpu * ) _DataStack_Pop ( ) ;
    _CpuState_Restore ( cpu ) ;
}

