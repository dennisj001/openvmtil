#include "../include/cfrtil.h"

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

// put this '<dbg>' into cfrtil code for a runtime break into the debugger

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
    if ( _Q_->Verbosity > 1 ) _Printf ( "\nCfrTil_DebugOn : at %s", Context_Location ( ) ) ;
    Context * cntx = _Context_ ;
    Debugger * debugger = _Debugger_ ;
    debugger->DebugESP = 0 ; // before Debugger_On
    Debugger_On ( debugger ) ;
    byte * nextToken = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0 ) ;
    debugger->EntryWord = Finder_Word_FindUsing ( cntx->Interpreter0->Finder0, nextToken, 0 ) ;
    debugger->StartHere = Here ;
    SetState ( debugger, DBG_PRE_DONE | DBG_INTERPRET_LOOP_DONE | DBG_AUTO_MODE, false ) ;
    SetState ( debugger, DBG_MENU | DBG_INFO, true ) ;
}

void
CfrTil_DebugOff ( )
{
    _Debugger_Off ( _Debugger_ ) ;
    DebugOff ;
}

void
CfrTil_DebugRuntimeBreakpoint ( )
{
    //CpuState * cpu = _CpuState_Save () ;
    Debugger * debugger = _Debugger_ ;
    d1 ( CfrTil_PrintDataStack ( ) ) ;
    if ( ! CompileMode )
    {
        if ( ! GetState ( debugger, DBG_ACTIVE ) )
        {
            if ( _Q_->Verbosity > 1 ) _Printf ( "\nCfrTil_DebugRuntimeBreakpoint : at %s", Context_Location ( ) ) ;
            // GetESP has been called by _Compile_Debug1 which calls this function
            Debugger_On ( debugger ) ;
            SetState ( debugger, DBG_NEWLINE | DBG_PROMPT | DBG_INFO, false ) ;
            SetState_TrueFalse ( debugger, DBG_RUNTIME | DBG_BRK_INIT | DBG_ACTIVE | DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, DBG_INTERPRET_LOOP_DONE ) ;
            Debugger_SetupStepping ( debugger, 1 ) ; // nb. after setting DBG_BRK_INIT true
            if ( _Q_->Verbosity > 1 )
            {
                DebugColors ;
                Debugger_CpuStateShow ( debugger ) ;
                DefaultColors ;
            }
            //Debugger_DebugWordListLogic ( debugger ) ;
            _Debugger_InterpreterLoop ( debugger ) ;
            SetState ( debugger, DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, false ) ;
            Debugger_Off ( debugger, 0 ) ;
            Word * word = debugger->w_Word ;
            if ( ( ! word ) || GetState ( word, STEPPED ) )
            {
                siglongjmp ( _Context_->JmpBuf0, 1 ) ; //in Word_Run
            }
        }
        else if ( GetState ( debugger, DBG_ACTIVE | DBG_STEPPING ) )
        {
            siglongjmp ( debugger->JmpBuf0, 1 ) ;
        }
        else
        {
            siglongjmp ( _Context_->JmpBuf0, 1 ) ;
        }
    }
}

#if 0

void
CfrTil_Debugger_Verbosity ( )
{
    _DataStack_Push ( ( int32 ) & _Debugger_->Verbosity ) ;
}
#endif

