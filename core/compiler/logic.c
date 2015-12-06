
#include "../../includes/cfrtil.h"

// ?!? this file could be reworked for simplity, elegance and integration ?!?

void
CfrTil_If ( )
{
    if ( CompileMode )
    {
        _Compile_Jcc ( 0, 0, NZ, ZERO_CC ) ;
        // N, ZERO : use inline|optimize logic which needs to get flags immediately from a 'cmp', jmp if the zero flag is not set
        // for non-inline|optimize ( reverse polarity : cf. _Compile_Jcc comment ) : jmp if cc is not true; cc is set by setcc after 
        // the cmp, or is a value on the stack. 
        // We cmp that value with zero and jmp if this second cmp sets the flag to zero else do the immediately following block code
        // ?? an explanation of the relation of the setcc terms with the flags is not clear to me yet (20110801) from the intel literature ?? 
        // but by trial and error this works; the logic i use is given in _Compile_Jcc.
        // ?? if there are problems check this area ?? cf. http://webster.cs.ucr.edu/AoA/Windows/HTML/IntegerArithmetic.html
        Stack_PointerToJmpOffset_Set ( ) ;
    }
    else
    {
        if ( String_IsPreviousCharA_ ( _Q_->OVT_Context->ReadLiner0->InputLine, _Q_->OVT_Context->Lexer0->TokenStart_ReadLineIndex - 1, '}' ) ) CfrTil_If2Combinator ( ) ;
        else if ( String_IsPreviousCharA_ ( _Q_->OVT_Context->ReadLiner0->InputLine, _Q_->OVT_Context->Lexer0->TokenStart_ReadLineIndex - 1, '#' ) ) CfrTil_If_ConditionalInterpret ( ) ;
        else if ( GetState ( _Q_->OVT_Context, C_SYNTAX | PREFIX_MODE | INFIX_MODE ) ) CfrTil_If_C_Combinator ( ) ;
        else
        {
            Interpreter * interp = _Q_->OVT_Context->Interpreter0 ;
            if ( _DataStack_Pop ( ) )
            {
                // interpret until "else" or "endif"
                int32 rtrn = _Interpret_Until_EitherToken ( interp, "else", "endif", 0 ) ;
                if ( ( rtrn == 2 ) || ( rtrn == 0 ) ) return ;
                Parse_SkipUntil_Token ( "endif" ) ;

            }
            else
            {
                // skip until "else"
                Parse_SkipUntil_Token ( "else" ) ;
                _Interpret_Until_Token ( interp, "endif", 0 ) ;
            }
        }
    }
}

// same as CfrTil_JMP

void
CfrTil_Else ( )
{
    if ( CompileMode )
    {
        _Compile_UninitializedJump ( ) ; // at the end of the 'if block' we need to jmp over the 'else block'
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
        Stack_PointerToJmpOffset_Set ( ) ;
    }
    else
    {
        if ( String_IsPreviousCharA_ ( _Q_->OVT_Context->ReadLiner0->InputLine, _Q_->OVT_Context->Lexer0->TokenStart_ReadLineIndex - 1, '#' ) ) CfrTil_Else_ConditionalInterpret ( ) ;
        else
        {
            _Interpret_Until_Token ( _Q_->OVT_Context->Interpreter0, "endif", 0 ) ;
        }
    }
}

void
CfrTil_EndIf ( )
{
    if ( CompileMode )
    {
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
    }
    //else { ; //nop  }
}

/*
 *  Logical operators
 */

void
Compile_LogicalAnd ( Compiler * compiler )
{
    int32 optFlag = CheckOptimize ( compiler, 5 ) ;
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        // optimizer sets up the two args in eax and ecx
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        Compile_JCC ( Z, ZERO_CC, Here + 18 ) ; // ?? jmp if z flag is 1 <== ( eax == 0  ) to next test where we can put a flag test
        _Compile_Move_Reg_To_Reg ( EAX, ECX ) ;
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        Compile_JCC ( Z, ZERO_CC, Here + 16 ) ; // ?? jmp if z flag is 1 <== ( eax == 0  )

        // return 1 :
        _Compile_MoveImm_To_Reg ( EAX, 1, CELL ) ; // 6 bytes
        _Compile_JumpWithOffset ( 6 ) ; // 6 bytes

        //return 0 :
        _Compile_MoveImm_To_Reg ( EAX, 0, CELL ) ;
        _Compiler_Setup_BI_tttn ( compiler, ZERO_CC, NZ ) ;
        _Compiler_CompileAndRecord_PushEAX ( compiler ) ;
    }
    else
    {
        Word *one = Compiler_WordStack ( compiler, - 1 ) ; // assumes two values ( n m ) on the DSP stack 
        if ( one->StackPushRegisterCode ) SetHere ( one->StackPushRegisterCode ) ;
        else _Compile_Stack_PopToReg ( DSP, EAX ) ;
        // drops 1 from the stack and leaves either a 1 or a 0 ( n && m ) 
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        Compile_JCC ( Z, ZERO_CC, Here + 26 ) ; // return 0
        Compile_Move_TOS_To_EAX ( DSP ) ;
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        Compile_JCC ( Z, ZERO_CC, Here + 16 ) ; // ?? jmp if z flag is 1 <== ( eax == 0  )

        // return 1 :
        _Compile_MoveImm_To_Reg ( EAX, 1, CELL ) ; // 6 bytes
        _Compile_JumpWithOffset ( 6 ) ; // 6 bytes

        //return 0 :
        _Compile_MoveImm_To_Reg ( EAX, 0, CELL ) ;
        _Compiler_Setup_BI_tttn ( compiler, ZERO_CC, NZ ) ;

        Word *zero = Compiler_WordStack ( compiler, 0 ) ;
        zero->StackPushRegisterCode = Here ;
        _Compile_Move_Reg_To_StackN ( DSP, 0, EAX ) ;
    }
}

void
Compile_LogicalNot ( Compiler * compiler )
{
    Word *zero = Compiler_WordStack ( compiler, 0 ) ;
    Word *one = Compiler_WordStack ( compiler, - 1 ) ;
    int optFlag = CheckOptimize ( compiler, 2 ) ;
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM )
        {
            _Compile_MoveImm_To_Reg ( EAX, compiler->Optimizer->Optimize_Imm, CELL ) ;
        }
        else if ( compiler->Optimizer->Optimize_Rm == DSP )
        {
            _Compile_Move_StackN_To_Reg ( EAX, DSP, 0 ) ;
        }
        else if ( compiler->Optimizer->Optimize_Rm != EAX )
        {
            _Compile_VarLitObj_RValue_To_Reg ( one, EAX ) ;
        }
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        _Compiler_Setup_BI_tttn ( compiler, ZERO_CC, Z ) ;
        Compile_JCC ( NZ, ZERO_CC, Here + 16 ) ; // ?? jmp if z flag is 1 <== ( eax == 0  )

        // return 1 :
        _Compile_MoveImm_To_Reg ( EAX, 1, CELL ) ; // 6 bytes
        _Compile_JumpWithOffset ( 6 ) ; // 6 bytes

        //return 0 :
        _Compile_MoveImm_To_Reg ( EAX, 0, CELL ) ;
        _Compiler_CompileAndRecord_PushEAX ( compiler ) ;
    }
    else
    {
        if ( one->StackPushRegisterCode ) SetHere ( one->StackPushRegisterCode ) ;
        else _Compile_Stack_PopToReg ( DSP, EAX ) ;
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ; //logical and eax eax => if ( eax > 0 ) 1 else 0
        _Compiler_Setup_BI_tttn ( compiler, ZERO_CC, Z ) ;
        Compile_JCC ( NZ, ZERO_CC, Here + 16 ) ; // ?? jmp if z flag is 1 <== ( eax == 0  )

        // return 1 :
        _Compile_MoveImm_To_Reg ( EAX, 1, CELL ) ; // 6 bytes
        //_Compile_SetStackN_WithObject ( DSP, 0, 1  ) ;
        _Compile_JumpWithOffset ( 6 ) ; // 6 bytes

        //return 0 :
        _Compile_MoveImm_To_Reg ( EAX, 0, CELL ) ;
        //zero->StackPushRegisterCode = Here ;
        //_Compile_Move_Reg_To_StackN ( DSP, 0, EAX ) ;
        //_Compile_SetStackN_WithObject ( DSP, 0, 0  ) ;
        _Compiler_CompileAndRecord_PushEAX ( compiler ) ;
        //int a, b, c= 0, d ; a = 1; b = !a, d= !c ; Printf ( "a = %d b = %d c =%d ~d = %d", a, b, c, d ) ;
    }
}

void
Compile_Equals ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, EQUAL, Z ) ;
}

void
Compile_DoesNotEqual ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, EQUAL, NZ ) ;
}

void
Compile_LessThan ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, LESS, Z ) ;
}

void
Compile_GreaterThan ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, LE, NZ ) ;
}

void
Compile_LessThanOrEqual ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, LE, Z ) ;
}

void
Compile_GreaterThanOrEqual ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, LESS, NZ ) ;
}

void
Compile_Logical_X ( Compiler * compiler, int32 op )
{
    int optFlag = CheckOptimize ( compiler, 5 ) ;
    if ( optFlag == OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        // TODO : this optimization somehow is *very* little used, why is that ?!? 
        // assumes we have unordered operands in eax, ecx
        _Compile_X_Group1 ( op, REG, REG, EAX, ECX, 0, 0, CELL ) ;
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        _Compiler_Setup_BI_tttn ( _Q_->OVT_Context->Compiler0, ZERO_CC, NZ ) ; // not less than 0 == greater than 0
        Word *zero = Compiler_WordStack ( compiler, 0 ) ;
        _Word_CompileAndRecord_PushEAX ( zero ) ;
    }
    else
    {
        // operands are still on the stack
        _Compile_Move_StackN_To_Reg ( EAX, DSP, 0 ) ;
        //_Compile_Group1 ( int32 code, int32 toRegOrMem, int32 mod, int32 reg, int32 rm, int32 sib, int32 disp, int32 osize )
        _Compile_X_Group1 ( op, REG, MEM, EAX, DSP, 0, - 4, CELL ) ;
        _Compile_Stack_DropN ( DSP, 2 ) ;
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        _Compiler_Setup_BI_tttn ( _Q_->OVT_Context->Compiler0, ZERO_CC, NZ ) ; // not less than 0 == greater than 0
        Word *zero = Compiler_WordStack ( compiler, 0 ) ;
        _Word_CompileAndRecord_PushEAX ( zero ) ;
    }
}
