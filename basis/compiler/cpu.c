#include "../../includes/cfrtil.h"

void
_CpuState_Show ( CpuState * cpu )
{
    Printf ( ( byte* ) "\nEAX 0x%08x", cpu->Eax ) ;
    Printf ( ( byte* ) " ECX 0x%08x", cpu->Ecx ) ;
    Printf ( ( byte* ) " EDX 0x%08x", cpu->Edx ) ;
    Printf ( ( byte* ) " EBX 0x%08x", cpu->Ebx ) ;
    Printf ( ( byte* ) "\nESP 0x%08x", cpu->Esp ) ;
    Printf ( ( byte* ) " EBP 0x%08x", cpu->Ebp ) ;
    Printf ( ( byte* ) " ESI 0x%08x", cpu->Esi ) ;
    Printf ( ( byte* ) " EDI 0x%08x", cpu->Edi ) ;
    if ( _Debugger_->DebugAddress ) Printf ( ( byte* ) "\nEIP 0x%08x", _Debugger_->DebugAddress ) ;
    else Printf ( ( byte* ) "\nEIP 0x%08x", cpu->Eip ) ;
    Printf ( ( byte* ) " EFlags :: 0x%-8x :: ", cpu->EFlags ) ;
    Print_Binary ( cpu->EFlags, 14, 14 ) ;
    Printf ( ( byte* ) " :: of:11 %d sf:7:%d, zf:6:%d, af:4:%d, pf:2:%d, cf:0:%d :: flag:bit-position:value\n",
        cpu->EFlags & OVERFLOW_FLAG ? 1 : 0, cpu->EFlags & SIGN_FLAG ? 1 : 0, cpu->EFlags & ZERO_FLAG ? 1 : 0,
        cpu->EFlags & AUX_FLAG ? 1 : 0, cpu->EFlags & PARITY_FLAG ? 1 : 0, cpu->EFlags & CARRY_FLAG ? 1 : 0
        ) ;
}

#if 0 //working but seem able to be improved
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

void
_Compile_CpuState_Restore ( CpuState * cpu )
{
    // push order for pushad
    // nb : intel stacks grow down toward lesser memory by subtracting from ESP to push and adding to pop
    // SoftwareDevelopersManual-253665.pdf, section 6.2
    // registers are pushed in this order ...
    // eax, ecx, edx, ebx, esp, ebp, esi, edi

    _Compile_PushReg ( EBX ) ; // save scratch reg
    
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->State, CELL ) ; // check to see if the registers have actually been saved by _Compile_CpuState_Save
    _Compile_Move_Rm_To_Reg ( EBX, EBX, 0 ) ;
    _Compile_TEST_Reg_To_Reg ( EBX, EBX ) ; 
    Compile_JCC ( NZ, ZERO_CC, Here + 7 ) ; 
    _Compile_PopToReg ( EBX ) ; // restore scratch reg
    _Compile_Return ( ) ; // x86 - return opcode
    
    // get the flags first 
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->EFlags, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EBX, EBX, 0 ) ;
    _Compile_PushReg ( EBX ) ; // the flags
    _Compile_PopFD ( ) ; // pops the pushed flags to flags reg
    
    // register values have already been stored in cpu->Exx by _Compile_CpuState_Save 
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Edi, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EDI, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Esi, CELL ) ;
    _Compile_Move_Rm_To_Reg ( ESI, EBX, 0 ) ;

#if 0 // ebp & esp can't be restored or a ret insn will return to the wrong place
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Ebp, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EBP, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Esp, CELL ) ;
    _Compile_Move_Rm_To_Reg ( ESP, EBX, 0 ) ;
#endif

    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Edx, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EDX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Ecx, CELL ) ;
    _Compile_Move_Rm_To_Reg ( ECX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Eax, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;

    _Compile_SetAddress_ThruReg ( (int32) & cpu->State, 0, EBX ) ;
    
    _Compile_PopToReg ( EBX ) ; // restore scratch reg
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Ebx, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EBX, EBX, 0 ) ;


    _Compile_Return ( ) ; 
}
#elif 1
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

void
_Compile_CpuState_Restore ( CpuState * cpu )
{
    // push order for pushad
    // nb : intel stacks grow down toward lesser memory by subtracting from ESP to push and adding to pop
    // SoftwareDevelopersManual-253665.pdf, section 6.2
    // registers are pushed in this order ...
    // eax, ecx, edx, ebx, esp, ebp, esi, edi

    _Compile_PushReg ( EBX ) ; // save scratch reg
    
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->State, CELL ) ; // check to see if the registers have actually been saved by _Compile_CpuState_Save
    _Compile_Move_Rm_To_Reg ( EBX, EBX, 0 ) ;
    _Compile_TEST_Reg_To_Reg ( EBX, EBX ) ; 
    Compile_JCC ( NZ, ZERO_CC, Here + 7 ) ; 
    _Compile_PopToReg ( EBX ) ; // restore scratch reg
    _Compile_Return ( ) ; // x86 - return opcode
    
    // get the flags first 
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->EFlags, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EBX, EBX, 0 ) ;
    _Compile_PushReg ( EBX ) ; // the flags
    _Compile_PopFD ( ) ; // pops the pushed flags to flags reg
    
    // register values have already been stored in cpu->Exx by _Compile_CpuState_Save 
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Edi, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EDI, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Esi, CELL ) ;
    _Compile_Move_Rm_To_Reg ( ESI, EBX, 0 ) ;

#if 0 // ebp & esp can't be restored or a ret insn will return to the wrong place
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Ebp, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EBP, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Esp, CELL ) ;
    _Compile_Move_Rm_To_Reg ( ESP, EBX, 0 ) ;
#endif

    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Edx, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EDX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Ecx, CELL ) ;
    _Compile_Move_Rm_To_Reg ( ECX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Eax, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;

    _Compile_SetAddress_ThruReg ( (int32) & cpu->State, 0, EBX ) ;
    
    _Compile_PopToReg ( EBX ) ; // pop to adjust the stack -- we pushed EBX in the beginining
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Ebx, CELL ) ; // restore EBX after stack adjust pop
    _Compile_Move_Rm_To_Reg ( EBX, EBX, 0 ) ;

    _Compile_Return ( ) ; 
}
#endif

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

#if 0

void
_Compile_CpuState_Restore_EbpEsp ( CpuState * cpu )
{
    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Ebp, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EBP, EAX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Esp, CELL ) ;
    _Compile_Move_Rm_To_Reg ( ESP, EAX, 0 ) ;
}

void
Compile_Debugger_CpuState_Entry_Restore_EbpEsp ( )
{
    _Compile_CpuState_Restore_EbpEsp ( _Debugger_->cs_CpuState_Entry ) ;
}
#endif


