
#include "../includes/cfrtil.h"

byte *
_Context_Location ( Context * cntx )
{
    Buffer * b = Buffer_New ( BUFFER_SIZE ) ;
    byte * buffer = Buffer_Data ( b ), *str ;
    sprintf ( ( char* ) buffer, "%s : %d.%d", ( char* ) cntx->ReadLiner0->Filename ? ( char* ) cntx->ReadLiner0->Filename : "<command line>", cntx->ReadLiner0->LineNumber, cntx->Lexer0->CurrentReadIndex ) ;
    cntx->Location = str = String_New ( buffer, TEMPORARY ) ;
    Buffer_SetAsUnused ( b ) ;
    return str ;
}

byte *
Context_Location ( )
{
    return _Context_Location ( _Q_->OVT_Context ) ;
}

#if 0    
void
Context_Delete ( Context * context )
{
    //MemList_FreeExactType ( context->MemoryType ) ;
    //OVT_MemListFree_ContextMemory ( ) ; // done in _CfrTil_Init_Core
    // currently this memory is freed when it's memory allocType is freed
    Compiler_Delete ( context->Compiler0 ) ;
    Interpreter_Delete ( context->Interpreter0 ) ;
    Lexer_Delete ( context->Lexer0 ) ;
    ReadLine_Delete ( context->ReadLiner0 ) ;
    Finder_Delete ( context->Finder0 ) ;
    System_Delete ( context->System0 ) ;
    Mem_FreeItem ( &_Q_->PermanentMemList, ( byte* ) context ) ;
}
#endif    

Context *
_Context_New ( CfrTil * cfrTil, int32 allocType )
{
    if ( allocType != OPENVMTIL ) allocType = CONTEXT ;
    Context * context = ( Context* ) Mem_Allocate ( sizeof ( Context ), allocType ), *context0 = cfrTil->Context0 ;
    _Q_->OVT_Context = context ;
    if ( context0 && context0->System0 ) context->System0 = System_Copy ( context0->System0, allocType ) ; // nb : in this case System is copied -- DataStack is shared
    else context->System0 = System_New ( allocType ) ;
    context->ContextDataStack = cfrTil->DataStack ;
    context->Interpreter0 = Interpreter_New ( allocType ) ;
    context->Lexer0 = context->Interpreter0->Lexer ; //Lexer_New ( allocType ) ;
    context->ReadLiner0 = context->Interpreter0->ReadLiner ; //->OurReadLiner ;
    context->Lexer0->OurInterpreter = context->Interpreter0 ;
    context->Finder0 = context->Interpreter0->Finder ; //Finder_New ( allocType ) ;
    context->Compiler0 = context->Interpreter0->Compiler ; //Compiler_New ( allocType ) ;
    context->MemoryType = allocType ;
    return context ;
}

void
_Context_Run_1 ( Context * cntx, ContextFunction_1 contextFunction, byte * arg )
{
    contextFunction ( cntx, arg ) ;
}

void
_Context_Run_2 ( Context * cntx, ContextFunction_2 contextFunction, byte * arg, int32 arg2 )
{
    contextFunction ( cntx, arg, arg2 ) ;
}

void
_Context_Run ( Context * cntx, ContextFunction contextFunction )
{
    contextFunction ( cntx ) ;
}

Context *
CfrTil_Context_PushNew ( CfrTil * cfrTil, int32 allocType )
{
    Context * cntx ;
    _Stack_Push ( cfrTil->ContextStack, ( int32 ) cfrTil->Context0 ) ;
    //_Stack_Print ( cfrTil->ContextStack, "ContextStack" ) ;
    cntx = _Context_New ( cfrTil, allocType ) ;
    cfrTil->Context0 = _Q_->OVT_Context = cntx ;
    return cntx ;
}

void
CfrTil_Context_PopDelete ( CfrTil * cfrTil )
{
    Context * context = cfrTil->Context0 ;
    cfrTil->Context0 = ( Context* ) _Stack_Pop ( cfrTil->ContextStack ) ;
    //_Stack_Print ( cfrTil->ContextStack, "ContextStack" ) ;
    //Context_Delete ( context ) ;
    _Q_->OVT_Context = cfrTil->Context0 ;
}

void
_CfrTil_Contex_NewRun_1 ( CfrTil * cfrTil, ContextFunction_1 contextFunction, byte *arg, int32 allocType )
{
    Context * cntx = CfrTil_Context_PushNew ( cfrTil, allocType ) ;
    _Context_Run_1 ( cntx, contextFunction, arg ) ;
    CfrTil_Context_PopDelete ( cfrTil ) ; // this could be coming back from wherever so the stack variables are gone
}

void
_CfrTil_Contex_NewRun_2 ( CfrTil * cfrTil, ContextFunction_2 contextFunction, byte *arg, int32 arg2, int32 allocType )
{
    Context * cntx = CfrTil_Context_PushNew ( cfrTil, allocType ) ;
    _Context_Run_2 ( cntx, contextFunction, arg, arg2 ) ;
    CfrTil_Context_PopDelete ( cfrTil ) ; // this could be coming back from wherever so the stack variables are gone
}

void
_CfrTil_Contex_NewRun_Void ( CfrTil * cfrTil, Word * word, int32 allocType )
{
    CfrTil_Context_PushNew ( cfrTil, allocType ) ;
    if ( word ) word->Definition ( ) ;
    CfrTil_Context_PopDelete ( cfrTil ) ; // this could be coming back from wherever so the stack variables are gone
}

void
_Context_InterpretString ( Context * cntx, byte *str )
{
    Interpreter * interp = cntx->Interpreter0 ;
    ReadLiner * rl = cntx->ReadLiner0 ;
    _SetEcho ( 0 ) ;
    int32 interpState = interp->State ;
    int32 svIndex = rl->ReadIndex ;
    int32 svState = rl->State ;
    byte * svLine = rl->InputLine ;
    Readline_Setup_OneStringInterpret ( cntx->ReadLiner0, str ) ;
    Interpret_UntilFlaggedWithInit ( cntx->Interpreter0, END_OF_STRING ) ;
    rl->ReadIndex = svIndex ;
    rl->State = svState ;
    rl->InputLine = svLine ;
    interp->State = interpState ;
}

#if 0
void
Context_AdjustParsingQidLogic ( Context * cntx, byte * token )
{
    if ( token && ( token [0] != '.' ) && GetState ( cntx, CONTEXT_PARSING_QID ) && Lexer_NextNonDelimiterChar ( cntx->Lexer0 ) != '.' )
    {
        SetState ( cntx, CONTEXT_LAST_WORD_IN_QID, true ) ;
        if ( String_Equal ( token, cntx->Lexer0->OriginalToken ) )
            SetState ( cntx, CONTEXT_PARSING_QID, false ) ;
    }
}
#endif

void
_CfrTil_ContextNew_InterpretString ( CfrTil * cfrTil, byte * str, int32 allocType )
{
    if ( str ) _CfrTil_Contex_NewRun_1 ( cfrTil, _Context_InterpretString, str, allocType ) ;
}

void
_Context_InterpretFile ( Context * cntx )
{
    if ( Debugger_GetState ( _Q_->OVT_CfrTil->Debugger0, DBG_AUTO_MODE ) )
    {
        _CfrTil_DebugContinue ( 0 ) ;
    }
    else Interpret_UntilFlaggedWithInit ( cntx->Interpreter0, END_OF_FILE | END_OF_STRING ) ;
}

void
_Context_IncludeFile ( Context * cntx, byte *filename, int32 interpretFlag )
{
    if ( filename )
    {
        FILE * file = fopen ( ( char* ) filename, "r" ) ;
        if ( file )
        {
            ReadLiner * rl = cntx->ReadLiner0 ;
            rl->Filename = String_New ( filename, DICTIONARY ) ;
            if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\nincluding %s ...\n", filename ) ;
            cntx->ReadLiner0->InputFile = file ;
            ReadLine_SetRawInputFunction ( rl, ReadLine_GetNextCharFromString ) ;
            System_SetState ( cntx->System0, ADD_READLINE_TO_HISTORY, false ) ;
            cntx->System0->IncludeFileStackNumber ++ ;
            _SetEcho ( 0 ) ;

            ReadLine_SetInputString ( rl, _File_ReadToString_ ( file ) ) ;
            fclose ( file ) ;

            if ( interpretFlag ) Interpret_UntilFlaggedWithInit ( cntx->Interpreter0, END_OF_STRING ) ;

            cntx->System0->IncludeFileStackNumber -- ;
            if ( ! cntx->System0->IncludeFileStackNumber ) Ovt_AutoVarOff ( ) ;
            if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\n%s included\n", filename ) ;
        }
        else Printf ( ( byte* ) "\nError : _CfrTil_IncludeFile : \"%s\" : not found!\n", filename ) ;
    }
}

void
_CfrTil_ContextNew_IncludeFile ( byte * filename )
{
    _CfrTil_Contex_NewRun_2 ( _Q_->OVT_CfrTil, _Context_IncludeFile, filename, 1, 0 ) ;
}

int32
_Context_StrCmpNextToken ( Context * cntx, byte * check )
{
    byte *token = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0 ) ;
    return strcmp ( token, check ) ;
}

// this is funny!?

void
_Context_DoubleQuoteMacro ( Context * cntx )
{
    Lexer * lexer = cntx->Lexer0 ;
    byte ichar ;
    if ( ! GetState ( _Q_->OVT_CfrTil, SOURCE_CODE_INITIALIZED ) )
    {
        CfrTil_InitSourceCode_WithCurrentInputChar ( ) ; // must be here for wdiss and add addToHistory
    }
    do
    {
        ichar = lexer->TokenInputCharacter = ReadLine_NextChar ( cntx->ReadLiner0 ) ;
        if ( ichar == '\\' ) _BackSlash ( lexer, 0 ) ;
        else Lexer_Append_ConvertedCharacterToTokenBuffer ( lexer ) ;
    }
    while ( ichar != '"' ) ;
    Lexer_SetState ( lexer, LEXER_DONE, true ) ;
    if ( GetState ( _Q_->OVT_CfrTil, STRING_MACROS_ON ) && GetState ( &_Q_->OVT_CfrTil->Sti, STI_INITIALIZED ) )
    {
        _CfrTil_StringMacros_Do ( lexer->TokenBuffer ) ;
    }
    Lexer_Do_ObjectToken_New ( cntx->Lexer0, String_New ( lexer->TokenBuffer, TEMPORARY ), 1 ) ;
}

void
Context_DoubleQuoteMacro ( )
{
    _Context_DoubleQuoteMacro ( _Q_->OVT_Context ) ;
}

void
_Tick ( Context * cntx )
{
    Word * word ;
    byte * token = ( byte* ) _DataStack_Pop ( ) ;
    if ( token )
    {
        word = Finder_FindQualifiedIDWord ( cntx->Finder0, token ) ;
        if ( word )
        {
            _Push ( ( int32 ) word ) ;
            return ;
        }
#if 1  
        else
        {
            Lexer * lexer = cntx->Lexer0 ;
            Lexer_ParseObject ( lexer, token ) ; // create a string from a 'raw' token

            if ( Lexer_GetState ( lexer, KNOWN_OBJECT ) ) token = ( byte* ) lexer->Literal ;
        }
#endif        
    }
    _Push ( ( int32 ) token ) ;
}

void
MultipleEscape ( )
{
    _MultipleEscape ( _Q_->OVT_Context->Lexer0 ) ;
}

void
Context_Interpret ( Context * cntx )
{
    Interpret_UntilFlaggedWithInit ( cntx->Interpreter0, END_OF_LINE | END_OF_FILE | END_OF_STRING ) ;
}

