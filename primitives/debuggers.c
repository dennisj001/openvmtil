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
    _DataStack_Push ( ( int32 ) & _Q_->OVT_CfrTil->Debugger0->Verbosity ) ;
}

// put this '<dbg>' into cfrtil code for a runtime break into the debugger

void
CfrTil_DebugRuntimeBreakpoint ( )
{
    Debugger * debugger = _Q_->OVT_CfrTil->Debugger0 ;
    if ( ! CompileMode )
    {
        // GetESP has been called by _Compile_Debug1 which calls this function
        SetState ( debugger, DBG_BRK_INIT, true ) ; // nb! : before _Debugger_Init because it must know this
        _Debugger_Init ( debugger, 0, 0 ) ;
        Debugger_SetupStepping ( debugger, 1, 1 ) ;
        SetState_TrueFalse ( debugger, DBG_STEPPING | DBG_RUNTIME | DBG_BRK_INIT | DBG_RESTORE_REGS | DBG_ACTIVE, 
            DBG_INTERPRET_LOOP_DONE | DBG_PRE_DONE | DBG_CONTINUE | DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_MENU ) ;
        _Debugger_InterpreterLoop ( debugger ) ;
        if ( ! GetState ( debugger, DBG_BRK_INIT ) )
        {
            SetState ( debugger, ( DBG_DONE | DBG_STEPPING ), false ) ;
            longjmp ( _Q_->OVT_Context->JmpBuf0, - 1 ) ;
        }
    }
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
CfrTil_DebugOn ( )
{
    Debugger * debugger = _Q_->OVT_CfrTil->Debugger0 ;
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, true ) ;
    SetState ( _Q_->OVT_CfrTil->Debugger0, DBG_PRE_DONE | DBG_INTERPRET_LOOP_DONE | DBG_AUTO_MODE, false ) ;
    debugger->StartHere = 0 ;
    SetState ( debugger, DBG_MENU, true ) ;
}

void
CfrTil_DebugOff ( )
{
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, false ) ;
}

