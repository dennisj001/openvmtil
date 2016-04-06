
#include "../includes/cfrtil.h"

byte *
Debugger_GetStateString ( Debugger * debugger )
{
    byte * buffer = Buffer_Data ( _Q_->OVT_CfrTil->DebugB ) ;
    sprintf ( ( char* ) buffer, "%s : %s : %s",
        GetState ( debugger, DBG_STEPPING ) ? "Stepping" : ( CompileMode ? ( char* ) "Compiling" : ( char* ) "Interpreting" ),
        ( CfrTil_GetState ( _Q_->OVT_CfrTil, INLINE_ON ) ? ( char* ) "InlineOn" : ( char* ) "InlineOff" ),
        ( GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) ? ( char* ) "OptimizeOn" : ( char* ) "OptimizeOff" )
        ) ;
    buffer = String_New ( ( byte* ) buffer, TEMPORARY ) ;
    return buffer ;
}

void
Debugger_CanWeStep ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    SetState ( debugger, DBG_CAN_STEP, false ) ; // debugger->State flag = false ;
    if ( word ) // then it wasn't a literal
    {
        if ( ! ( word->CType & ( CPRIMITIVE | DLSYM_WORD ) ) ) SetState ( debugger, DBG_CAN_STEP, true ) ;
    }
}

void
Debugger_NextToken ( Debugger * debugger )
{
    if ( ReadLine_IsThereNextChar ( _Q_->OVT_Context->ReadLiner0 ) ) debugger->Token = Lexer_ReadToken ( _Q_->OVT_Context->Lexer0 ) ;
    else debugger->Token = 0 ;
}

void
Debugger_CurrentToken ( Debugger * debugger )
{
    debugger->Token = _Q_->OVT_Context->Lexer0->OriginalToken ;
}

void
Debugger_Parse ( Debugger * debugger )
{
    Lexer_ParseObject ( _Q_->OVT_Context->Lexer0, _Q_->OVT_Context->Lexer0->OriginalToken ) ;
}

void
_Debugger_FindAny ( Debugger * debugger )
{
    if ( debugger->Token ) debugger->w_Word = _CfrTil_FindInAnyNamespace ( debugger->Token ) ;
}

void
Debugger_FindAny ( Debugger * debugger )
{
    _Debugger_FindAny ( debugger ) ;
    if ( debugger->w_Word ) Printf ( ( byte* ) ( byte* ) "\nFound Word :: %s.%s\n", _Q_->OVT_Context->Finder0->FoundWordNamespace->Name, debugger->w_Word->Name ) ;
    else Printf ( ( byte* ) ( byte* ) "\nToken not found : %s\n", debugger->Token ) ;
}

void
Debugger_FindUsing ( Debugger * debugger )
{
    if ( debugger->Token )
    {
        debugger->w_Word = Finder_Word_FindUsing ( _Q_->OVT_Context->Finder0, debugger->Token, 0 ) ;
    }

}

void
Debugger_Variables ( Debugger * debugger )
{
    CfrTil_Variables ( ) ;
}

void
Debugger_Using ( Debugger * debugger )
{
    CfrTil_Using ( ) ;
}
// by 'eval' we stop debugger->Stepping and //continue thru this word as if we hadn't stepped

void
Debugger_Eval ( Debugger * debugger )
{
    debugger->SaveStackDepth = DataStack_Depth ( ) ;
    debugger->WordDsp = Dsp ;

    if ( Debugger_IsStepping ( debugger ) )
    {
        Debugger_Stepping_Off ( debugger ) ;
        Debugger_Info ( debugger ) ;
    }
    //SetState ( debugger, DBG_PRE_DONE, true ) ;
    SetState ( debugger, DBG_INTERPRET_LOOP_DONE, true ) ;
}

void
Debugger_SetupNextToken ( Debugger * debugger )
{
    Debugger_NextToken ( debugger ) ;
    Debugger_FindUsing ( debugger ) ;
}

void
Debugger_Info ( Debugger * debugger )
{
    SetState ( debugger, DBG_INFO, true ) ;
}

void
Debugger_DoMenu ( Debugger * debugger )
{
    SetState ( debugger, DBG_MENU | DBG_PROMPT | DBG_NEWLINE, true ) ;
}

void
Debugger_Stack ( Debugger * debugger )
{
    CfrTil_PrintDataStack ( ) ;
}

void
_Debugger_Verbosity ( Debugger * debugger )
{
    Printf ( ( byte* ) "\nDebuggerVerbosity = %d", _Q_->OVT_CfrTil->DebuggerVerbosity ) ;
}

void
Debugger_Source ( Debugger * debugger )
{
    _CfrTil_Source ( debugger->w_Word, 0 ) ;
}

void
Debugger_Registers ( Debugger * debugger )
{
    //if ( ! Debugger_IsStepping ( debugger ) )
    debugger->RestoreCpuState ( ) ;
    _CpuState_Show ( debugger->cs_CpuState ) ;
    Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r", ( byte* ) "\r" ) ; // current insn
}

void
Debugger_Continue ( Debugger * debugger )
{
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, false ) ;
    SetState ( debugger, DBG_INTERPRET_LOOP_DONE, true ) ;
    SetState ( debugger, DBG_STEPPING, false ) ;
    debugger->w_Word = 0 ;
    debugger->StartWord = 0 ;
    debugger->StartHere = 0 ;
}

void
Debugger_Quit ( Debugger * debugger )
{
    Debugger_Stepping_Off ( debugger ) ;
    SetState_TrueFalse ( _Q_->OVT_CfrTil->Debugger0, DBG_DONE, DBG_CONTINUE | DBG_ACTIVE ) ;
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, false ) ;
    SetState ( debugger, DBG_INTERPRET_LOOP_DONE, true ) ;
    _Throw ( QUIT ) ;
}

void
Debugger_Abort ( Debugger * debugger )
{
    Debugger_Stepping_Off ( debugger ) ;
    SetState_TrueFalse ( _Q_->OVT_CfrTil->Debugger0, DBG_DONE | DBG_INTERPRET_LOOP_DONE, DBG_CONTINUE | DBG_ACTIVE ) ;
    _Throw ( ABORT ) ;
}

void
Debugger_Stop ( Debugger * debugger )
{
    Printf ( ( byte* ) "\nStop!\n" ) ;
    Debugger_Stepping_Off ( debugger ) ;
    SetState_TrueFalse ( _Q_->OVT_CfrTil->Debugger0, DBG_DONE, DBG_CONTINUE | DBG_ACTIVE ) ;
    _Q_->OVT_CfrTil->SaveDsp = Dsp ;
    _Throw ( STOP ) ;
}

void
Debugger_InterpretLine ( )
{
    _CfrTil_Contex_NewRun_1 ( _Q_->OVT_CfrTil, ( ContextFunction_1 ) CfrTil_InterpretPromptedLine, 0, 0 ) ; // can't clone cause we may be in a file and we want input from stdin
    Buffer_Clear ( _Q_->OVT_CfrTil->InputLineB ) ;

}

void
Debugger_Escape ( Debugger * debugger )
{
    Boolean saveStateBoolean = System_GetState ( _Q_->OVT_Context->System0, ADD_READLINE_TO_HISTORY ) ;
    System_SetState ( _Q_->OVT_Context->System0, ADD_READLINE_TO_HISTORY, true ) ;
    SetState_TrueFalse ( debugger, DBG_COMMAND_LINE | DBG_ESCAPED, DBG_ACTIVE ) ;
    _Q_->OVT_CfrTil->Debugger0 = Debugger_Copy ( debugger, TEMPORARY ) ;
    DefaultColors ;
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, false ) ;
    Debugger_InterpretLine ( ) ;
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, true ) ;
    DebugColors ;
    _Q_->OVT_CfrTil->Debugger0 = debugger ;
    System_SetState ( _Q_->OVT_Context->System0, ADD_READLINE_TO_HISTORY, saveStateBoolean ) ; // reset state 
    SetState_TrueFalse ( debugger, DBG_ACTIVE | DBG_INFO | DBG_NEWLINE, DBG_COMMAND_LINE | DBG_ESCAPED ) ;
}

void
Debugger_AutoMode ( Debugger * debugger )
{
    if ( ( debugger->SaveKey == 's' ) || ( debugger->SaveKey == 'o' ) || ( debugger->SaveKey == 'e' ) || ( debugger->SaveKey == 'c' ) ) // not everything makes sense here
    {
        AlertColors ;
        if ( debugger->SaveKey == 'c' )
        {
            Printf ( ( byte* ) "\nContinuing : automatically repeating key \'e\' ..." ) ;
            debugger->SaveKey = 'e' ;
        }
        else Printf ( ( byte* ) "\nDebugger :: Starting AutoMode : automatically repeating key :: \'%c\' ...", debugger->SaveKey ) ;
        DefaultColors ;
        debugger->Key = debugger->SaveKey ;
        SetState ( debugger, DBG_AUTO_MODE, true ) ;
    }
}

void
Debugger_OptimizeToggle ( Debugger * debugger )
{
    if ( GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) ) SetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON, false ) ;
    else SetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON, true ) ;
    _CfrTil_SystemState_Print ( 0 ) ;
}

void
Debugger_CodePointerUpdate ( Debugger * debugger )
{
    if ( debugger->w_Word && ( ! debugger->DebugAddress ) )
    {
        debugger->DebugAddress = ( byte* ) debugger->w_Word->Definition ;
        Printf ( ( byte* ) "\ncodePointer = 0x%08x", ( int32 ) debugger->DebugAddress ) ;
    }
}

void
Debugger_Dump ( Debugger * debugger )
{
    if ( ! debugger->w_Word )
    {
        if ( debugger->DebugAddress ) __CfrTil_Dump ( ( int32 ) debugger->DebugAddress, ( int32 ) ( Here - ( int32 ) debugger->DebugAddress ), 8 ) ;
    }
    else __CfrTil_Dump ( ( int32 ) debugger->w_Word->CodeStart, ( int32 ) debugger->w_Word->S_CodeSize, 8 ) ;
}

void
Debugger_Default ( Debugger * debugger )
{
    if ( isgraph ( debugger->Key ) )
    {
        Printf ( ( byte* ) "\rdbg :> %c <: is not an assigned key code", debugger->Key ) ;
    }
    else
    {
        Printf ( ( byte* ) "\rdbg :> <%d> <: is not an assigned key code", debugger->Key ) ;
    }
    SetState ( debugger, DBG_MENU | DBG_PROMPT | DBG_NEWLINE, true ) ;
}

void
Debugger_Stepping_Off ( Debugger * debugger )
{
    if ( Debugger_IsStepping ( debugger ) )
    {
        //if ( debugger->WordDsp ) Dsp = debugger->WordDsp ; // by 'eval' we stop debugger stepping and //continue thru this word as if we hadn't stepped
        //if ( debugger->SaveDsp ) Dsp = debugger->SaveDsp ; // by 'eval' stops debugger->Stepping and //continues thru this word as if we hadn't stepped
        Debugger_SetStepping ( debugger, false ) ;
        debugger->DebugAddress = 0 ;
    }
}

void
_Debugger_SetupStepping ( Debugger * debugger, int32 sflag, int32 iflag )
{
    Word * word ;
    Printf ( ( byte* ) "\nSetting up stepping ..." ) ;
    if ( ! debugger->DebugAddress ) debugger->DebugAddress = ( byte* ) debugger->w_Word->Definition ;
    else
    {
        if ( ! debugger->w_Word )
        {
            debugger->w_Word = word = Word_GetFromCodeAddress ( debugger->DebugAddress ) ;
            if ( word )
            {
                if ( sflag ) _Word_ShowSourceCode ( word ) ;
                _Interpreter_SetupFor_MorphismWord ( _Q_->OVT_Context->Interpreter0, debugger->w_Word ) ; //since we're not calling the interpret for eval, setup the word 
            }
        }
    }
    if ( iflag ) Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\nNext stepping instruction\n", ( byte* ) "\r" ) ;
    SetState_TrueFalse ( debugger, DBG_STEPPING, DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_MENU ) ;
    debugger->SaveDsp = Dsp ; // saved before we start stepping
}

void
Debugger_SetupStepping ( Debugger * debugger, int32 sflag, int32 iflag )
{
    Stack_Init ( debugger->DebugStack ) ;
    _Debugger_SetupStepping ( debugger, sflag, iflag ) ;
}

// simply : copy the current insn to a ByteArray buffer along with
// prefix and postfix instructions that restore and
// save the cpu state; then run that ByteArray code buffer

void
Debugger_Step ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    if ( ! Debugger_IsStepping ( debugger ) )
    {
        if ( word )
        {
            if ( ! ( word->CType & CFRTIL_WORD ) && ! ( word->LType & T_LISP_DEFINE ) )
            {
                Debugger_Eval ( debugger ) ;
                return ;
            }
            debugger->WordDsp = Dsp ; // by 'eval' we stop debugger->Stepping and //continue thru this word as if we hadn't stepped
            debugger->PreHere = Here ;
            Debugger_CanWeStep ( debugger ) ;
            if ( ! Debugger_GetState ( debugger, DBG_CAN_STEP ) )
            {
                Debugger_Eval ( debugger ) ;
            }
            else
            {
                Debugger_SetupStepping ( debugger, 1, 0 ) ;
                Printf ( ( byte* ) "\nNext stepping instruction ...\n" ) ;
                Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ;
                SetState ( debugger, DBG_NEWLINE | DBG_PROMPT | DBG_INFO, false ) ;
            }
        }
        else SetState_TrueFalse ( debugger, DBG_NEWLINE, DBG_STEPPING | DBG_RESTORE_REGS ) ;
        return ;
    }
    else
    {
        Debugger_StepOneInstruction ( debugger ) ;
        if ( ( int32 ) debugger->DebugAddress ) // set by StepOneInstruction
        {
            Debugger_GetWordFromAddress ( debugger ) ;
            SetState_TrueFalse ( debugger, DBG_STEPPING | DBG_RESTORE_REGS, ( DBG_INFO | DBG_MENU | DBG_PROMPT ) ) ;
            debugger->SteppedWord = word ;
        }
        else
        {
            SetState_TrueFalse ( debugger, DBG_PRE_DONE | DBG_STEPPED | DBG_NEWLINE | DBG_PROMPT | DBG_INFO, DBG_AUTO_MODE | DBG_STEPPING | DBG_RESTORE_REGS ) ;
            SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, false ) ;
            return ;
        }
    }
}

void
_Debugger_DoNewline ( Debugger * debugger )
{
    Printf ( ( byte* ) "\n%s=> ", GetState ( debugger, DBG_RUNTIME ) ? ( byte* ) "<dbg>" : ( byte* ) "dbg" ) ; //, (char*) ReadLine_GetPrompt ( _Q_->OVT_Context->ReadLiner0 ) ) ;
    Debugger_SetNewLine ( debugger, false ) ;
}

void
_Debugger_DoState ( Debugger * debugger )
{
    if ( ! GetState ( debugger, DBG_STEPPING ) ) Printf ( ( byte* ) "\n" ) ;
    if ( GetState ( debugger, DBG_RETURN ) ) Printf ( ( byte* ) "\r" ) ;
    if ( GetState ( debugger, DBG_MENU ) ) Debugger_Menu ( ) ;
    Debugger_SetMenu ( debugger, false ) ;
    if ( GetState ( debugger, DBG_INFO ) ) Debugger_ShowInfo ( debugger, GetState ( debugger, DBG_RUNTIME ) ? ( byte* ) "<dbg>" : ( byte* ) "dbg", 0 ) ;
    else if ( GetState ( debugger, DBG_PROMPT ) ) Debugger_ShowState ( debugger, GetState ( debugger, DBG_RUNTIME ) ? ( byte* ) "<dbg>" : ( byte* ) "dbg" ) ;
    if ( GetState ( debugger, DBG_NEWLINE ) ) _Debugger_DoNewline ( debugger ) ;
#if 0    
    if ( GetState ( debugger, DBG_BRK_INIT ) )
    {
        //Printf ( "\n<dbrk> :: " UINT_FRMT_0x08 , debugger->DebugAddress ) ;
        Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\n", ( byte* ) "" ) ;
        //SetState ( debugger, DBG_BRK_INIT, false ) ;
    }
#endif    
}

void
_Debugger_PreSetup ( Debugger * debugger, byte * token, Word * word )
{
    if ( word && ( ! word->Name ) ) word->Name = ( byte* ) "" ;
    //SetState ( debugger, DBG_PRE_DONE, false ) ;
    if ( debugger->TokenStart_ReadLineIndex == _Q_->OVT_Context->Lexer0->TokenStart_ReadLineIndex ) return ;

    if ( GetState ( debugger, DBG_STEPPED ) && ( word && ( word == debugger->SteppedWord ) ) ) return ; // is this needed anymore ?!?
    SetState ( debugger, DBG_COMPILE_MODE, CompileMode ) ;
    DebugColors ;
    if ( ! GetState ( debugger, DBG_ACTIVE ) )
    {
        SetState_TrueFalse ( debugger, DBG_ACTIVE | DBG_INFO | DBG_NEWLINE | DBG_MENU | DBG_PROMPT, DBG_PRE_DONE | DBG_CONTINUE | DBG_STEPPING ) ;
    }
    SetState ( debugger, DBG_INTERPRET_LOOP_DONE, false ) ;
    debugger->TokenStart_ReadLineIndex = _Q_->OVT_Context->Lexer0->TokenStart_ReadLineIndex ;
    debugger->w_Word = word ;
    debugger->SaveDsp = Dsp ;
    if ( ! debugger->StartHere ) debugger->StartHere = Here ;
    if ( ! debugger->StartWord ) debugger->StartWord = word ;
    debugger->PreHere = Here ;
    debugger->WordDsp = Dsp ;
    debugger->SaveTOS = TOS ;
    if ( word ) debugger->Token = word->Name ;
    else
    {
        debugger->Token = token ;
    }
    _Debugger_InterpreterLoop ( debugger ) ;

    debugger->SteppedWord = 0 ;
    debugger->LastShowWord = 0 ;
    debugger->OptimizedCodeAffected = 0 ;
    debugger->LastToken = token ;
    DefaultColors ;
}

void
_Debugger_PostShow ( Debugger * debugger, byte * token, Word * word )
{
    if ( ( debugger->LastShowWord == word ) || ( token && debugger->LastShowWord && ( String_Equal ( token, debugger->LastShowWord->Name ) ) ) ) return ; // don't reshow result
    else
    {
        debugger->w_Word = word ;
        debugger->Token = token ;
        Debugger_ShowWrittenCode ( debugger, 0 ) ;
        debugger->LastShowWord = debugger->w_Word ;
        debugger->LastShowToken = token ;
    }
    DefaultColors ;
}

void
_Debugger_InterpreterLoop ( Debugger * debugger )
{
    do
    {
        _Debugger_DoState ( debugger ) ;
        if ( ! Debugger_GetState ( _Q_->OVT_CfrTil->Debugger0, DBG_AUTO_MODE ) )
        {
            debugger->Key = Key ( ) ;
            if ( debugger->Key != 'z' ) debugger->SaveKey = debugger->Key ;
        }
        debugger->CharacterFunctionTable [ debugger->CharacterTable [ debugger->Key ] ] ( debugger ) ;
    }
    while ( GetState ( debugger, DBG_STEPPING ) || ( ! GetState ( debugger, DBG_INTERPRET_LOOP_DONE ) ) ) ;
}

