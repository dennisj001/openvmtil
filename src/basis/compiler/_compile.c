#include "../../include/cfrtil.h"

void
_Compile_CallEAX ( void )
{
    //_Compile_Group5 ( int64 code, int64 mod, int64 rm, int64 sib, int64 disp, int64 size )
    _Compile_Group5 ( CALL, 3, 0, 0, 0, 0 ) ;
}

void
Compile_DataStack_PopAndCall ( void )
{
    Compile_Pop_ToEAX_AndCall ( DSP, CELL ) ;
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
    _Compile_Stack_PushReg ( DSP, ESP, CELL ) ;
}

// rsp@

void
_Compile_Rsp_Fetch ( )
{
    _Compile_Move_Reg_To_Reg ( EAX, ESP, CELL ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 * CELL, CELL ) ;
    _Compile_Stack_PushReg ( DSP, EAX, CELL ) ;
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
    Compile_Store ( _Context_->Compiler0, DSP, CELL ) ;
}

Word *
_CfrTil_VariableGet ( Namespace * ns, byte * name )
{
    Word * word = Finder_FindWord_InOneNamespace ( _Finder_, ns, name ) ;
    return word ;
}

int64
_CfrTil_VariableValueGet ( byte* nameSpace, byte * name )
{
    return _Namespace_VariableValueGet ( Namespace_Find ( nameSpace ), name ) ;
}

// set the value at address to reg - value in reg

void
_Compile_SetAtAddress_WithReg ( int64 * address, int64 reg, int64 thruReg, int64 size )
{
    _Compile_Move_Literal_Immediate_To_Reg ( thruReg, ( int64 ) address, size ) ;
    _Compile_Move_Reg_To_Rm ( thruReg, reg, 0, size ) ;
}

void
_Compile_Move_Literal_Immediate_To_Reg ( int64 reg, int64 value, int64 size )
{
    _Compile_MoveImm_To_Reg ( reg, value, CELL ) ;
}

void
_Compile_LocalOrStackVar_RValue_To_Reg ( Word * word, int64 reg, int64 size )
{
    SC_DWL_Push ( word ) ;
    word->Coding = Here ; // we don't need the word's code if compiling -- this is an optimization though
    if ( word->CProperty & REGISTER_VARIABLE )
    {
        if ( word->RegToUse == reg ) return ;
        else _Compile_Move_Reg_To_Reg ( reg, word->RegToUse, size ) ;
    }
    else if ( word->CProperty & LOCAL_VARIABLE )
    {
        _Compile_Move_StackN_To_Reg ( reg, FP, LocalVarOffset ( word ), size ) ; // 2 : account for saved fp and return slot
    }
    else if ( word->CProperty & PARAMETER_VARIABLE )
    {
        _Compile_Move_StackN_To_Reg ( reg, FP, ParameterVarOffset ( word ), size ) ; // account for stored bp and return value
    }
    else if ( word->CProperty & ( OBJECT | THIS ) )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int64 ) * word->W_PtrToValue, size ) ;
    }
}

void
Do_ObjectOffset ( Word * word, int64 reg )
{
    Compiler * compiler = _Context_->Compiler0 ;
    int64 offset = word->AccumulatedOffset ;
    if ( GetState ( _CfrTil_, IN_OPTIMIZER ) && ( offset == 0 ) ) return ;
    Compile_ADDI ( REG, reg, 0, offset, CELL ) ;
    compiler->AccumulatedOffsetPointer = ( int64* ) ( Here - CELL ) ; // offset will be calculated as we go along by ClassFields and Array accesses
}

void
_Compile_GetVarLitObj_RValue_To_Reg ( Word * word, int64 reg, int64 size )
{
    SC_DWL_Push ( word ) ;
    word->Coding = Here ; // we don't need the word's code if compiling -- this is an optimization though
    if ( word->CProperty & REGISTER_VARIABLE )
    {
        return ;
        //if ( word->RegToUse == reg ) return ;
        //else _Compile_Move_Reg_To_Reg ( reg, word->RegToUse ) ;
    }
    else if ( word->CProperty & LOCAL_VARIABLE )
    {
        _Compile_Move_StackN_To_Reg ( reg, FP, LocalVarOffset ( word ), size ) ; // 2 : account for saved fp and return slot
    }
    else if ( word->CProperty & PARAMETER_VARIABLE )
    {
        _Compile_Move_StackN_To_Reg ( reg, FP, ParameterVarOffset ( word ), size ) ; // account for stored bp and return value
    }
    else if ( word->CProperty & NAMESPACE_VARIABLE )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int64 ) word->W_PtrToValue, size ) ;
        _Compile_Move_Rm_To_Reg ( reg, reg, 0, size ) ;
    }
    else if ( word->CProperty & ( LITERAL | CONSTANT | OBJECT | THIS ) )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int64 ) * word->W_PtrToValue, size ) ;
    }
    else SyntaxError ( QUIT ) ;
    if ( word->CProperty & ( OBJECT | THIS ) )
    {
        Do_ObjectOffset ( word, reg ) ;
        //if ( ! ( word->LProperty & LOCAL_OBJECT ) ) _Compile_Move_Rm_To_Reg ( reg, reg, 0 ) ; // ?? this for LOCAL_OBJECT seems like we need to better integrate LOCAL_OBJECT
        _Compile_Move_Rm_To_Reg ( reg, reg, 0, size ) ;
    }
}

// this is not tested for VARIABLE or REGISTER_VARIABLE

void
_Compile_SetVarLitObj_With_Reg ( Word * word, int64 reg, int64 thruReg, int64 size )
{
    SC_DWL_Push ( word ) ;
    word->Coding = Here ; // we don't need the word's code if compiling -- this is an optimization though
    if ( word->CProperty & REGISTER_VARIABLE )
    {
        if ( word->RegToUse == reg ) return ;
        else _Compile_Move_Reg_To_Reg ( word->RegToUse, reg, size ) ;
    }
    else if ( word->CProperty & LOCAL_VARIABLE )
    {
        _Compile_Move_Reg_To_StackN ( FP, LocalVarOffset ( word ), reg, size ) ;
    }
    else if ( word->CProperty & PARAMETER_VARIABLE )
    {
        _Compile_Move_Reg_To_StackN ( FP, ParameterVarOffset ( word ), reg, size ) ;
    }
    else if ( word->CProperty & NAMESPACE_VARIABLE )
    {
        //_Compile_Move_Literal_Immediate_To_Reg ( thruReg, ( int64 ) word->W_PtrToValue ) ;
        //_Compile_Move_Reg_To_Rm ( thruReg, reg, 0 ) ;
        _Compile_SetAtAddress_WithReg ( ( int64* ) word->W_PtrToValue, reg, thruReg, size ) ;
    }
}

void
_Compile_GetVarLitObj_LValue_To_Reg ( Word * word, int64 reg, int64 size )
{
    SC_DWL_Push ( word ) ;
    word->Coding = Here ;
    if ( word->CProperty & REGISTER_VARIABLE )
    {
        if ( word->RegToUse == reg ) return ;
        else _Compile_Move_Reg_To_Reg ( reg, word->RegToUse, size ) ;
    }
    else if ( word->CProperty & ( OBJECT | THIS ) || ( word->WProperty & WT_QID ) ) //pointers
    {
        _Compile_LocalOrStackVar_RValue_To_Reg ( word, reg, size ) ;
    }
    else if ( word->CProperty & LOCAL_VARIABLE )
    {
        _Compile_LEA ( reg, FP, 0, LocalVarIndex_Disp ( LocalVarOffset ( word ) ), size ) ; // 2 : account for saved fp and return slot
    }
    else if ( word->CProperty & PARAMETER_VARIABLE )
    {
        _Compile_LEA ( reg, FP, 0, LocalVarIndex_Disp ( ParameterVarOffset ( word ) ), size ) ;
    }
    else if ( word->CProperty & NAMESPACE_VARIABLE )
    {
        int64 value ;
        if ( GetState ( _Context_->Compiler0, LC_ARG_PARSING ) || ( GetState ( _Context_, C_SYNTAX ) && ( ! Is_LValue ( word ) ) ) )//GetState ( _Context_, C_RHS ) )
        {
            value = ( int64 ) * word->W_PtrToValue ;
        }
        else value = ( int64 ) word->W_PtrToValue ;
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int64 ) value, size ) ;
    }
    else SyntaxError ( QUIT ) ;
    if ( word->CProperty & ( OBJECT | THIS ) || ( word->WProperty & WT_QID ) )
    {
        Do_ObjectOffset ( word, reg ) ;
    }
}

