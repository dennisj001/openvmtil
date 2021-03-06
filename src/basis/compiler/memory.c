#include "../../include/cfrtil32.h"

void
_Compile_Set_C_LValue_WithImm_ThruReg ( int32 address, int32 value, int32 reg )
{
    _Compile_MoveImm_To_Reg ( reg, address, CELL ) ;
    _Compile_MoveImm_To_Mem ( reg, value, CELL ) ;
}

void
_Compile_Get_C_Value_ToReg ( int32 reg, int32 value )
{
   _Compile_MoveImm_To_Reg ( reg, ( int32 ) value, CELL ) ;
}

// nb. necessary for esp/ebp
void
_Compile_Get_FromCAddress_ToReg_ThruReg ( int32 reg, byte * address, int32 thruReg )
{
    _Compile_MoveMem_To_Reg ( reg, (byte*) address, thruReg, CELL ) ;    
}

void
_Compile_Get_FromCAddress_ToReg ( int32 reg, byte * address )
{
    _Compile_MoveMem_To_Reg_NoThru ( reg, (byte*) address, CELL ) ;    
}

// thru reg is a 'scratch' reg
void
_Compile_Set_CAddress_WithRegValue_ThruReg ( byte * address, int32 reg, int32 thruReg )
{
    _Compile_MoveReg_To_Mem ( reg, address, thruReg, CELL ) ;
}

void
Compile_Peek ( Compiler * compiler, int32 stackReg ) // @
{
    int optFlag = CheckOptimize ( compiler, 3 ) ;
    if ( optFlag & OPTIMIZE_DONE ) return ;
    _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0 ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;
    _Compile_Move_Reg_To_Rm ( stackReg, EAX, 0 ) ;
}

// '!' ( value address  -- ) // store value at address - dpans94 - address is on top - value was pushed first, leftmost in rpn, then address

void
Compile_Store ( Compiler * compiler, int32 stackReg ) // !
{
    int optFlag = CheckOptimize ( compiler, 5 ) ;
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        // _Compile_MoveImm ( cell direction, cell rm, cell disp, cell imm, cell operandSize )
        if ( compiler->optInfo->OptimizeFlag & OPTIMIZE_IMM ) _Compile_MoveImm ( compiler->optInfo->Optimize_Dest_RegOrMem,
            compiler->optInfo->Optimize_Rm, 0, compiler->optInfo->Optimize_Disp, compiler->optInfo->Optimize_Imm, CELL ) ;
        else if ( compiler->optInfo->OptimizeFlag & OPTIMIZE_REGISTER )
        {
            // allow for one of these to be EAX which is 0
            if ( compiler->optInfo->Optimize_SrcReg || compiler->optInfo->Optimize_DstReg ) _Compile_Move_Reg_To_Reg ( compiler->optInfo->Optimize_DstReg, compiler->optInfo->Optimize_SrcReg ) ;
                //_Compile_Move ( int32 direction, int32 reg, int32 rm, int32 sib, int32 disp )
            else _Compile_Move ( compiler->optInfo->Optimize_Dest_RegOrMem, compiler->optInfo->Optimize_Reg, compiler->optInfo->Optimize_Rm, 0, 0 ) ;
        }
        else _Compile_Move_Reg_To_Rm ( compiler->optInfo->Optimize_Rm, compiler->optInfo->Optimize_Reg, compiler->optInfo->Optimize_Disp ) ;
    }
    else
    {
        Word * word ;
        if ( ( word = ( Word* ) Compiler_WordList ( 0 ) ) && word->StackPushRegisterCode ) SetHere ( word->StackPushRegisterCode ) ;
        else _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0 ) ;
        _Compile_Move_Rm_To_Reg ( ECX, stackReg, - CELL_SIZE ) ;
        _Compile_Move_Reg_To_Rm ( EAX, ECX, 0 ) ;
        Compile_SUBI ( REG, stackReg, 0, ( word && word->StackPushRegisterCode ) ? 4 : 8, BYTE ) ;
    }
}

// ( address value -- ) store value at address - reverse order of parameters from '!'
// ( addr n -- ) // (*addr) = n

void
Compile_Poke ( Compiler * compiler, int32 stackReg ) // =
{
    int optFlag = CheckOptimize ( compiler, 7 ) ;
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        if ( compiler->optInfo->OptimizeFlag & OPTIMIZE_IMM )
        {
            // _Compile_MoveImm ( cell direction, cell rm, cell disp, cell imm, cell operandSize )
            _Compile_MoveImm ( compiler->optInfo->Optimize_Dest_RegOrMem,
                compiler->optInfo->Optimize_Rm, 0, compiler->optInfo->Optimize_Disp, compiler->optInfo->Optimize_Imm, CELL ) ;
        }
        else if ( compiler->optInfo->OptimizeFlag & OPTIMIZE_REGISTER )
        {
            // allow for one of these to be EAX which is 0
            if ( compiler->optInfo->Optimize_SrcReg || compiler->optInfo->Optimize_DstReg ) _Compile_Move_Reg_To_Reg ( compiler->optInfo->Optimize_DstReg, compiler->optInfo->Optimize_SrcReg ) ;
                //_Compile_Move ( int32 direction, int32 reg, int32 rm, int32 sib, int32 disp )
            else _Compile_Move ( compiler->optInfo->Optimize_Dest_RegOrMem, compiler->optInfo->Optimize_Reg, compiler->optInfo->Optimize_Rm, 0, 0 ) ;
        }
        else _Compile_Move_Reg_To_Rm ( compiler->optInfo->Optimize_Rm, compiler->optInfo->Optimize_Reg, compiler->optInfo->Optimize_Disp ) ;
    }
    else
    {
        _Compile_Move_Rm_To_Reg ( ECX, stackReg, 0 ) ;
        _Compile_Move_Rm_To_Reg ( EAX, stackReg, - CELL_SIZE ) ;
        _Compile_Move_Reg_To_Rm ( EAX, ECX, 0 ) ;
        //if ( ! GetState ( _Context_, C_SYNTAX ) ) 
        Compile_SUBI ( REG, stackReg, 0, 8, BYTE ) ;
    }
}

// n = *m
// ( n m -- )
// 

void
Compile_AtEqual ( int32 stackReg ) // !
{
    _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0 ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;
    _Compile_Move_Rm_To_Reg ( ECX, stackReg, - CELL_SIZE ) ;
    _Compile_Move_Reg_To_Rm ( ECX, EAX, 0 ) ;
    Compile_SUBI ( REG, stackReg, 0, CELL_SIZE * 2, BYTE ) ;
}

