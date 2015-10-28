
#include "../includes/cfrtil.h"

void
_InterpretString_InContext ( byte *str )
{
    _Context_InterpretString ( _Q_->OVT_Context, str ) ;
}

void
Interpreter_EvalQualifiedID ( Word * qid )
{
    _InterpretString_InContext ( qid->Name ) ;
}

void
_InterpretString ( byte *str )
{
    _CfrTil_ContextNew_InterpretString ( _Q_->OVT_CfrTil, str, SESSION ) ;
}

int32
_Interpret_Until_EitherToken ( Interpreter * interp, byte * end1, byte * end2, byte * delimiters )
{
    byte * token ;
    while ( 1 )
    {
        token = _Lexer_ReadToken ( interp->Lexer, delimiters ) ;
        if ( ! token ) return 0 ;
        if ( String_Equal ( token, end1 ) ) return 1 ;
        else if ( String_Equal ( token, end2 ) ) return 2 ;
        else _Interpreter_InterpretAToken ( interp, token ) ;
    }
    return 0 ;
}

void
_Interpret_Until_Token ( Interpreter * interp, byte * end, byte * delimiters )
{
    byte * token ;
    while ( 1 )
    {
        token = _Lexer_ReadToken ( interp->Lexer, delimiters ) ;
        if ( String_Equal ( token, end ) ) break ;
        else _Interpreter_InterpretAToken ( interp, token ) ;
    }
}

void
_Interpret_PrefixFunction_Until_Token ( Interpreter * interp, Word * prefixFunction, byte * end, byte * delimiters )
{
#if 0    
    byte * token ;
    while ( 1 )
    {
        token = _Lexer_ReadToken ( interp->Lexer, delimiters ) ;
        if ( String_Equal ( token, end ) ) break ;
        else _Interpreter_InterpretAToken ( interp, token ) ;
    }
#endif    
    _Interpret_Until_Token ( interp, end, delimiters ) ;
    if ( prefixFunction ) _Interpret_MorphismWord_Default ( interp, prefixFunction ) ;
}

void
_Interpret_PrefixFunction_Until_RParen ( Interpreter * interp, Word * prefixFunction )
{
    Word * word ;
    byte * token ;
    int32 svs_c_rhs ;
    while ( 1 )
    {
        token = Lexer_ReadToken ( interp->Lexer ) ; // skip the opening left paren
        if ( ! String_Equal ( token, "(" ) )
        {
            if ( word = Finder_Word_FindUsing ( interp->Finder, token ) )
            {
                if ( word->CType & DEBUG_WORD )
                {
                    continue ;
                }
            }
            Error ( "\nSyntax Error : Prefix function with no opening left parenthesis!\n", QUIT ) ;
        }
        else break ;
    }
    if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) )
    {
        svs_c_rhs = GetState ( _Q_->OVT_Context, C_RHS ) ;
        SetState ( _Q_->OVT_Context, C_LHS, false ) ;
        SetState ( _Q_->OVT_Context, C_RHS, true ) ;
    }
    _Interpret_PrefixFunction_Until_Token ( interp, prefixFunction, ")", ( byte* ) " ,\n\r\t" ) ;
    SetState ( _Q_->OVT_Context, C_RHS, svs_c_rhs ) ;
}

void
_Interpret_UntilFlagged ( Interpreter * interp, int32 doneFlags )
{
    while ( ( ! Interpreter_IsDone ( interp, doneFlags | INTERPRETER_DONE ) ) )
    {
        Interpreter_InterpretNextToken ( interp ) ;
    }
}

void
_Interpret_ToEndOfLine ( Interpreter * interp )
{
    while ( 1 )
    {
        Interpreter_InterpretNextToken ( interp ) ;
        if ( GetState ( interp->Lexer, END_OF_LINE ) ) break ; // either the lexer with get a newline or the readLiner
        if ( ReadLine_PeekNextChar ( interp->ReadLiner ) == '\n' ) break ;
    }
}

void
Interpret_UntilFlaggedWithInit ( Interpreter * interp, int32 doneFlags )
{
    Interpreter_Init ( interp ) ;
    _Interpret_UntilFlagged ( interp, doneFlags ) ;
}

// #if
// "#if" stack pop is 'true' interpret until "#else" and this does nothing ; if stack pop 'false' skip to "#else" token skip those tokens and continue interpreting

void
_CfrTil_ConditionalInterpret ( int32 ifFlag )
{
    char * token ;
    int32 ifStack = 1, status ;
    int32 svcm = Compiling ;
    SetState ( _Q_->OVT_Context->Compiler0, COMPILE_MODE, false ) ;
    if ( ifFlag )
    {
        Finder_SetNamedQualifyingNamespace ( _Q_->OVT_Context->Interpreter0->Finder, "PreProcessor" ) ; // so we can properly deal with parenthesized values here
        _Interpret_ToEndOfLine ( _Q_->OVT_Context->Interpreter0 ) ;
        status = _DataStack_Pop ( ) ;
    }
    else status = 0 ;
    //nb : if condition is not true we skip interpreting with this block until "#else" 
    if ( ( ! ifFlag ) || ( ! status ) )
    {
        SetState ( _Q_->OVT_Context->Compiler0, COMPILE_MODE, svcm ) ;
        while ( 1 )
        {
            int inChar = ReadLine_PeekNextChar ( _Q_->OVT_Context->ReadLiner0 ) ;
            if ( ( inChar == - 1 ) || ( inChar == eof ) ) break ;

            if ( ( token = ( char* ) Lexer_ReadToken ( _Q_->OVT_Context->Lexer0 ) ) )
            {
                if ( String_Equal ( token, "//" ) ) CfrTil_CommentToEndOfLine ( ) ;
                else if ( String_Equal ( token, "/*" ) ) CfrTil_ParenthesisComment ( ) ;
                else if ( String_Equal ( token, "#" ) )
                {
                    if ( ( token = ( char* ) Lexer_ReadToken ( _Q_->OVT_Context->Lexer0 ) ) )
                    {
                        if ( String_Equal ( token, "endif" ) )
                        {
                            if ( -- ifStack == 0 )
                            {
                                break ;
                            }
                        }
                        else if ( String_Equal ( token, "if" ) ) ifStack ++ ;
                        else if ( String_Equal ( token, "else" ) )
                        {
                            if ( ifStack == 1 )
                            {
                                break ;
                            }
                        }
                    }
                }
            }
        }
    }
    SetState ( _Q_->OVT_Context->Compiler0, COMPILE_MODE, svcm ) ;
}

void
Interpreter_Init ( Interpreter * interp )
{
    if ( _Q_->OVT_CfrTil->Debugger0 ) SetState ( _Q_->OVT_CfrTil->Debugger0, DBG_AUTO_MODE, false ) ;
    interp->State = 0 ;
    _Q_->OVT_Interpreter = _Q_->OVT_Context->Interpreter0 = interp ;
}

Interpreter *
Interpreter_New ( int32 type )
{
    Interpreter * interp = ( Interpreter * ) Mem_Allocate ( sizeof (Interpreter ), type ) ;

    interp->Lexer = Lexer_New ( type ) ;
    interp->ReadLiner = interp->Lexer->ReadLiner ;
    interp->Lexer->OurInterpreter = interp ;
    interp->Finder = Finder_New ( type ) ;
    interp->Compiler = Compiler_New ( type ) ;

    Interpreter_Init ( interp ) ;
    return interp ;
}

void
_Interpreter_Copy ( Interpreter * interp, Interpreter * interp0 )
{
    memcpy ( interp, interp0, sizeof (Interpreter ) ) ;
}

Interpreter *
Interpreter_Copy ( Interpreter * interp0, int32 type )
{
    Interpreter * interp = ( Interpreter * ) Mem_Allocate ( sizeof (Interpreter ), type ) ;
    _Interpreter_Copy ( interp, interp0 ) ;
    Interpreter_Init ( interp ) ;
    return interp ;
}

#if 0

void
Interpreter_Delete ( Interpreter * interp )
{
    Mem_FreeItem ( _Q_->PermanentMemList, ( byte* ) interp ) ;
}
#endif

int32
Interpreter_IsDone ( Interpreter * interp, int32 flags )
{
    return Interpreter_GetState ( interp, flags | INTERPRETER_DONE ) ;
}

