#include "../../includes/cfrtil.h"

void
_Compile_CallEAX ( void )
{
    //_Compile_Group5 ( int32 code, int32 mod, int32 rm, int32 sib, int32 disp, int32 size )
    _Compile_Group5 ( CALL, 3, 0, 0, 0, 0 ) ;
}

void
Compile_DataStack_PopAndCall ( void )
{
    Compile_Pop_ToEAX_AndCall ( DSP ) ;
}

// >R - Rsp to

void
_Compile_Rsp_To ( ) // data stack pop to rsp
{
    Compile_DspPop_EspPush ( ) ;
}

// rdrop

void
_Compile_Rsp_Drop ( )
{
    _Compile_DropN_ESP ( 1 ) ;
}

// rsp

void
_Compile_Rsp_Get ( )
{
    _Compile_Stack_PushReg ( DSP, ESP ) ;
}

// rsp@

void
_Compile_Rsp_Fetch ( )
{
    _Compile_Move_Reg_To_Reg ( EAX, ESP ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 * CELL ) ;
    _Compile_Stack_PushReg ( DSP, EAX ) ;
}

// r< - r from

void
_Compile_Rsp_From ( )
{
    _Compile_Rsp_Fetch ( ) ;
    _Compile_Rsp_Drop ( ) ;
}

// Rsp! - Rsp store

void
_Compile_Rsp_Store ( ) // data stack pop to rsp [0] !
{
    _Compile_Rsp_From ( ) ;
    Compile_Store ( _Context_->Compiler0, DSP ) ;
}

Word *
_CfrTil_VariableGet ( Namespace * ns, byte * name )
{
    Word * word = Word_FindInOneNamespace ( ns, name ) ;
    return word ;
}

int32
_CfrTil_VariableValueGet ( byte* nameSpace, byte * name )
{
    return _Namespace_VariableValueGet ( Namespace_Find ( nameSpace ), name ) ;
}

// set the value at address to reg - value in reg
void
_Compile_SetAtAddress_WithReg ( int * address, int32 reg, int32 thruReg )
{
    _Compile_Move_Literal_Immediate_To_Reg ( thruReg, ( int32 ) address ) ;
    _Compile_Move_Reg_To_Rm ( thruReg, reg, 0 ) ;
}

void
_Compile_Move_Literal_Immediate_To_Reg ( int32 reg, int32 value )
{
    _Compile_MoveImm_To_Reg ( reg, value, CELL ) ;
}

void
_Compile_LocalOrStackVar_RValue_To_Reg ( Word * word, int32 reg )
{
    word->Coding = Here ; // we don't need the word's code if compiling -- this is an optimization though
    if ( word->CProperty & REGISTER_VARIABLE )
    {
        if ( word->RegToUse == reg ) return ;
        else _Compile_Move_Reg_To_Reg ( reg, word->RegToUse ) ;
    }
    else if ( word->CProperty & LOCAL_VARIABLE )
    {
        _Compile_Move_StackN_To_Reg ( reg, FP, LocalVarOffset ( word ) ) ; // 2 : account for saved fp and return slot
    }
    else if ( word->CProperty & PARAMETER_VARIABLE )
    {
        _Compile_Move_StackN_To_Reg ( reg, FP, ParameterVarOffset ( word ) ) ; // account for stored bp and return value
    }
    else if ( word->CProperty & ( OBJECT | THIS ) )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) * word->W_PtrToValue ) ;
    }
}

void
Do_ObjectOffset ( Word * word, int32 reg )
{
    Compiler * compiler = _Context_->Compiler0 ;
    int32 offset = word->AccumulatedOffset ;
    Compile_ADDI ( REG, reg, 0, offset, CELL ) ;
    compiler->AccumulatedOffsetPointer = ( int32* ) ( Here - CELL ) ; // offset will be calculated as we go along by ClassFields and Array accesses
}

void
_Compile_GetVarLitObj_RValue_To_Reg ( Word * word, int32 reg )
{
    word->Coding = Here ; // we don't need the word's code if compiling -- this is an optimization though
    if ( word->CProperty & REGISTER_VARIABLE )
    {
        return ;
        //if ( word->RegToUse == reg ) return ;
        //else _Compile_Move_Reg_To_Reg ( reg, word->RegToUse ) ;
    }
    else if ( word->CProperty & LOCAL_VARIABLE )
    {
        _Compile_Move_StackN_To_Reg ( reg, FP, LocalVarOffset ( word ) ) ; // 2 : account for saved fp and return slot
    }
    else if ( word->CProperty & PARAMETER_VARIABLE )
    {
        _Compile_Move_StackN_To_Reg ( reg, FP, ParameterVarOffset ( word ) ) ; // account for stored bp and return value
    }
    else if ( word->CProperty & NAMESPACE_VARIABLE )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) word->W_PtrToValue ) ;
        _Compile_Move_Rm_To_Reg ( reg, reg, 0 ) ;
    }
    else if ( word->CProperty & ( LITERAL | CONSTANT | OBJECT | THIS ) )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) * word->W_PtrToValue ) ;
    }
    else SyntaxError ( QUIT ) ;
    if ( word->CProperty & ( OBJECT | THIS ) )
    {
        Do_ObjectOffset ( word, reg ) ;
        _Compile_Move_Rm_To_Reg ( reg, reg, 0 ) ;
    }
}

// this is not tested for VARIABLE or REGISTER_VARIABLE

void
_Compile_SetVarLitObj_With_Reg ( Word * word, int32 reg, int32 thruReg )
{
    word->Coding = Here ; // we don't need the word's code if compiling -- this is an optimization though
    if ( word->CProperty & REGISTER_VARIABLE )
    {
        if ( word->RegToUse == reg ) return ;
        else _Compile_Move_Reg_To_Reg ( word->RegToUse, reg ) ;
    }
    else if ( word->CProperty & LOCAL_VARIABLE )
    {
        _Compile_Move_Reg_To_StackN ( FP, LocalVarOffset ( word ), reg ) ;
    }
    else if ( word->CProperty & PARAMETER_VARIABLE )
    {
        _Compile_Move_Reg_To_StackN ( FP, ParameterVarOffset ( word ), reg ) ;
    }
    else if ( word->CProperty & NAMESPACE_VARIABLE )
    {
        //_Compile_Move_Literal_Immediate_To_Reg ( thruReg, ( int32 ) word->W_PtrToValue ) ;
        //_Compile_Move_Reg_To_Rm ( thruReg, reg, 0 ) ;
        _Compile_SetAtAddress_WithReg ( (int*) word->W_PtrToValue, reg, thruReg ) ;
    }
}

void
_Compile_GetVarLitObj_LValue_To_Reg ( Word * word, int32 reg )
{
    word->Coding = Here ;
    if ( word->CProperty & REGISTER_VARIABLE )
    {
        if ( word->RegToUse == reg ) return ;
        else _Compile_Move_Reg_To_Reg ( reg, word->RegToUse ) ;
    }
    else if ( word->CProperty & ( OBJECT | THIS ) || ( word->WProperty & WT_QID ) ) //pointers
    {
        _Compile_LocalOrStackVar_RValue_To_Reg ( word, reg ) ;
    }
    else if ( word->CProperty & LOCAL_VARIABLE )
    {
        _Compile_LEA ( reg, FP, 0, LocalVarIndex_Disp ( LocalVarOffset ( word ) ) ) ; // 2 : account for saved fp and return slot
    }
    else if ( word->CProperty & PARAMETER_VARIABLE )
    {
        _Compile_LEA ( reg, FP, 0, LocalVarIndex_Disp ( ParameterVarOffset ( word ) ) ) ;
    }
    else if ( word->CProperty & NAMESPACE_VARIABLE )
    {
        int32 value ;
        if ( GetState ( _Context_, C_SYNTAX ) && ( ! IsLValue ( word ) ) ) //GetState ( _Context_, C_RHS ) )
        {
            value = ( int32 ) * word->W_PtrToValue ;
        }
        else value = ( int32 ) word->W_PtrToValue ;
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) value ) ;
    }
    else SyntaxError ( QUIT ) ;
    if ( word->CProperty & ( OBJECT | THIS ) || ( word->WProperty & WT_QID ) )
    {
        Do_ObjectOffset ( word, reg ) ;
    }
}

