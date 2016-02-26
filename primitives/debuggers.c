#include "../includes/cfrtil.h"

void
CfrTil_Debug_AtAddress ( )
{
    byte * address ;
    address = ( byte* ) _DataStack_Pop ( ) ;
    _CfrTil_Debug_AtAddress ( address ) ;
}

void
CfrTil_LocalsShow ( )
{
    Debugger_Locals_Show ( _Q_->OVT_CfrTil->Debugger0 ) ;
}

void
CfrTil_Debugger_Verbosity ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->OVT_CfrTil->DebuggerVerbosity ) ;
}

// put this '<dbg>' into cfrtil code for a runtime break into the debugger

void
CfrTil_DebugRuntimeBreakpoint ( )
{
    if ( ! CompileMode ) //Debugger_GetState ( debugger, DBG_ACTIVE ) )
    {
        Debugger * debugger = _Q_->OVT_CfrTil->Debugger0 ;
        // GetESP has been called by _Compile_Debug1 which calls this function
        _Debugger_Init ( debugger, 0, 0 ) ;
        SetState ( debugger, DBG_RUNTIME|DBG_BRK_INIT|DBG_RESTORE_REGS, true ) ;
        _Debugger_PreSetup ( debugger, debugger->Token, debugger->w_Word ) ;
    }
}

#if 0
// are we still using this? it seems wrong with ThrowIt, etc.
void
CfrTil_Debug ( )
{
    Debugger * debugger = _Q_->OVT_CfrTil->Debugger0 ;
    if ( GetState ( _Q_->OVT_CfrTil, DEBUG_MODE ) )
    {
        _Debugger_Init ( debugger, 0, 0 ) ;
    }
    else if ( ! Debugger_GetState ( debugger, DBG_ACTIVE ) )
    {
        //debugger->SaveCpuState ( ) ;
        _Debugger_Init ( debugger, 0, 0 ) ;
        if ( ! Debugger_GetState ( debugger, DBG_DONE | DBG_AUTO_MODE ) )
        {
            ThrowIt ( "CfrTil_Debug" ) ; // back to the _Word_Run try-catchAll
        }
    }
}
#endif
