
#include "../includes/cfrtil.h"

Boolean
_Interpreter_IsPrefixWord ( Interpreter * interp, Word * word )
{
    if ( ( GetState ( _Q_->OVT_Context, PREFIX_MODE ) ) && ( ! _Namespace_IsUsing ( _Q_->OVT_CfrTil->LispNamespace ) ) )
        //if ( ! _Namespace_IsUsing ( _Q_->CfrTil->LispNamespace ) ) 
    {
        // with this any postfix word that is not a keyword or a c rtl arg word can now be used prefix with parentheses 
        byte c = Lexer_NextNonDelimiterChar ( interp->Lexer ) ;
        if ( ( c == '(' ) && ( ! ( word->CType & KEYWORD ) ) && ( ! ( word->WType & WT_C_PREFIX_RTL_ARGS ) ) )
        {
            return true ;
        }
    }
    return false ;
}

Word *
Compiler_CheckAndCopyDuplicates ( Compiler * compiler, Word * word0, Stack * stack )
{
    Word * word2 = word0 ;
    if ( CompileMode && GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) )
    {
        Stack * wstk = compiler->WordStack ;
        // we sometimes refer to more than one field of the same object, eg. 'this' in a block
        // each reference may be to a different labeled field each with a different offset so we must 
        // create copies of the multiply referenced word to hold the referenced offsets for the optimizer
        // 'word' is the 'baseObject' word. If it is already on the Object word Stack certain optimizations can be made.
        // we also need to prevent a null StackPushRegisterCode for operator words used more than once in an optimization
        //if ( word0->CType & ( VARIABLE | LOCAL_VARIABLE | NAMESPACE | CLASS |
        //    OBJECT_FIELD | OBJECT | DOBJECT | C_TYPE | C_CLASS | CLASS_CLONE | PARAMETER_VARIABLE ) )
        {
            int32 i, stackSize = Stack_Depth ( wstk ) ;
            Word * word1 ;
            for ( i = 0 ; i < stackSize ; i ++ )
            {
                word1 = ( Word* ) ( Compiler_WordStack ( compiler, - i ) ) ;
                if ( word0 == word1 )
                {
                    word2 = Word_Copy ( word0, TEMPORARY ) ; // especially for "this" so we can use a different Code & AccumulatedOffsetPointer not the existing 
                    break ;
                }
            }
        }
    }
    Stack_Push ( stack, ( int32 ) word2 ) ;
    //d1 ( if ( DebugOn ) Compiler_ShowWordStack ( "\nInterpreter - end of CheckAndCopyDuplicates :: " ) ) ;
    return word2 ;
}

void
_Interpreter_SetupFor_MorphismWord ( Interpreter * interp, Word * word )
{
    Compiler * compiler = interp->Compiler ;
    if ( ( word->CType & INFIXABLE ) && ( GetState ( _Q_->OVT_Context, INFIX_MODE ) ) ) // nb. INFIX_MODE must be in Interpreter because it is effective for more than one word
    {
        Interpreter_InterpretNextToken ( interp ) ;
        // then continue and interpret this 'word' - just one out of lexical order
    }
    if ( ! ( word->CType & PREFIX ) ) interp->CurrentPrefixWord = 0 ; // prefix words are now processed in _Interpreter_DoMorphismToken
    if ( ( ! GetState ( _Q_->OVT_Context, CONTEXT_PARSING_QUALIFIED_ID ) ) && ( ! ( word->CType & ( OBJECT | OBJECT_FIELD | OBJECT_OPERATOR | DEBUG_WORD ) ) ) )
    {
        //interp->BaseObject = 0 ;
        Finder_SetQualifyingNamespace ( interp->Finder, 0 ) ;
    }
    if ( ! ( word->CType & ( DEBUG_WORD ) ) )
    {
        word = Compiler_CheckAndCopyDuplicates ( compiler, word, compiler->WordStack ) ;
    }
    interp->w_Word = word ;
    word->StackPushRegisterCode = 0 ; // nb. used! by the rewriting optimizer
    // keep track in the word itself where the machine code is to go if this word is compiled or causes compiling code - used for optimization
    word->Coding = Here ;
}

// TODO : this ... well just look at it... 

void
_Interpreter_Do_MorphismWord ( Interpreter * interp, Word * word )
{
    int32 svs_c_rhs, svs_c_lhs ;
    if ( word )
    {
        if ( _Interpreter_IsPrefixWord ( interp, word ) )
        {
            _Interpret_PrefixFunction_Until_RParen ( interp, word ) ;
        }
        else
        {
            switch ( word->WType )
            {
                case WT_PREFIX:
                {
                    _Interpret_PrefixFunction_Until_RParen ( interp, word ) ;
                    break ;
                }
                case WT_C_PREFIX_RTL_ARGS:
                {
                    _Interpreter_SetupFor_MorphismWord ( interp, word ) ;
                    if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) )
                    {
                        svs_c_rhs = GetState ( _Q_->OVT_Context, C_RHS ) ;
                        svs_c_lhs = GetState ( _Q_->OVT_Context, C_LHS ) ;
                        SetState ( _Q_->OVT_Context, C_RHS, true ) ;
                    }
                    LC_CompileRun_ArgList ( word ) ;
                    if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) )
                    {
                        SetState ( _Q_->OVT_Context, C_RHS, svs_c_rhs ) ;
                        SetState ( _Q_->OVT_Context, C_LHS, svs_c_lhs ) ;
                    }
                    break ;
                }
                case WT_POSTFIX:
                case WT_INFIXABLE: // cf. also _Interpreter_SetupFor_MorphismWord
                default:
                {
                    _Interpreter_SetupFor_MorphismWord ( interp, word ) ;
                    _Word_Eval ( interp->w_Word ) ;
                    break ;
                }
            }
        }
    }
}

void
_Interpret_MorphismWord_Default ( Interpreter * interp, Word * word )
{
    _Interpreter_SetupFor_MorphismWord ( interp, word ) ;
    _Word_Eval ( interp->w_Word ) ;
}

void
Interpret_MorphismWord_Default ( Word * word )
{
    _Interpreter_SetupFor_MorphismWord ( _Q_->OVT_Context->Interpreter0, word ) ;
    _Word_Eval ( _Q_->OVT_Context->Interpreter0->w_Word ) ;
}

// interpret with find after parse for known objects
// !! this version eliminates the possiblity of numbers being used as words !!
// objects and morphismsm - terms from category theory

Word *
_Interpreter_InterpretAToken ( Interpreter * interp, byte * token )
{
    Word * word = 0 ;
    if ( token )
    {
        DEBUG_START ;
        interp->Token = token ;
        word = Finder_Word_FindUsing ( interp->Finder, token ) ; // ?? find after Literal - eliminate make strings or numbers words ??
        if ( word )
        {
            interp->w_Word = word ;
            _Interpreter_Do_MorphismWord ( interp, word ) ;
        }
        else
        {
            word = Lexer_Do_ObjectToken_New ( interp->Lexer, token, 1 ) ;
            interp->w_Word = word ;
        }
        DEBUG_SHOW ;
    }
    return word ;
}

void
Interpreter_InterpretNextToken ( Interpreter * interp )
{
    byte * token = Lexer_ReadToken ( interp->Lexer ) ;
    _Interpreter_InterpretAToken ( interp, token ) ;
}

