#include "../../includes/cfrtil.h"

void
_Compile_SetAddress_ThruReg ( int32 address, int32 value, int32 reg )
{
    _Compile_MoveImm_To_Reg ( reg, address, CELL ) ;
    _Compile_MoveImm_To_Mem ( reg, value, CELL ) ;
}

void
_Compile_MoveAddressValueToReg_ThruReg ( int32 reg, int32 address, int32 thruReg )
{
    _Compile_MoveImm_To_Reg ( thruReg, address, CELL ) ;
    _Compile_Move_Rm_To_Reg ( thruReg, thruReg, 0 ) ;
    _Compile_Move_Reg_To_Reg ( reg, thruReg ) ;
}

void
_Compile_MoveRegToAddress_ThruReg ( int32 address, int32 reg, int32 thruReg )
{
    _Compile_MoveImm_To_Reg ( thruReg, address, CELL ) ;
    _Compile_Move_Reg_To_Rm ( thruReg, reg, 0 ) ;
}

void
Compile_Peek ( Compiler * compiler, int32 stackReg ) // @
{
    int optFlag = CheckOptimize ( compiler, 2 ) ;
    if ( optFlag & OPTIMIZE_DONE ) return ;
    _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0 ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;
    _Compile_Move_Reg_To_Rm ( stackReg, EAX, 0 ) ;
}

// '!' ( value address  -- ) // store value at address - dpans94 - address is on top - value was pushed first, leftmost in rpn, then address

void
Compile_Store ( Compiler * compiler, int32 stackReg ) // !
{
    int optFlag = CheckOptimize ( compiler, 4 ) ;
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
        _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0 ) ;
        _Compile_Move_Rm_To_Reg ( ECX, stackReg, - CELL_SIZE ) ;
        _Compile_Move_Reg_To_Rm ( EAX, ECX, 0 ) ;
        Compile_SUBI ( REG, stackReg, 0, 8, BYTE ) ;
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

