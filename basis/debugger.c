
#include "../includes/cfrtil.h"

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
    //debugger->CharacterTable [ 'U' ] = 9 ;
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
    //debugger->CharacterTable [ 'B' ] = 23 ;
    debugger->CharacterTable [ 'U' ] = 29 ;

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
    //debugger->CharacterFunctionTable [ 3 ] = Debugger_InterpretTokenWriteCode ;
    debugger->CharacterFunctionTable [ 0 ] = Debugger_Default ;
    debugger->CharacterFunctionTable [ 4 ] = Debugger_Dis ;
    debugger->CharacterFunctionTable [ 5 ] = Debugger_Info ;
    debugger->CharacterFunctionTable [ 6 ] = Debugger_DoMenu ;
    debugger->CharacterFunctionTable [ 7 ] = Debugger_Stack ;
    debugger->CharacterFunctionTable [ 8 ] = _Debugger_Verbosity ;
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
Debugger_C_StackPrint ( Debugger * debugger, int i )
{
    debugger->GetEIP ( ) ;
    for ( i = 0 ; i < 100 ; i ++ )
    {
        Printf ( ( byte* ) "\n%02d : 0x%08x", i, ( ( int32* ) debugger->DebugAddress ) [i] ) ;
    }
}

// remember : this stuff is used a little differently since 0.754.10x

void
_Debugger_Init ( Debugger * debugger, Word * word, byte * address )
{
    DebugColors ;
    Debugger_UdisInit ( debugger ) ;
    debugger->SaveDsp = Dsp ;
    debugger->SaveTOS = TOS ;
    debugger->Key = 0 ;
    debugger->State = DBG_MENU | DBG_INFO | DBG_PROMPT ;
    debugger->w_Word = word ;
    //_ByteArray_ReInit ( debugger->StepInstructionBA ) ; // debugger->StepInstructionBA = _ByteArray_AllocateNew ( 64, SESSION ) ;
    Stack_Init ( debugger->DebugStack ) ;

    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, true ) ;
    if ( address )
    {
        debugger->DebugAddress = address ;
    }
    else
    {
        // remember : _Q_->CfrTil->Debugger0->GetESP is called already thru _Compile_Debug
        if ( debugger->DebugESP )
        {
            debugger->DebugAddress = ( byte* ) debugger->DebugESP [0] ; // EIP
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
                debugger->DebugAddress = 0 ;
            }
        }
    }
    if ( ( ! debugger->DebugAddress ) && ( ! debugger->w_Word ) )
    {
        Debugger_NextToken ( debugger ) ;
        Debugger_FindUsing ( debugger ) ;
    }
    if ( debugger->w_Word ) debugger->Token = debugger->w_Word->Name ;
    else
    {
        debugger->w_Word = _Q_->OVT_Context->CurrentRunWord ;
        debugger->Token = _Q_->OVT_Context->CurrentRunWord->Name ;
    }
    debugger->OptimizedCodeAffected = 0 ;
    //debugger->SaveCpuState ( ) ;
}

Debugger *
_Debugger_New ( uint32 type )
{
    Debugger * debugger = ( Debugger * ) Mem_Allocate ( sizeof (Debugger ), type ) ;
    debugger->cs_CpuState = CpuState_New ( type ) ;
    debugger->StepInstructionBA = _ByteArray_AllocateNew ( 64, type ) ;
    debugger->DebugStack = Stack_New ( 256, type ) ;
    debugger->AddressAfterJmpCallStack = Stack_New ( 256, type ) ;
    Debugger_TableSetup ( debugger ) ;
    SetState ( debugger, DBG_ACTIVE, true ) ;
    Debugger_UdisInit ( debugger ) ;
    return debugger ;
}

void
_CfrTil_DebugInfo ( )
{
    Debugger_ShowInfo ( _Q_->OVT_CfrTil->Debugger0, ( byte* ) "\ninfo", 0 ) ;
}

void
CfrTil_DebugInfo ( )
{
    if ( _Q_->Verbosity )
    {
        _CfrTil_DebugInfo ( ) ;
        Debugger_Source ( _Q_->OVT_CfrTil->Debugger0 ) ;
    }
}

void
_CfrTil_Debug_AtAddress ( byte * address )
{
    if ( ! Debugger_GetState ( _Q_->OVT_CfrTil->Debugger0, DBG_ACTIVE ) )
    {
        _Debugger_Init ( _Q_->OVT_CfrTil->Debugger0, 0, address ) ;
    }
    else
    {
        _CfrTil_DebugContinue ( 1 ) ;
    }
}

void
_CfrTil_DebugContinue ( int autoFlagOff )
{
    if ( Debugger_GetState ( _Q_->OVT_CfrTil->Debugger0, DBG_AUTO_MODE ) )
    {
        if ( autoFlagOff ) SetState ( _Q_->OVT_CfrTil->Debugger0, DBG_AUTO_MODE, false ) ;
    }
}
