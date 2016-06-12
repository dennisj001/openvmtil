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
    Debugger_Locals_Show ( _Debugger_ ) ;
}

void
CfrTil_Debugger_Verbosity ( )
{
    _DataStack_Push ( ( int32 ) & _Debugger_->Verbosity ) ;
}

// put this '<dbg>' into cfrtil code for a runtime break into the debugger

void
CfrTil_DebugRuntimeBreakpoint ( )
{
    Debugger * debugger = _Debugger_ ;
    if ( ! CompileMode )
    {
        // GetESP and debugger->SaveCpuState ( ) has been called by _Compile_Debug1 which calls this function
        SetState ( debugger, DBG_BRK_INIT, true ) ; // nb! : before _Debugger_Init because it must know this
        _Debugger_Init ( debugger, 0, 0 ) ;
        Debugger_SetupStepping ( debugger, 1, 1 ) ;
        SetState_TrueFalse ( debugger, DBG_STEPPING | DBG_RUNTIME | DBG_BRK_INIT | DBG_RESTORE_REGS | DBG_ACTIVE,
            DBG_INTERPRET_LOOP_DONE | DBG_PRE_DONE | DBG_CONTINUE | DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_MENU ) ;
        SetState ( _Debugger_, DEBUG_SHTL_OFF, true ) ;
        if ( debugger->Verbosity > 1 )
        {
            DebugColors ;
            Printf ( "\ndbgVerbosity == %d\n\n", debugger->Verbosity ) ;
            Debugger_Registers ( debugger ) ;
            DefaultColors ;
        }
        _Debugger_InterpreterLoop ( debugger ) ;
    }
}

void
CfrTil_DebugInfo ( )
{
    if ( _Q_->Verbosity )
    {
        _CfrTil_DebugInfo ( ) ;
        Debugger_Source ( _Debugger_ ) ;
    }
}

void
CfrTil_DebugOn ( )
{
    Context * cntx = _Context_ ;
    Debugger * debugger = _Debugger_ ;
    _Debugger_Init ( debugger, 0, 0 ) ;
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, true ) ;
    byte * nextToken = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0 ) ;
    debugger->EntryWord = Finder_Word_FindUsing ( cntx->Interpreter0->Finder0, nextToken, 0 ) ;
    SetState ( _Debugger_, DBG_PRE_DONE | DBG_INTERPRET_LOOP_DONE | DBG_AUTO_MODE, false ) ;
    debugger->StartHere = 0 ;
    debugger->LastSetupWord = 0 ;
    SetState ( debugger, DBG_MENU, true ) ;
    DebugShow_ON ;
}

void
CfrTil_DebugOff ( )
{
    SetState ( _Q_->OVT_CfrTil, DEBUG_MODE, false ) ;
    DebugShow_ON ;
}

