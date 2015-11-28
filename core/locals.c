#include "../includes/cfrtil.h"

// ?? is the frame pointer needed ?? 
// remember LocalsStack is not pushed or popped so ...

/* ------------------------------------------------------
 *     a Locals Stack Frame on the DataStack - referenced by DSP
 * ------------------------------------------------------
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *     higher memory adresses -- nb. reverse from intel push/pop where push moves esp to a lower memory address and pop moves esp to a higher memory address. This seemed more intuitive.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * <--------------------------< new DSP - sp [ 0 ]
 * s
 * t    "local variable" slot ...   fp [etc]
 * a    "local variable" slot 5     fp [ 5 ]
 * c    "local variable" slot 4     fp [ 4 ]
 * k    "local variable" slot 3     fp [ 3 ]
 *      "local variable" slot 2     fp [ 2 ]
 * f    "local variable" slot 1     fp [ 1 ]
 * r  -------------------------
 * a
 * m    saved old fp                fp [ 0 ]    <-- new fp - FP points here
 * e
 * <--------------------------< old DSP - sp [ 0 ] >-----------------------
 *      "stack variables"           fp [ -1 ]   --- already on the "locals function" incoming stack
 *      "stack variables"           fp [ -2 ]   --- already on the "locals function" incoming stack
 *      "stack variables"           fp [-etc]   --- already on the "locals function" incoming stack
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *     lower memory addresses  on DataStack - referenced by DSP
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                      notations : fp = FP = Fp   = EDI 
 *                                  sp = DSP = Dsp = ESI
 */

void
_Compiler_AddLocalFrame ( Compiler * compiler )
{
    _Compile_Move_Reg_To_StackN ( DSP, 1, FP ) ; // save old fp
    _Compile_LEA ( FP, DSP, 0, LocalVarIndex_Disp ( 1 ) ) ; // set new fp
    Compile_ADDI ( REG, DSP, 0, compiler->LocalsFrameSize, CELL ) ; // add stack frame
    compiler->FrameSizeCellOffset = ( int32* ) ( Here - CELL ) ; // in case we have to add to the framesize with nested locals
}

void
Compiler_SetLocalsFrameSize_AtItsCellOffset ( Compiler * compiler )
{
    compiler->LocalsFrameSize = compiler->NumberOfRegisterVariables ? ( compiler->NumberOfRegisterVariables - compiler->NumberOfStackVariables + 1 ) * CELL : ( compiler->NumberOfLocals + 1 ) * CELL ; // 1 : space for fp
    *( ( compiler )->FrameSizeCellOffset ) = LocalsFrameSize ( compiler ) ;
}

void
_Compiler_RemoveLocalFrame ( Compiler * compiler )
{
    int32 stackVarsSubAmount, returnValueFlag ;
    Compiler_SetLocalsFrameSize_AtItsCellOffset ( compiler ) ;
    stackVarsSubAmount = compiler->NumberOfStackVariables * CELL ; // remove stackVariables like C ...
    returnValueFlag = ( _Q_->OVT_CfrTil->CurrentRunWord->CType & C_RETURN ) || ( GetState ( compiler, RETURN_TOS | RETURN_EAX ) ) || IsWordRecursive || compiler->ReturnVariableWord ;
    Word * word = compiler->ReturnVariableWord ;
    if ( word )
    {
        _Compile_VarLitObj_RValue_To_Reg ( word, EAX ) ; // nb. these variables have no lasting lvalue - they exist on the stack - therefore we can only return there rvalue
    }
    else if ( compiler->NumberOfStackVariables && returnValueFlag && ( ! compiler->NumberOfRegisterVariables ) && ( ! GetState ( compiler, RETURN_EAX ) ) ) 
    {
        Compile_Move_TOS_To_EAX ( DSP ) ; // save TOS to EAX so we can set return it as TOS below
    }
    else if ( GetState ( compiler, RETURN_TOS ) )
    {
        Compile_Move_TOS_To_EAX ( DSP ) ;
    }
    _Compile_LEA ( DSP, FP, 0, - LocalVarIndex_Disp ( 1 ) ) ; // restore sp - automatically releases locals stack frame
    _Compile_Move_StackN_To_Reg ( FP, DSP, 1 ) ; // restore the saved old fp - cf AddLocalsFrame
    stackVarsSubAmount -= returnValueFlag * CELL ; // reduce the subtract amount to make room for the return value
    if ( stackVarsSubAmount > 0 )
    {
        Compile_SUBI ( REG, DSP, 0, stackVarsSubAmount, CELL ) ; // remove stack variables
    }
    else if ( stackVarsSubAmount < 0 )
    {
        Compile_ADDI ( REG, DSP, 0, - stackVarsSubAmount, CELL ) ; // add a place on the stack for return value
    }
    if ( returnValueFlag && ( ! GetState ( compiler, RETURN_EAX ) ) )
    {
        // nb : stack was already adjusted accordingly for this above by reducing the SUBI subAmount or adding if there weren't any stack variables
        Compile_Move_EAX_To_TOS ( DSP ) ;
    }
#if NO_GLOBAL_REGISTERS       
    _Compile_ESI_To_Dsp ( ) ;
#endif        
}

// rvalue

void
CfrTil_LocalsAndStackVariablesBegin ( )
{
    _CfrTil_Parse_LocalsAndStackVariables ( 1, 0, 0, 0 ) ;
}

void
CfrTil_LocalVariablesBegin ( )
{

    _CfrTil_Parse_LocalsAndStackVariables ( 0, 0, 0, 0 ) ;
}

void
CheckAddLocalFrame ( Compiler * compiler )
{
    if ( Compiler_GetState ( compiler, ADD_FRAME ) )
    {

        _Compiler_AddLocalFrame ( compiler ) ;
        Compiler_SetState ( compiler, ADD_FRAME, false ) ; // only one frame necessary
    }
}

void
CheckCompileRemoveLocalFrame ( Compiler * compiler )
{
    if ( _Compiler_IsFrameNecessary ( compiler ) )
    {
        _Compiler_RemoveLocalFrame ( compiler ) ;
    }
}

