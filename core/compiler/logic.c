
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
        Compile_JCC ( Z, ZERO_CC, Here + 7 ) ; // ?? jmp if z flag is 1 <== ( eax == 0  ) to next test where we can put a flag test
        _Compile_Move_Reg_To_Reg ( EAX, ECX ) ;
        //_Compile_TEST_Reg_To_Reg ( ECX, ECX ) ;
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        _Compiler_Setup_BI_tttn ( _Q_->OVT_Context->Compiler0, ZERO_CC, NZ ) ; // not less than 0 == greater than 0
        Compile_JCC ( Z, ZERO_CC, Here + 16 ) ; // ?? jmp if z flag is 1 <== ( eax == 0  )

        // return 1 :
        _Compile_MoveImm_To_Reg ( EAX, 1, CELL ) ;


        _Compile_JumpWithOffset ( 6 ) ;
        //return 0 :
        _Compile_MoveImm_To_Reg ( EAX, 0, CELL ) ;
        {
            //if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) ) _Stack_DropN ( _Q_->OVT_Context->Compiler0->WordStack, 2 ) ;
            _Compiler_CompileAndRecord_PushEAX ( compiler ) ;
        }
        
    }
    else
    { 
        // assumes two values ( n m ) on the DSP stack 
        // drops 1 from the stack and leaves either a 1 or a 0 ( n && m ) 
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        Compile_JCC ( Z, ZERO_CC, Here + (((int32) _Q_->OVT_Context->Compiler0->CurrentCreatedWord) ? 11 : 8 ) ) ; // CurrentCreatedWord : see below; here we are considering the stack drop or not
        _Compile_Move_StackN_To_Reg ( EAX, DSP, - 1 ) ;
        if ( _Q_->OVT_Context->Compiler0->CurrentCreatedWord ) _Compile_Stack_DropN ( DSP, 1 ) ; // CurrentCreatedWord : if we are in the midst of compiling a word here as opposed to not
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        _Compiler_Setup_BI_tttn ( _Q_->OVT_Context->Compiler0, ZERO_CC, NZ ) ; // not less than 0 == greater than 0
        Compile_JCC ( NZ, ZERO_CC, Here + 16  ) ; // jmp to set tos 0

        // set tos 0 :
        _Compile_SetStackN_WithObject ( DSP, 0, 0 ) ; 
        _Compile_JumpWithOffset ( 6 ) ;
        
        // set tos 1 :
        _Compile_SetStackN_WithObject ( DSP, 0, 1 ) ; 
    }
}

void
Compile_Equals ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, EQUAL, 0 ) ;
}

void
Compile_DoesNotEqual ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, EQUAL, NZ ) ;
}

void
Compile_LessThan ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, LESS, 0 ) ;
}

void
Compile_GreaterThan ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, LE, NZ ) ;
}

void
Compile_LessThanOrEqual ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, LE, 0 ) ;
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
