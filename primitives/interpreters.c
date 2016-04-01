#include "../includes/cfrtil.h"

void
CfrTil_Setup_WordEval ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Interpreter_SetupFor_MorphismWord ( _Q_->OVT_Context->Interpreter0, word ) ;
}

void
CfrTil_Do_MorphismWord ( )
{
    Word * word = ( Word* ) _DataStack_Pop ( ) ;
    _Interpreter_Do_MorphismWord ( _Q_->OVT_Context->Interpreter0, word ) ;
}

void
CfrTil_CommentToEndOfLine ( )
{
    ReadLiner_CommentToEndOfLine ( _Q_->OVT_Context->ReadLiner0 ) ;
    String_RemoveEndWhitespaceAndAddNewline ( _Q_->OVT_CfrTil->SourceCodeScratchPad ) ;
    _CfrTil_UnAppendTokenFromSourceCode ( _Q_->OVT_Context->Lexer0->OriginalToken ) ;
    SC_ScratchPadIndex_Init ( ) ;
    SetState ( _Q_->OVT_Context->Lexer0, LEXER_END_OF_LINE, true ) ;
}

void
CfrTil_ParenthesisComment ( )
{
    while ( 1 )
    {
        int inChar = ReadLine_PeekNextChar ( _Q_->OVT_Context->ReadLiner0 ) ;
        if ( ( inChar == - 1 ) || ( inChar == eof ) ) break ;
        char * token = ( char* ) Lexer_ReadToken ( _Q_->OVT_Context->Lexer0 ) ;
        if ( strcmp ( token, "*/" ) == 0 ) return ;
    }
}

void
CfrTil_If_ConditionalInterpret ( )
{
    _CfrTil_ConditionalInterpret ( 1 ) ;
}

void
CfrTil_Else_ConditionalInterpret ( )
{
    _CfrTil_ConditionalInterpret ( 0 ) ;
}

void
CfrTil_Interpreter_IsDone ( )
{
    _DataStack_Push ( Interpreter_GetState ( _Q_->OVT_Context->Interpreter0, END_OF_FILE | END_OF_STRING | INTERPRETER_DONE ) ) ;
}

void
CfrTil_Interpreter_Done ( )
{
    Interpreter_SetState ( _Q_->OVT_Context->Interpreter0, INTERPRETER_DONE, true ) ;
}

void
CfrTil_Interpreter_Init ( )
{
    Interpreter_Init ( _Q_->OVT_Context->Interpreter0 ) ;
}

void
CfrTil_InterpretNextToken ( )
{
    Interpreter_InterpretNextToken ( _Q_->OVT_Context->Interpreter0 ) ;
}

void
CfrTil_Interpret ( )
{
    _Context_InterpretFile ( _Q_->OVT_Context ) ;
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
    _InterpretString ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_Interpreter_EvalWord ( )
{
    _Interpreter_Do_MorphismWord ( _Q_->OVT_Context->Interpreter0, ( Word* ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_InterpretALiteralToken ( )
{
    Word * word = Lexer_Do_ObjectToken_New ( _Q_->OVT_Context->Lexer0, ( byte* ) _DataStack_Pop ( ), 1 ) ;
    _Interpreter_InterpretWord ( _Q_->OVT_Context->Interpreter0, word ) ;
}

void
_CfrTil_Interpret ( CfrTil * cfrTil )
{
    do
    {
        _CfrTil_Init_SessionCore ( cfrTil, 1, 1 ) ;
        Context_Interpret ( cfrTil->Context0 ) ;
    }
    while ( GetState ( cfrTil, CFRTIL_RUN ) ) ;
}

void
CfrTil_InterpreterRun ( )
{
    _CfrTil_Interpret ( _Q_->OVT_CfrTil ) ;
}

void
CfrTil_InterpreterStop ( )
{
    SetState ( _Q_->OVT_Context->Interpreter0, INTERPRETER_DONE, true ) ;
    SetState ( _Q_->OVT_CfrTil, CFRTIL_RUN, false ) ;
}

