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

#if 0
void
_Compile_CpuState_AdjustESI ( CpuState * cpu )
{
    _Compile_TEST_Reg_To_Reg ( ESI, ESI ) ; // generally a better test here may be needed
    Compile_JCC ( NZ, ZERO_CC, Here + 15 ) ; // to ret :: ?? jmp if z flag is 1 <== ( eax == 0  )
    _Compile_MoveImm_To_Reg ( ESI, ( int32 ) & _DataStackPointer_, CELL ) ;
    _Compile_Move_Rm_To_Reg ( ESI, ESI, 0 ) ;
    _Compile_Move_Reg_To_Reg ( EDI, ESI ) ;
}

void
_Compile_CpuState_AdjustEDI ( CpuState * cpu )
{
    _Compile_PushReg ( EBX ) ;
    _Compile_PushReg ( EAX ) ;
    _Compile_Move_Reg_To_Reg ( EBX, ESI ) ;
    _Compile_Move_Reg_To_Reg ( EAX, EDI ) ;
    _Compile_SUB_Reg_From_Reg ( EAX, EBX ) ; // generally a better test here may be needed
    //_Compile_TEST_Reg_To_Reg ( EAX, EAX ) ; // generally a better test here may be needed
    Compile_JCC ( LESS, NZ, Here + 7 ) ; // to ret :: ?? jmp if z flag is 1 <== ( eax == 0  )
    //_Compile_MoveImm_To_Reg ( ESI, ( int32 ) & _DataStackPointer_, CELL ) ;
    //_Compile_Move_Rm_To_Reg ( ESI, ESI, 0 ) ;
    _Compile_Move_Reg_To_Reg ( EDI, ESI ) ;
    _Compile_PopToReg ( EAX ) ;
    _Compile_PopToReg ( EBX ) ;
}
#endif

#if 0
void
_Compile_CpuState_Save ( CpuState * cpu )
{
    // push order for pushad
    // nb : intel stacks grow down toward lesser memory by subtracting from ESP to push and adding to pop
    // SoftwareDevelopersManual-253665.pdf, section 6.2
    // registers are pushed in this order ...
    // eax, ecx, edx, ebx, esp, ebp, esi, edi
    
    _Compile_PushAD ( ) ;

    // get flags first
    _Compile_PushFD ( ) ;
    //_Compile_Lahf ( ) ; // doesn't work with core 2 duo
    _Compile_MoveImm_To_Reg ( EAX, 0, CELL ) ; // clear eax
    _Compile_PopToReg ( EAX ) ;
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->EFlags, CELL ) ;
    _Compile_Move_Reg_To_Rm ( EBX, EAX, 0 ) ;

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

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Ebx, CELL ) ;
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

    _Compile_PushReg ( EBX ) ; // save our scratch register for all of this
    _Compile_SetAddress_ThruReg ( (int32) & cpu->State, 1, EBX ) ;
    _Compile_PopToReg ( EBX ) ;

    // we want this logic at runtime not at compile time
    // ESI/EDI are used by C for string instructions or are null from start
    // which will crash a CfrTil stack (frame) instruction so ...
    //if ( cpu->Esi == 0 ) cpu->Esi = ( int32 ) Dsp ;
    //if ( cpu->Edi < cpu->Esi ) cpu->Edi = cpu->Esi ;
    //_Compile_CpuState_AdjustEDI ( cpu ) ;
    //_Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->State, CELL ) ;

    _Compile_Return ( ) ; // x86 - return opcode
}

void
_Compile_CpuState_Restore ( CpuState * cpu )
{
    // push order for pushad
    // nb : intel stacks grow down toward lesser memory by subtracting from ESP to push and adding to pop
    // SoftwareDevelopersManual-253665.pdf, section 6.2
    // registers are pushed in this order ...
    // eax, ecx, edx, ebx, esp, ebp, esi, edi
    //int32 saveEAX = cpu->Eax ; 
    //_Compile_PushReg ( EAX ) ; // save our scratch register for all of this

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->State, CELL ) ;
    _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ; // test insn logical and src op and dst op sets zf to result
    Compile_JCC ( Z, ZERO_CC, Here + 5 ) ; // if eax is zero return not(EAX) == 1 else return 0
    //_Compile_PopToReg ( EAX ) ;
    _Compile_Return ( ) ; // x86 - return opcode
    
    
    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Edi, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EDI, EAX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Esi, CELL ) ;
    _Compile_Move_Rm_To_Reg ( ESI, EAX, 0 ) ;

#if 0 // ebp & esp should n't be restored
    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Ebp, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EBP, EAX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Esp, CELL ) ;
    _Compile_Move_Rm_To_Reg ( ESP, EAX, 0 ) ;
#endif

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Ebx, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EBX, EAX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Edx, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EDX, EAX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->EFlags, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;
    //_Compile_Sahf ( ) ; // store al to eflags ?? doesn't work ?? core 2 duo ??
    _Compile_PushReg ( EAX ) ;
    _Compile_PopFD ( ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Ecx, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EBX, EAX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Eax, CELL ) ;
    //_Compile_MoveImm_To_Reg ( EAX, ( int32 ) & saveEAX, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;

    //_Compile_PopToReg ( EAX ) ;
    // we want this logic at runtime not at compile time
    //if ( cpu->Esi == 0 ) cpu->Esi = ( int32 ) Dsp ;
    //if ( cpu->Edi > cpu->Esi ) cpu->Edi = cpu->Esi ;
    //_Compile_CpuState_AdjustEDI ( cpu ) ;
    _Compile_Return ( ) ; // x86 - return opcode
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
    
    //_Compile_PushReg ( EAX ) ; 
    //_Compile_PushReg ( EBX ) ; // save our scratch register for all of this
    
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
    _Compile_Move_Reg_To_Reg ( EAX, EBX ) ;

    _Compile_SetAddress_ThruReg ( (int32) & cpu->State, 1, EBX ) ;

    // restore scratch reg
    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Ebx, CELL ) ;
    _Compile_Move_Reg_To_Rm ( EBX, EBX, 0 ) ;
    
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

    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Ebx, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EBX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Edx, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EDX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EBX, ( int32 ) & cpu->Ecx, CELL ) ;
    _Compile_Move_Rm_To_Reg ( ECX, EBX, 0 ) ;

    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) & cpu->Eax, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;

    _Compile_SetAddress_ThruReg ( (int32) & cpu->State, 0, EBX ) ;
    
    _Compile_PopToReg ( EBX ) ; // restore scratch reg

    _Compile_Return ( ) ; 
}

#endif

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

void
_Compile_ESP_Restore ( )
{
    _Compile_Move_Rm_To_Reg ( ESP, EDI, 4 ) ; // 4 : placeholder
    _Context_->Compiler0->EspRestoreOffset = Here - 1 ;
}

void
_Compile_ESP_Save ( )
{
    _Compile_Move_Reg_To_Rm ( ESI, ESP, 4 ) ; // 4 : placeholder
    _Context_->Compiler0->EspSaveOffset = Here - 1 ; // only takes one byte for _Compile_Move_Reg_To_Rm ( ESI, 4, ESP )
    // TO DO : i think this (below) is what it should be but some adjustments need to be made to make it work 
    //byte * here = Here ;
    //_Compile_Stack_Push_Reg ( DSP, ESP ) ;
    //compiler->EspSaveOffset = here ; // only takes one byte for _Compile_Move_Reg_To_Rm ( ESI, 4, ESP )
}

void
_ESP_Setup ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    if ( compiler->EspSaveOffset ) *( compiler->EspSaveOffset ) = ( compiler->NumberOfLocals + 2 ) * CELL ; // 2 : fp + esp
    if ( compiler->EspRestoreOffset ) *( compiler->EspRestoreOffset ) = ( compiler->NumberOfLocals + 1 ) * CELL ; // 1 : esp - already based on fp
    compiler->LocalsFrameSize += CELL ; // add esp
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

