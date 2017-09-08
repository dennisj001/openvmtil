
#include "../../include/cfrtil.h"

// keep this stack code correlated with core/cstack.c

void
_Compile_Stack_Drop ( int64 stackReg )
{
    Compile_SUBI ( REG, stackReg, 0, sizeof (int64 ), BYTE ) ; // 3 bytes long
}

void
_Compile_Stack_DropN ( int64 stackReg, int64 n )
{
    Compile_SUBI ( REG, stackReg, 0, n * sizeof ( int64 ), 0 ) ;
}

// RDrop : drop the sytem return, ESP based, stack
// system stack is backward to cfrTil - grows downward

void
_Compile_DropN_ESP ( int64 n )
{
    Compile_ADDI ( REG, ESP, 0, n * sizeof ( int64 ), 0 ) ;
}

void
_Compile_SetStackN_WithObject ( int64 stackReg, int64 n, int64 obj )
{
    //_Compile_MoveImm ( int64 direction, int64 rm, int64 sib, int64 disp, int64 imm, int64 operandSize )
    _Compile_MoveImm ( MEM, stackReg, 0, n * CELL, obj, CELL ) ;
}

void
_Compile_Stack_Push ( int64 stackReg, int64 obj ) // runtime
{
    Compile_ADDI ( REG, stackReg, 0, sizeof (int64 ), BYTE ) ;
    //_Compile_MoveImm ( MEM, stackReg, 0, 0, obj, CELL ) ;
    _Compile_SetStackN_WithObject ( DSP, 0, obj ) ;
}

void
_Compile_Move_StackN_To_Reg ( int64 reg, int64 stackReg, int64 index, int64 size )
{
    _Compile_Move_Rm_To_Reg ( reg, stackReg, index * CELL, size ) ;
}

void
_Compile_Move_Reg_To_StackN ( int64 stackReg, int64 index, int64 reg, int64 size )
{
    _Compile_Move_Reg_To_Rm ( stackReg, reg, index * CELL, size ) ;
}

void
_Compile_Move_StackNRm_To_Reg ( int64 reg, int64 stackReg, int64 index, int64 size )
{
    _Compile_Move_StackN_To_Reg ( reg, stackReg, index, size ) ;
    _Compile_Move_Rm_To_Reg ( reg, reg, 0, size ) ; // *x
}

//  ( reg sreg n ) mov_reg_to_stacknMemAddr

void
_Compile_Move_Reg_To_StackNRm_UsingReg ( int64 stackReg, int64 index, int64 reg, int64 ureg, int64 size )
{
    _Compile_Move_StackN_To_Reg ( ureg, stackReg, index, size ) ;
    _Compile_Move_Reg_To_Rm ( ureg, reg, 0, size ) ;
}

// remember to use a negative number to access an existing stack item

void
_Compile_Stack_PushReg ( int64 stackReg, int64 reg, int64 size )
{
    Compile_ADDI ( REG, stackReg, 0, sizeof (int64 ), BYTE ) ;
    _Compile_Move_Reg_To_StackN ( stackReg, 0, reg, size ) ;
}

void
_Compile_Stack_PopToReg ( int64 stackReg, int64 reg, int64 size )
{
    _Compile_Move_StackN_To_Reg ( reg, stackReg, 0, size ) ;
    Compile_SUBI ( REG, stackReg, 0, sizeof (int64 ), BYTE ) ;
}

void
Compile_Stack_PushEAX ( int64 stackReg, int64 size )
{
    _Compile_Stack_PushReg ( stackReg, EAX, size ) ;
}

void
Compile_Move_TOS_To_EAX ( int64 stackReg, int64 size )
{
    _Compile_Move_StackN_To_Reg ( EAX, stackReg, 0, size ) ;
}

void
Compile_Move_EAX_To_TOS ( int64 stackReg, int64 size )
{
    _Compile_Move_Reg_To_StackN ( stackReg, 0, EAX, size ) ;
}

void
Compile_Pop_To_EAX ( int64 stackReg, int64 size )
{
    _Compile_Stack_PopToReg ( stackReg, EAX, size ) ;
}

void
Compile_Pop_ToEAX_AndCall ( int64 stackReg, int64 size )
{
    _Compile_Stack_PopToReg ( stackReg, EAX, size ) ;
    _Compile_CallEAX ( ) ;
}

void
Compile_MoveImm_To_TOS ( int64 stackReg, int64 imm, int64 size )
{
    _Compile_MoveImm ( MEM, stackReg, 0, 0, imm, size ) ;
}

#if 0
// for n < 64

void
_Compile_Stack_NDup ( int64 stackReg )
{
    _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0 ) ;
    Compile_ADDI ( REG, stackReg, 0, sizeof (int64 ), BYTE ) ; // 3 bytes long
    _Compile_Move_Reg_To_Rm ( stackReg, EAX, 0 ) ;
}
#endif

void
_Compile_Stack_Dup ( int64 stackReg )
{
    Compiler * compiler = _Context_->Compiler0 ;
    int64 optFlag = CheckOptimize ( compiler, 3 ) ;
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else
    {
        _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0, CELL ) ;
        //Word *zero = Compiler_WordStack ( 0 ) ; // refers to this current multiply insn word
        Word *zero = Compiler_WordList ( 0 ) ; // refers to this current multiply insn word
        zero->StackPushRegisterCode = Here ;
        Compile_ADDI ( REG, stackReg, 0, sizeof (int64 ), BYTE ) ; // 3 bytes long
        _Compile_Move_Reg_To_Rm ( stackReg, EAX, 0, CELL ) ;
    }
}

// pick ( n -- stack[n] )
// the pick is from stack below top index; nb. 0 indexed stack, ie. 0 is the first indexed item on the stack, 1 is the second
// 0 pick is Dsp [ 0] - TOS 
// 1 pick is Dsp [-1]
// ..., etc.
// the result replaces the index on the stack; the rest of the stack remains the same
// so 'pick' just picks the indexed item from the stack and places it on top
// nb. should not try to optimize because it needs the argument slot for it's result
void
_Compile_Stack_Pick ( int64 stackReg ) // pick
{
    _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0, CELL ) ;
    Compile_NOT ( REG, EAX, 0, 0 ) ; // negate eax
    _Compile_Move ( REG, EAX, stackReg, _CalculateSib ( SCALE_CELL, EAX, ESI ), 0, CELL ) ; // move eax, [esi + eax * 4 ] ; but remember eax is now a negative number
    _Compile_Move_Reg_To_Rm ( stackReg, EAX, CELL, CELL ) ;
}

void
_Compile_Stack_Swap ( int64 stackReg )
{
    _Compile_Move_Rm_To_Reg ( ECX, stackReg, 0, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EBX, stackReg, - CELL, CELL ) ;
    _Compile_Move_Reg_To_Rm ( stackReg, ECX, - CELL, CELL ) ;
    _Compile_Move_Reg_To_Rm ( stackReg, EBX, 0, CELL ) ;
}

void
Compile_DataStack_PushEAX ( )
{
    Compile_Stack_PushEAX ( DSP, CELL ) ;
}

void
_Compile_Esp_Push ( int64 value )
{
    _Compile_MoveImm_To_Reg ( EAX, value, CELL ) ;
    _Compile_PushReg ( EAX ) ;
}

void
Compile_DspPop_EspPush ( )
{
    _Compile_Stack_PopToReg ( DSP, EAX, CELL ) ;
    // _Compile_Stack_Push_Reg ( ESP, ECX ) ; // no such op
    _Compile_PushReg ( EAX ) ;
}

#if 0

void
Compile_EspPop_DspPush ( )
{
    _Compile_PopToReg ( EAX ) ; // intel pop is pop esp
    Compile_DataStack_PushEAX ( ) ; // no such op
}
#endif

