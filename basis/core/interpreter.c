
#include "../../includes/cfrtil.h"

#if 0

void
_InterpretString_InContext ( byte *str, )
{
    _Context_InterpretString ( _Q_->OVT_Context, str, state ) ;
}
#endif

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
        token = _Lexer_ReadToken ( interp->Lexer0, delimiters ) ;
        if ( ! token ) return 0 ;
        if ( String_Equal ( token, end1 ) )
        {
            _Interpret_CheckToken ( token ) ;
            return 1 ;
        }
        else if ( String_Equal ( token, end2 ) )
        {
            _Interpret_CheckToken ( token ) ;
            return 2 ;
        }
        else _Interpreter_InterpretAToken ( interp, token ) ;
    }
    return 0 ;
}

void
_Interpret_Until_Token ( Interpreter * interp, byte * end, byte * delimiters )
{
    byte * token ;
    byte buffer [128] ;
    while ( 1 )
    {
        token = _Lexer_ReadToken ( interp->Lexer0, delimiters ) ;
        if ( String_Equal ( token, end ) )
        {
            _Interpret_CheckToken ( token ) ;
            break ;
        }
        else
        {
            snprintf ( (char*) buffer, 128, "\n_Interpret_Until_Token : before interpret of %s", (char*) token ) ;
            d0 ( if ( IsDebugOn ) Compiler_ShowWordStack ( buffer ) ) ;
            _Interpreter_InterpretAToken ( interp, token ) ;
            snprintf ( (char*) buffer, 128, "\n_Interpret_Until_Token : after interpret of %s", (char*) token ) ;
            d0 ( if ( IsDebugOn ) Compiler_ShowWordStack ( buffer ) ) ;
        }
    }
}

void
_Interpret_PrefixFunction_Until_Token ( Interpreter * interp, Word * prefixFunction, byte * end, byte * delimiters )
{
    _Interpret_Until_Token ( interp, end, delimiters ) ;
    SetState ( _Q_->OVT_Context->Compiler0, PREFIX_ARG_PARSING, false ) ;
    SetState ( _Q_->OVT_Context->Compiler0, PREFIX_PARSING, true ) ;
    if ( prefixFunction ) _Interpret_MorphismWord_Default ( interp, prefixFunction ) ;
    SetState ( _Q_->OVT_Context->Compiler0, PREFIX_PARSING, false ) ;
}

void
_Interpret_PrefixFunction_Until_RParen ( Interpreter * interp, Word * prefixFunction )
{
    Word * word ;
    byte * token ;
    int32 svs_c_rhs ;
    prefixFunction->W_StartCharRlIndex = interp->Lexer0->TokenStart_ReadLineIndex ;
    while ( 1 )
    {
        token = Lexer_ReadToken ( interp->Lexer0 ) ; // skip the opening left paren
        if ( ! String_Equal ( token, "(" ) )
        {
            if ( word = Finder_Word_FindUsing ( interp->Finder0, token, 1 ) )
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
    d0 ( if ( IsDebugOn ) Compiler_ShowWordStack ( "\n_Interpret_PrefixFunction_Until_RParen" ) ) ;
    SetState ( _Q_->OVT_Context->Compiler0, PREFIX_ARG_PARSING, true ) ;
    _Interpret_PrefixFunction_Until_Token ( interp, prefixFunction, (byte*) ")", ( byte* ) " ,\n\r\t" ) ;
    SetState ( _Q_->OVT_Context->Compiler0, PREFIX_ARG_PARSING, false ) ;
    if ( GetState ( _Q_->OVT_Context, C_SYNTAX ) ) SetState ( _Q_->OVT_Context, C_RHS, svs_c_rhs ) ;
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
        //SetState ( interp->Lexer, LEXER_END_OF_LINE, false ) ;
        Interpreter_InterpretNextToken ( interp ) ;
        if ( GetState ( interp->Lexer0, LEXER_END_OF_LINE ) ) break ; // either the lexer with get a newline or the readLiner
        if ( ReadLine_PeekNextChar ( interp->ReadLiner0 ) == '\n' ) break ;
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
    Context * cntx = _Q_->OVT_Context ;
    byte * token ;
    int32 ifStack = 1, status ;
    int32 svcm = Compiling ;
    SetState ( cntx->Compiler0, COMPILE_MODE, false ) ;
    if ( ifFlag )
    {
        Finder_SetNamedQualifyingNamespace ( cntx->Interpreter0->Finder0, (byte*) "PreProcessor" ) ; // so we can properly deal with parenthesized values here
        _Interpret_ToEndOfLine ( cntx->Interpreter0 ) ;
        status = _DataStack_Pop ( ) ;
    }
    else status = 0 ;
    //nb : if condition is not true we skip interpreting with this block until "#else" 
    if ( ( ! ifFlag ) || ( ! status ) )
    {
        SetState ( cntx->Compiler0, COMPILE_MODE, svcm ) ;
        while ( 1 )
        {
            int inChar = ReadLine_PeekNextChar ( cntx->ReadLiner0 ) ;
            if ( ( inChar == - 1 ) || ( inChar == eof ) ) break ;

            if ( ( token = Lexer_ReadToken ( cntx->Lexer0 ) ) )
            {
                if ( String_Equal ( token, "//" ) ) CfrTil_CommentToEndOfLine ( ) ;
                else if ( String_Equal ( token, "/*" ) ) CfrTil_ParenthesisComment ( ) ;
                else if ( String_Equal ( token, "#" ) )
                {
                    if ( ( token = Lexer_ReadToken ( cntx->Lexer0 ) ) )
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
                        else if ( String_Equal ( token, "elif" ) )
                        {
                            if ( ifStack == 1 )
                            {
                                _CfrTil_AddTokenToHeadOfTokenList ( token ) ;
                                break ;
                            }
                        }
                    }
                }
            }
        }
    }
    SetState ( cntx->Compiler0, COMPILE_MODE, svcm ) ;
}

void
Interpreter_Init ( Interpreter * interp )
{
    if ( _Q_->OVT_CfrTil->Debugger0 ) SetState ( _Q_->OVT_CfrTil->Debugger0, DBG_AUTO_MODE, false ) ;
    _Q_->OVT_Interpreter = _Q_->OVT_Context->Interpreter0 = interp ;
    //SetState ( interp, INTERPRETER_DONE, false ) ;
    interp->State = 0 ;
}

Interpreter *
Interpreter_New ( uint32 type )
{
    Interpreter * interp = ( Interpreter * ) Mem_Allocate ( sizeof (Interpreter ), type ) ;

    interp->Lexer0 = Lexer_New ( type ) ;
    interp->ReadLiner0 = interp->Lexer0->ReadLiner0 ;
    interp->Lexer0->OurInterpreter = interp ;
    interp->Finder0 = Finder_New ( type ) ;
    interp->Compiler0 = Compiler_New ( type ) ;

    Interpreter_Init ( interp ) ;
    return interp ;
}

void
_Interpreter_Copy ( Interpreter * interp, Interpreter * interp0 )
{
    memcpy ( interp, interp0, sizeof (Interpreter ) ) ;
}

Interpreter *
Interpreter_Copy ( Interpreter * interp0, uint32 type )
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
#if 0

void
Interpret_EvalWord_QualifiedID ( Interpreter * interp, Word * word )
{
    Context * cntx = _Q_->OVT_Context ;
    Finder * finder = interp->Finder ;
    interp->w_Word = word ;
    byte * token = interp->Token ;
    int32 done = 0 ;

    DEBUG_INIT ;
    while ( word )
    {
        DEBUG_PRE ;
        interp->w_Word = word ;
        if ( word->CType & NON_MORPHISM_TYPE )
        {
            word = Compiler_PushCheckAndCopyDuplicates ( interp->Compiler, word, interp->Compiler->WordStack ) ;
            word->WType |= WT_QID ;
            Interpreter_DataObject_Run ( word ) ;
            word->WType &= ~ WT_QID ;
            if ( done ) break ;
        }
        else
        {
            if ( token [0] == '.' ) _Interpreter_Do_MorphismWord ( interp, word ) ;
            else break ;
        }

        DEBUG_SHOW ;
        if ( word->CType & NAMESPACE_TYPE ) Finder_SetQualifyingNamespace ( finder, word ) ; // the previous, last and already interpreted word
        token = Lexer_ReadToken ( interp->Lexer ) ;
        interp->Token = token ;
        word = Finder_Word_FindUsing ( interp->Finder, token ) ; // ?? find after Literal - eliminate make strings or numbers words ??
        if ( ( token [0] == '.' ) && ( ! Lexer_IsTokenForwardDotted ( _Q_->OVT_Context->Lexer0 ) ) ) done = 1 ;
    }
    cntx->State &= ~ ( CONTEXT_LAST_WORD_IN_QID | CONTEXT_PARSING_QID ) ;
    DEBUG_SHOW ;
}
#endif

