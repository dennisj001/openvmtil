
#include "../../include/cfrtil.h"

// ?!? this file could be reworked for simplity, elegance and integration ?!?

void
CfrTil_If ( )
{
    if ( CompileMode )
    {
        //_Set_SCA ( 0 ) ;
        _Compile_Jcc ( 0, 0, NZ, ZERO_TTT ) ;
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
        if ( String_IsPreviousCharA_ ( _Context_->ReadLiner0->InputLine, _Context_->Lexer0->TokenStart_ReadLineIndex - 1, '}' ) ) CfrTil_If2Combinator ( ) ;
        else if ( String_IsPreviousCharA_ ( _Context_->ReadLiner0->InputLine, _Context_->Lexer0->TokenStart_ReadLineIndex - 1, '#' ) ) CfrTil_If_ConditionalInterpret ( ) ;
        else if ( GetState ( _Context_, C_SYNTAX | PREFIX_MODE | INFIX_MODE ) ) CfrTil_If_C_Combinator ( ) ;
        else
        {
            Interpreter * interp = _Context_->Interpreter0 ;
            if ( _DataStack_Pop ( ) )
            {
                // interpret until "else" or "endif"
                byte * token = _Interpret_C_Until_EitherToken ( interp, ( byte* ) "else", ( byte* ) "endif", 0 ) ;
                //if ( ( rtrn == 2 ) || ( rtrn == 0 ) ) return ;
                if ( ( token == 0 ) || ( String_Equal ( token, "endif" ) ) ) return ;
                Parse_SkipUntil_Token ( ( byte* ) "endif" ) ;

            }
            else
            {
                // skip until "else"
                Parse_SkipUntil_Token ( ( byte* ) "else" ) ;
                _Interpret_Until_Token ( interp, ( byte* ) "endif", 0 ) ;
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
        //_Set_SCA ( 0 ) ;
        _Compile_UninitializedJump ( ) ; // at the end of the 'if block' we need to jmp over the 'else block'
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
        Stack_PointerToJmpOffset_Set ( ) ;
    }
    else
    {
        if ( String_IsPreviousCharA_ ( _Context_->ReadLiner0->InputLine, _Context_->Lexer0->TokenStart_ReadLineIndex - 1, '#' ) ) CfrTil_Else_ConditionalInterpret ( ) ;
        else
        {
            _Interpret_Until_Token ( _Context_->Interpreter0, ( byte* ) "endif", 0 ) ;
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

// ttt n : notation from intel manual 253667 ( N-Z ) - table B-10 : ttt = condition codes, n is a negation bit
// tttn notation is used with the SET and JCC instructions

// note : intex syntax  : instruction dst, src
//        att   syntax  : instruction src, dst
// cmp : compares by subtracting src from dst, dst - src, and setting eflags like a "sub" insn 
// eflags affected : cf of sf zf af pf : Intel Instrucion Set Reference Manual for "cmp"
// ?? can this be done better with test/jcc ??
// want to use 'test eax, 0' as a 0Branch (cf. jonesforth) basis for all block conditionals like if/else, do/while, for ...

void
Compile_Cmp_Set_tttn_Logic ( Compiler * compiler, int32 ttt, int32 negateFlag )
{
    //SC_ForcePush ( _Context_->CurrentlyRunningWord ) ;
    SC_SetForcePush ( true ) ;
    int32 optFlag = CheckOptimize ( compiler, 5 ) ;
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        //_Set_SCA ( 0 ) ;
        if ( ( optFlag == 2 ) && ( compiler->optInfo->Optimize_Rm == DSP ) )
        {
            _Compile_Stack_PopToReg ( DSP, ECX ) ; // assuming optimize always uses EAX first
            compiler->optInfo->Optimize_Rm = ECX ;
            compiler->optInfo->Optimize_Mod = REG ;
        }
        if ( compiler->optInfo->OptimizeFlag & OPTIMIZE_IMM )
        {
            if ( ( ttt == EQUAL ) && ( compiler->optInfo->Optimize_Imm == 0 ) ) //Compile_TEST ( compiler->optInfo->Optimize_Mod, compiler->optInfo->Optimize_Rm, 0, compiler->optInfo->Optimize_Disp, compiler->optInfo->Optimize_Imm, CELL ) ;
            {
                if ( compiler->optInfo->O_two->StackPushRegisterCode ) SetHere ( compiler->optInfo->O_two->StackPushRegisterCode ) ; // leave optInfo->O_two value in EAX we don't need to push it
                _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
            }
            else
            {
                // Compile_CMPI( mod, operandReg, offset, immediateData, size
                Compile_CMPI ( compiler->optInfo->Optimize_Mod,
                    compiler->optInfo->Optimize_Rm, compiler->optInfo->Optimize_Disp, compiler->optInfo->Optimize_Imm, CELL ) ;
            }
        }
        else
        {
            // Compile_CMP( toRegOrMem, mod, reg, rm, sib, disp )
            Compile_CMP ( compiler->optInfo->Optimize_Dest_RegOrMem, compiler->optInfo->Optimize_Mod,
                compiler->optInfo->Optimize_Reg, compiler->optInfo->Optimize_Rm, 0, compiler->optInfo->Optimize_Disp, CELL ) ;
        }
    }
    else
    {
        _Compile_Move_StackN_To_Reg ( ECX, DSP, 0 ) ;
        _Compile_Move_StackN_To_Reg ( EAX, DSP, - 1 ) ;
        // must do the DropN before the CMP because CMP sets eflags 
        _Compile_Stack_DropN ( DSP, 2 ) ; // before cmp 
        Compile_CMP ( REG, REG, EAX, ECX, 0, 0, CELL ) ;
    }
    _Compile_SET_tttn_REG ( ttt, negateFlag, EAX ) ; // immediately after the 'cmp' insn which changes the flags appropriately
    _Compile_MOVZX_REG ( EAX ) ;
    _Compiler_CompileAndRecord_PushEAX ( compiler ) ;
}

BlockInfo *
_Compiler_Setup_BI_tttn ( Compiler * compiler, int32 ttt, int32 negFlag, int32 overWriteSize )
{
    BlockInfo *bi = ( BlockInfo * ) _Stack_Top ( compiler->CombinatorBlockInfoStack ) ;
    if ( bi )
    {
        BlockInfo_Set_tttn ( bi, ttt, negFlag, overWriteSize ) ;
    }
    return bi ;
}

// SET : 0x0f 0x9tttn mod 000 rm/reg
// ?!? wanna use TEST insn here to eliminate need for _Compile_MOVZX_REG insn ?!? is that possible

void
_Compile_SETcc ( int32 ttt, int32 negFlag, int32 reg )
{
    //SC_ForcePush ( _Context_->CurrentlyRunningWord ) ;
    SC_SetForcePush ( true ) ;
    Compile_StartOpCode_Int8 ( ( byte ) 0x0f ) ;
    _Compile_Int8 ( ( 0x9 << 4 ) | ( ttt << 1 ) | negFlag ) ;
    _Compile_Int8 ( _CalculateModRmByte ( REG, 0x00, reg, 0, 0 ) ) ;
}

void
_Compile_SET_tttn_REG ( int32 ttt, int32 negFlag, int32 reg )
{
    _Compiler_Setup_BI_tttn ( _Context_->Compiler0, ttt, negFlag, 3 ) ;
    _Compile_SETcc ( ttt, negFlag, reg ) ;
}

void
Compile_GetLogicFromTOS ( BlockInfo *bi )
{
    Compile_Pop_To_EAX ( DSP ) ;
    SC_SetForcePush ( true ) ;
    _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
}

int32
Compile_CheckReConfigureLogicInBlock ( BlockInfo * bi, int32 overwriteFlag )
{
    if ( GetState ( _CfrTil_, OPTIMIZE_ON | INLINE_ON ) )
    {
        byte * saveHere = Here ;
        if ( bi->LogicCode ) // && ( bi->LogicCodeWord->Symbol->Category & CATEGORY_LOGIC ) )
        {
            SetHere ( bi->LogicCode ) ;
            // standard compile of logic is overwritten for optimize and inline
            if ( overwriteFlag )
            {
                int32 n ;
                _Compile_Return ( ) ;
                bi->bp_Last = Here ;
                for ( n = bi->OverWriteSize - 1 ; n ; n -- ) _Compile_Noop ( ) ; // adjust so Disassemble doesn't get an "invalid" insn; we overwrite a 3 byte insn ( 0fb6c0 : movzx eax, al ) with RET NOP NOP
                SetHere ( saveHere ) ;
            }
            return true ;
        }
    }
    return false ;
}

/*
 *  Logical operators
 */

void
_Compile_LogicResult ( int32 reg )
{
    // return 0 in EAX :
    _Compile_MoveImm_To_Reg ( reg, 0, CELL ) ; // 6 bytes
    _Compile_JumpWithOffset ( 6 ) ; // 6 bytes

    //return 1 in EAX :
    _Compile_MoveImm_To_Reg ( reg, 1, CELL ) ;
}

void
_Compile_LogicalAnd ( Compiler * compiler )
{
    //_Set_SCA ( 0 ) ;
    _Compile_TEST_Reg_To_Reg ( ECX, ECX ) ;
    _Compiler_Setup_BI_tttn ( _Context_->Compiler0, ZERO_TTT, NZ, 3 ) ; // not less than 0 == greater than 0
    Compile_JCC ( Z, ZERO_TTT, Here + 13 ) ; // if eax is zero return not(EAX) == 1 else return 0
    _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
    _Compiler_Setup_BI_tttn ( _Context_->Compiler0, ZERO_TTT, NZ, 3 ) ; // not less than 0 == greater than 0
    Compile_JCC ( NZ, ZERO_TTT, Here + 16 ) ; // if eax is zero return not(EAX) == 1 else return 0
    _Compile_LogicResult ( EAX ) ;
    _Compiler_CompileAndRecord_PushEAX ( compiler ) ;
}

void
Compile_LogicalAnd ( Compiler * compiler )
{
    int32 optFlag = CheckOptimize ( compiler, 5 ) ;
    if ( optFlag & OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        _Compile_LogicalAnd ( compiler ) ;
    }
    else
    {
        //Word *one = Compiler_WordStack ( - 1 ) ; // assumes two values ( n m ) on the DSP stack 
        Word *one = Compiler_WordList ( 1 ) ; // assumes two values ( n m ) on the DSP stack 
        if ( one->StackPushRegisterCode ) SetHere ( one->StackPushRegisterCode ) ;
        else _Compile_Stack_PopToReg ( DSP, EAX ) ;
        _Compile_Stack_PopToReg ( DSP, ECX ) ;
        _Compile_LogicalAnd ( compiler ) ;
    }
}

void
Compile_LogicalNot ( Compiler * compiler )
{
    //int32 negFlag = Z ;
    _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ; // test insn logical and src op and dst op sets zf to result
    _Compiler_Setup_BI_tttn ( compiler, ZERO_TTT, Z, 3 ) ; // if eax is zero zf will equal 1 which is not(EAX) and if eax is not zero zf will equal 0 which is not(EAX)
    Compile_JCC ( Z, ZERO_TTT, Here + 16 ) ; // if eax is zero return not(EAX) == 1 else return 0
    _Compile_LogicResult ( EAX ) ;
    _Compiler_CompileAndRecord_PushEAX ( compiler ) ;
}

void
_Compile_LogicalNot ( Compiler * compiler )
{
    //Word *one = Compiler_WordStack ( - 1 ) ; // assumes two values ( n m ) on the DSP stack 
    Word *one = Compiler_WordList ( 1 ) ; // assumes two values ( n m ) on the DSP stack 
    int optFlag = CheckOptimize ( compiler, 2 ) ; // check especially for cases that optimize literal ops
    if ( optFlag & OPTIMIZE_DONE ) return ;
        // just need to get to valued to be operated on ( not'ed ) in eax
    else if ( optFlag ) //&& ( ! GetState ( _Context_->Compiler0, PREFIX_PARSING ) ) )
    {
        if ( compiler->optInfo->OptimizeFlag & OPTIMIZE_IMM )
        {
            _Compile_MoveImm_To_Reg ( EAX, compiler->optInfo->Optimize_Imm, CELL ) ;
        }
        else if ( compiler->optInfo->Optimize_Rm == DSP )
        {
            _Compile_Move_StackN_To_Reg ( EAX, DSP, 0 ) ;
            //_Compile_Stack_PopToReg ( DSP, EAX ) ;
        }
        else if ( compiler->optInfo->Optimize_Rm != EAX )
        {
            _Compile_GetVarLitObj_RValue_To_Reg ( one, EAX ) ;
        }
    }
    else
    {
        //if ( ( ! GetState ( _Context_->Compiler0, PREFIX_PARSING ) )  && one->StackPushRegisterCode ) SetHere ( one->StackPushRegisterCode ) ; // PREFIX_PARSING : nb! could be a prefix function 
        if ( one->StackPushRegisterCode ) SetHere ( one->StackPushRegisterCode ) ; // PREFIX_PARSING : nb! could be a prefix function 
        else _Compile_Stack_PopToReg ( DSP, EAX ) ;
        //int a, b, c= 0, d ; a = 1; b = !a, d= !c ; _Printf ( "a = %d b = %d c =%d ~d = %d", a, b, c, d ) ;
    }
    Compile_LogicalNot ( compiler ) ;
}

//  logical equals - "=="
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
        _Compiler_Setup_BI_tttn ( _Context_->Compiler0, ZERO_TTT, NZ, 3 ) ; // not less than 0 == greater than 0
        _Compiler_CompileAndRecord_PushEAX ( compiler ) ;
    }
    else
    {
        // operands are still on the stack
        _Compile_Move_StackN_To_Reg ( EAX, DSP, 0 ) ;
        //_Compile_Group1 ( int32 code, int32 toRegOrMem, int32 mod, int32 reg, int32 rm, int32 sib, int32 disp, int32 osize )
        _Compile_X_Group1 ( op, REG, MEM, EAX, DSP, 0, - 4, CELL ) ;
        _Compile_Stack_DropN ( DSP, 2 ) ;

        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        _Compiler_Setup_BI_tttn ( _Context_->Compiler0, ZERO_TTT, NZ, 3 ) ; // not less than 0 == greater than 0
        _Compiler_CompileAndRecord_PushEAX ( compiler ) ;
    }
}
