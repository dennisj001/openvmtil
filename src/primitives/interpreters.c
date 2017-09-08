#include "../include/cfrtil.h"

void
CfrTil_DoWord ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Interpreter_DoWord ( _Context_->Interpreter0, word, - 1 ) ;
}

void
CfrTil_CommentToEndOfLine ( )
{
    _CfrTil_UnAppendTokenFromSourceCode ( _CfrTil_, _Context_->Lexer0->OriginalToken ) ;
    Lexer_SourceCodeOff ( _Lexer_ ) ;
    ReadLiner_CommentToEndOfLine ( _Context_->ReadLiner0 ) ;
    String_RemoveEndWhitespace ( _CfrTil_->SC_ScratchPad ) ;
    _CfrTil_SC_ScratchPadIndex_Init ( _CfrTil_ ) ;
    SetState ( _Context_->Lexer0, LEXER_END_OF_LINE, true ) ;
    Lexer_SourceCodeOn ( _Lexer_ ) ;
}

void
CfrTil_ParenthesisComment ( )
{
    _CfrTil_UnAppendTokenFromSourceCode ( _CfrTil_, _Context_->Lexer0->OriginalToken ) ;
    Lexer_SourceCodeOff ( _Lexer_ ) ;
    while ( 1 )
    {
        int64 inChar = ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) ;
        if ( ( inChar == - 1 ) || ( inChar == eof ) ) break ;
        char * token = ( char* ) Lexer_ReadToken ( _Context_->Lexer0 ) ;
        if ( strcmp ( token, "*/" ) == 0 ) return ;
    }
    Lexer_SourceCodeOn ( _Lexer_ ) ;
}

void
CfrTil_If_ConditionalInterpret ( )
{
    if ( ! GetIfStatus ( ) ) SkipPreprocessorCode ( ) ;
}

void
CfrTil_Elif_ConditionalInterpret ( )
{
    if ( ! GetElifStatus ( ) ) SkipPreprocessorCode ( ) ;
}

void
CfrTil_Else_ConditionalInterpret ( )
{
    if ( ! GetElseStatus ( ) ) SkipPreprocessorCode ( ) ;
}

void
CfrTil_Endif_ConditionalInterpret ( )
{
    if ( ! GetEndifStatus ( ) ) SkipPreprocessorCode ( ) ;
}

void
CfrTil_PreProcessor ( )
{
    _CfrTil_UnAppendTokenFromSourceCode ( _CfrTil_, _Context_->Lexer0->OriginalToken ) ;
    Lexer_SourceCodeOff ( _Lexer_ ) ;
    Finder_SetNamedQualifyingNamespace ( _Context_->Finder0, ( byte* ) "PreProcessor" ) ;
    SetState ( _Context_->Interpreter0, PREPROCESSOR_MODE, true ) ;
    _Interpret_ToEndOfLine ( _Context_->Interpreter0 ) ;
    SetState ( _Context_->Interpreter0, PREPROCESSOR_MODE, false ) ;
    if ( GetState ( _Context_->Interpreter0, PREPROCESSOR_DEFINE ) )
    {
        int64 locals = Stack_Depth ( _Context_->Compiler0->LocalsNamespacesStack ) ;
        SetState ( _Context_->Interpreter0, PREPROCESSOR_DEFINE, false ) ;
        CfrTil_SemiColon ( ) ;
        CfrTil_Inline ( ) ;
        if ( locals ) CfrTil_Prefix ( ) ;
    }
    Lexer_SourceCodeOn ( _Lexer_ ) ;
}

void
CfrTil_Define ( )
{
    SetState ( _Context_->Interpreter0, PREPROCESSOR_DEFINE, true ) ;
    CfrTil_Colon ( ) ;
}

void
CfrTil_Interpreter_IsDone ( )
{
    _DataStack_Push ( GetState ( _Context_->Interpreter0, END_OF_FILE | END_OF_STRING | INTERPRETER_DONE ) ) ;
}

void
CfrTil_Interpreter_Done ( )
{
    SetState ( _Context_->Interpreter0, INTERPRETER_DONE, true ) ;
}

void
CfrTil_Interpreter_Init ( )
{
    Interpreter_Init ( _Context_->Interpreter0 ) ;
}

void
CfrTil_InterpretNextToken ( )
{
    Interpreter_InterpretNextToken ( _Context_->Interpreter0 ) ;
}

void
CfrTil_Interpret ( )
{
    _Context_InterpretFile ( _Context_ ) ;
}

void
CfrTil_InterpretPromptedLine ( )
{
    CfrTil_DoPrompt ( ) ;
    Context_Interpret ( _CfrTil_->Context0 ) ;
}

void
CfrTil_InterpretString ( )
{
    _Interpret_String ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_Interpreter_EvalWord ( )
{

    _Interpreter_DoWord ( _Context_->Interpreter0, ( Word* ) _DataStack_Pop ( ), - 1 ) ;
}

void
CfrTil_TokenToWord ( )
{
    byte * token = ( byte* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int64 ) _Interpreter_TokenToWord ( _Context_->Interpreter0, token ) ) ;
}

void
CfrTil_InterpreterStop ( )
{

    SetState ( _Context_->Interpreter0, INTERPRETER_DONE, true ) ;
    SetState ( _CfrTil_, CFRTIL_RUN, false ) ;
}

dllist *
_CfrTil_Interpret_ReadToList ( )
{
    byte * token ;
    Interpreter * interp = _Context_->Interpreter0 ;
    interp->InterpList = List_New ( ) ;
    while ( token = Lexer_ReadToken ( _Lexer_ ) )
    {
        if ( String_Equal ( token, ";l" ) ) break ;
        Word * word = _Interpreter_TokenToWord ( interp, token ) ;
        if ( word )
        {

            _Word_Interpret ( word ) ;
            //List_Push_A_1Value_Node ( interp->InterpList, word ) ;
            List_Push ( interp->InterpList, word, COMPILER_TEMP ) ;
        }
    }
    return interp->InterpList ;
}

void
CfrTil_Interpret_ReadToList ( )
{

    dllist * interpList = _CfrTil_Interpret_ReadToList ( ) ;
    _DataStack_Push ( ( int64 ) interpList ) ;
}

void
CfrTil_Interpret_List ( )
{
    dllist * interpList = ( dllist* ) _DataStack_Pop ( ) ;
    List_Interpret ( interpList ) ;
}

#if 0
#include "/usr/local/include/python3.7m/Python.h"

void
CfrTil_Interpret_Python ( )
{
    byte * pstring = ( byte* ) _DataStack_Pop ( ) ;
    wchar_t *program = Py_DecodeLocale ( "python3.7", NULL ) ;
    Py_SetProgramName ( program ) ; /* optional but recommended */
    Py_Initialize ( ) ;
    PyRun_SimpleString ( pstring ) ;
    PyMem_RawFree ( program ) ;
}
#endif
