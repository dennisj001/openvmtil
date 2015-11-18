
#include "../../includes/cfrtil.h"

/*
void
Compile_Test ( Compiler * compiler )
{
    if ( CheckOptimizeOperands ( compiler, 5 ) )
    {
        _Compile_Test ( compiler->Optimizer->Optimize_Mod, compiler->Optimizer->Optimize_Reg,
                compiler->Optimizer->Optimize_Rm, compiler->Optimizer->Optimize_Disp, compiler->Optimizer->Optimize_Imm ) ;
    }
    else
    {
        _Compile_Stack_PopToReg ( DSP, EAX ) ;
        _Compile_Stack_PopToReg ( DSP, ECX ) ;
        _Compile_Test ( REG, EAX, ECX, 0, 0 ) ;
    }
   _Compiler_Setup_BI_tttn ( _Context->Compiler0, ZERO, N ) ;
}

void
Compile_BitWise_TEST ( Compiler * compiler )
{
    Compile_Test ( compiler ) ;
}
 
void
CfrTil_TEST ( )
{
    if ( Compiler_GetState( _Q_->OVT_Context->Compiler0, BLOCK_MODE ) )
    {
        Compile_BitWise_TEST ( _Context->Compiler0 ) ;
    }
    else
    {
        Error ( ( byte* ) "\n\"TEST\" can be used only in compile mode.", 1 ) ;
    }
}
 */

void
Compile_X_Group3 ( Compiler * compiler, int32 code ) //OP_1_ARG
{
    int optFlag = CheckOptimize ( compiler, 5 ) ; //OP_1_ARG
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        //_Compile_Group3 ( cell code, cell mod, cell rm, cell sib, cell disp, cell imm, cell size )
        _Compile_Group3 ( code, compiler->Optimizer->Optimize_Mod,
            compiler->Optimizer->Optimize_Rm, 0, compiler->Optimizer->Optimize_Disp, compiler->Optimizer->Optimize_Imm, 0 ) ;
        if ( compiler->Optimizer->Optimize_Rm != DSP ) // if the result is not already tos
        {
            if ( compiler->Optimizer->Optimize_Rm != EAX ) _Compile_Move_Rm_To_Reg ( EAX, compiler->Optimizer->Optimize_Rm,
                compiler->Optimizer->Optimize_Disp ) ;
            _Compiler_CompileAndRecord_PushEAX ( compiler ) ;
        }
    }
    else
    {
        _Compile_Group3 ( code, MEM, DSP, 0, 0, 0, 0 ) ;
    }
}

void
Compile_X_Shift ( Compiler * compiler, int32 op, int32 stackFlag )
{
    int optFlag = CheckOptimize ( compiler, 5 ) ; //OP_1_ARG
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        // _Compile_Group2 ( int mod, int regOpCode, int rm, int sib, cell disp, cell imm )
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM )
        {
            _Compile_Group2 ( compiler->Optimizer->Optimize_Mod,
                op, compiler->Optimizer->Optimize_Rm, 0, compiler->Optimizer->Optimize_Disp, compiler->Optimizer->Optimize_Imm ) ;
        }
        else //if ( ( compiler->Optimizer->Optimize_Imm == 0 ) && ( compiler->Optimizer->Optimize_Rm != EAX ) ) // this logic is prototype maybe not precise 
        {
            _Compile_Group2_CL ( MEM, op, compiler->Optimizer->Optimize_Rm, 0, compiler->Optimizer->Optimize_Disp ) ;
        }
        if ( stackFlag && ( compiler->Optimizer->Optimize_Rm != DSP ) ) // if the result is not already tos
        {
            if ( compiler->Optimizer->Optimize_Rm != EAX ) _Compile_Move_Rm_To_Reg ( EAX, compiler->Optimizer->Optimize_Rm,
                compiler->Optimizer->Optimize_Disp ) ;
            _Compiler_CompileAndRecord_PushEAX ( compiler ) ;
        }
    }
    else
    {
        _Compile_Move_StackN_To_Reg ( ECX, DSP, 0 ) ;
        Compile_SUBI ( REG, ESI, 0, CELL, BYTE ) ;
        //_Compile_Group2_CL ( int mod, int regOpCode, int rm, int sib, cell disp )
        _Compile_Group2_CL ( MEM, op, DSP, 0, 0 ) ;
    }
}

void
Compile_BitWise_NOT ( Compiler * compiler )
{
    Compile_X_Group3 ( compiler, NOT ) ;
}

// two complement

void
Compile_BitWise_NEG ( Compiler * compiler )
{
    Compile_X_Group3 ( compiler, NEG ) ;
}

void
Compile_ShiftLeft ( )
{
    Compile_X_Shift ( _Q_->OVT_Context->Compiler0, SHL, 1 ) ;
}

void
Compile_ShiftRight ( )
{
    Compile_X_Shift ( _Q_->OVT_Context->Compiler0, SHR, 1 ) ;
}

