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

#define PP_SKIP 0
#define PP_INTERPRET 1
#define PP_INTERP PP_INTERPRET

// #if
// "#if" stack pop is 'true' interpret until "#else" and this does nothing ; if stack pop 'false' skip to "#else" token skip those tokens and continue interpreting

/*
 * GetElseStatus () returns 1 (true) if else block should be executed else returns 0 (false)
 */
int32
GetOuterBlockStatus ( )
{
    int32 i, llen = List_Length ( _Context_->Interpreter0->PreprocessorStackList ), status ;
    if ( llen > 1 ) status = List_GetN ( _Context_->Interpreter0->PreprocessorStackList, 1 ) ;
    else return 1 ; // no outer block -> we should be interpreting
    if ( status && ( llen > 1 ) ) // a non existing list element will have 0 status
    {
        for ( i = 2 ; i < llen ; i ++ )
            status = status && List_GetN ( _Context_->Interpreter0->PreprocessorStackList, i ) ;
    }
    return status ;
}

int32
_GetCondStatus ( )
{
    Context * cntx = _Context_ ;
    int32 status ;
    int32 svcm = GetState ( cntx->Compiler0, COMPILE_MODE ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, false ) ;
    _Interpret_ToEndOfLine ( cntx->Interpreter0 ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, svcm ) ;
    status = _DataStack_Pop ( ) ;
    return status ;
}

int32
GetIfStatus ( )
{
    int32 status = _GetCondStatus ( ) ;
    status = status && GetOuterBlockStatus ( ) ;
    List_Push ( _Context_->Interpreter0->PreprocessorStackList, status ) ;
    return status ;
}

int32
GetElifStatus ( )
{
    int32 status = _GetCondStatus ( ), top = List_Top ( _Context_->Interpreter0->PreprocessorStackList ) ;
    if ( ! top )
    {
        status = status &&  GetOuterBlockStatus ( ) ;
        List_SetTop ( _Context_->Interpreter0->PreprocessorStackList, status ) ; 
    }
    return status ;
}

int32
GetEndifStatus ( )
{
    int32 status = GetOuterBlockStatus ( ) ;
    List_Pop ( _Context_->Interpreter0->PreprocessorStackList ) ;
    return status ;
}

int32
GetElseStatus ( )
{
    int32 top = List_Top ( _Context_->Interpreter0->PreprocessorStackList ), status ;
    if ( ! top )
    {
        status = GetOuterBlockStatus ( ) ;
    }
    else status = 0 ; // if top is true the 'else' block should not be interpreted
    List_SetTop ( _Context_->Interpreter0->PreprocessorStackList, status ) ;
    return status ;
}

void
SkipPreprocessorCode ( )
{
    Context * cntx = _Context_ ;
    byte * token ; //= ( byte* ) 1 ;
    do
    {
        int inChar = ReadLine_PeekNextChar ( cntx->ReadLiner0 ) ;
        if ( ( inChar == - 1 ) || ( inChar == eof ) )
        {
            SetState ( cntx->Lexer0, LEXER_END_OF_LINE, true ) ;
            return ;
        }
        token = Lexer_ReadToken ( cntx->Lexer0 ) ;
        if ( token )
        {
            if ( String_Equal ( token, "//" ) ) CfrTil_CommentToEndOfLine ( ) ;
            else if ( String_Equal ( token, "/*" ) ) CfrTil_ParenthesisComment ( ) ;
            else if ( String_Equal ( token, "#" ) )
            {
                byte * token1 = Lexer_ReadToken ( cntx->Lexer0 ) ;
                if ( token1 )
                {
                    //Finder_SetNamedQualifyingNamespace ( _Context_->Finder0, ( byte* ) "PreProcessor" ) ;
                    if ( String_Equal ( token1, "if" ) )
                    {
                        if ( GetIfStatus ( ) ) return ; // PP_INTERP
                    }
                    else if ( String_Equal ( token1, "else" ) )
                    {
                        if ( GetElseStatus ( ) ) return ;
                    }
                    else if ( String_Equal ( token1, "elif" ) )
                    {
                        if ( GetElifStatus ( ) ) return ;
                    }
                    else if ( String_Equal ( token1, "endif" ) )
                    {
                        if ( GetEndifStatus ( ) ) return ;
                    }
                    //else syntax error
                }
                else return ;
            }
        }
    }
    while ( token ) ;
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
    Finder_SetNamedQualifyingNamespace ( _Context_->Finder0, ( byte* ) "PreProcessor" ) ;
#if 1  
    _Interpret_ToEndOfLine ( _Context_->Interpreter0 ) ;
    if ( GetState ( _Context_->Interpreter0, PREPROCESSOR_DEFINE ) )
    {
        int32 locals = Stack_Depth ( _Context_->Compiler0->LocalNamespaces ) ;
        SetState ( _Context_->Interpreter0, PREPROCESSOR_DEFINE, false ) ;
        CfrTil_SemiColon ( ) ;
        CfrTil_Inline ( ) ;

        if ( locals ) CfrTil_Prefix ( ) ;
    }
#endif    
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
CfrTil_TokenToWord ( )
{

    byte * token = ( byte* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int32 ) _Interpreter_TokenToWord ( _Context_->Interpreter0, token ) ) ;
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

