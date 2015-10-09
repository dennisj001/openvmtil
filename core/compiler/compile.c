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
    Compile_Store ( _Q_->OVT_Context->Compiler0, DSP ) ;
}

Word *
_CfrTil_VariableGet ( Namespace * ns, byte * name )
{
    Word * word = Word_FindInOneNamespace ( ns, name ) ;
    return word ;
}

void
_CfrTil_VariableValueSet ( Namespace * ns, byte * name, int32 value )
{
    Word * word = _CfrTil_VariableGet ( ns, name ) ;
    if ( word ) word->W_Object = ( byte* ) value ; // value of variable
}

int32
_CfrTil_VariableValueGet ( byte* namespace, byte * name )
{
    return _Namespace_VariableValueGet ( Namespace_Find ( namespace ), name ) ;
}

void
_Compile_C_Var_To_Reg ( int32 reg, int32 * ptrToCVar )
{
    _Compile_Move_Literal_Immediate_To_Reg ( EAX, ( int32 ) ptrToCVar ) ; // use pointer because value has to be taken at run time not compile time
    _Compile_Move_Rm_To_Reg ( reg, EAX, 0 ) ;
}

void
_Compile_Reg_To_C_Var ( int32 reg, int32 * ptrToCVar )
{
    _Compile_Move_Literal_Immediate_To_Reg ( EAX, ( int32 ) ptrToCVar ) ; // use pointer because value has to be taken at run time not compile time
    _Compile_Move_Reg_To_Rm ( EAX, 0, reg ) ;
}

void
_Compile_Move_Literal_Immediate_To_Reg ( int32 reg, int32 value )
{
    _Compile_MoveImm_To_Reg ( reg, value, CELL ) ;
}

void
_Compile_VarConstOrLit_RValue_To_Reg ( Word * word, int32 reg )
{
    if ( word->CType & REGISTER_VARIABLE )
    {
        if ( word->RegToUse == reg ) return ;
        else _Compile_Move_Reg_To_Reg ( reg, word->RegToUse ) ;
    }
    else if ( word->CType & LOCAL_VARIABLE )
    {
        _Compile_Move_StackN_To_Reg ( reg, FP, LocalVarOffset ( word ) ) ; // 2 : account for saved fp and return slot
    }
    else if ( word->CType & STACK_VARIABLE )
    {
        _Compile_Move_StackN_To_Reg ( reg, FP, StackVarOffset ( word ) ) ; // account for stored bp and return value
    }
    else if ( word->CType & VARIABLE )
    {
        if ( reg == EAX ) _Compile_Move_AddressValue_To_EAX ( ( int32 ) word->PtrObject ) ;
        else
        {
            _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) word->PtrObject ) ; // remember we want be getting runtime value not the compile time value
            _Compile_Move_Rm_To_Reg ( reg, reg, 0 ) ;
        }
    }
    else if ( word->CType & ( OBJECT | THIS ) )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) word->W_Object ) ;
    }
    else if ( word->CType & ( LITERAL | CONSTANT ) )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) word->W_Object ) ;
    }
    else SyntaxError ( QUIT ) ;
}

void
_Compile_LValue_ClassFieldToReg ( Word * word, int32 reg )
{
    if ( word->CType & REGISTER_VARIABLE )
    {
        _Compile_Move_Reg_To_Reg ( reg, word->RegToUse ) ;
    }
    else if ( word->CType & ( STACK_VARIABLE | LOCAL_VARIABLE ) )
    {
        _Compile_VarConstOrLit_RValue_To_Reg ( word, reg ) ; // nb.!! : compile rvalue for stack/local variables for object/class field !!
    }
    else if ( word->CType & VARIABLE )
    {
        if ( reg == EAX ) _Compile_Move_AddressValue_To_EAX ( ( int32 ) word->PtrObject ) ;
        else
        {
            _Compile_Move_Literal_Immediate_To_Reg ( ECX, ( int32 ) word->PtrObject ) ; // intel insn set doesnt include move [mem immediate] to ecx like eax
            _Compile_Move_Rm_To_Reg ( ECX, ECX, 0 ) ;
        }
    }
    else if ( word->CType & ( THIS | OBJECT ) )
    {
        if ( reg == EAX ) _Compile_Move_AddressValue_To_EAX ( ( int32 ) word->PtrObject ) ;
        else
        {
            _Compile_Move_Literal_Immediate_To_Reg ( ECX, ( int32 ) word->PtrObject ) ; // intel insn set doesnt include move [mem immediate] to ecx like eax
            _Compile_Move_Rm_To_Reg ( ECX, ECX, 0 ) ;
        }
    }
    else SyntaxError ( QUIT ) ;
    int32 offset = word->AccumulatedOffset ;
    if ( offset ) Compile_ADDI ( REG, reg, 0, offset, CELL ) ; // nb : rpn 
}

void
_Compile_VarConstOrLit_LValue_To_Reg ( Word * word, int32 reg )
{
    if ( word->CType & REGISTER_VARIABLE )
    {
        if ( word->RegToUse == reg ) return ;
        else _Compile_Move_Reg_To_Reg ( reg, word->RegToUse ) ;
    }
    else if ( word->CType & LOCAL_VARIABLE )
    {
        _Compile_LEA ( reg, FP, 0, LocalVarIndex_Disp ( LocalVarOffset ( word ) ) ) ; // 2 : account for saved fp and return slot
    }
    else if ( word->CType & STACK_VARIABLE )
    {
        _Compile_LEA ( reg, FP, 0, LocalVarIndex_Disp ( StackVarOffset ( word ) ) ) ;
    }
    else if ( word->CType & ( OBJECT | THIS ) )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) word->PtrObject ) ;
    }
    else if ( word->CType & VARIABLE )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) word->PtrObject ) ;
    }
    else SyntaxError ( QUIT ) ;
}

