
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
    debugger->CharacterTable [ 'V' ] = 8 ;
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
    debugger->CharacterFunctionTable [ 10 ] = Debugger_Registers ;
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
_Debugger_State ( Debugger * debugger )
{
    Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
    byte * buf = Buffer_Data ( buffer ) ;
    _CfrTil_GetSystemState_String0 ( buf ) ;
    Printf ( ( byte* ) buf ) ;
    Buffer_SetAsUnused ( buffer ) ;
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
_Debugger_Init ( Debugger * debugger, Word * word, byte * address )
{
    DebugColors ;
    Debugger_UdisInit ( debugger ) ;
    debugger->SaveDsp = Dsp ; //Edi = Dsp ;
    debugger->SaveTOS = TOS ;
    debugger->Key = 0 ;
    debugger->State = DBG_MENU | DBG_INFO | DBG_PROMPT | DBG_INTERPRET_LOOP_DONE ;
    debugger->w_Word = word ;

    DebugOn ; //SetState ( _CfrTil_, DEBUG_MODE, true ) ;
    if ( address )
    {
        debugger->DebugAddress = address ;
    }
    else
    {
        // remember : _Q_->CfrTil->Debugger0->GetESP is called thru _Compile_Debug : <dbg>
        if ( debugger->DebugESP ) //debugger->GetESP ( ) ;
        {
            debugger->DebugAddress = ( byte* ) debugger->DebugESP [0] ; // -1 is <dbg>
        }
        if ( debugger->DebugAddress )
        {
            debugger->w_Word = word = Word_GetFromCodeAddress ( debugger->DebugAddress ) ; //+ 1 + CELL + * ( int32* ) ( debugger->DebugAddress + 1 ) ) ;
            if ( ! word ) debugger->w_Word = word = Word_GetFromCodeAddress ( debugger->DebugAddress + 1 + CELL + * ( int32* ) ( debugger->DebugAddress + 1 ) ) ;
            if ( ! word )
            {
                Printf ( ( byte* ) "\n\nCan't find the Word, but here is some disassembly at the considered \"EIP address\" : \n" ) ;
                _Debugger_Disassemble ( debugger, debugger->DebugAddress, 16, 0 ) ;
                Debugger_NextToken ( debugger ) ;
                Debugger_FindUsing ( debugger ) ;
                debugger->DebugAddress = 0 ; // ?
            }
        }
    }
    if ( debugger->w_Word ) debugger->Token = debugger->w_Word->Name ;
    else
    {
        debugger->w_Word = _Context_->CurrentlyRunningWord ;
        if ( _Context_->CurrentlyRunningWord ) debugger->Token = _Context_->CurrentlyRunningWord->Name ;
    }
    debugger->OptimizedCodeAffected = 0 ;
    debugger->ReturnStackCopyPointer = 0 ;
    SetState ( debugger, (DBG_STACK_OLD), true ) ;
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
    debugger->cs_CpuState = CpuState_New ( type ) ;
    debugger->StepInstructionBA = Debugger_ByteArray_AllocateNew ( 2 * K, type ) ;
    debugger->DebugStack = Stack_New ( 256, type ) ;
    Debugger_TableSetup ( debugger ) ;
    SetState ( debugger, DBG_ACTIVE | DBG_INTERPRET_LOOP_DONE, true ) ;
    //debugger->WordList = List_New ( ) ;
    Debugger_UdisInit ( debugger ) ;
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
_Debugger_PreSetup ( Debugger * debugger, Word * word )
{
    if ( ! sigsetjmp ( debugger->JmpBuf0, 0 ) )
    {
        if ( Is_DebugOn && ( ! GetState ( _Debugger_, DBG_AUTO_MODE | DBG_STEPPING ) ) )
        {
            if ( ! word ) word = _Context_->CurrentlyRunningWord ;
            if ( word && ( ! word->W_OriginalWord ) ) word->W_OriginalWord = word ;
            debugger->w_Word = word ;
            if ( word && word->Name[0] && ( word != debugger->LastSetupWord ) )
            {
                if ( ! word->Name ) word->Name = ( byte* ) "" ;
                SetState ( debugger, DBG_COMPILE_MODE, CompileMode ) ;
                SetState_TrueFalse ( debugger, DBG_ACTIVE | DBG_INFO | DBG_PROMPT, DBG_INTERPRET_LOOP_DONE | DBG_PRE_DONE | DBG_CONTINUE | DBG_STEPPING | DBG_STEPPED ) ;
                debugger->TokenStart_ReadLineIndex = word->W_StartCharRlIndex ; //_Context_->Lexer0->TokenStart_ReadLineIndex ;
                debugger->SaveDsp = Dsp ;
                if ( ! debugger->StartHere ) debugger->StartHere = Here ;
                debugger->PreHere = Here ;
                debugger->WordDsp = Dsp ;
                debugger->SaveTOS = TOS ;
                debugger->Token = word->Name ;

                DebugColors ;
                if ( debugger->DebugESP )
                {
                    debugger->DebugAddress = ( byte* ) debugger->DebugESP [0] ;
                }
                else debugger->DebugAddress = ( byte* ) word->Definition ;

                _Debugger_InterpreterLoop ( debugger ) ; // core of this function

                debugger->DebugAddress = 0 ;
                DefaultColors ;

                debugger->OptimizedCodeAffected = 0 ;
                SetState ( debugger, DBG_MENU, false ) ;
                debugger->LastSetupWord = word ;
            }
            else debugger->LastSetupWord = 0 ;
        }
    }
}

void
_Debugger_PostShow ( Debugger * debugger )//, byte * token, Word * word )
{
    Debugger_ShowEffects ( debugger, 0 ) ;
    DefaultColors ;
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
        SetState ( debugger->w_Word, STEPPED, true ) ;
        debugger->cs_CpuState->State = 0 ;
        debugger->w_Word = 0 ;
        SetState ( debugger, ( DBG_DONE | DBG_STEPPING | DBG_STEPPED ), false ) ;
        DebugOff ;
        if ( GetState ( debugger, DBG_RUNTIME_BREAKPOINT ) )
        {
            SetState ( debugger, DBG_RUNTIME_BREAKPOINT, false ) ;
            //siglongjmp ( _Context_->JmpBuf0, 1 ) ; //in Word_Run
        }
        //else siglongjmp ( debugger->JmpBuf0, 1 ) ; // in _Debugger_PreSetup
    }
}

