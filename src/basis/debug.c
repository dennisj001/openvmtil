
#include "../include/cfrtil.h"

byte *
GetStateString ( Debugger * debugger )
{
    byte * buffer = Buffer_Data ( _CfrTil_->DebugB ) ;
    sprintf ( ( char* ) buffer, "%s : %s : %s",
        GetState ( debugger, DBG_STEPPING ) ? "Stepping" : ( CompileMode ? ( char* ) "Compiling" : ( char* ) "Interpreting" ),
        ( GetState ( _CfrTil_, INLINE_ON ) ? ( char* ) "InlineOn" : ( char* ) "InlineOff" ),
        ( GetState ( _CfrTil_, OPTIMIZE_ON ) ? ( char* ) "OptimizeOn" : ( char* ) "OptimizeOff" )
        ) ;
    buffer = String_New ( ( byte* ) buffer, TEMPORARY ) ;
    return buffer ;
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
    if ( debugger->w_Word ) _Printf ( ( byte* ) ( byte* ) "\nFound Word :: %s.%s\n", _Context_->Finder0->FoundWordNamespace->Name, debugger->w_Word->Name ) ;
    else _Printf ( ( byte* ) ( byte* ) "\nToken not found : %s\n", debugger->Token ) ;
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
    if ( ! debugger->PreHere ) debugger->PreHere = _Compiler_GetCodeSpaceHere ( ) ; // Here ;
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
    if ( GetState ( debugger, DBG_STEPPING ) && debugger->cs_CpuState->State )
    {
        _CfrTil_SetStackPointerFromDebuggerCpuState ( _CfrTil_ ) ;
        _Stack_Print ( _DataStack_, ( byte* ) "DataStack" ) ;
        _Printf ( ( byte* ) "\n" ) ;
        SetState ( debugger, DBG_INFO, true ) ;
    }
    else CfrTil_PrintDataStack ( ) ;
}

void
Debugger_ReturnStack ( Debugger * debugger )
{
    _CfrTil_PrintNReturnStack ( 8 ) ;
}

void
Debugger_Source ( Debugger * debugger )
{
    _CfrTil_Source ( debugger->w_Word, 0 ) ;
}

void
Debugger_CpuState_Show ( )
{
    _CpuState_Show ( _Debugger_->cs_CpuState ) ;
    _Printf ( ( byte* ) "\n\r" ) ;
}

void
_Debugger_Registers ( Debugger * debugger )
{
    Debugger_CpuState_Show ( ) ;
    Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r\r", ( byte* ) "" ) ; // current insn
}

void
Debugger_SaveCpuState ( Debugger * debugger )
{
    if ( ! ( debugger->cs_CpuState->State ) )
    {
        debugger->SaveCpuState ( ) ;
        SetState ( debugger, DBG_REGS_SAVED, true ) ;
    }
}

void
Debugger_Registers ( Debugger * debugger )
{
    if ( ! ( debugger->cs_CpuState->State ) ) Debugger_SaveCpuState ( debugger ) ;
    _Debugger_Registers ( debugger ) ;
}

void
Debugger_Continue ( Debugger * debugger )
{
    if ( GetState ( debugger, DBG_STEPPING ) && debugger->DebugAddress )
    {
        while ( * debugger->DebugAddress != _RET )
        {
            Debugger_StepOneInstruction ( debugger ) ;
        }
        SetState ( debugger, DBG_STEPPED, true ) ;
    }
    SetState ( debugger, DBG_INTERPRET_LOOP_DONE, true ) ;
    SetState ( debugger, DBG_STEPPING, false ) ;
    Stack_Init ( debugger->DebugStack ) ;
    debugger->StartHere = 0 ;
    debugger->PreHere = 0 ;
    debugger->DebugAddress = 0 ;
    debugger->SaveDsp = Dsp ;
    DebugOff ;
}

void
Debugger_Quit ( Debugger * debugger )
{
#if 1    
    Debugger_Stepping_Off ( debugger ) ;
    SetState_TrueFalse ( _Debugger_, DBG_DONE, DBG_CONTINUE | DBG_ACTIVE ) ;
    DebugOff ; //SetState ( _CfrTil_, DEBUG_MODE | _DEBUG_SHOW_, false ) ;
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
    _Printf ( ( byte* ) "\nStop!\n" ) ;
    Debugger_Stepping_Off ( debugger ) ;
    SetState_TrueFalse ( _Debugger_, DBG_DONE, DBG_CONTINUE | DBG_ACTIVE ) ;
    _CfrTil_->SaveDsp = Dsp ;
    _Throw ( STOP ) ;
}

void
Debugger_InterpretLine ( )
{
    _CfrTil_Contex_NewRun_1 ( _CfrTil_, ( ContextFunction_1 ) CfrTil_InterpretPromptedLine, 0 ) ; // can't clone cause we may be in a file and we want input from stdin
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
    int32 svcm = Get_CompileMode ( ) ;
    Set_CompileMode ( false ) ;

    _Printf ( "\n" ) ;
    Debugger_InterpretLine ( ) ;
    if ( _Context_->ReadLiner0->OutputLineCharacterNumber ) _Printf ( "\n" ) ;

    Set_CompileMode ( svcm ) ;
    DebugOn ;
    DebugColors ;
    _Debugger_ = debugger ;
    SetState ( _Context_->System0, ADD_READLINE_TO_HISTORY, saveSystemState ) ; // reset state 
    debugger->State = saveDebuggerState ;
    _Context_->System0->State = saveSystemState ;
    SetState_TrueFalse ( debugger, DBG_ACTIVE | DBG_INFO, DBG_COMMAND_LINE | DBG_ESCAPED ) ;
}

void
Debugger_AutoMode ( Debugger * debugger )
{
    if ( ! GetState ( debugger, DBG_AUTO_MODE ) )
    {
        if ( ( debugger->SaveKey == 's' ) || ( debugger->SaveKey == 'o' )  || ( debugger->SaveKey == 'i' ) || ( debugger->SaveKey == 'e' ) || ( debugger->SaveKey == 'c' ) ) // not everything makes sense here
        {
            AlertColors ;
            if ( debugger->SaveKey == 'c' )
            {
                _Printf ( ( byte* ) "\nContinuing : automatically repeating key \'e\' ..." ) ;
                debugger->SaveKey = 'e' ;
            }
            else _Printf ( ( byte* ) "\nDebugger :: Starting AutoMode : automatically repeating key :: \'%c\' ...", debugger->SaveKey ) ;
            DefaultColors ;
            SetState ( debugger, DBG_AUTO_MODE, true ) ;
        }
        else _Printf ( ( byte* ) "\nDebugger :: AutoMode : does not support repeating key :: \'%c\' ...", debugger->SaveKey ) ;
    }
    debugger->Key = debugger->SaveKey ;
}

void
Debugger_OptimizeToggle ( Debugger * debugger )
{
    if ( GetState ( _CfrTil_, OPTIMIZE_ON ) ) SetState ( _CfrTil_, OPTIMIZE_ON, false ) ;
    else SetState ( _CfrTil_, OPTIMIZE_ON, true ) ;
    _CfrTil_SystemState_Print ( 0 ) ;
}

void
Debugger_CodePointerUpdate ( Debugger * debugger )
{
    if ( debugger->w_Word && ( ! debugger->DebugAddress ) )
    {
        debugger->DebugAddress = ( byte* ) debugger->w_Word->Definition ;
        _Printf ( ( byte* ) "\ncodePointer = 0x%08x", ( int32 ) debugger->DebugAddress ) ;
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
        _Printf ( ( byte* ) "\rdbg :> %c <: is not an assigned key code", debugger->Key ) ;
    }
    else
    {
        _Printf ( ( byte* ) "\rdbg :> <%d> <: is not an assigned key code", debugger->Key ) ;
    }
    //SetState ( debugger, DBG_MENU | DBG_PROMPT | DBG_NEWLINE, true ) ;
}

void
Debugger_DebugOff ( Debugger * debugger )
{
    DebugOff ;
    debugger->DebugAddress = 0 ;
    debugger->PreHere = 0 ;
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

int32
Debugger_CanWeStep ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    if ( word && ( Compiling || ( word->CProperty & ( ALIAS | IMMEDIATE | CPRIMITIVE | DLSYM_WORD ) ) || ( word->LProperty & T_LISP_DEFINE ) ) ) //|| ( CompileMode && ( ! ( word->CProperty & IMMEDIATE ) ) ) )
    {
        SetState ( debugger, DBG_CAN_STEP, false ) ; // debugger->State flag = false ;
        return false ;
    }
    else
    {
        SetState ( debugger, DBG_CAN_STEP, true ) ;
        return true ;
    }
}

void
Debugger_SetupStepping ( Debugger * debugger, int32 sflag, int32 iflag )
{
    Word * word = debugger->w_Word ;
    _Printf ( ( byte* ) "\nSetting up stepping ..." ) ;
    if ( ( ! debugger->DebugAddress ) || (! GetState ( debugger, (DBG_BRK_INIT) ))) debugger->DebugAddress = ( byte* ) debugger->w_Word->Definition ;
    SetState_TrueFalse ( debugger, DBG_STEPPING, DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_MENU ) ;
    if ( iflag ) Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\nNext stepping instruction", ( byte* ) "" ) ;
    debugger->SaveDsp = Dsp ; // saved before we start stepping
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
        debugger->cs_CpuState->State = 0 ;
        _CfrTil_->CurrentSCSPIndex = 0 ;
        if ( word )
        {
            debugger->WordDsp = Dsp ; // by 'eval' we stop debugger->Stepping and //continue thru this word as if we hadn't stepped
            Debugger_CanWeStep ( debugger ) ;
            if ( ! GetState ( debugger, DBG_CAN_STEP ) )
            {
                Debugger_Eval ( debugger ) ;
                if ( Compiling )
                {
                    _Printf ( "\nCompiling : Stepping turned off" ) ;
                }
                else
                {
                    _Printf ( "\nStepping turned off for this word : %s%s%s%s",
                        c_ud ( word->S_ContainingNamespace ? word->S_ContainingNamespace->Name : ( byte* ) "<literal> " ),
                        word->S_ContainingNamespace ? ( byte* ) "." : ( byte* ) "", c_du ( word->Name ), GetState ( debugger, DBG_AUTO_MODE ) ? " : automode turned off" : "" ) ;
                }
                return ;
            }
            else
            {
                Debugger_SetupStepping ( debugger, 1, 0 ) ;
                _Printf ( ( byte* ) "\nNext stepping instruction ..." ) ;
                Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ;
                SetState ( debugger, DBG_NEWLINE | DBG_PROMPT | DBG_INFO, false ) ;
            }
        }
        else SetState_TrueFalse ( debugger, DBG_NEWLINE, DBG_STEPPING | DBG_RESTORE_REGS ) ;
        return ;
    }
    else
    {
        Debugger_SaveCpuState ( debugger ) ;
        Debugger_StepOneInstruction ( debugger ) ;

        _CfrTil_SetStackPointerFromDebuggerCpuState ( _CfrTil_ ) ;

        if ( ( int32 ) debugger->DebugAddress ) // set by StepOneInstruction
        {
            debugger->w_Word = Debugger_GetWordFromAddress ( debugger ) ;
            SetState_TrueFalse ( debugger, DBG_STEPPING | DBG_RESTORE_REGS, ( DBG_INFO | DBG_MENU | DBG_PROMPT ) ) ;
            debugger->SteppedWord = word ;
        }
        else
        {
            SetState_TrueFalse ( debugger, DBG_PRE_DONE | DBG_STEPPED | DBG_NEWLINE | DBG_PROMPT | DBG_INFO, DBG_STEPPING | DBG_RESTORE_REGS ) ;
            if ( GetState ( debugger, DBG_DONE ) ) SetState ( _CfrTil_, DEBUG_MODE, false ) ;
            return ;
        }
    }
}

