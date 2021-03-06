
#include "../../include/cfrtil32.h"

void
_Namespace_Do_C_Type ( Namespace * ns )
{
  Context * cntx = _Context_ ;
  Lexer * lexer = cntx->Lexer0 ;
  byte * token1, *token2 ;
  if ( !GetState ( cntx->Compiler0, DOING_C_TYPE ) )
  {
      SetState ( cntx->Compiler0, DOING_C_TYPE, true ) ;
      if ( !GetState ( cntx->Compiler0, LC_ARG_PARSING ) )
      {
          if ( ( !Compiling ) )
          {
              _CfrTil_InitSourceCode_WithName ( _CfrTil_, ns->Name ) ;
          }
          if ( GetState ( cntx, C_SYNTAX ) ) //&& ( cntx->System0->IncludeFileStackNumber ) )
          {
              LambdaCalculus * svlc = _Q_->OVT_LC ;
              _Q_->OVT_LC = 0 ;
              // ?? parts of this could be screwing up other things and adds an unnecessary level of complexity
              // for parsing C functions 
              token1 = _Lexer_NextNonDebugOrCommentTokenWord ( lexer, 1 ) ;
              int32 token1TokenStart_ReadLineIndex = lexer->TokenStart_ReadLineIndex ;
              token2 = Lexer_PeekNextNonDebugTokenWord ( lexer, 1 ) ;
              if ( token2 [0] == '(' )
              {
                  SetState ( _Compiler_, C_COMBINATOR_PARSING, true ) ;
                  _Namespace_ActivateAsPrimary ( ns ) ;
                  Word * word = Word_New ( token1 ) ;
                  _DataStack_Push ( (int32) word ) ; // token1 is the function name 
                  CfrTil_RightBracket ( ) ; //Set_CompileMode ( true ) ; //SetState ( _Context_->Compiler0, COMPILE_MODE, true ) ;
                  CfrTil_BeginBlock ( ) ;
                  cntx->Compiler0->C_BackgroundNamespace = _Namespace_FirstOnUsingList ( ) ; //nb! must be before CfrTil_LocalsAndStackVariablesBegin else CfrTil_End_C_Block will 
                  CfrTil_LocalsAndStackVariablesBegin ( ) ;
                  Ovt_AutoVarOn ( ) ;
                  do
                  {
                      byte * token = Lexer_ReadToken ( lexer ) ;
                      if ( String_Equal ( token, "s{" ) )
                      {
                          Interpreter_InterpretAToken ( _Interpreter_, token, -1 ) ;
                          break ;
                      }
                      else if ( token [ 0 ] == '{' )
                      {
#if 1                   // this is a little strange but for now i want this capacity somehow ??             
                          byte * token = Lexer_ReadToken ( lexer ) ;
                          if ( String_Equal ( token, "<dbg>" ) )
                          {
                              Compiler * compiler = _Context_->Compiler0 ;
                              BlockInfo * bi = (BlockInfo *) _Stack_Top ( compiler->BlockStack ) ;
                              SetHere ( bi->FrameStart ) ; // before _Compile_AddLocalFrame
                              Interpreter_InterpretAToken ( _Interpreter_, token, -1 ) ;
                              _Compiler_AddLocalFrame ( compiler ) ;
                              Compile_InitRegisterParamenterVariables ( compiler ) ; // this function is called twice to deal with words that have locals before the first block and regular colon words
                              bi->Start = Here ; // after _Compiler_AddLocalFrame and Compile_InitRegisterVariables
                          }
                          else if ( !_Lexer_ConsiderDebugAndCommentTokens ( token, 1, 0 ) ) _CfrTil_AddTokenToTailOfTokenList ( token ) ;
#endif            
                          break ; // take nothing else (would be Syntax Error ) -- we have already done CfrTil_BeginBlock
                      }
                      else _Lexer_ConsiderDebugAndCommentTokens ( token, 1, 0 ) ;
                  }
                  while ( 1 ) ;
                  goto rtrn ;
              }
              else
              {
                  //next :
                  if ( Compiling ) Ovt_AutoVarOn ( ) ;
                  _Namespace_DoNamespace ( ns, 1 ) ;
                  // remember : we have already gotten a token
                  Interpreter_InterpretAToken ( cntx->Interpreter0, token1, token1TokenStart_ReadLineIndex ) ;
                  if ( Compiling )
                  {
                      cntx->Compiler0->C_BackgroundNamespace = _Namespace_FirstOnUsingList ( ) ;
                      while ( 1 )
                      {
                          byte * token = _Interpret_C_Until_EitherToken ( cntx->Interpreter0, (byte*) ",", (byte*) ";", 0 ) ;
                          if ( !token ) break ;
                          if ( ( String_Equal ( token, "," ) ) )
                          {
                              cntx->Compiler0->LHS_Word = 0 ;
                              if ( GetState ( cntx->Compiler0, DOING_A_PREFIX_WORD ) ) break ;
                              else continue ;
                          }
                          if ( ( String_Equal ( token, ";" ) ) )
                          {
                              _CfrTil_AddTokenToHeadOfTokenList ( token ) ;
                              if ( cntx->Compiler0->C_BackgroundNamespace ) _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
                              break ;
                          }
                          else
                          {
                              if ( ( String_Equal ( token, ")" ) ) )
                              {
                                  if ( GetState ( cntx->Compiler0, DOING_A_PREFIX_WORD ) ) _CfrTil_AddTokenToHeadOfTokenList ( token ) ; // add ahead of token2 :: ?? this could be screwing up other things and adds an unnecessary level of complexity
                              }
                              cntx->Compiler0->LHS_Word = 0 ;
                              if ( cntx->Compiler0->C_BackgroundNamespace ) _CfrTil_Namespace_InNamespaceSet ( cntx->Compiler0->C_BackgroundNamespace ) ;
                              break ;
                          }
                      }
                  }
                  //Ovt_AutoVarOff ( ) ;
              }
              _Q_->OVT_LC = svlc ;
          }
      }
      else _Namespace_DoNamespace ( ns, 1 ) ;
rtrn:
      SetState ( cntx->Compiler0, DOING_C_TYPE, false ) ;
  }
}

void
_CfrTil_Do_ClassField ( Word * word )
{
  Context * cntx = _Context_ ;
  Compiler * compiler = cntx->Compiler0 ;
  cntx->Interpreter0->CurrentObjectNamespace = word ; // update this namespace 
  compiler->ArrayEnds = 0 ;
  uint32 accumulatedAddress ;

  if ( ( CompileMode ) || GetState ( compiler, LC_ARG_PARSING ) )
  {
      if ( word->Offset )
      {
          Compiler_IncrementCurrentAccumulatedOffset ( compiler, word->Offset ) ;
      }
      if ( CompileMode ) WordList_Pop ( cntx->Compiler0->WordList, 0 ) ;
  }
  else
  {
      accumulatedAddress = _DataStack_Pop ( ) ;
      accumulatedAddress += word->Offset ;
      if ( GetState ( cntx, C_SYNTAX ) && ( !Is_LValue ( word ) ) && ( !GetState ( _Context_, ADDRESS_OF_MODE ) ) )
      {
          DSP_Push ( * (int32*) accumulatedAddress ) ;
      }
      else
      {
          DSP_Push ( accumulatedAddress ) ;
          //SetState ( _Context_, ADDRESS_OF_MODE, false ) ;
      }
  }
  if ( Lexer_IsTokenForwardDotted ( cntx->Lexer0 ) ) Finder_SetQualifyingNamespace ( cntx->Finder0, word->ClassFieldTypeNamespace ) ;
}

// nb. 'word' is the previous word to the '.' (dot) cf. CfrTil_Dot so it can be recompiled, a little different maybe, as an object

void
CfrTil_Dot ( ) // .
{
  Context * cntx = _Context_ ;
  if ( !cntx->Interpreter0->BaseObject )
  {
      SetState ( cntx, CONTEXT_PARSING_QID, true ) ;
      d0 ( if ( Is_DebugModeOn ) Compiler_Show_WordList ( "\nCfrTil_Dot" ) ) ;

      Word * word = Compiler_PreviousNonDebugWord ( -1 ) ; // 0 : rem: we just popped the WordStack above
      if ( word )
      {
          if ( word->CProperty & NAMESPACE_TYPE )
          {
              Finder_SetQualifyingNamespace ( cntx->Finder0, word ) ;
          }
          else
          {
              cntx->Interpreter0->BaseObject = word ;
              _DataObject_Run ( word ) ;
          }
      }
  }
  WordList_Pop ( cntx->Compiler0->WordList, 0 ) ; // nb. first else problems with DataObject_Run ( word ) 
}

// rvalue - rhs value - right hand side of '=' - the actual value, used on the right hand side of C statements

void
_Word_CompileAndRecord_PushReg ( Word * word, int32 reg )
{
  if ( word )
  {
      word->StackPushRegisterCode = Here ; // nb. used! by the rewriting optInfo
      if ( word->CProperty & REGISTER_VARIABLE ) _Compile_Stack_PushReg ( DSP, word->RegToUse ) ;
      else _Compile_Stack_PushReg ( DSP, reg ) ;
  }
}

void
_Do_Literal ( int32 value )
{
  if ( CompileMode )
  {
      _Compile_MoveImm_To_Reg ( EAX, value, CELL ) ;
      _Compiler_CompileAndRecord_PushEAX ( _Context_->Compiler0 ) ; // does word == top of word stack always
  }
  else DSP_Push ( value ) ;
}

// a constant is, of course, a literal

void
_Namespace_DoNamespace ( Namespace * ns, int32 immFlag )
{
  Context * cntx = _Context_ ;
  if ( ( !immFlag ) && CompileMode && ( Lexer_NextNonDelimiterChar ( cntx->Lexer0 ) != '.' ) && ( !GetState ( cntx->Compiler0, LC_ARG_PARSING ) ) )
  {
      _Compile_C_Call_1_Arg ( (byte*) _Namespace_DoNamespace, (int32) ns ) ;
  }
  if ( !Lexer_IsTokenForwardDotted ( cntx->Lexer0 ) )
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
  Context * cntx = _Context_ ;
  Lexer * lexer = cntx->Lexer0 ;
  DObject * ndobject ;
  byte * token ;
  while ( Lexer_IsTokenForwardDotted ( lexer ) )
  {
      Lexer_ReadToken ( lexer ) ; // the '.'
      token = Lexer_ReadToken ( lexer ) ;
      if ( String_Equal ( "prototype", (char*) token ) )
      {
          dobject = dobject->ContainingNamespace ;
          continue ;
      }
      if ( !( ndobject = _DObject_FindSlot_BottomUp ( dobject, token ) ) )
      {
          dobject = _DObject_NewSlot ( dobject, token, 0 ) ;
      }
      else dobject = ndobject ;
  }
  cntx->Interpreter0->CurrentObjectNamespace = TypeNamespace_Get ( dobject ) ;
  if ( CompileMode )
  {
      _Compile_DataStack_Push ( (int32) dobject->W_PtrToValue ) ; //& dobject->W_DObjectValue ) ; //dobject ) ;
  }
  else
  {
      DSP_Push ( (int32) dobject->W_PtrToValue ) ; //& dobject->W_DObjectValue ) ; //dobject ) ;
  }
}

void
_Do_Variable ( Word * word )
{
  Context * cntx = _Context_ ;
  if ( GetState ( cntx, C_SYNTAX ) || GetState ( cntx->Compiler0, LC_ARG_PARSING ) )
  {
      if ( Is_LValue ( word ) ) //_Context_->CurrentRunWord ) ) // word ) ) // ?? not sure exactly why this is necessary 
      {
          //word = _Context_->CurrentRunWord ;
          //if ( GetState ( cntx->Compiler0, DOING_C_TYPE ) ) _Compile_VarLitObj_LValue_To_Reg ( word, EAX ) ;
          cntx->Compiler0->LHS_Word = word ;
          word->Coding = Here ;
      }
      else
      {
          if ( GetState ( _Context_, ADDRESS_OF_MODE ) )
          {
              _Compile_GetVarLitObj_LValue_To_Reg ( word, EAX ) ;
              //SetState ( _Context_, ADDRESS_OF_MODE, false ) ; // only good for one variable
          }
          else _Compile_GetVarLitObj_RValue_To_Reg ( word, EAX ) ;
          _Word_CompileAndRecord_PushReg ( word, EAX ) ;
      }
  }
  else
  {
      _Compile_GetVarLitObj_LValue_To_Reg ( word, EAX ) ;
      _Word_CompileAndRecord_PushReg ( word, EAX ) ;
  }
}

void
_CfrTil_Do_Literal ( Word * word )
{
  if ( CompileMode )
  {
      if ( GetState ( _Context_, C_SYNTAX ) || GetState ( _Compiler_, LC_ARG_PARSING ) ) // for now until we have time to integrate this optimization
      {
          _Compile_GetVarLitObj_RValue_To_Reg ( word, EAX ) ;
          _Word_CompileAndRecord_PushReg ( word, EAX ) ;
      }
      else
      {
          Compile_ADDI ( REG, DSP, 0, sizeof (int32 ), BYTE ) ;
          _Compile_MoveImm_To_Mem ( DSP, ( int32 ) * word->W_PtrToValue, CELL ) ;
      }

  }
  else
  {
      if ( word->CProperty & T_STRING | T_RAW_STRING ) DSP_Push ( word->W_PtrValue ) ;
      else DSP_Push ( * word->W_PtrToValue ) ;

  }
}

void
_CfrTil_Do_LispSymbol ( Word * word )
{
  // rvalue - rhs for stack var
  _Compile_Move_StackN_To_Reg ( EAX, FP, ParameterVarOffset ( word ) ) ;
  _Word_CompileAndRecord_PushReg ( word, EAX ) ;
}

void
_CfrTil_Do_Variable ( Word * word )
{
  Context * cntx = _Context_ ;
  // since we can have multiple uses of the same word in a block we make copies in Compiler_CheckAndCopyDuplicates 
  // so be sure to use the current copy on top of the WordStack
  if ( CompileMode && GetState ( _CfrTil_, OPTIMIZE_ON ) && ( !_Q_->OVT_LC ) ) word = Compiler_WordList ( 0 ) ; //_Context_->CurrentlyRunningWord ; //WordStack ( 0 ) ;
  if ( !( word->CProperty & ( NAMESPACE_VARIABLE ) ) )
  {
      if ( word->CProperty & ( OBJECT | THIS | QID ) || GetState ( word, QID ) ) //Finder_GetQualifyingNamespace ( cntx->Finder0 ) )
      {
          word->AccumulatedOffset = 0 ;
          word->Coding = Here ;
          cntx->Interpreter0->BaseObject = word ;
          cntx->Interpreter0->CurrentObjectNamespace = TypeNamespace_Get ( word ) ;
          cntx->Compiler0->AccumulatedOffsetPointer = 0 ;
          cntx->Compiler0->AccumulatedOptimizeOffsetPointer = &word->AccumulatedOffset ;
          word->CProperty |= OBJECT ;
          if ( word->CProperty & THIS )
          {
              word->S_ContainingNamespace = _Context_->Interpreter0->ThisNamespace ;
          }
      }
  }
  if ( CompileMode )
  {
      if ( word->CProperty & OBJECT )
      {
          Word * bo = cntx->Interpreter0->BaseObject ;
          if ( ( bo ) && ( bo->S_ContainingNamespace == word->S_ContainingNamespace ) )
          {
              SetHere ( bo->Coding ) ; // ?? not fully understood but it solved some of the remaining problem
          }
      }
      _Do_Variable ( word ) ;
  }
  else
  {
      if ( word->CProperty & ( OBJECT | THIS ) )
      {
          if ( cntx->Compiler0->AccumulatedOffsetPointer )
          {
              if ( GetState ( cntx, C_SYNTAX ) && ( !Is_LValue ( word ) ) && ( Lexer_NextNonDelimiterChar ( cntx->Lexer0 ) != '.' ) )
              {
                  DSP_Push ( * (int32*) word->W_PtrToValue + word->AccumulatedOffset ) ;
              }
              else DSP_Push ( word->W_PtrToValue + word->AccumulatedOffset ) ;
          }
          else DSP_Push ( *word->W_PtrToValue ) ;
      }
      else if ( word->CProperty & NAMESPACE_VARIABLE )
      {
          int32 value ;
          if ( GetState ( cntx, C_SYNTAX ) )
          {
              if ( Is_LValue ( word ) ) value = (int32) word->W_PtrToValue ;
              else value = ( int32 ) * word->W_PtrToValue ;
          }
          else value = (int32) word->W_PtrToValue ;
          DSP_Push ( value ) ;
      }
  }
  //SetState ( _Context_, ADDRESS_OF_MODE, false ) ; // only good for one variable
}

void
_Do_LocalObject_AllocateInit ( Namespace * typeNamespace, byte ** value, int32 size )
{
  byte * obj = _CfrTil_NamelessObjectNew ( size, TEMPORARY ) ; //CfrTil_NamelessObjectNew ( size ) ;
  _Class_Object_Init ( obj, typeNamespace ) ;
  * value = (byte*) obj ;
}

// 'Run' :: this is part of runtime in the interpreter/compiler for data objects
// they are compiled to much more optimized native code

void
_DataObject_Run ( Word * word )
{
  Context * cntx = _Context_ ;
  cntx->Interpreter0->w_Word = word ; // for ArrayBegin : all literals are run here
  cntx->CurrentlyRunningWord = word ;
  if ( word->LProperty & LOCAL_OBJECT )
  {
      if ( ( word->CProperty & LOCAL_VARIABLE ) && ( !GetState ( word, W_INITIALIZED ) ) ) // this is a local variable so it is initialed at creation 
      {
          int32 size = _Namespace_VariableValueGet ( word->TypeNamespace, (byte*) "size" ) ;
          _Compile_MoveImm_To_Reg ( EAX, (int32) size, CELL ) ;
          _Compile_PushReg ( EAX ) ;
          _Compile_LEA ( EAX, FP, 0, LocalVarIndex_Disp ( LocalVarOffset ( word ) ) ) ; // 2 : account for saved fp and return slot
          _Compile_PushReg ( EAX ) ;
          _Compile_MoveImm_To_Reg ( EAX, (int32) word->TypeNamespace, CELL ) ;
          _Compile_PushReg ( EAX ) ;
          Compile_Call ( (byte*) _Do_LocalObject_AllocateInit ) ; // we want to only allocate this object once and only at run time; and not at compile time
          Compile_ADDI ( REG, ESP, 0, 3 * sizeof (int32 ), 0 ) ;
          SetState ( word, W_INITIALIZED, true ) ;
      }
      _CfrTil_Do_Variable ( word ) ;
  }
  else if ( word->CProperty & T_LISP_SYMBOL )
  {
      if ( !GetState ( cntx->Compiler0, LC_CFRTIL ) ) _CfrTil_Do_LispSymbol ( word ) ;
      else _CfrTil_Do_Variable ( word ) ;
  }
  else if ( word->CProperty & DOBJECT )
  {
      _CfrTil_Do_DynamicObject ( word ) ;
  }
  else if ( word->CProperty & ( LITERAL | CONSTANT ) )
  {
      _CfrTil_Do_Literal ( word ) ;
  }
  else if ( word->CProperty & OBJECT_FIELD )
  {
      _CfrTil_Do_ClassField ( word ) ;
  }
  else if ( word->CProperty & ( NAMESPACE_VARIABLE | THIS | OBJECT | LOCAL_VARIABLE | PARAMETER_VARIABLE ) )
  {
      _CfrTil_Do_Variable ( word ) ;
  }
  else if ( word->CProperty & ( C_TYPE | C_CLASS ) )
  {
      _Namespace_Do_C_Type ( word ) ;
  }
  else if ( word->CProperty & ( NAMESPACE | CLASS | CLASS_CLONE ) )
  {
      _Namespace_DoNamespace ( word, 0 ) ;
  }
}

void
DataObject_Run ( )
{
  Word * word = _Context_->CurrentlyRunningWord ;
  DEBUG_SETUP ( word ) ;
  _DataObject_Run ( word ) ;
  DEBUG_SHOW ;
}

