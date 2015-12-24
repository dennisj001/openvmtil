
#include "../../includes/cfrtil.h"

void
_Compile_C_Call_1_Arg ( byte* function, int32 arg )
{
    _Compile_Esp_Push ( arg ) ;
    Compile_Call ( function ) ;
    _Compile_Rsp_Drop ( ) ;
}

void
_Namespace_Do_C_Type ( Namespace * ns )
{
    Context * cntx = _Q_->OVT_Context ;
    Lexer * lexer = cntx->Lexer0 ;
    byte * token1, *token2 ;
    if ( ( ! Compiling ) && ! GetState ( cntx->Compiler0, LC_ARG_PARSING ) )
    {
        _CfrTil_InitSourceCode_WithName ( ns->Name ) ;
    }
    LambdaCalculus * lc = _Q_->OVT_LC ;
    _Q_->OVT_LC = 0 ;
    if ( GetState ( cntx, C_SYNTAX ) && ( cntx->System0->IncludeFileStackNumber ) ) //&& ( strlen ( cntx->ReadLiner0->InputLine ) != strlen ( ns->Name ) ) )
    {
        // ?? parts of this could be screwing up other things and adds an unnecessary level of complexity
        // for parsing C functions 
        token1 = _Lexer_NextNonDebugTokenWord ( lexer ) ;
        if ( token1 [0] != '"' )
        {
            token2 = Lexer_PeekNextNonDebugTokenWord ( lexer ) ;
            if ( token2 [0] == '(' )
            {
                Finder_SetQualifyingNamespace ( cntx->Finder0, ns ) ; // _Lexer_NextNonDebugTokenWord clears QualifyingNamespace
                Word * word = _Word_Create ( token1 ) ;
                _DataStack_Push ( ( int32 ) word ) ; // token1 is the function name 
                CfrTil_RightBracket ( ) ; //Compiler_SetState ( _Q_->OVT_Context->Compiler0, COMPILE_MODE, true ) ;
                CfrTil_BeginBlock ( ) ;
                CfrTil_LocalsAndStackVariablesBegin ( ) ;
                byte * token = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0 ) ;
                if ( token [ 0 ] == '{' )
                {
                    Lexer_ReadToken ( lexer ) ;
                }
                return ;
            }
            else _CfrTil_AddTokenToHeadOfTokenList ( token1 ) ; // add ahead of token2 :: ?? this could be screwing up other things and adds an unnecessary level of complexity
        }
        _Namespace_DoNamespace ( ns, 1 ) ;
    }
    _Q_->OVT_LC = lc ;
}

void
_CfrTil_Do_ClassField ( Word * word )
{
    Context * cntx = _Q_->OVT_Context ;
    Compiler * compiler = cntx->Compiler0 ;
    cntx->Interpreter0->ObjectNamespace = word ;
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
        if ( GetState ( cntx, C_SYNTAX ) && ( ! IsLValue ( word ) ) ) //GetState ( cntx, C_RHS ) && ( Lexer_NextNonDelimiterChar ( cntx->Lexer0 ) != '.' ) )
        {
            _Push ( * ( int32* ) accumulatedAddress ) ;
        }
        else _Push ( accumulatedAddress ) ;
    }
    if ( Lexer_IsTokenForwardDotted ( cntx->Lexer0 ) ) Finder_SetQualifyingNamespace ( cntx->Finder0, word->ClassFieldTypeNamespace ) ;
    Stack_Pop ( cntx->Compiler0->WordStack ) ;
}

// nb. 'word' is the previous word to the '.' (dot) cf. CfrTil_Dot so it can be recompiled, a little different maybe, as an object

void
CfrTil_Dot ( ) // .
{
    Context * cntx = _Q_->OVT_Context ;
    if ( ! cntx->Interpreter0->BaseObject )
    {
        SetState ( cntx, CONTEXT_PARSING_QID, true ) ;
        d0 ( if ( DebugOn ) Compiler_ShowWordStack ( "\nCfrTil_Dot" ) ) ;

        Word * word = Compiler_PreviousNonDebugWord ( - 1 ) ; // 0 : rem: we just popped the WordStack above
        if ( word->CType & NAMESPACE_TYPE )
        {
            Finder_SetQualifyingNamespace ( cntx->Finder0, word ) ;
        }
        else
        {
            cntx->Interpreter0->BaseObject = word ;
            DataObject_Run ( word ) ;
        }
    }
    Stack_Pop ( cntx->Compiler0->WordStack ) ; // nb. first else problems with DataObject_Run ( word ) 
}

// rvalue - rhs value - right hand side of '=' - the actual value, used on the right hand side of C statements

void
_Word_CompileAndRecord_PushEAX ( Word * word )
{
    if ( word )
    {
        word->StackPushRegisterCode = Here ; // nb. used! by the rewriting optimizer
        Compile_Stack_PushEAX ( DSP ) ;
        //_Compiler_WordStack_Record_PushEAX ( cntx->Compiler0 ) ; // does word == top of word stack always
    }
}

void
_Do_Literal ( int32 value )
{
    if ( CompileMode )
    {
        _Compile_MoveImm_To_Reg ( EAX, value, CELL ) ;
        _Compiler_CompileAndRecord_PushEAX ( _Q_->OVT_Context->Compiler0 ) ; // does word == top of word stack always
        //_Word_CompileAndRecord_PushEAX ( word ) ;
        //_Compile_Stack_Push ( DSP, value ) ;
    }
    else _Push ( value ) ;
}

// a constant is, of course, a literal

void
_Namespace_DoNamespace ( Namespace * ns, int32 immFlag )
{
    Context * cntx = _Q_->OVT_Context ;
    if ( ( ! immFlag ) && CompileMode && ( Lexer_NextNonDelimiterChar ( cntx->Lexer0 ) != '.' ) && ( ! GetState ( cntx->Compiler0, LC_ARG_PARSING ) ) )
    {
        _Compile_C_Call_1_Arg ( ( byte* ) _Namespace_DoNamespace, ( int32 ) ns ) ;
    }
    if ( ! Lexer_IsTokenForwardDotted ( cntx->Lexer0 ) )
    {
        _Namespace_ActivateAsPrimary ( ns ) ;
    }
    else
    {
        Finder_SetQualifyingNamespace ( cntx->Finder0, ns ) ;
    }
}

void
_CfrTil_Do_DynamicObject ( DObject * dobject )
{
    Context * cntx = _Q_->OVT_Context ;
    Lexer * lexer = cntx->Lexer0 ;
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
    cntx->Interpreter0->ObjectNamespace = TypeNamespace_Get ( dobject ) ;
    if ( CompileMode )
    {
        _Compile_DataStack_Push ( ( int32 ) & dobject->W_DObjectValue ) ; //dobject ) ;
    }
    else
    {
        _Push ( ( int32 ) & dobject->W_DObjectValue ) ; //dobject ) ;
    }
}

void
_Do_Variable ( Word * word )
{
    Context * cntx = _Q_->OVT_Context ;
    if ( GetState ( cntx, C_SYNTAX ) )
    {
        if ( IsLValue ( word ) )
        {
            cntx->Compiler0->LHS_Word = word ;
        }
        else
        {
            if ( GetState ( _Q_->OVT_Context, ADDRESS_OF_MODE ) ) _Compile_VarLitObj_LValue_To_Reg ( word, EAX ) ;
            else _Compile_VarLitObj_RValue_To_Reg ( word, EAX ) ;
            _Word_CompileAndRecord_PushEAX ( word ) ;
        }
    }
    else
    {
        _Compile_VarLitObj_LValue_To_Reg ( word, EAX ) ;
        _Word_CompileAndRecord_PushEAX ( word ) ;
    }
}

void
_CfrTil_Do_Literal ( Word * word )
{
    if ( CompileMode )
    {
        _Compile_VarLitObj_RValue_To_Reg ( word, EAX ) ;
        _Word_CompileAndRecord_PushEAX ( word ) ;
    }
    else _Push ( * word->W_PtrToValue ) ; //word->W_Value ) ;
}

void
_CfrTil_Do_LispSymbol ( Word * word )
{
    // rvalue - rhs for stack var
    _Compile_Move_StackN_To_Reg ( EAX, FP, ParameterVarOffset ( word ) ) ;
    _Word_CompileAndRecord_PushEAX ( word ) ;
}

void
_CfrTil_Do_Variable ( Word * word )
{
    // this block may need to be reworked -- too compilicated
    Context * cntx = _Q_->OVT_Context ;
    // since we can have multiple uses of the same word we make copies in Compiler_CheckAndCopyDuplicates 
    // so use the current copy on top of the WordStack
    if ( CompileMode && GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) && ( ! _Q_->OVT_LC ) ) word = WordStack ( 0 ) ;
    if ( word->CType & ( OBJECT | THIS | QID ) || Finder_GetQualifyingNamespace ( cntx->Finder0 ) )
    {
        word->AccumulatedOffset = 0 ;
        word->Coding = Here ;
        cntx->Interpreter0->BaseObject = word ;
        cntx->Interpreter0->ObjectNamespace = TypeNamespace_Get ( word ) ;
        cntx->Compiler0->AccumulatedOffsetPointer = 0 ;
        cntx->Compiler0->AccumulatedOptimizeOffsetPointer = & word->AccumulatedOffset ;
        word->CType |= OBJECT ;
    }
    if ( CompileMode )
    {
        _Do_Variable ( word ) ;
    }
    else
    {
        if ( word->CType & ( OBJECT | THIS ) )
        {
            if ( cntx->Compiler0->AccumulatedOffsetPointer )
            {
                if ( GetState ( cntx, C_SYNTAX ) && ( ! IsLValue ( word ) ) && ( Lexer_NextNonDelimiterChar ( cntx->Lexer0 ) != '.' ) )
                {
                    _Push ( * ( int32* ) word->W_PtrToValue + word->AccumulatedOffset ) ;
                }
                else _Push ( word->W_PtrToValue + word->AccumulatedOffset ) ;
            }
            else _Push ( *word->W_PtrToValue ) ;
        }
        else if ( word->CType & VARIABLE )
        {
            int32 value ;
            if ( GetState ( cntx, C_SYNTAX ) )
            {
                if ( IsLValue ( word ) ) value = ( int32 ) word->W_PtrToValue ;
                else value = ( int32 ) * word->W_PtrToValue ;
            }
            else value = ( int32 ) word->W_PtrToValue ;
            _Push ( value ) ;
        }
    }
    SetState ( _Q_->OVT_Context, ADDRESS_OF_MODE, false ) ; // only good for one variable
}
// 'Run' :: this is part of runtime in the interpreter/compiler for data objects
// they are compiled to much more optimized native code

void
DataObject_Run ( Word * word )
{
    Context * cntx = _Q_->OVT_Context ;
    cntx->Interpreter0->w_Word = word ; // for ArrayBegin : all literals are run here
    if ( word->CType & T_LISP_SYMBOL )
    {
        _CfrTil_Do_LispSymbol ( word ) ;
    }
    else if ( word->CType & ( LITERAL | CONSTANT ) )
    {
        _CfrTil_Do_Literal ( word ) ;
    }
    else if ( word->CType & ( VARIABLE | THIS | OBJECT | LOCAL_VARIABLE | PARAMETER_VARIABLE ) )
    {
        _CfrTil_Do_Variable ( word ) ;
    }
    else if ( word->CType & OBJECT_FIELD )
    {
        _CfrTil_Do_ClassField ( word ) ;
    }
    else if ( word->CType & DOBJECT )
    {
        _CfrTil_Do_DynamicObject ( word ) ;
    }
    else if ( word->CType & ( C_TYPE | C_CLASS ) )
    {
        _Namespace_Do_C_Type ( word ) ;
    }
    else if ( word->CType & ( NAMESPACE | CLASS | CLASS_CLONE ) )
    {
        _Namespace_DoNamespace ( word, 0 ) ;
    }
}

