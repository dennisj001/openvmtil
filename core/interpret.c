
#include "../includes/cfrtil.h"

Boolean
_Interpreter_IsWordPrefixing ( Interpreter * interp, Word * word )
{
    if ( ( GetState ( _Q_->OVT_Context, PREFIX_MODE ) ) && ( ! _Q_->OVT_LC ) ) //_Namespace_IsUsing ( _Q_->OVT_CfrTil->LispNamespace ) ) )
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
Compiler_PushCheckAndCopyDuplicates ( Compiler * compiler, Word * word0, Stack * stack )
{
    Word *word1, * word2 = word0 ;
    int32 i, stackDepth ;
    // we sometimes refer to more than one field of the same object, eg. 'this' in a block
    // each reference may be to a different labeled field each with a different offset so we must 
    // create copies of the multiply referenced word to hold the referenced offsets for the optimizer
    // 'word' is the 'baseObject' word. If it is already on the Object word Stack certain optimizations can be made.
    // we also need to prevent a null StackPushRegisterCode for operator words used more than once in an optimization
    stackDepth = Stack_Depth ( stack ) ;
    for ( i = 0 ; i < stackDepth ; i ++ )
    {
        word1 = ( Word* ) ( Compiler_WordStack ( compiler, - i ) ) ;
        if ( word0 == word1 )
        {
            word2 = Word_Copy ( word0, TEMPORARY ) ; // especially for "this" so we can use a different Code & AccumulatedOffsetPointer not the existing 
            break ;
        }
    }
    Stack_Push ( stack, ( int32 ) word2 ) ;
    //if ( DebugOn ) Compiler_ShowWordStack ( "\nInterpreter - end of CheckAndCopyDuplicates :: " ) ;
    return word2 ;
}

void
_Interpreter_SetupFor_MorphismWord ( Interpreter * interp, Word * word )
{
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    if ( ( word->CType & INFIXABLE ) && ( GetState ( _Q_->OVT_Context, INFIX_MODE ) ) ) // nb. INFIX_MODE must be in Interpreter because it is effective for more than one word
    {
        Finder_SetNamedQualifyingNamespace ( _Q_->OVT_Context->Finder0, "Infix" ) ;
        Interpreter_InterpretNextToken ( interp ) ;
        // then continue and interpret this 'word' - just one out of lexical order
    }
    if ( ! ( word->CType & ( DEBUG_WORD ) ) ) // NB. here so optimize will be 
    {
        word = Compiler_PushCheckAndCopyDuplicates ( compiler, word, compiler->WordStack ) ;
    }
    word->StackPushRegisterCode = 0 ; // nb. used! by the rewriting optimizer
    // keep track in the word itself where the machine code is to go if this word is compiled or causes compiling code - used for optimization
    word->Coding = Here ;
    interp->w_Word = word ;
}

// TODO : this ... well just look at it... 

void
_Interpreter_Do_MorphismWord ( Interpreter * interp, Word * word )
{
    Context * cntx = _Q_->OVT_Context ;
    int32 svs_c_rhs, svs_c_lhs ;
    if ( word )
    {
        if ( ( word->WType == WT_PREFIX ) || _Interpreter_IsWordPrefixing ( interp, word ) ) // with this almost any rpn function can be used prefix with a following '(' :: this checks for that condition
        {
            _Interpret_PrefixFunction_Until_RParen ( interp, word ) ;
        }
        else if ( word->WType == WT_C_PREFIX_RTL_ARGS )
        {
            if ( GetState ( cntx, C_SYNTAX ) )
            {
                svs_c_rhs = GetState ( cntx, C_RHS ) ;
                svs_c_lhs = GetState ( cntx, C_LHS ) ;
                SetState ( cntx, C_RHS, true ) ;
            }
            LC_CompileRun_ArgList ( word ) ;
            if ( GetState ( cntx, C_SYNTAX ) )
            {
                SetState ( cntx, C_RHS, svs_c_rhs ) ;
                SetState ( cntx, C_LHS, svs_c_lhs ) ;
            }
        }
        else _Interpret_MorphismWord_Default ( interp, word ) ; //  case WT_POSTFIX: case WT_INFIXABLE: // cf. also _Interpreter_SetupFor_MorphismWord
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
    _Word_Eval ( _Q_->OVT_CfrTil->CurrentRunWord ) ;
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
        word = Finder_Word_FindUsing ( interp->Finder, token, 0 ) ;
        if ( word )
        {
            _Interpreter_Do_MorphismWord ( interp, word ) ;
        }
        else
        {
            word = Lexer_Do_ObjectToken_New ( interp->Lexer, token, 1 ) ;
        }
        interp->w_Word = word ;
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

