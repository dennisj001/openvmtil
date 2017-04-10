
#include "../include/cfrtil.h"

void
Debugger_TableSetup ( Debugger * debugger )
{
    int32 i ;
    for ( i = 0 ; i < 128 ; i ++ ) debugger->CharacterTable [ i ] = 0 ;
    debugger->CharacterTable [ 0 ] = 0 ;
    debugger->CharacterTable [ 's' ] = 1 ;
    debugger->CharacterTable [ 'e' ] = 2 ;
    debugger->CharacterTable [ 'w' ] = 3 ;
    debugger->CharacterTable [ 'd' ] = 4 ;
    debugger->CharacterTable [ 'I' ] = 5 ;
    debugger->CharacterTable [ 'm' ] = 6 ;
    debugger->CharacterTable [ 't' ] = 7 ;
    //debugger->CharacterTable [ 'V' ] = 8 ;
    debugger->CharacterTable [ 'r' ] = 10 ;
    debugger->CharacterTable [ 'c' ] = 11 ;
    debugger->CharacterTable [ 'q' ] = 12 ;
    debugger->CharacterTable [ 'o' ] = 1 ;
    debugger->CharacterTable [ 'i' ] = 1 ;
    debugger->CharacterTable [ 'u' ] = 9 ;
    debugger->CharacterTable [ 'f' ] = 14 ;
    debugger->CharacterTable [ '\\'] = 15 ;
    debugger->CharacterTable [ 'G' ] = 16 ;
    debugger->CharacterTable [ 'n' ] = 17 ;
    debugger->CharacterTable [ 'p' ] = 18 ;
    debugger->CharacterTable [ 'h' ] = 19 ;
    debugger->CharacterTable [ 'a' ] = 20 ;
    debugger->CharacterTable [ 'z' ] = 21 ;
    debugger->CharacterTable [ 'w' ] = 22 ;
    debugger->CharacterTable [ 'B' ] = 23 ;
    debugger->CharacterTable [ 'P' ] = 24 ;
    debugger->CharacterTable [ 'l' ] = 25 ;
    debugger->CharacterTable [ 'v' ] = 26 ;
    debugger->CharacterTable [ 'S' ] = 27 ;
    debugger->CharacterTable [ 'A' ] = 28 ;
    debugger->CharacterTable [ 'U' ] = 29 ;
    debugger->CharacterTable [ 'R' ] = 30 ;
    debugger->CharacterTable [ '\n' ] = 15 ;
    debugger->CharacterTable [ 27 ] = 15 ;
    debugger->CharacterTable [ ' ' ] = 11 ;

    // debugger : system related
    debugger->CharacterFunctionTable [ 1 ] = Debugger_Step ;
    debugger->CharacterFunctionTable [ 2 ] = Debugger_Eval ;
    debugger->CharacterFunctionTable [ 21 ] = Debugger_AutoMode ;
    debugger->CharacterFunctionTable [ 11 ] = Debugger_Continue ;
    debugger->CharacterFunctionTable [ 12 ] = Debugger_Quit ;
    debugger->CharacterFunctionTable [ 13 ] = Debugger_Parse ;
    debugger->CharacterFunctionTable [ 14 ] = Debugger_FindAny ;
    debugger->CharacterFunctionTable [ 15 ] = Debugger_Escape ;

    // debugger internal
    debugger->CharacterFunctionTable [ 0 ] = Debugger_Default ;
    debugger->CharacterFunctionTable [ 4 ] = Debugger_Dis ;
    debugger->CharacterFunctionTable [ 5 ] = Debugger_Info ;
    debugger->CharacterFunctionTable [ 6 ] = Debugger_DoMenu ;
    debugger->CharacterFunctionTable [ 7 ] = Debugger_Stack ;
    //debugger->CharacterFunctionTable [ 8 ] = _Debugger_Verbosity ;
    debugger->CharacterFunctionTable [ 9 ] = Debugger_Source ;
    debugger->CharacterFunctionTable [ 10 ] = Debugger_State_CheckSaveShow_UdisOneInsn ;
    debugger->CharacterFunctionTable [ 16 ] = Debugger_OptimizeToggle ;
    debugger->CharacterFunctionTable [ 17 ] = Debugger_CodePointerUpdate ;
    debugger->CharacterFunctionTable [ 18 ] = Debugger_Dump ;
    debugger->CharacterFunctionTable [ 19 ] = Debugger_ConsiderAndShowWord ;
    debugger->CharacterFunctionTable [ 20 ] = Debugger_DisassembleAccumulated ;
    debugger->CharacterFunctionTable [ 22 ] = Debugger_WDis ;
    debugger->CharacterFunctionTable [ 23 ] = Debugger_Abort ;
    debugger->CharacterFunctionTable [ 24 ] = Debugger_Stop ;
    debugger->CharacterFunctionTable [ 25 ] = Debugger_Locals_Show ;
    debugger->CharacterFunctionTable [ 26 ] = Debugger_Variables ;
    debugger->CharacterFunctionTable [ 27 ] = _Debugger_State ;
    debugger->CharacterFunctionTable [ 28 ] = Debugger_DisassembleTotalAccumulated ;
    debugger->CharacterFunctionTable [ 29 ] = Debugger_Using ;
    debugger->CharacterFunctionTable [ 30 ] = Debugger_ReturnStack ;
}

void
_Debugger_InterpreterLoop ( Debugger * debugger )
{
    do
    {
        _Debugger_DoState ( debugger ) ;
        if ( ! GetState ( _Debugger_, DBG_AUTO_MODE | DBG_AUTO_MODE_ONCE ) )
        {
            debugger->Key = Key ( ) ;
            if ( debugger->Key != 'z' ) debugger->SaveKey = debugger->Key ;
        }
        SetState ( _Debugger_, DBG_AUTO_MODE_ONCE, false ) ;
        debugger->CharacterFunctionTable [ debugger->CharacterTable [ debugger->Key ] ] ( debugger ) ;
    }
    while ( GetState ( debugger, DBG_STEPPING ) || ( ! GetState ( debugger, DBG_INTERPRET_LOOP_DONE ) ) ) ;
    SetState ( debugger, DBG_STACK_OLD, true ) ;
    if ( GetState ( debugger, DBG_STEPPED ) )
    {
        if ( debugger->w_Word ) SetState ( debugger->w_Word, STEPPED, true ) ;
    }
    Debugger_Off ( debugger, 0 ) ;
}

void
_Debugger_PreSetup ( Debugger * debugger, Word * word )
{
    if ( Is_DebugOn )
    {
        if ( ! GetState ( debugger, DBG_AUTO_MODE | DBG_STEPPING ) )
        {
            if ( ! word ) word = _Context_->CurrentlyRunningWord ;
            if ( word && ( ! word->W_OriginalWord ) ) word->W_OriginalWord = word ;
            debugger->w_Word = word ;
            if ( word && word->Name[0] && ( word != debugger->LastSetupWord ) )
            {
                Debugger_DebugWordListLogic ( debugger ) ;
                if ( ! word->Name ) word->Name = ( byte* ) "" ;
                SetState ( debugger, DBG_COMPILE_MODE, CompileMode ) ;
                SetState_TrueFalse ( debugger, DBG_ACTIVE | DBG_INFO | DBG_PROMPT, DBG_INTERPRET_LOOP_DONE | DBG_PRE_DONE | DBG_CONTINUE | DBG_STEPPING | DBG_STEPPED ) ;
                debugger->TokenStart_ReadLineIndex = word->W_StartCharRlIndex ; 
                debugger->SaveDsp = Dsp ;
                if ( ! debugger->StartHere ) debugger->StartHere = Here ;

                debugger->WordDsp = Dsp ;
                debugger->SaveTOS = TOS ;
                debugger->Token = word->Name ;
                debugger->PreHere = Here ;
#if 0
                if ( debugger->DebugESP )
                {
                    debugger->DebugAddress = ( byte* ) debugger->DebugESP [0] ;
                }
                else debugger->DebugAddress = ( byte* ) word->Definition ;
#endif
                DebugColors ;
                _Debugger_InterpreterLoop ( debugger ) ; // core of this function
                DefaultColors ;

                debugger->DebugAddress = 0 ;

                debugger->OptimizedCodeAffected = 0 ;
                SetState ( debugger, DBG_MENU, false ) ;
                debugger->LastSetupWord = word ;
            }
        }
    }
}

void
_Debugger_PostShow ( Debugger * debugger )//, byte * token, Word * word )
{
    Debugger_ShowEffects ( debugger, 0 ) ;
    DefaultColors ;
}

byte *
Debugger_GetStateString ( Debugger * debugger )
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
        Debugger_Continue ( debugger ) ;
    }
    if ( ! debugger->PreHere ) debugger->PreHere = _Compiler_GetCodeSpaceHere ( ) ; // Here ;
    SetState_TrueFalse ( debugger, DBG_INTERPRET_LOOP_DONE, DBG_ACTIVE | DBG_STEPPING ) ;
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
    if ( GetState ( debugger, DBG_STEPPING ) && debugger->cs_Cpu->State )
    {
        Debugger_SyncStackPointersFromCpuState ( debugger ) ;
        _CfrTil_PrintDataStack ( ) ;
        _Printf ( ( byte* ) "\n" ) ;
        SetState ( debugger, DBG_INFO, true ) ;
    }
    else CfrTil_PrintDataStack ( ) ;
}

void
Debugger_ReturnStack ( Debugger * debugger )
{
    _CfrTil_PrintNReturnStack ( 4 ) ;
}

void
Debugger_Source ( Debugger * debugger )
{
    _CfrTil_Source ( debugger->w_Word ? debugger->w_Word : debugger->DebugWordListWord, 0 ) ;
    SetState ( debugger, DBG_INFO, true ) ;
}

void
_Debugger_CpuState_Show ( )
{
    _CpuState_Show ( _Debugger_->cs_Cpu ) ;
}

void
Debugger_CheckSaveCpuState ( Debugger * debugger )
{
    if ( ! ( debugger->cs_Cpu->State ) )
    {
        debugger->SaveCpuState ( ) ;
        SetState ( debugger, DBG_REGS_SAVED, true ) ;
    }
}

void
Debugger_CheckSaveCpuStateShow ( Debugger * debugger )
{
    Debugger_CheckSaveCpuState ( debugger ) ;
    _Debugger_CpuState_Show ( ) ;
}

void
Debugger_State_CheckSaveShow_UdisOneInsn ( Debugger * debugger )
{
    Debugger_CheckSaveCpuStateShow ( debugger ) ;
    Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r\r", ( byte* ) "" ) ; // current insn
}

void
Debugger_On ( Debugger * debugger )
{
    _Debugger_Init ( debugger, 0, 0 ) ;
    SetState_TrueFalse ( _Debugger_, DBG_MENU | DBG_INFO, DBG_PRE_DONE | DBG_INTERPRET_LOOP_DONE ) ;
    debugger->StartHere = Here ;
    debugger->LastSetupWord = 0 ;
    debugger->LastSourceCodeIndex = 0 ;
    DebugOn ;
    DebugShow_On ;
}

void
_Debugger_Off ( Debugger * debugger )
{
    Stack_Init ( debugger->DebugStack ) ;
    debugger->StartHere = 0 ;
    debugger->PreHere = 0 ;
    debugger->DebugAddress = 0 ;
    debugger->DebugWordListWord = 0 ;
    debugger->DebugWordList = 0 ;
    debugger->cs_Cpu->State = 0 ;
    SetState ( debugger, DBG_STACK_OLD, true ) ;
    debugger->ReturnStackCopyPointer = 0 ;
    SetState ( _Debugger_, DBG_BRK_INIT | DBG_ACTIVE | DBG_STEPPING | DBG_PRE_DONE | DBG_AUTO_MODE, false ) ;
    Debugger_SyncStackPointersFromCpuState ( debugger ) ;
}

void
Debugger_Off ( Debugger * debugger, int32 debugOffFlag )
{
    _Debugger_Off ( debugger ) ;
    if ( debugOffFlag ) DebugOff ;
}

void
Debugger_Continue ( Debugger * debugger )
{
    if ( GetState ( debugger, DBG_RUNTIME_BREAKPOINT ) || GetState ( debugger, DBG_STEPPING ) && debugger->DebugAddress )
    {
        // continue stepping thru
        do
        {
            Debugger_Step ( debugger ) ;
        }
        while ( debugger->DebugAddress ) ;
        SetState_TrueFalse ( debugger, DBG_STEPPED, DBG_STEPPING ) ;
    }
    Debugger_Off ( debugger, 1 ) ;
    SetState ( debugger, DBG_INTERPRET_LOOP_DONE, true ) ;
}

void
Debugger_Quit ( Debugger * debugger )
{
    _Printf ( ( byte* ) "\nDebugger_Quit.\n" ) ;
    Debugger_Off ( debugger, 1 ) ;
    _Throw ( QUIT ) ;
}

void
Debugger_Abort ( Debugger * debugger )
{
    _Printf ( ( byte* ) "\nDebugger_Abort.\n" ) ;
    Debugger_Off ( debugger, 1 ) ;
    _Throw ( ABORT ) ;
}

void
Debugger_Stop ( Debugger * debugger )
{
    _Printf ( ( byte* ) "\nDebugger_Stop.\n" ) ;
    Debugger_Off ( debugger, 1 ) ;
    _Throw ( STOP ) ;
}

void
Debugger_InterpretLine_WithStartString ( byte * str )
{
    _CfrTil_Contex_NewRun_1 ( _CfrTil_, ( ContextFunction_1 ) CfrTil_InterpretPromptedLine, str ) ; // can't clone cause we may be in a file and we want input from stdin
}

void
Debugger_InterpretLine ( )
{
    //_CfrTil_Contex_NewRun_1 ( _CfrTil_, ( ContextFunction_1 ) CfrTil_InterpretPromptedLine, 0 ) ; // can't clone cause we may be in a file and we want input from stdin
    Debugger_InterpretLine_WithStartString ( "" ) ;
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
        if ( ( debugger->SaveKey == 's' ) || ( debugger->SaveKey == 'o' ) || ( debugger->SaveKey == 'i' ) || ( debugger->SaveKey == 'e' ) || ( debugger->SaveKey == 'c' ) ) // not everything makes sense here
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
        _Printf ( ( byte* ) "\ndbg :> %c <: is not an assigned key code", debugger->Key ) ;
    }
    else
    {
        _Printf ( ( byte* ) "\ndbg :> <%d> <: is not an assigned key code", debugger->Key ) ;
    }
    //SetState ( debugger, DBG_MENU | DBG_PROMPT | DBG_NEWLINE, true ) ;
}

void
_Debugger_State ( Debugger * debugger )
{
    byte * buf = Buffer_Data ( _CfrTil_->DebugB2 ) ;
    _CfrTil_GetSystemState_String0 ( buf ) ;
    _Printf ( ( byte* ) buf ) ;
}

void
_Debugger_Copy ( Debugger * debugger, Debugger * debugger0 )
{
    memcpy ( debugger, debugger0, sizeof (Debugger ) ) ;
}

Debugger *
Debugger_Copy ( Debugger * debugger0, uint32 type )
{
    Debugger * debugger = ( Debugger * ) Mem_Allocate ( sizeof (Debugger ), type ) ;
    _Debugger_Copy ( debugger, debugger0 ) ;
    return debugger ;
}

void
Debugger_Delete ( Debugger * debugger )
{
    Mem_FreeItem ( &_Q_->PermanentMemList, ( byte* ) debugger ) ;
}

void
CpuState_AdjustEdi ( Cpu * cpu, uint32 * dsp, Word * word )
{
    if ( cpu->State )
    {
        if ( word ) dsp = word->W_InitialRuntimeDsp ;
        if ( dsp ) cpu->Esi = dsp ;
        if ( cpu->Esi )
        {
            cpu->Edi = cpu->Esi + 1 ;
            *( cpu->Edi ) = ( uint32 ) cpu->Esi ;
        }
    }
}

void
Debugger_AdjustEdi ( Debugger * debugger, uint32* dsp, Word * word )
{
    CpuState_AdjustEdi ( debugger->cs_Cpu, dsp, word ) ;
}

void
_Debugger_Init ( Debugger * debugger, Word * word, byte * address )
{
    DebugColors ;
    Debugger_UdisInit ( debugger ) ;
    debugger->SaveDsp = Dsp ; //Edi = Dsp ;
    debugger->SaveTOS = TOS ;
    debugger->Key = 0 ;
    debugger->State = DBG_MENU | DBG_INFO | DBG_PROMPT | DBG_INTERPRET_LOOP_DONE ;
    debugger->w_Word = word ;

    DebugOn ;
    if ( address )
    {
        debugger->DebugAddress = address ;
    }
    else
    {
        // remember : _Q_->CfrTil->Debugger0->GetESP is called thru _Compile_Debug : <dbg>
        if ( debugger->DebugESP ) //debugger->GetESP ( ) ;
        {
            //debugger->DebugAddress = ( byte* ) debugger->DebugESP [1] ; // 0 is <dbg>
            debugger->DebugAddress = ( byte* ) debugger->cs_Cpu->Esp [0] ; // 0 is <dbg>
        }
        if ( debugger->DebugAddress )
        {
            byte * da ;
            debugger->w_Word = word = Word_GetFromCodeAddress ( debugger->DebugAddress ) ;
            byte * offsetAddress = Calculate_Address_FromOffset_ForCallOrJump ( debugger->DebugAddress ) ;
            if ( ! word )
            {
                da = debugger->DebugAddress ;
                debugger->w_Word = word = Word_GetFromCodeAddress ( offsetAddress ) ;
            }
            if ( ! word )
            {
                AlertColors ;
                _CfrTil_PrintNReturnStack ( 8 ) ;
                debugger->w_Word = _Context_->CurrentlyRunningWord ;
                debugger->DebugAddress = debugger->w_Word->CodeStart ; //Definition ; //CodeAddress ;
                _Printf ( ( byte* ) "\n\n%s : Can't find a word at this address : 0x%08x : or it offset adress : 0x%08x :  "
                    "\nUsing _Context_->CurrentlyRunningWord : \'%s\' : address = 0x%08x : debugger->DebugESP [1] = 0x%08x",
                    Context_Location (), da, offsetAddress, debugger->w_Word->Name, debugger->DebugAddress, debugger->DebugESP ? debugger->DebugESP [1] : 0 ) ; //but here is some disassembly at the considered \"EIP address\" : \n" ) ;
                DebugColors ;
            }
            if ( _Q_->Verbosity > 3 ) _CfrTil_PrintNReturnStack ( 4 ) ;
        }
    }
    if ( debugger->w_Word ) debugger->Token = debugger->w_Word->Name ;
    else
    {
        debugger->w_Word = _Context_->CurrentlyRunningWord ;
        if ( _Context_->CurrentlyRunningWord ) debugger->Token = _Context_->CurrentlyRunningWord->Name ;
    }
    //Debugger_AdjustEdi ( debugger, Dsp ) ;
    Debugger_DebugWordListLogic ( debugger ) ;
    debugger->OptimizedCodeAffected = 0 ;
    debugger->ReturnStackCopyPointer = 0 ;
    SetState ( debugger, ( DBG_STACK_OLD ), true ) ;
    Stack_Init ( debugger->DebugStack ) ;
    debugger->CurrentlyRunningWord = _Context_->CurrentlyRunningWord ;
}

// nb! _Debugger_New needs this distinction for memory accounting 

ByteArray *
Debugger_ByteArray_AllocateNew ( int32 size, uint32 type )
{
    ByteArray * ba = ( ByteArray* ) Mem_Allocate ( size + sizeof ( ByteArray ), type ) ; // nb! _Debugger_New needs this distinction for memory accounting 
    ByteArray_Init ( ba, size, type ) ;
    return ba ;
}

Debugger *
_Debugger_New ( uint32 type )
{
    Debugger * debugger = ( Debugger * ) Mem_Allocate ( sizeof (Debugger ), type ) ;
    debugger->cs_Cpu = CpuState_New ( type ) ;
    debugger->StepInstructionBA = Debugger_ByteArray_AllocateNew ( 2 * K, type ) ;
    debugger->DebugStack = Stack_New ( 256, type ) ;
    Debugger_TableSetup ( debugger ) ;
    SetState ( debugger, DBG_ACTIVE | DBG_INTERPRET_LOOP_DONE, true ) ;
    //debugger->WordList = List_New ( ) ;
    Debugger_UdisInit ( debugger ) ;
    //int32 tw = GetTerminalWidth ( ) ;
    debugger->TerminalLineWidth =  120 ;// (tw > 145) ? tw : 145 ;
    return debugger ;
}

void
_CfrTil_DebugInfo ( )
{
    Debugger_ShowInfo ( _Debugger_, ( byte* ) "\ninfo", 0 ) ;
}

// nb! : not test for a while

void
_CfrTil_Debug_AtAddress ( byte * address )
{
    if ( ! GetState ( _Debugger_, DBG_ACTIVE ) )
    {
        _Debugger_Init ( _Debugger_, 0, address ) ;
    }
    else
    {
        _CfrTil_DebugContinue ( 1 ) ;
    }
}

void
_CfrTil_DebugContinue ( int autoFlagOff )
{
    if ( GetState ( _Debugger_, DBG_AUTO_MODE ) )
    {
        if ( autoFlagOff ) SetState ( _Debugger_, DBG_AUTO_MODE, false ) ;
    }
}

void
Debugger_DebugWordListLogic ( Debugger * debugger )
{
    if ( debugger->w_Word && debugger->w_Word->DebugWordList )
    {
        debugger->DebugWordListWord = debugger->w_Word ;
        debugger->DebugWordList = debugger->w_Word->DebugWordList ;
        _CfrTil_->DebugWordList = debugger->DebugWordList ;
    }
}


