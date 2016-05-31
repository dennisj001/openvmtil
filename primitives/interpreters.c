#include "../includes/cfrtil.h"

void
CfrTil_Do_MorphismWord ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Interpreter_Do_MorphismWord ( _Context_->Interpreter0, word, - 1 ) ;
}

void
CfrTil_CommentToEndOfLine ( )
{
    ReadLiner_CommentToEndOfLine ( _Context_->ReadLiner0 ) ;
    String_RemoveEndWhitespaceAndAddNewline ( _Q_->OVT_CfrTil->SourceCodeScratchPad ) ;
    _CfrTil_UnAppendTokenFromSourceCode ( _Context_->Lexer0->OriginalToken ) ;
    SC_ScratchPadIndex_Init ( ) ;
    SetState ( _Context_->Lexer0, LEXER_END_OF_LINE, true ) ;
}

void
CfrTil_ParenthesisComment ( )
{
    while ( 1 )
    {
        int inChar = ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) ;
        if ( ( inChar == - 1 ) || ( inChar == eof ) ) break ;
        char * token = ( char* ) Lexer_ReadToken ( _Context_->Lexer0 ) ;
        if ( strcmp ( token, "*/" ) == 0 ) return ;
    }
}

void
CfrTil_If_ConditionalInterpret ( )
{
    _dllist_PushValue ( _Context_->Interpreter0->PreprocessorStackList, 0, COMPILER_TEMP ) ;
    _Interpret_Preprocessor ( 1 ) ;
}

void
CfrTil_Elif_ConditionalInterpret ( )
{
    if ( _dllist_GetTopValue ( _Context_->Interpreter0->PreprocessorStackList ) )
    {
        _Interpret_Preprocessor ( 0 ) ; // skip all code until preprocessor logic
    }
    else CfrTil_CommentToEndOfLine ( ) ;
}

void
CfrTil_Else_ConditionalInterpret ( )
{
    if ( _dllist_GetTopValue ( _Context_->Interpreter0->PreprocessorStackList ) )
    {
        _Interpret_Preprocessor ( 0 ) ;
    }
    else CfrTil_CommentToEndOfLine ( ) ;
}

void
CfrTil_Endif_ConditionalInterpret ( )
{
    _dllist_PopValue ( _Context_->Interpreter0->PreprocessorStackList ) ;
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
    _DoPrompt ( ) ;
    Context_Interpret ( _Q_->OVT_CfrTil->Context0 ) ;
}

void
CfrTil_InterpretString ( )
{
    _Interpret_String ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_Interpreter_EvalWord ( )
{
    _Interpreter_Do_MorphismWord ( _Context_->Interpreter0, ( Word* ) _DataStack_Pop ( ), - 1 ) ;
}

void
CfrTil_InterpretALiteralToken ( )
{
    _Interpreter_Do_NewObjectToken ( _Context_->Interpreter0, ( byte* ) _DataStack_Pop ( ), 1, _Context_->Interpreter0->Lexer0->TokenStart_ReadLineIndex ) ;
}

void
CfrTil_InterpreterStop ( )
{
    SetState ( _Context_->Interpreter0, INTERPRETER_DONE, true ) ;
    SetState ( _Q_->OVT_CfrTil, CFRTIL_RUN, false ) ;
}

