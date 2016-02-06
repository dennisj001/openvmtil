
#include "../../includes/cfrtil.h"

Boolean
_Interpreter_IsWordPrefixing ( Interpreter * interp, Word * word )
{
    if ( ( GetState ( _Q_->OVT_Context, PREFIX_MODE ) ) && ( ! _Q_->OVT_LC ) ) //_Namespace_IsUsing ( _Q_->OVT_CfrTil->LispNamespace ) ) )
    {
        // with this any postfix word that is not a keyword or a c rtl arg word can now be used prefix with parentheses 
        byte c = Lexer_NextNonDelimiterChar ( interp->Lexer0 ) ;
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
    Word *word1, * word2 ;
    int32 i, stackDepth ;
    // we sometimes refer to more than one field of the same object, eg. 'this' in a block
    // each reference may be to a different labeled field each with a different offset so we must 
    // create copies of the multiply referenced word to hold the referenced offsets for the optimizer
    // 'word' is the 'baseObject' word. If it is already on the Object word Stack certain optimizations can be made.
    // we also need to prevent a null StackPushRegisterCode for operator words used more than once in an optimization
    stackDepth = Stack_Depth ( stack ) ;
    for ( i = 0, word2 = word0 ; i < stackDepth ; i ++ )
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
    if ( ( word->CType & INFIXABLE ) && ( GetState ( _Q_->OVT_Context, INFIX_MODE ) ) ) // nb. Interpreter must be in INFIX_MODE because it is effective for more than one word
    {
        Finder_SetNamedQualifyingNamespace ( _Q_->OVT_Context->Finder0, ( byte* ) "Infix" ) ;
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
    if ( IS_MORPHISM_TYPE ( word ) ) SetState ( _Q_->OVT_Context, ADDRESS_OF_MODE, false ) ;
}

void
_Interpreter_Do_MorphismWord ( Interpreter * interp, Word * word )
{
    if ( word )
    {
        if ( ( word->WType == WT_PREFIX ) || _Interpreter_IsWordPrefixing ( interp, word ) ) // with this almost any rpn function can be used prefix with a following '(' :: this checks for that condition
        {
            _Interpret_PrefixFunction_Until_RParen ( interp, word ) ;
        }
        else if ( word->WType == WT_C_PREFIX_RTL_ARGS )
        {
            LC_CompileRun_ArgList ( word ) ;
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

// interpret with find after parse for known objects
// !! this version eliminates the possiblity of numbers being used as words !!
// objects and morphismsm - terms from category theory

Word *
_Interpreter_InterpretAToken ( Interpreter * interp, byte * token )
{
    Word * word = 0 ;
    if ( token )
    {
        interp->Token = token ;
        word = Finder_Word_FindUsing ( interp->Finder0, token, 0 ) ;
        if ( word )
        {
            word->W_StartCharRlIndex = _Q_->OVT_Context->Lexer0->TokenStart_ReadLineIndex ;
            _Q_->OVT_Context->CurrentRunWord = word ;
            _Interpreter_Do_MorphismWord ( interp, word ) ;
        }
        else
        {
            word = Lexer_Do_ObjectToken_New ( interp->Lexer0, token, 1 ) ;
        }
        interp->w_Word = word ;
    }
    return word ;
}

void
Interpreter_InterpretNextToken ( Interpreter * interp )
{
    byte * token = Lexer_ReadToken ( interp->Lexer0 ) ;
    _Interpreter_InterpretAToken ( interp, token ) ;
}

