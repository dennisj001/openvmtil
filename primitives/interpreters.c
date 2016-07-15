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
    //List_Push_A_1Value_Node ( _Context_->Interpreter0->PreprocessorStackList, 0 ) ;
    List_Push ( _Context_->Interpreter0->PreprocessorStackList, 0 ) ;
    _Interpret_Preprocessor ( 1 ) ;
}

void
CfrTil_Elif_ConditionalInterpret ( )
{
    if ( List_Top ( _Context_->Interpreter0->PreprocessorStackList ) )
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
    //_dllist_PopValue ( _Context_->Interpreter0->PreprocessorStackList ) ;
    List_Pop ( _Context_->Interpreter0->PreprocessorStackList ) ;
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
    _Interpreter_DoWord ( _Context_->Interpreter0, ( Word* ) _DataStack_Pop ( ), - 1 ) ;
}

#if 0
void
CfrTil_InterpretALiteralToken ( )
{
    Word * word = _Interpreter_ObjectWord_New ( _Context_->Interpreter0, ( byte* ) _DataStack_Pop ( ), 1 ) ;
    _Interpreter_DoWord ( _Context_->Interpreter0, word, - 1 ) ;
}
#endif

void
CfrTil_TokenToWord ()
{
    byte * token = (byte*) _DataStack_Pop ( ) ;
    _DataStack_Push ( (int32) _Interpreter_TokenToWord ( _Context_->Interpreter0, token ) ) ;
}

void
CfrTil_InterpreterStop ( )
{
    SetState ( _Context_->Interpreter0, INTERPRETER_DONE, true ) ;
    SetState ( _Q_->OVT_CfrTil, CFRTIL_RUN, false ) ;
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
            List_Push ( interp->InterpList, word ) ;
        }
    } 
    return interp->InterpList ;
}

void
CfrTil_Interpret_ReadToList ( )
{
    dllist * interpList = _CfrTil_Interpret_ReadToList ( ) ;
    _DataStack_Push ( ( int32 ) interpList ) ;
}

void
CfrTil_Interpret_List ( )
{
    dllist * interpList = ( dllist* ) _DataStack_Pop ( ) ;
    List_Interpret ( interpList ) ;
}

