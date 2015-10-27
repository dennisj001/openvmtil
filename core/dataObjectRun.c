
#include "../includes/cfrtil.h"

void
_Compile_C_Call_1_Arg ( byte* function, int32 arg )
{
    _Compile_Esp_Push ( arg ) ;
    Compile_Call ( function ) ;
    _Compile_Rsp_Drop ( ) ;
}

#if 0

void
_Compile_C_Call_Pop_1_Arg ( byte* function )
{
    //_Compile_Esp_Push (  ) ;
    Compile_DspPop_EspPush ( ) ;
    Compile_Call ( function ) ;
    _Compile_Rsp_Drop ( ) ;
}
#endif 

void
_CfrTil_Do_ClassField ( Word * word )
{
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    _Q_->OVT_Context->Interpreter0->ObjectNamespace = word ;
    compiler->ArrayEnds = 0 ;
    uint32 accumulatedAddress ;

    if ( CompileMode )
    {
        if ( word->Offset )
        {
            IncrementCurrentAccumulatedOffset ( word->Offset ) ;
        }
    }
    else
    {
        accumulatedAddress = _DataStack_Pop ( ) ;
        accumulatedAddress += word->Offset ;
        if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) && GetState ( _Q_->OVT_Context, C_RHS ) && ( Lexer_NextNonDelimiterChar ( _Q_->OVT_Context->Lexer0 ) != '.' ) ) _Push ( * ( int32* ) accumulatedAddress ) ;
        else _Push ( accumulatedAddress ) ;
    }
    if ( Lexer_IsTokenForwardDotted ( _Q_->OVT_Context->Lexer0 ) ) Finder_SetQualifyingNamespace ( _Q_->OVT_Context->Finder0, word->ClassFieldTypeNamespace ) ;
    else if ( CompileMode & ( word->Offset ) )
    {
        _Word_CompileAndRecord_PushEAX ( word ) ;
    }

    Stack_Pop ( _Q_->OVT_Context->Compiler0->WordStack ) ;
}

// nb. 'word' is the previous word to the '.' (dot) cf. CfrTil_Dot so it can be recompiled, a little different maybe, as an object

void
CfrTil_Dot ( ) // .
{
    Stack_Pop ( _Q_->OVT_Context->Compiler0->WordStack ) ; // nb. first else problems with DataObject_Run ( word ) 
    if ( ! _Q_->OVT_Context->Interpreter0->BaseObject )
    {
        SetState ( _Q_->OVT_Context, CONTEXT_PARSING_QUALIFIED_ID, true ) ;
        Word * word = Compiler_PreviousNonDebugWord ( ) ;
        if ( word->CType & ( NAMESPACE | CLASS | DOBJECT ) )
        {
            Finder_SetQualifyingNamespace ( _Q_->OVT_Context->Finder0, word ) ;
        }
        //SyntaxError ( 1 ) ;//
        else DataObject_Run ( word ) ; //_CfrTil_Do_Object ( word ) ;
    }
}

// rvalue - rhs value - right hand side of '=' - the actual value, used on the right hand side of C statements

_Word_CompileAndRecord_PushEAX ( Word * word )
{
    if ( word )
    {
        word->StackPushRegisterCode = Here ; // nb. used! by the rewriting optimizer
        Compile_Stack_PushEAX ( DSP ) ;
        //_Compiler_WordStack_Record_PushEAX ( _Q_->OVT_Context->Compiler0 ) ; // does word == top of word stack always
    }
}

void
_Do_Literal ( int32 value )
{
    if ( CompileMode )
    {
        //_Compile_MoveImm_To_Reg ( EAX, value, CELL ) ;
        //_Word_CompileAndRecord_PushEAX ( word ) ;
        _Compile_Stack_Push ( DSP, value ) ;
    }

    else _Push ( value ) ;
}

// a constant is, of course, a literal

void
_Namespace_DoNamespace ( Namespace * ns )
{
    if ( CompileMode && ( Lexer_NextNonDelimiterChar ( _Q_->OVT_Context->Lexer0 ) != '.' ) ) // ( ! GetState ( _Q_->OVT_Context, CONTEXT_PARSING_QUALIFIED_ID ) ) ) //&& ( ! GetState ( _Q_->OVT_Context, C_SYNTAX ) ) )
    {
        _Compile_C_Call_1_Arg ( ( byte* ) _Namespace_DoNamespace, ( int32 ) ns ) ;
        //_Compile_Stack_Push ( DSP, (int32) ns ) ;
        //_Compile_C_Call_Pop_1_Arg ( ( byte* ) _Namespace_DoNamespace ) ;
    }
    if ( ! Lexer_IsTokenForwardDotted ( _Q_->OVT_Context->Lexer0 ) )
    {
        _Namespace_ActivateAsPrimary ( ns ) ;
    }
    else
    {
        Finder_SetQualifyingNamespace ( _Q_->OVT_Context->Finder0, ns ) ;
    }
}

void
_CfrTil_Do_DynamicObject ( DObject * dobject )
{
    Lexer * lexer = _Q_->OVT_Context->Lexer0 ;
    DObject * ndobject ;
    byte * token ;
    while ( Lexer_IsTokenForwardDotted ( lexer ) )
    {
        Lexer_ReadToken ( lexer ) ; // the '.'
        token = Lexer_ReadToken ( lexer ) ;
        if ( String_Equal ( "prototype", ( char* ) token ) )
        {
            dobject = dobject->ContainingNamespace ;
            continue ;
        }
        if ( ! ( ndobject = _DObject_FindSlot_BottomUp ( dobject, token ) ) )
        {
            _Q_->OVT_CfrTil->InNamespace = dobject ; // put the new dynamic object in the namespace of it's mother object
            dobject = _DObject_NewSlot ( dobject, token, 0 ) ;
        }
        else dobject = ndobject ;
    }
    _Q_->OVT_Context->Interpreter0->ObjectNamespace = TypeNamespace_Get ( dobject ) ;
    if ( CompileMode )
    {
        _Compile_DataStack_Push ( ( int32 ) dobject ) ;
    }
    else
    {
        _Push ( ( int32 ) dobject ) ;
    }
}

void
Do_LocalOrStackVariable ( Word * word )
{
    if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) )
    {
        if ( GetState ( _Q_->OVT_Context, C_RHS ) )
        {
            _Compile_VarLitObj_RValue_To_Reg ( word, EAX ) ;
            _Word_CompileAndRecord_PushEAX ( word ) ;
        }
        else if GetState ( _Q_->OVT_Context, C_LHS )
        {
            SetState ( _Q_->OVT_Context, C_LHS, false ) ;
            _Q_->OVT_Context->Compiler0->LHS_Word = word ;
        }
    }
    else
    {
        _Compile_VarLitObj_LValue_To_Reg ( word, EAX ) ;
        _Word_CompileAndRecord_PushEAX ( word ) ;
    }
}
// 'Run' :: this is part of runtime in the interpreter/compiler for data objects
// they are compiled to much more optimized native code

void
DataObject_Run ( Word * word )
{
    _Q_->OVT_Context->Interpreter0->w_Word = word ; // for ArrayBegin : all literals are run here
    if ( word->CType & T_LISP_SYMBOL )
    {
        // rvalue - rhs for stack var
        _Compile_Move_StackN_To_Reg ( EAX, FP, ParameterVarOffset ( word ) ) ;
        _Word_CompileAndRecord_PushEAX ( word ) ;
    }
    else if ( word->CType & ( LITERAL | CONSTANT ) )
    {
        if ( CompileMode )
        {
            _Compile_VarLitObj_RValue_To_Reg ( word, EAX ) ;
            _Word_CompileAndRecord_PushEAX ( word ) ;
        }
        else _Push ( word->W_Value ) ;
    }
    else if ( word->CType & ( VARIABLE | THIS | OBJECT | LOCAL_VARIABLE | PARAMETER_VARIABLE ) )
    {
        if ( CompileMode && GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) )  word = WordStack ( 0 ) ;
        if ( word->CType & ( OBJECT | THIS ) )
        {
            word->AccumulatedOffset = 0 ;
            word->Coding = Here ;
            _Q_->OVT_Context->Interpreter0->BaseObject = word ;
            _Q_->OVT_Context->Interpreter0->ObjectNamespace = TypeNamespace_Get ( word ) ;
            _Q_->OVT_Context->Compiler0->AccumulatedOffsetPointer = 0 ;
            _Q_->OVT_Context->Compiler0->AccumulatedOptimizeOffsetPointer = & word->AccumulatedOffset ;

            if ( ( ! _Q_->OVT_Context->Finder0->QualifyingNamespace ) && Lexer_IsTokenForwardDotted ( _Q_->OVT_Context->Lexer0 ) )
            {
                Finder_SetQualifyingNamespace ( _Q_->OVT_Context->Finder0, word->ContainingNamespace ) ;
            }
        }
        if ( CompileMode )
        {
            _Compile_VarLitObj_LValue_To_Reg ( word, EAX ) ;
            _Word_CompileAndRecord_PushEAX ( word ) ;
        }
        else
        {
            if ( word->CType & ( OBJECT | THIS ) )
            {
#if 1                
                _Push ( word->W_Value ) ;
#else                
                if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) && GetState ( _Q_->OVT_Context, C_RHS ) && ( Lexer_NextNonDelimiterChar ( _Q_->OVT_Context->Lexer0 ) != '.' ) )
                {
                    _Push ( * ( int32* ) word->W_Value + word->AccumulatedOffset ) ;
                }
                else _Push ( word->W_Value + word->AccumulatedOffset ) ;
#endif                
            }
            else _Push ( &word->W_Value ) ;
        }
    }
    else if ( word->CType & OBJECT_FIELD )
    {
        _CfrTil_Do_ClassField ( word ) ;
    }
    else if ( word->CType & DOBJECT )
    {
        _CfrTil_Do_DynamicObject ( word ) ;
    }
    else if ( word->CType & ( NAMESPACE | CLASS | CLASS_CLONE ) )
    {
        _Namespace_DoNamespace ( word ) ;
    }
    else if ( word->CType & ( C_TYPE | C_CLASS ) )
    {
        _Namespace_Do_C_Type ( word ) ;
    }
}

#if 0

void
_This_Run ( )
{
    Word * word ;
    // find 'this' in inNamespace 
    word = Word_FindInOneNamespace ( _CfrTil_InNamespace ( ), "this" ) ;
    _Push ( ( int32 ) word->W_Value ) ;
}

void
This_Run ( )
{
    if ( CompileMode )
    {
        _Compile_Call ( ( int32 ) _This_Run ) ;
    }
    else
    {
        _This_Run ( ) ;
    }
}
#endif
