
#include "../includes/cfrtil.h"

byte *
GetStateString ( Debugger * debugger )
{
    byte * buffer = Buffer_Data ( _Q_->OVT_CfrTil->DebugB ) ;
    sprintf ( ( char* ) buffer, "%s : %s : %s",
        GetState ( debugger, DBG_STEPPING ) ? "Stepping" : ( CompileMode ? ( char* ) "Compiling" : ( char* ) "Interpreting" ),
        ( GetState ( _Q_->OVT_CfrTil, INLINE_ON ) ? ( char* ) "InlineOn" : ( char* ) "InlineOff" ),
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
        if ( ! ( word->CProperty & ( CPRIMITIVE | DLSYM_WORD ) ) ) SetState ( debugger, DBG_CAN_STEP, true ) ;
    }
}

void
Debugger_NextToken ( Debugger * debugger )
{
    //if ( ReadLine_IsThereNextChar ( _Context_->ReadLiner0 ) ) debugger->Token = Lexer_PeekNextNonDebugTokenWord ( _Context_->Lexer0  ) ; //Lexer_ReadToken ( _Context_->Lexer0 ) ;
    if ( ReadLine_IsThereNextChar ( _Context_->ReadLiner0 ) ) debugger->Token = Lexer_ReadToken ( _Context_->Lexer0 ) ;
    else debugger->Token = 0 ;
}

void
Debugger_CurrentToken ( Debugger * debugger )
{
    debugger->Token = _Context_->Lexer0->OriginalToken ;
}

void
Debugger_Parse ( Debugger * debugger )
{
    Lexer_ParseObject ( _Context_->Lexer0, _Context_->Lexer0->OriginalToken ) ;
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
    if ( debugger->w_Word ) Printf ( ( byte* ) ( byte* ) "\nFound Word :: %s.%s\n", _Context_->Finder0->FoundWordNamespace->Name, debugger->w_Word->Name ) ;
    else Printf ( ( byte* ) ( byte* ) "\nToken not found : %s\n", debugger->Token ) ;
}

void
Debugger_FindUsing ( Debugger * debugger )
{
    if ( debugger->Token )
    {
        debugger->w_Word = Finder_Word_FindUsing ( _Context_->Finder0, debugger->Token, 0 ) ;
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
    SetState ( debugger, DBG_INFO, true ) ;
}

void
Debugger_ReturnStack ( Debugger * debugger )
{
    _PrintNStack ( debugger->DebugESP, "Return Stack", "Esp (ESP)", 3 ) ;
    _Stack_PrintValues ( ( byte* ) "DebugStack ", debugger->DebugStack->StackPointer, Stack_Depth ( debugger->DebugStack ) ) ;
}

void
_Debugger_Verbosity ( Debugger * debugger )
{
    Printf ( ( byte* ) "\nDebuggerVerbosity = %d", debugger->Verbosity ) ;
}

void
Debugger_Source ( Debugger * debugger )
{
    _CfrTil_Source ( debugger->w_Word, 0 ) ;
}

void
_Debugger_Registers ( Debugger * debugger )
{
    //debugger->RestoreCpuState ( ) ;
    _CpuState_Show ( debugger->cs_CpuState ) ;
    Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r", ( byte* ) "\r" ) ; // current insn
}

void
Debugger_SaveCpuState ( Debugger * debugger )
{
    debugger->SaveCpuState ( ) ;
}

void
Debugger_Registers ( Debugger * debugger )
{
    if ( debugger->cs_CpuState->State ) _Debugger_Registers ( debugger ) ;
    else
    {
        Debugger_SaveCpuState ( debugger ) ;
        SetState ( debugger, DBG_REGS_SAVED, true ) ;
        _Debugger_Registers ( debugger ) ;
    }
}

void
Debugger_Continue ( Debugger * debugger )
{
    if ( GetState ( debugger, DBG_STEPPING ) && debugger->DebugAddress )
    {
        ByteArray * svcs = _Q_CodeByteArray ;
        _ByteArray_ReInit ( debugger->StepInstructionBA ) ; // we are only compiling one insn here so clear our BA before each use
        Set_CompilerSpace ( debugger->StepInstructionBA ) ;
        Compile_Call ( ( byte* ) debugger->RestoreCpuState ) ;
        _Compile_JumpToAddress ( ( byte* ) debugger->DebugAddress ) ;
        //_Compile_Return ( ) ;
        Set_CompilerSpace ( svcs ) ; // before "do it" in case "do it" calls the compiler
        ( ( VoidFunction ) debugger->StepInstructionBA->BA_Data ) ( ) ;
        SetState ( _Debugger_, DBG_STEPPED, true ) ;
    }
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE | _DEBUG_SHOW_, false ) ;
    SetState ( debugger, DBG_INTERPRET_LOOP_DONE, true ) ;
    SetState ( debugger, DBG_STEPPING, false ) ;
    Stack_Init ( debugger->DebugStack ) ;
    debugger->w_Word = 0 ;
    //debugger->StartWord = 0 ;
    debugger->StartHere = 0 ;
    debugger->DebugAddress = 0 ;
    DebugOff ;
}

void
Debugger_Quit ( Debugger * debugger )
{
#if 0    
    Debugger_Stepping_Off ( debugger ) ;
    SetState_TrueFalse ( _Debugger_, DBG_DONE, DBG_CONTINUE | DBG_ACTIVE ) ;
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE | _DEBUG_SHOW_, false ) ;
    SetState ( debugger, DBG_INTERPRET_LOOP_DONE, true ) ;
#endif    
    _Throw ( QUIT ) ;
}

void
Debugger_Abort ( Debugger * debugger )
{
    Debugger_Stepping_Off ( debugger ) ;
    SetState_TrueFalse ( _Debugger_, DBG_DONE | DBG_INTERPRET_LOOP_DONE, DBG_CONTINUE | DBG_ACTIVE ) ;
    _Throw ( ABORT ) ;
}

void
Debugger_Stop ( Debugger * debugger )
{
    Printf ( ( byte* ) "\nStop!\n" ) ;
    Debugger_Stepping_Off ( debugger ) ;
    SetState_TrueFalse ( _Debugger_, DBG_DONE, DBG_CONTINUE | DBG_ACTIVE ) ;
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
    Boolean saveSystemState = _Context_->System0->State ;
    Boolean saveDebuggerState = debugger->State ;
    SetState ( _Context_->System0, ADD_READLINE_TO_HISTORY, true ) ;
    SetState_TrueFalse ( debugger, DBG_COMMAND_LINE | DBG_ESCAPED, DBG_ACTIVE ) ;
    _Debugger_ = Debugger_Copy ( debugger, TEMPORARY ) ;
    DefaultColors ;
    DebugOff ;
    int32 svcm = Get_CompileMode ();
    Set_CompileMode ( false ) ;
    Debugger_InterpretLine ( ) ;
    Set_CompileMode ( svcm ) ;
    DebugOn ;
    DebugColors ;
    int32 verbosity = _Debugger_->Verbosity ; // allows us to change verbosity at the escape command line
    _Debugger_ = debugger ;
    debugger->Verbosity = verbosity ; // allows us to change verbosity at the escape command line
    SetState ( _Context_->System0, ADD_READLINE_TO_HISTORY, saveSystemState ) ; // reset state 
    debugger->State = saveDebuggerState ;
    _Context_->System0->State = saveSystemState ;
    SetState_TrueFalse ( debugger, DBG_ACTIVE | DBG_INFO | DBG_NEWLINE, DBG_COMMAND_LINE | DBG_ESCAPED ) ;
}

void
Debugger_AutoMode ( Debugger * debugger )
{
    if ( ! GetState ( debugger, DBG_AUTO_MODE ) )
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
            SetState ( debugger, DBG_AUTO_MODE, true ) ;
        }
        else Printf ( ( byte* ) "\nDebugger :: AutoMode : does not support repeating key :: \'%c\' ...", debugger->SaveKey ) ;
    }
    debugger->Key = debugger->SaveKey ;
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
    SetState ( debugger, DBG_INFO, true ) ;
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
        if ( debugger->w_Word && iflag ) Printf ( ( byte* ) " in word : %s.%s", c_ud ( debugger->w_Word->S_ContainingNamespace->Name ), c_dd ( debugger->w_Word->Name ) ) ;
        else
        {
            debugger->w_Word = word = Word_GetFromCodeAddress ( debugger->DebugAddress ) ;
            if ( word )
            {
                if ( sflag ) _Word_ShowSourceCode ( word ) ;
                Compiler_CopyDuplicates ( debugger->w_Word ) ; //since we're not calling the interpret for eval, setup the word 
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
    //Stack_Init ( debugger->DebugStack ) ;
    _Debugger_SetupStepping ( debugger, sflag, iflag ) ;
}

// simply : copy the current insn to a ByteArray buffer along with
// prefix and postfix instructions that restore and
// save the cpu state; then run that ByteArray code buffer

void
Debugger_Step ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    if ( ! GetState ( debugger, DBG_STEPPING ) )
    {
        if ( word )
        {
            if ( ( ( ! ( word->CProperty & CFRTIL_WORD ) ) && ( ! ( word->LProperty & T_LISP_DEFINE ) ) ) || ( CompileMode && ( ! ( word->CProperty & IMMEDIATE ) ) ) )
            {
                Debugger_Eval ( debugger ) ;
                return ;
            }
            debugger->WordDsp = Dsp ; // by 'eval' we stop debugger->Stepping and //continue thru this word as if we hadn't stepped
            debugger->PreHere = Here ;
            Debugger_CanWeStep ( debugger ) ;
            if ( ! GetState ( debugger, DBG_CAN_STEP ) )
            {
                Debugger_Eval ( debugger ) ;
                return ;
            }
            else
            {
                //Debugger_SaveCpuState ( debugger ) ;
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
            debugger->w_Word = Debugger_GetWordFromAddress ( debugger ) ;
            SetState_TrueFalse ( debugger, DBG_STEPPING | DBG_RESTORE_REGS, ( DBG_INFO | DBG_MENU | DBG_PROMPT ) ) ;
            debugger->SteppedWord = word ;
        }
        else
        {
            SetState_TrueFalse ( debugger, DBG_PRE_DONE | DBG_STEPPED | DBG_NEWLINE | DBG_PROMPT | DBG_INFO, DBG_AUTO_MODE | DBG_STEPPING | DBG_RESTORE_REGS ) ;
            if ( GetState ( debugger, DBG_DONE ) ) SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, false ) ;
            return ;
        }
    }
}

