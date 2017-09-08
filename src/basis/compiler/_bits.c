
#include "../../include/cfrtil.h"

/*
void
Compile_Test ( Compiler * compiler )
{
    if ( CheckOptimizeOperands ( compiler, 5 ) )
    {
        _Compile_Test ( compiler->optInfo->Optimize_Mod, compiler->optInfo->Optimize_Reg,
                compiler->optInfo->Optimize_Rm, compiler->optInfo->Optimize_Disp, compiler->optInfo->Optimize_Imm ) ;
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
    if ( GetState( _Context_->Compiler0, BLOCK_MODE ) )
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
Compile_X_Group3 ( Compiler * compiler, int64 code )
{
    //if ( CheckOptimizeOperands ( compiler, 5 ) )
    int64 optFlag = CheckOptimizeOperands ( compiler, 5 ) ; 
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        //_Compile_Group3 ( cell code, cell mod, cell rm, cell sib, cell disp, cell imm, cell size )
        _Compile_Group3 ( code, compiler->optInfo->Optimize_Mod,
                compiler->optInfo->Optimize_Rm, 0, compiler->optInfo->Optimize_Disp, compiler->optInfo->Optimize_Imm, 0 ) ;
        if ( compiler->optInfo->Optimize_Rm != DSP ) // if the result is not already tos
        {
            //Word *zero = Compiler_WordStack ( 0 ) ; // refers to this current multiply insn word
            Word *zero = Compiler_WordList ( 0 ) ; // refers to this current multiply insn word
            if ( compiler->optInfo->Optimize_Rm != EAX ) _Compile_Move_Rm_To_Reg ( EAX, compiler->optInfo->Optimize_Rm,
                    compiler->optInfo->Optimize_Disp ) ;
            zero->StackPushRegisterCode = Here ;
            _Compile_Stack_Push_Reg ( DSP, EAX ) ;
        }
    }
    else
    {
        _Compile_Group3 ( code, MEM, DSP, 0, 0, 0, 0 ) ;
    }
}

void
Compile_X_Shift ( Compiler * compiler, int64 op, int64 stackFlag )
{
    //if ( CheckOptimizeOperands ( compiler, 5 ) )
    int64 optFlag = CheckOptimizeOperands ( compiler, 5 ) ;
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        // _Compile_Group2 ( int64 mod, int64 regOpCode, int64 rm, int64 sib, cell disp, cell imm )
        _Compile_Group2 ( compiler->optInfo->Optimize_Mod,
                op, compiler->optInfo->Optimize_Rm, 0, compiler->optInfo->Optimize_Disp, compiler->optInfo->Optimize_Imm ) ;
        if ( stackFlag && ( compiler->optInfo->Optimize_Rm != DSP ) ) // if the result is not already tos
        {
            //Word *zero = Compiler_WordStack ( 0 ) ; // refers to this current multiply insn word
            Word *zero = Compiler_WordList ( 0 ) ; // refers to this current multiply insn word
            if ( compiler->optInfo->Optimize_Rm != EAX ) _Compile_Move_Rm_To_Reg ( EAX, compiler->optInfo->Optimize_Rm,
                    compiler->optInfo->Optimize_Disp ) ;
            zero->StackPushRegisterCode = Here ;
            _Compile_Stack_Push_Reg ( DSP, EAX ) ;
        }
    }
    else
    {
        _Compile_Move_StackN_To_Reg ( ECX, DSP, 0 ) ;
        Compile_SUBI ( REG, ESI, 0, CELL, BYTE ) ;
        //_Compile_Group2_CL ( int64 mod, int64 regOpCode, int64 rm, int64 sib, cell disp )
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

