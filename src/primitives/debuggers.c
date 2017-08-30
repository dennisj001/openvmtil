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
    debugger->DebugESP = 0 ; 
    Debugger_On ( debugger ) ;
    byte * nextToken = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0, 0 ) ;
    debugger->EntryWord = Finder_Word_FindUsing ( cntx->Interpreter0->Finder0, nextToken, 0 ) ;
    debugger->StartHere = Here ;
}

void
CfrTil_DebugOff ( )
{
    Debugger_Off ( _Debugger_, 1 ) ;
}

void
CfrTil_DebugRuntimeBreakpoint ( )
{
    Debugger * debugger = _Debugger_ ;
    if ( ! CompileMode )
    {
        if ( GetState ( debugger, DBG_INTERPRET_LOOP_DONE ) )//|| GetState ( debugger, DBG_CONTINUE_MODE|DBG_AUTO_MODE ) )
        {
            // GetESP and debugger->SaveCpuState ( ) has been called by _Compile_Debug1 which calls this function
            Debugger_On ( debugger ) ;
            SetState ( debugger, (DBG_BRK_INIT), true ) ; 
            debugger->StartHere = Here ;
            Debugger_SetupStepping ( debugger, 1 ) ;
            SetState_TrueFalse ( debugger, DBG_STEPPING | DBG_RUNTIME | DBG_BRK_INIT | DBG_RESTORE_REGS | DBG_ACTIVE | DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF,
                DBG_INTERPRET_LOOP_DONE | DBG_PRE_DONE | DBG_CONTINUE | DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_MENU ) ;
            //SetState ( debugger, DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, true ) ;
            _Debugger_InterpreterLoop ( debugger ) ;
            SetState ( debugger, DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, false ) ;
            // we just stepped this word and used it's arguments in the source code ; if we just return the interpreter will attempt to interpret the arguments
            Word * word = debugger->w_Word ;
            if ( ( ! word ) || GetState ( word, STEPPED ) )
            {
                _CfrTil_->SaveDsp = Dsp ;
                siglongjmp ( _Context_->JmpBuf0, 1 ) ; //in Word_Run
            }
        }
    }
}

