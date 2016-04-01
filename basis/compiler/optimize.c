#include "../../includes/cfrtil.h"
#define old 0
// TODO : this file is kinda rough and should/could be made a lot more elegant and checked for strict accuracy

/* ************************************************
 * Optimization : observations, assumptions and reasoning ( bottom up heuristics )
 * Optimization here is nothing but a logical reduction of already compiled code and overwriting it with the optimized code.
 * - It seems somewhat similar to the shift-reduce cycle in (top down) compiler theory.
 * ------------------------------------------------
 * - currently optimizable words are : operators (ops), variables, or literals 
 * -- operands are variables or literals and can be the output or function return value of ops (operators)
 * -- the system is either compiling or interpreting, in a block or not
 *      --- if interpreting there are no local variables (they are not implemented yet)
 * -- variables are either stack, local, or global
 * -- local variables are indexed by the FP register
 * -- stacks are indexed by DSP or by FP when there are also local variables
 * -- operators always leave their output operands on the stack unless when optimized by a following operand (which will rewrite the code to leave it in a register)
 * -- operators take either 1 or 2 operands : cf. defines.h
 *      --- 1 operand : @, ~, not, dup, ++, --
 *      --- 2 operand operators are of optimizer->O_two types, ie. taking either ordered or unordered operands
 *          ---- ordered operands : /, -, logic, op_equal : =, +=, -=, *=, etc
 *                  ------ requiring register for a operand : op_equal
 *          ---- unordered operands : +, * 
 * -- ordered operators example subtract : subtract second operand from first and store result in first
- Intel addressing can have 1 or 2 regs or 1 mem address and 1 reg per instruction
 * Assuming :
 * -- reg use order EAX, ECX, EBX, EDX => 1st op EAX, 2nd op ECX, ... (nb: div/mod always uses edx)
 * -- result of any op is always in EAX which may or may not be pushed on the stack (Dsp [0] = TOS = top of stack)
 * -- functions return a value as TOS unless declared different in the locals declaration
 * So :
 * - we take different combinations of previous words that leave operands on the stack
 *  - and optimize access to them for different word categories and let those words know by
 *  - means of Optimize_Flag (cf. defines.h) the state of the operands - whether reg or stack (mem) and 
 *  - in what order or registers
 * 
 * - tail recursion from http://c2.com/cgi/wiki?TailCallOptimization
 * int factorial( int n )
 {
        int accum = 1;
        while ( n >= 2 )
                accum *= n--;
        return accum;
 }
 * *************************************************/

// this function maybe needs to be refined - rethought

void
_GetRmDispImm ( CompileOptimizer * optimizer, Word * word, int32 suggestedReg )
{
    if ( word->CType & REGISTER_VARIABLE )
    {
        optimizer->OptimizeFlag |= OPTIMIZE_REGISTER ;
        optimizer->Optimize_Dest_RegOrMem = REG ;
        optimizer->Optimize_Mod = REG ;
        optimizer->Optimize_Rm = ( suggestedReg != - 1 ) ? suggestedReg : word->RegToUse ;
        optimizer->Optimize_Reg = ( suggestedReg != - 1 ) ? suggestedReg : word->RegToUse ;
    }
    else if ( word->CType & LOCAL_VARIABLE )
    {
        optimizer->Optimize_Rm = FP ;
        optimizer->Optimize_Disp = LocalVarIndex_Disp ( LocalVarOffset ( word ) ) ;
        optimizer->OptimizeFlag |= OPTIMIZE_RM ;
        optimizer->Optimize_Dest_RegOrMem = REG ; // changed in 0.738.212 on 8-22-2013
    }
    else if ( word->CType & PARAMETER_VARIABLE )
    {
        optimizer->Optimize_Rm = FP ;
        optimizer->Optimize_Disp = LocalVarIndex_Disp ( ParameterVarOffset ( word ) ) ;
        optimizer->OptimizeFlag |= OPTIMIZE_RM ;
        optimizer->Optimize_Dest_RegOrMem = REG ; // changed in 0.737.941.2 on
    }
    else if ( word->CType & ( LITERAL | CONSTANT ) )
    {
        optimizer->Optimize_Imm = ( int32 ) * word->W_PtrToValue ;
        optimizer->OptimizeFlag |= OPTIMIZE_IMM ;
    }
    else if ( word->CType & ( VARIABLE ) )
    {
        if ( suggestedReg == - 1 ) suggestedReg = EAX ;
        _Compile_Move_Literal_Immediate_To_Reg ( suggestedReg, ( int32 ) word->W_PtrToValue ) ; // ?? should this be here ??
        optimizer->Optimize_Rm = suggestedReg ;
        optimizer->OptimizeFlag |= OPTIMIZE_RM ;
    }
    else //if ( word->CType & CATEGORY_OP_1_ARG )
    {
        optimizer->Optimize_Rm = DSP ;
        optimizer->OptimizeFlag |= OPTIMIZE_RM ;
    }
}

void
PeepHole_Optimize ( )
{
    if ( GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) )
    {
        byte * here = CompilerMemByteArray->EndIndex ;
        byte sub_Esi_04__add_Esi_04 [ ] = { 0x83, 0xee, 0x04, 0x83, 0xc6, 0x04 } ;
        byte add_esi_04__mov_tos_eax_sub_esi_04 [ ] = { 0x83, 0xc6, 0x04, 0x89, 0x06, 0x83, 0xee, 0x04 } ;
        if ( * ( int* ) ( here - 4 ) == 0x068b0689 ) // 0x89068b06 little endian - movEaxToTos_movTosToEax
        {
            _ByteArray_UnAppendSpace ( CompilerMemByteArray, 2 ) ;
        }
        else if ( ! memcmp ( sub_Esi_04__add_Esi_04, here - 6, 6 ) )
        {
            _ByteArray_UnAppendSpace ( CompilerMemByteArray, 6 ) ;
        }
        else if ( ! memcmp ( add_esi_04__mov_tos_eax_sub_esi_04, here - 8, 8 ) )
        {
            _ByteArray_UnAppendSpace ( CompilerMemByteArray, 8 ) ;
        }
    }
}

// rpn therefore look behind 
// translate word classes into bit patterns

int64
_GetWordStackState ( Compiler * compiler, int count )
{
    int64 state = 0, category ;
    int32 i, j, op, objectFieldFlag = 0 ;
    Word * word ;
    for ( j = 0, i = 0 ; j < count ; j ++, i ++ )
    {
        word = Compiler_WordStack ( compiler, - j ) ;
        if ( ! word ) break ;
        if ( GetState ( word, OPTIMIZE_OFF ) )
        {
            compiler->OptimizeOffWord = word ;
            return 0 ;
        }
        category = word->CType ;
        if ( category & ( DEBUG_WORD ) )
        {
            i -- ;
            continue ;
        }
        else if ( category & ( THIS | OBJECT | OBJECT_FIELD ) )
        {
            if ( objectFieldFlag )
            {
                // this case comes up with arrays
                -- i ;
                if ( category & ( THIS | OBJECT ) )
                {
                    -- count ;
                    objectFieldFlag = false ;
                }
                continue ; // don't record an OBJECT_FIELD
            }
            else if ( category & OBJECT_FIELD )
            {
                if ( ! objectFieldFlag ) ++ count ; // only increment/decrement count once per object
                objectFieldFlag = true ;
                continue ; // don't record an OBJECT_FIELD
            }
            //op = OP_OBJECT ;
            op = OP_VAR ;
        }
        else
        {
            objectFieldFlag = false ;
            if ( category & ( LITERAL | CONSTANT ) ) op = OP_LC ;
                //else if ( category & ( THIS | OBJECT | OBJECT_FIELD ) ) op = OP_OBJECT ;
            else if ( category & ( LOCAL_VARIABLE | PARAMETER_VARIABLE | VARIABLE ) ) op = OP_VAR ;
            else if ( category & ( CATEGORY_EQUAL ) ) op = OP_EQUAL ;
            else if ( category & ( CATEGORY_OP_EQUAL ) ) op = OP_OPEQUAL ;
            else if ( category & ( CATEGORY_OP_1_ARG ) ) op = OP_1_ARG ;
            else if ( category & ( CATEGORY_LOGIC ) ) op = OP_LOGIC ;
            else if ( category & ( CATEGORY_OP_UNORDERED ) ) op = OP_UNORDERED ;
            else if ( category & ( CATEGORY_OP_STORE ) ) op = OP_STORE ;
            else if ( category & ( CATEGORY_OP_ORDERED ) ) op = OP_ORDERED ;
            else if ( category & ( CATEGORY_DUP ) ) op = OP_DUP ;
                //else if ( category & ( CATEGORY_RECURSIVE ) ) op = OP_RECURSE ;
            else if ( category & ( CATEGORY_OP_LOAD ) ) op = OP_FETCH ;
            else if ( category & ( CATEGORY_OP_DIVIDE ) ) op = OP_DIVIDE ;
            else if ( category & ( CPRIMITIVE | BLOCK ) ) op = OP_CPRIMITIVE ;
                //else if ( category & ( STACKING ) ) op = OP_STACK ;
            else
            {
                break ;
            }
        }
        state |= ( op << ( i * O_BITS ) ) ;
    }
    return state ;
}


// A rewriting optimizer with smart operators :
// we have operands on the stack, this optimizes their locations for an operation
// such as add, sub, mul, div, shl, shr, =, etc.
// the op will then be : op dst, src -- intel syntax
// generally set it up so that src operand is in eax register and dst operand is tos
// if we can set this up return true else false
//
// the optimization is simple reason applied to the situation where it 
// occurs based solely on what words are already in the Compiler Word Stack (compiler->WordStack)
// eg. an operator that takes 2 args : if there is optimizer->O_one in the optimize window 
// the other must be on stack so we assume it to be there and rewrite the code
// as if it was there, if it wasn't there the user has made an error by using the
// operator in the first place and also, of course, the compiled code will probably crash

int32
_CheckOptimizeOperands ( Compiler * compiler, int32 maxOperands )
{
    int32 i = 0 ;
    if ( GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) )
    {
        CompileOptimizer_Init ( compiler ) ;
        CompileOptimizer * optimizer = compiler->Optimizer ;
        int64 state = _GetWordStackState ( compiler, maxOperands ) ;
        int32 depth = Stack_Depth ( compiler->WordStack ) ;
        if ( maxOperands > depth ) maxOperands = depth ;
        for ( i = maxOperands ; i > 0 ; i -- )
        {
            long int mask = 0xf << ( i * O_BITS ) ;
            if ( state )
            {
                state &= ~ mask ;
                switch ( state )
                    // these cases of bitwised ORed values represent the "optimize window" ; the right most ORed value is top of word stack = current word
                    // the leftmost ORed value is fartherest down in the stack, etc.
                {
                    case ( OP_VAR << ( 3 * O_BITS ) | OP_FETCH << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_VAR << ( 3 * O_BITS ) | OP_FETCH << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_VAR << ( 3 * O_BITS ) | OP_FETCH << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_VAR << ( 3 * O_BITS ) | OP_FETCH << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_LOGIC ):
                    {
                        SetHere ( optimizer->O_three->Coding ) ;
                        if ( compiler->NumberOfRegisterVariables )
                        {
                            _GetRmDispImm ( optimizer, optimizer->O_one, - 1 ) ;
                            _GetRmDispImm ( optimizer, optimizer->O_three, - 1 ) ;
                        }
                        else
                        {
                            _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_three, EAX ) ;
                            if ( optimizer->O_zero->CType & CATEGORY_OP_DIVIDE ) _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_one, ECX ) ;
                            _GetRmDispImm ( optimizer, optimizer->O_one, ECX ) ;
                            optimizer->Optimize_Dest_RegOrMem = REG ;
                            optimizer->Optimize_Mod = REG ;
                        }
                        return i ;
                    }
                        // CONSTANT_FOLDING 
                        // this is the code to collapse (fold) constants and literals with integer operators
                        // can collapse a pipeline of constants with operators
                    case ( OP_LC << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_LC << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_LC << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_LC << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_LOGIC ):
#if 0                        
                    {
                        int32 value ;
                        SetHere ( optimizer->O_two->Coding ) ;
                        // a little tricky here ...
                        // ?? maybe we should setup and use a special compiler stack and use it here ... ??
                        Compiler_SetState ( compiler, COMPILE_MODE, false ) ;
                        _Push ( ( int32 ) * optimizer->O_two->W_PtrToValue ) ;
                        _Push ( ( int32 ) * optimizer->O_one->W_PtrToValue ) ;
                        _Word_Run ( optimizer->O_zero ) ;
                        value = _DataStack_Pop ( ) ;
                        *optimizer->O_zero->W_PtrToValue = value ;
                        Compiler_SetState ( compiler, COMPILE_MODE, true ) ;
                        _Compile_MoveImm_To_Reg ( EAX, value, CELL ) ;
                        _Word_CompileAndRecord_PushEAX ( optimizer->O_zero ) ;
                        _Stack_DropN ( compiler, 3 ) ;
                        d0 ( if ( IsDebugOn ) Compiler_ShowWordStack ( ( byte* ) "\n_CheckOptimizeOperands : after DropN ( 3 ) :" ) ) ;
                        _Stack_Push ( compiler, ( int32 ) optimizer->O_zero ) ;
                        return OPTIMIZE_DONE ;
                    }
#else
                    {
                        int32 value ;
                        SetHere ( optimizer->O_two->Coding ) ;
                        // a little tricky here ...
                        // ?? maybe we should setup and use a special compiler stack and use it here ... ??
                        _Push ( ( int32 ) *optimizer->O_two->W_PtrToValue ) ;
                        _Push ( ( int32 ) *optimizer->O_one->W_PtrToValue ) ;
                        Compiler_SetState ( compiler, COMPILE_MODE, false ) ;
                        _Word_Run ( optimizer->O_zero ) ;
                        Compiler_SetState ( compiler, COMPILE_MODE, true ) ;
                        value = _DataStack_Pop ( ) ;
                        if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) )
                        {
                            _Compile_MoveImm_To_Reg ( EAX, value, CELL ) ;
                        }
                        else _Compile_Stack_Push ( DSP, value ) ;
                        d1 ( if ( IsDebugOn ) Compiler_ShowWordStack ( (byte*) "\n_CheckOptimizeOperands : before DropN ( 2 ) :" ) ) ;
                        // 'optimizer->O_two' is left on the WordStack but its value is replaced by result value 
#if 1                        
                        _Stack_DropN ( compiler->WordStack, 2 ) ;
                        *optimizer->O_two->W_PtrToValue = value ;// ?? ad hoc hack :: i don't like this -- changing the value of a literal -- not functional ??
#else                        
                        _Stack_DropN ( compiler->WordStack, 3 ) ;
                        Compiler_SetState ( compiler, COMPILE_MODE, false ) ;
                        Word * word = _DataObject_New ( LITERAL, 0, "temp", 0, 0, 0, value, 0 ) ;
                        Compiler_SetState ( compiler, COMPILE_MODE, true ) ;
                        _Stack_Push ( compiler->WordStack, (int32) word ) ;
#endif                        
                        return OPTIMIZE_DONE ;
                    }
#endif                    
                    case ( OP_VAR << ( 4 * O_BITS ) | OP_FETCH << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_VAR << ( 4 * O_BITS ) | OP_FETCH << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_VAR << ( 4 * O_BITS ) | OP_FETCH << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_VAR << ( 4 * O_BITS ) | OP_FETCH << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_LOGIC ):
                        // v 0.737.941.2               
                    {
                        SetHere ( optimizer->O_four->Coding ) ;
                        if ( compiler->NumberOfRegisterVariables )
                        {
                            optimizer->Optimize_DstReg = optimizer->O_four->RegToUse ;
                            optimizer->Optimize_SrcReg = optimizer->O_two->RegToUse ;
                            _GetRmDispImm ( optimizer, optimizer->O_two, EAX ) ;
                        }
                        else
                        {
                            _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_four, EAX ) ;
                            _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_two, ECX ) ;
                            optimizer->Optimize_Dest_RegOrMem = REG ;
                            optimizer->Optimize_Mod = REG ;
                            optimizer->Optimize_Reg = EAX ; // shouldn't need this but some code still references this as the rm ?? fix ??
                            optimizer->Optimize_Rm = ECX ;
                        }
                        return i ;
                    }
                    case ( OP_LC << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_LC << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_LC << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_LOGIC ):
                    case ( OP_LC << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_DIVIDE ):
                    {
                        SetHere ( optimizer->O_three->Coding ) ;
                        _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_three, EAX ) ;
                        if ( optimizer->O_zero->CType & CATEGORY_OP_DIVIDE ) _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_two, ECX ) ;
                        _GetRmDispImm ( optimizer, optimizer->O_three, - 1 ) ;
                        optimizer->OptimizeFlag |= OPTIMIZE_IMM ;
                        optimizer->Optimize_Dest_RegOrMem = REG ;
                        optimizer->Optimize_Mod = REG ;
                        return i ;
                    }
                        // ?? assume correct first operand is there TOS - it would already be a user error if they were not ??
#if 0 // need to teach this a few things first                    
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_DIVIDE ):
#endif                        
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_LOGIC ):
                    {
                        SetHere ( optimizer->O_two->Coding ) ;
                        _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_two, ECX ) ;
                        optimizer->Optimize_Dest_RegOrMem = MEM ;
                        optimizer->Optimize_Mod = MEM ;
                        optimizer->Optimize_Reg = EAX ; // shouldn't need this but some code still references this as the rm ?? fix ??
                        optimizer->Optimize_Rm = DSP ;
                        return i ;
                    }
                    case ( OP_DUP << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_DUP << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_DUP << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_DUP << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_LOGIC ):
                    {
                        SetHere ( optimizer->O_two->Coding ) ;
                        _GetRmDispImm ( optimizer, optimizer->O_one, - 1 ) ;
                        optimizer->Optimize_Dest_RegOrMem = MEM ;
                        optimizer->Optimize_Mod = MEM ;
                        optimizer->Optimize_Rm = DSP ;
                        return i ;
                    }
                        // assume correct first operand must be there on the stack
                    case ( OP_LC << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_LC << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_LC << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_LC << ( 1 * O_BITS ) | OP_LOGIC ):
                    {
                        SetHere ( optimizer->O_one->Coding ) ;
                        _GetRmDispImm ( optimizer, optimizer->O_one, - 1 ) ;
                        optimizer->Optimize_Dest_RegOrMem = MEM ;
                        optimizer->Optimize_Mod = MEM ;
                        optimizer->Optimize_Rm = DSP ;
                        return i ;
                    }
                    case ( OP_DIVIDE << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_DIVIDE << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_DIVIDE << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_ORDERED << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_UNORDERED << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_ORDERED << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_UNORDERED << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_ORDERED << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_UNORDERED << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_ORDERED ):
                    {
                        if ( optimizer->O_two->StackPushRegisterCode )
                        {
                            SetHere ( optimizer->O_two->StackPushRegisterCode ) ; // leave optimizer->O_two value in EAX we don't need to push it
                            _GetRmDispImm ( optimizer, optimizer->O_one, - 1 ) ;
                            optimizer->Optimize_Dest_RegOrMem = REG ;
                            optimizer->Optimize_Mod = REG ;
                            optimizer->Optimize_Reg = EAX ; // only for "mod" will it be edx else eax
                            optimizer->Optimize_Rm = EAX ;
                            return i ;
                        }
                        else return 0 ;
                    }
                    case ( OP_UNORDERED << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_ORDERED << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_DIVIDE << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_DIVIDE << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_DIVIDE << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_ORDERED << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_UNORDERED << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_ORDERED << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_UNORDERED << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_ORDERED ):
                    {
                        if ( optimizer->O_three->StackPushRegisterCode )
                        {
                            SetHere ( optimizer->O_three->StackPushRegisterCode ) ; // leave optimizer->O_two value in EAX we don't need to push it
                            _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_two, ECX ) ;
                            optimizer->Optimize_Dest_RegOrMem = REG ;
                            optimizer->Optimize_Mod = REG ;
                            optimizer->Optimize_Reg = EAX ; // shouldn't need this but some code still references this as the rm ?? fix ??
                            optimizer->Optimize_Rm = ECX ;
                            return i ;
                        }
                        else return 0 ;
                    }
                    case ( OP_VAR << ( 3 * O_BITS ) | OP_FETCH << ( 2 * O_BITS ) | OP_DUP << ( 1 * O_BITS ) | OP_1_ARG ):
                    {
                        SetHere ( optimizer->O_three->Coding ) ;
                        _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_three, EAX ) ;
                        Compile_ADDI ( REG, DSP, 0, 2 * CELL, BYTE ) ;
                        _Compile_Move_Reg_To_StackN ( DSP, - 1, EAX ) ;
                        _Compile_Move_Reg_To_StackN ( DSP, 0, EAX ) ;
                        optimizer->Optimize_Mod = MEM ;
                        optimizer->Optimize_Rm = DSP ;
                        return i ;
                    }
                        // CONSTANT_FOLDING - 1 arg op
                        // this is the code to collapse (fold) constants and literals with integer operators
                        // can collapse a pipeline of constants with operators
                    case ( OP_LC << ( 1 * O_BITS ) | OP_1_ARG ):
                    {
                        // compile to a constant
                        int32 value ;
                        SetHere ( optimizer->O_one->Coding ) ;
                        // a little tricky here ...
                        // ?? maybe we should setup and use a special compiler stack and use it here ... ??
                        //_DataStack_Push ( (int32) optimizer->O_two->Object ) ;
                        _Push ( ( int32 ) * optimizer->O_one->W_PtrToValue ) ;
                        Compiler_SetState ( compiler, COMPILE_MODE, false ) ;
                        _Word_Run ( optimizer->O_zero ) ;
                        Compiler_SetState ( compiler, COMPILE_MODE, true ) ;
                        value = _DataStack_Pop ( ) ;
                        //if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) )
                        {
                            _Compile_MoveImm_To_Reg ( EAX, value, CELL ) ;
                        }
                        _Word_CompileAndRecord_PushEAX ( optimizer->O_zero ) ;
                        d0 ( if ( IsDebugOn ) Compiler_ShowWordStack ( ( byte* ) "\n_CheckOptimizeOperands : before DropN ( 2 ) :" ) ) ;
                        _Stack_DropN ( compiler->WordStack, 2 ) ;
                        _Stack_Push ( compiler->WordStack, ( int32 ) optimizer->O_zero ) ;

                        // 'optimizer->O_two' is left on the WordStack but its value is replaced by result value 
                        *optimizer->O_zero->W_PtrToValue = value ;
                        return OPTIMIZE_DONE ;
                    }
                    case ( OP_VAR << ( 1 * O_BITS ) | OP_1_ARG ):
                    {
                        SetHere ( optimizer->O_one->Coding ) ;
                        _GetRmDispImm ( optimizer, optimizer->O_one, - 1 ) ;
                        return i ;
                    }
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_UNORDERED << ( 1 * O_BITS ) | OP_1_ARG ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_ORDERED << ( 1 * O_BITS ) | OP_1_ARG ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_LOGIC << ( 1 * O_BITS ) | OP_1_ARG ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_1_ARG << ( 1 * O_BITS ) | OP_1_ARG ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_DIVIDE << ( 1 * O_BITS ) | OP_1_ARG ):
                    {
                        // this is mainly for postfix inc/dec in C_SYNTAX
                        if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) )
                        {
                            if ( optimizer->O_one->StackPushRegisterCode )
                            {
                                SetHere ( optimizer->O_one->StackPushRegisterCode ) ;
                            }
                            _GetRmDispImm ( optimizer, optimizer->O_two, - 1 ) ;
                            return i ;
                        }
                        else return 0 ;
                    }
                    case ( OP_UNORDERED << ( 1 * O_BITS ) | OP_1_ARG ):
                    case ( OP_ORDERED << ( 1 * O_BITS ) | OP_1_ARG ):
                    case ( OP_LOGIC << ( 1 * O_BITS ) | OP_1_ARG ):
                    case ( OP_1_ARG << ( 1 * O_BITS ) | OP_1_ARG ):
                    case ( OP_DIVIDE << ( 1 * O_BITS ) | OP_1_ARG ):
                    {
                        if ( ! GetState ( _Q_->OVT_Context, C_SYNTAX ) )
                        {
                            if ( optimizer->O_one->StackPushRegisterCode ) // leave value in EAX, don't push it
                            {
                                SetHere ( optimizer->O_one->StackPushRegisterCode ) ; // leave optimizer->O_two value in EAX we don't need to push it
                                compiler->Optimizer->Optimize_Rm = EAX ;
                                optimizer->Optimize_Mod = REG ;
                            }
                            else compiler->Optimizer->Optimize_Rm = DSP ;
                            return i ;
                        }
                        else return 0 ;
                    }
                    case ( OP_STACK << ( 1 * O_BITS ) | OP_1_ARG ):
                    {
                        optimizer->Optimize_Mod = MEM ;
                        optimizer->Optimize_Rm = DSP ;
                        return i ;
                    }
                        // op equal :: ...
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_EQUAL ):
                    {
                        SetHere ( optimizer->O_two->Coding ) ;
                        if ( optimizer->O_two->CType & REGISTER_VARIABLE ) compiler->Optimizer->Optimize_Dest_RegOrMem = REG ;
                        else optimizer->Optimize_Dest_RegOrMem = MEM ;
                        _Compile_VarLitObj_LValue_To_Reg ( optimizer->O_two, EAX ) ;
                        _GetRmDispImm ( optimizer, optimizer->O_one, - 1 ) ;
                        optimizer->Optimize_Reg = EAX ; // shouldn't need this but some code still references this as the rm ?? fix ??
                        return (i | OPTIMIZE_RESET ) ;
                    }
                    case ( OP_VAR << ( 3 * O_BITS ) | OP_FETCH << ( 2 * O_BITS ) | OP_DIVIDE << ( 1 * O_BITS ) | OP_EQUAL ):
                    case ( OP_VAR << ( 3 * O_BITS ) | OP_FETCH << ( 2 * O_BITS ) | OP_LOGIC << ( 1 * O_BITS ) | OP_EQUAL ):
                    case ( OP_VAR << ( 3 * O_BITS ) | OP_FETCH << ( 2 * O_BITS ) | OP_UNORDERED << ( 1 * O_BITS ) | OP_EQUAL ):
                    case ( OP_VAR << ( 3 * O_BITS ) | OP_FETCH << ( 2 * O_BITS ) | OP_ORDERED << ( 1 * O_BITS ) | OP_EQUAL ):
                    {
                        if ( optimizer->O_one->StackPushRegisterCode )
                        {
                            SetHere ( optimizer->O_one->StackPushRegisterCode ) ;
                            _Compile_Move_StackN_To_Reg ( ECX, DSP, 0 ) ;
                            _Compile_Move_Reg_To_Rm ( ECX, EAX, 0 ) ;
                            return ( OPTIMIZE_DONE | OPTIMIZE_RESET ) ;
                        }
                        else return 0 ;
                    }
                        // this pattern occurs in factorial
                    case ( OP_VAR << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_EQUAL ):
                    {
                        SetHere ( optimizer->O_three->Coding ) ;
                        // arg order is maintained here
                        _Compile_VarLitObj_LValue_To_Reg ( optimizer->O_three, EAX ) ;
                        SetHere ( optimizer->O_three->StackPushRegisterCode ) ;
                        _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_two, ECX ) ;
                        _Compile_Move_Reg_To_Rm ( EAX, ECX, 0 ) ;
                        return ( OPTIMIZE_DONE | OPTIMIZE_RESET ) ;
                    }
                    case ( OP_VAR << ( 3 * O_BITS ) | OP_VAR << ( 2 * O_BITS ) | OP_FETCH << ( 1 * O_BITS ) | OP_OPEQUAL ):
                    {
                        SetHere ( optimizer->O_three->Coding ) ;
                        if ( optimizer->O_zero->CType & BIT_SHIFT )
                        {
                            _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_two, ECX ) ;
                            _GetRmDispImm ( optimizer, optimizer->O_three, - 1 ) ;
                        }
                        else
                        {
                            _Compile_VarLitObj_LValue_To_Reg ( optimizer->O_three, EAX ) ;
                            _GetRmDispImm ( optimizer, optimizer->O_two, - 1 ) ;
                        }
                        return (i | OPTIMIZE_RESET ) ;
                    }
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_OPEQUAL ):
                    {
                        SetHere ( optimizer->O_two->Coding ) ;
                        if ( optimizer->O_two->CType & REGISTER_VARIABLE ) compiler->Optimizer->Optimize_Dest_RegOrMem = REG ;
                        else optimizer->Optimize_Dest_RegOrMem = MEM ;
                        _Compile_VarLitObj_LValue_To_Reg ( optimizer->O_two, EAX ) ;
                        _GetRmDispImm ( optimizer, optimizer->O_one, - 1 ) ;
                        return ( i | OPTIMIZE_RESET ) ;
                    }
#if 0                    
                        //case ( OP_VAR << ( 2 * O_BITS ) | OP_CPRIMITIVE << ( 1 * O_BITS ) | OP_EQUAL ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_EQUAL ):
                    {
                        if ( optimizer->O_two->StackPushRegisterCode )
                        {
                            SetHere ( optimizer->O_two->Coding ) ;
                            // arg order is maintained here
                            _Compile_VarLitObj_LValue_To_Reg ( optimizer->O_two, EAX ) ;
                            if ( optimizer->O_two->StackPushRegisterCode ) SetHere ( optimizer->O_two->StackPushRegisterCode ) ;
                            _Compile_VarLitObj_LValue_To_Reg ( optimizer->O_one, ECX ) ;
                            optimizer->Optimize_Rm = EAX ;
                            optimizer->Optimize_Reg = ECX ;
                            return (i | OPTIMIZE_RESET ) ;
                        }
                        else return 0 ;
                    }
#endif                    
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_STORE ): // "!" - store
                    {
                        if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) ) //&& ( optimizer->O_two->StackPushRegisterCode ) )
                        {
                            //assume operand is in the EAX reg
                            SetHere ( optimizer->O_two->Coding ) ;
                            _Compile_VarLitObj_LValue_To_Reg ( optimizer->O_one, EAX ) ;
                            _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_two, ECX ) ;
                            _Compile_Move_Reg_To_Rm ( EAX, ECX, 0 ) ;
                            return ( OPTIMIZE_DONE | OPTIMIZE_RESET ) ;
                        }
                        else return 0 ;
                    }
                    case ( OP_LC << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_STORE ): // "!" - store
                    {
                        SetHere ( optimizer->O_two->Coding ) ;
                        _GetRmDispImm ( optimizer, optimizer->O_two, - 1 ) ;
                        if ( optimizer->O_one->CType & REGISTER_VARIABLE ) _Compile_MoveImm ( REG, EAX, 0, 0, optimizer->Optimize_Imm, CELL ) ;
                        else
                        {
                            _Compile_VarLitObj_LValue_To_Reg ( optimizer->O_one, EAX ) ;
                            _Compile_MoveImm ( MEM, EAX, 0, 0, optimizer->Optimize_Imm, CELL ) ;
                        }
                        return ( OPTIMIZE_DONE | OPTIMIZE_RESET ) ;
                    }
#if 1                   
                    case ( OP_DUP << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_STORE ): // "!" - store
                    {
                        SetHere ( optimizer->O_two->Coding ) ;
                        //_GetRmDispImm ( optimizer, optimizer->O_two, - 1 ) ;
                        if ( ! ( optimizer->O_one->CType & REGISTER_VARIABLE ) ) //_Compile_MoveImm ( REG, EAX, 0, 0, optimizer->Optimize_Imm, CELL ) ;
                        {
                            _Compile_VarLitObj_LValue_To_Reg ( optimizer->O_one, ECX ) ;
                        }
                        _Compile_Move_Reg_To_Rm ( ECX, EAX, 0 ) ;
                        return ( OPTIMIZE_DONE | OPTIMIZE_RESET ) ;
                    }
#endif                    
                    case ( OP_DIVIDE << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_STORE ): // "!" - store
                    case ( OP_ORDERED << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_STORE ): // "!" - store
                    case ( OP_UNORDERED << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_STORE ): // "!" - store
                    case ( OP_LOGIC << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_STORE ): // "!" - store
                    {
                        if ( optimizer->O_two->StackPushRegisterCode )
                        {
                            SetHere ( optimizer->O_two->StackPushRegisterCode ) ; // leave optimizer->O_two value in EAX we don't need to push it
                            if ( optimizer->O_one->CType & REGISTER_VARIABLE ) // this logic may need to be refined but it works with our C.factorial 
                            {
                                return OPTIMIZE_DONE ;
                            }
                            _Compile_VarLitObj_LValue_To_Reg ( optimizer->O_one, ECX ) ;
                            _Compile_Move_Reg_To_Rm ( ECX, EAX, 0 ) ;
                            return ( OPTIMIZE_DONE | OPTIMIZE_RESET ) ;
                        }
                        else return 0 ;
                    }
                    case ( OP_FETCH << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_STORE ): // "!" - store
                    {
                        if ( optimizer->O_two->StackPushRegisterCode )
                        {
                            SetHere ( optimizer->O_two->StackPushRegisterCode ) ; //Coding ) ;
                            _Compile_VarLitObj_LValue_To_Reg ( optimizer->O_one, ECX ) ;
                            _Compile_Move_Reg_To_Rm ( ECX, EAX, 0 ) ;
                            return ( OPTIMIZE_DONE | OPTIMIZE_RESET ) ; // reset after '=' or store 
                        }
                        else return 0 ;
                    }
                        // C Syntax :: ...
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_LOGIC ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_LC << ( 1 * O_BITS ) | OP_DIVIDE ):
                    {
                        if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) )
                        {
                            SetHere ( optimizer->O_two->Coding ) ;
                            if ( compiler->NumberOfRegisterVariables )
                            {
                                _GetRmDispImm ( optimizer, optimizer->O_one, - 1 ) ;
                                _GetRmDispImm ( optimizer, optimizer->O_two, - 1 ) ;
                            }
                            else
                            {
                                if ( optimizer->O_two->CType & REGISTER_VARIABLE ) _GetRmDispImm ( optimizer, optimizer->O_two, - 1 ) ;
                                else _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_two, EAX ) ;
                                _GetRmDispImm ( optimizer, optimizer->O_one, ECX ) ;
                                //_Compile_VarLitObj_RValue_To_Reg ( optimizer->O_one, ECX ) ;
                                optimizer->Optimize_Dest_RegOrMem = REG ;
                                optimizer->Optimize_Mod = REG ;
                                optimizer->Optimize_Reg = EAX ; // shouldn't need this but some code still references this as the rm ?? fix ??
                            }
                            return i ;
                        }
                        else return 0 ; // save time; instead of continue ;
                    }
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_ORDERED ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_LOGIC ):
                    case ( OP_VAR << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_DIVIDE ):
                    {
                        if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) )
                        {
                            SetHere ( optimizer->O_two->Coding ) ;
                            if ( compiler->NumberOfRegisterVariables )
                            {
                                optimizer->Optimize_DstReg = optimizer->O_two->RegToUse ;
                                optimizer->Optimize_SrcReg = optimizer->O_one->RegToUse ;
                                _GetRmDispImm ( optimizer, optimizer->O_one, EAX ) ;
                            }
                            else
                            {
                                _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_two, EAX ) ;
                                //_GetRmDispImm ( optimizer, optimizer->O_two, - 1 ) ;
                                _GetRmDispImm ( optimizer, optimizer->O_one, - 1 ) ;
                            }
                            return i ;
                        }
                        else continue ;
                    }
                    case ( OP_LC << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_LOGIC ):
                    case ( OP_LC << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_DIVIDE ):
                    case ( OP_LC << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_UNORDERED ):
                    case ( OP_LC << ( 2 * O_BITS ) | OP_VAR << ( 1 * O_BITS ) | OP_ORDERED ):
                    {
                        if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) )
                        {
                            SetHere ( optimizer->O_two->Coding ) ;
                            _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_one, ECX ) ;
                            //_GetRmDispImm ( optimizer, optimizer->O_two, - 1 ) ;
                            _GetRmDispImm ( optimizer, optimizer->O_two, - 1 ) ;
                            //_Compile_VarConstOrLit_LValue_To_Reg ( optimizer->O_two, EAX)
                            optimizer->Optimize_Mod = REG ;
                            optimizer->Optimize_Dest_RegOrMem = REG ;
                            return i ;
                        }
                        else continue ;
                    }
                    case ( OP_VAR << ( 1 * O_BITS ) | OP_FETCH ):
                    {

                        if ( ! ( optimizer->O_one->CType & REGISTER_VARIABLE ) )
                        {
                            SetHere ( optimizer->O_one->Coding ) ;
                            _Compile_VarLitObj_RValue_To_Reg ( optimizer->O_one, EAX ) ;
                            _Word_CompileAndRecord_PushEAX ( optimizer->O_one ) ;
                            optimizer->O_zero->StackPushRegisterCode = optimizer->O_one->StackPushRegisterCode ; // used in further optimization
                        }
                        return ( OPTIMIZE_DONE | OPTIMIZE_DONT_RESET ) ;
                    }
                    default: continue ;
                }
            }
        }
    }
    return i ;
}

int32
CheckOptimize ( Compiler * compiler, int32 maxOperands )
{
    int32 rtrn ;
    d1 ( if ( IsDebugOn ) Compiler_ShowWordStack ( ( byte* ) "\nCheckOptimize : before optimize :" ) ) ;
    rtrn = _CheckOptimizeOperands ( compiler, maxOperands ) ;
    d0 ( if ( IsDebugOn ) Compiler_ShowWordStack ( "\nCheckOptimize : after optimize :" ) ) ;
    if ( rtrn & OPTIMIZE_RESET ) Stack_Init ( compiler->WordStack ) ;
    if ( compiler->OptimizeOffWord )
    {
        SetState ( compiler->OptimizeOffWord, OPTIMIZE_OFF, false ) ;
        compiler->OptimizeOffWord = 0 ; // compiler is being reset often
    }
    return rtrn ;
}


