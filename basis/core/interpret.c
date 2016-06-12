
#include "../../includes/cfrtil.h"

Boolean
_Interpreter_IsWordPrefixing ( Interpreter * interp, Word * word )
{
    if ( ( GetState ( _Context_, PREFIX_MODE ) ) && ( ! _Q_->OVT_LC ) ) //_Namespace_IsUsing ( _Q_->OVT_CfrTil->LispNamespace ) ) )
    {
        // with this any postfix word that is not a keyword or a c rtl arg word can now be used prefix with parentheses 
        byte c = Lexer_NextNonDelimiterChar ( interp->Lexer0 ) ;
        if ( ( c == '(' ) && ( ! ( word->CProperty & KEYWORD ) ) && ( ! ( word->WProperty & WT_C_PREFIX_RTL_ARGS ) ) )
        {
            return true ;
        }
    }
    return false ;
}

Word *
Word_GetOriginalWord ( Word * word )
{
    Word * ow1, *ow0 ;
    for ( ow0 = word, ow1 = ow0->W_OriginalWord ; ow1 && ( ow1 != ow1->W_OriginalWord ) ; ow0 = ow1, ow1 = ow0->W_OriginalWord ) ;
    if ( ! ow0 ) ow0 = word ;
    return ow0 ;
}

Word *
Compiler_CopyDuplicates ( Compiler * compiler, Word * word )
{
    Word *word0, * word1 ;
    int32 i, depth ;
    dllist * list = compiler->WordList ;
    // we sometimes refer to more than one field of the same object, eg. 'this' in a block
    // each reference may be to a different labeled field each with a different offset so we must 
    // create copies of the multiply referenced word to hold the referenced offsets for the optInfo
    // 'word' is the 'baseObject' word. If it is already on the Object word Stack certain optimizations can be made.
    // we also need to prevent a null StackPushRegisterCode for operator words used more than once in an optimization
    depth = List_Depth ( list ) ;
    word1 = word ;
    word1->W_OriginalWord = word1 ;
    for ( i = 0 ; i < depth ; i ++ )
    {
        word0 = ( Word* ) ( Compiler_WordList ( i ) ) ;
        if ( word == word0 )
        {
            word1 = Word_Copy ( word, TEMPORARY ) ; // especially for "this" so we can use a different Code & AccumulatedOffsetPointer not the existing 
            word1->W_OriginalWord = Word_GetOriginalWord ( word ) ;
            break ;
        }
    }
    _Compiler_WordList_PushWord ( compiler, word1 ) ;
    return word1 ;
}

Word *
_Interpreter_SetupFor_MorphismWord ( Interpreter * interp, Word * word )
{
    Compiler * compiler = _Context_->Compiler0 ;
    if ( ! ( word->CProperty & ( DEBUG_WORD ) ) ) // NB. here so optimize will be 
    {
        word = Compiler_CopyDuplicates ( compiler, word ) ;
    }
    return word ;
}

void
_Interpreter_DoMorphismWord_Default ( Interpreter * interp, Word * word )
{
    word = _Interpreter_SetupFor_MorphismWord ( interp, word ) ;
    interp->w_Word = word ;
    if ( IS_MORPHISM_TYPE ( word ) ) SetState ( _Context_, ADDRESS_OF_MODE, false ) ;
    _Word_Eval ( word ) ;
}

void
_Interpreter_Do_NonMorphismWord ( Word * word )
{
    _Compiler_WordList_PushWord ( _Context_->Compiler0, word ) ;
    Interpreter_DataObject_Run ( word ) ;
}

// four types of words related to syntax
// 1. regular rpn - reverse polish notation
// 2. regular polish, prefix notation where the function precedes the arguments - lisp
// 3. infix which takes one following 'arg' and then becomes regular rpn
// 4. C arg lists which are left to right but are evaluated right to left, ie. to righmost operand goes on the stack first and so on such that topmost is the left operand
// we just rearrange the functions and args such that they all become regular rpn - forth like

void
_Interpreter_Do_MorphismWord ( Interpreter * interp, Word * word, int32 tokenStartReadLineIndex )
{
    if ( word )
    {
        word->W_StartCharRlIndex = ( tokenStartReadLineIndex == - 1 ) ? interp->Lexer0->TokenStart_ReadLineIndex : tokenStartReadLineIndex ;
        _DEBUG_SETUP ( word ) ;
        Context * cntx = _Context_ ;
        cntx->CurrentRunWord = word ;
        interp->w_Word = word ;
        if ( ( word->WProperty == WT_INFIXABLE ) && ( GetState ( cntx, INFIX_MODE ) ) ) // nb. Interpreter must be in INFIX_MODE because it is effective for more than one word
        {
            Finder_SetNamedQualifyingNamespace ( cntx->Finder0, ( byte* ) "Infix" ) ;
            Interpreter_InterpretNextToken ( interp ) ;
            // then continue and interpret this 'word' - just one out of lexical order
            _Interpreter_DoMorphismWord_Default ( interp, word ) ;
        }
        else if ( ( word->WProperty == WT_PREFIX ) || _Interpreter_IsWordPrefixing ( interp, word ) ) // with this almost any rpn function can be used prefix with a following '(' :: this checks for that condition
        {
            SetState ( cntx->Compiler0, DOING_A_PREFIX_WORD, true ) ;
            _Interpret_PrefixFunction_Until_RParen ( interp, word ) ;
            SetState ( cntx->Compiler0, DOING_A_PREFIX_WORD, false ) ;
        }
        else if ( word->WProperty == WT_C_PREFIX_RTL_ARGS )
        {
            word->W_StartCharRlIndex = interp->Lexer0->TokenStart_ReadLineIndex ;
            LC_CompileRun_C_ArgList ( word ) ;
        }
        else _Interpreter_DoMorphismWord_Default ( interp, word ) ; //  case WT_POSTFIX: case WT_INFIXABLE: // cf. also _Interpreter_SetupFor_MorphismWord
        DEBUG_SHOW ;
    }
}

// interpret with find after parse for known objects
// !! this version eliminates the possiblity of numbers being used as words !!
// objects and morphismsm - terms from category theory

void
_Interpreter_Do_NewObjectToken ( Interpreter * interp, byte * token, int32 parseFlag, int32 tokenStartReadLineIndex )
{
    if ( token )
    {
        Word * word = Lexer_ObjectToken_New ( interp->Lexer0, token, parseFlag ) ;
        if ( word )
        {
            if ( ( interp->Lexer0->TokenType & T_RAW_STRING ) && ( GetState ( _Q_, AUTO_VAR ) ) )
            {
                if ( Compiling && GetState ( _Context_, C_SYNTAX ) )
                {
                    byte * token2 = Lexer_PeekNextNonDebugTokenWord ( _Lexer_ ) ;
                    if ( ! String_Equal ( token2, "=" ) ) return ;
                }
            }
            word->W_StartCharRlIndex = ( tokenStartReadLineIndex == - 1 ) ? interp->Lexer0->TokenStart_ReadLineIndex : tokenStartReadLineIndex ;
            _DEBUG_SETUP ( word ) ;
            _Interpreter_Do_NonMorphismWord ( word ) ;
            DEBUG_SHOW ;
        }
    }
}

Word *
_Interpreter_InterpretAToken ( Interpreter * interp, byte * token, int32 tokenStartReadLineIndex )
{
    Word * word = 0 ;
    if ( token )
    {
        interp->Token = token ;
        word = Finder_Word_FindUsing ( interp->Finder0, token, 0 ) ;
        interp->w_Word = word ;
        if ( word )
        {
            _Interpreter_Do_MorphismWord ( interp, word, tokenStartReadLineIndex ) ;
        }
        else
        {
            _Interpreter_Do_NewObjectToken ( interp, token, 1, tokenStartReadLineIndex ) ; //interp->Lexer0->TokenStart_ReadLineIndex ) ;
        }
        if ( word && ( ! ( word->CProperty & DEBUG_WORD ) ) ) interp->LastWord = word ;
    }
    return word ;
}

void
Interpreter_InterpretNextToken ( Interpreter * interp )
{
    byte * token = Lexer_ReadToken ( interp->Lexer0 ) ;
    _Interpreter_InterpretAToken ( interp, token, - 1 ) ;
}

void
_Interpret_ListNode ( dlnode * node )
{
    Word * word = ( Word * ) DynoInt_GetValue ( node ) ;
    _Interpreter_InterpretAToken ( _Interpreter_, word->Name, - 1 ) ;
}

