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
#if 0
void
CfrTil_DebugRuntimeBreakpoint ( )
{
    Debugger * debugger = _Debugger_ ;
    d1 ( CfrTil_PrintDataStack ( ) ) ;
    if ( ! CompileMode )
    {
        if ( ! GetState ( debugger, DBG_ACTIVE ) ) // ?? necessary/correct
        {
            // GetESP has been called by _Compile_Debug1 which calls this function
            Debugger_On ( debugger ) ;
            Lexer_ReadToken ( _Interpreter_->Lexer0 ) ; // clear the word we are in token because we didn't call that word normally
#if 1            
            byte * stopAddress = debugger->DebugAddress - 5 ;
            debugger->DebugAddress = ( byte* ) debugger->w_Word->Definition ;
            d1 ( Debugger_CheckSaveCpuStateShow ( debugger ) ) ;
            //Debugger_SaveCpuState ( debugger ) ;
            while ( debugger->DebugAddress != stopAddress )
            {
                Debugger_Step ( debugger ) ;
            }
            debugger->DebugAddress += 5 ; // skip over the <dbg> call
#endif            
            SetState ( debugger, DBG_NEWLINE | DBG_PROMPT | DBG_INFO, false ) ;
            SetState_TrueFalse ( debugger, DBG_RUNTIME | DBG_BRK_INIT | DBG_ACTIVE | DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, DBG_INTERPRET_LOOP_DONE ) ;
            if ( _Q_->Verbosity ) _Printf ( "\nCfrTil_DebugRuntimeBreakpoint : at %s", Context_Location ( ) ) ;
            Debugger_SetupStepping ( debugger, 1 ) ; // nb. after setting DBG_BRK_INIT true
            d0 ( if ( _Q_->Verbosity > 1 )
            {
                DebugColors ;
                Debugger_CheckSaveCpuStateShow ( debugger ) ;
                DefaultColors ;
            } ) ;
            _Debugger_InterpreterLoop ( debugger ) ;
            SetState ( debugger, DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, false ) ;
            Debugger_Off ( debugger, 0 ) ;
            // we just stepped this word and used it's arguments in the source code ; if we just return the interpreter will attempt to interpret the arguments
            Word * word = debugger->w_Word ;
            if ( ( ! word ) || GetState ( word, STEPPED ) )
            {
                siglongjmp ( _Context_->JmpBuf0, 1 ) ; //in Word_Run
            }
        }
        else
        {
            siglongjmp ( _Context_->JmpBuf0, 1 ) ;
        }
    }
}
#else
void
CfrTil_DebugRuntimeBreakpoint ( )
{
    Debugger * debugger = _Debugger_ ;
    if ( ! CompileMode )
    {
        if ( GetState ( debugger, DBG_INTERPRET_LOOP_DONE ) )
        {
            // GetESP and debugger->SaveCpuState ( ) has been called by _Compile_Debug1 which calls this function
            SetState ( debugger, (DBG_BRK_INIT), true ) ;
            Debugger_On ( debugger ) ;
            debugger->StartHere = Here ;
            Debugger_SetupStepping ( debugger, 1 ) ;
            SetState_TrueFalse ( debugger, DBG_STEPPING | DBG_RUNTIME | DBG_BRK_INIT | DBG_RESTORE_REGS | DBG_ACTIVE,
                DBG_INTERPRET_LOOP_DONE | DBG_PRE_DONE | DBG_CONTINUE | DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_MENU ) ;
            SetState ( debugger, DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, true ) ;
            d0 ( if ( _Q_->Verbosity > 1 )
            {
                DebugColors ;
                Debugger_CheckSaveCpuStateShow ( debugger ) ;
                DefaultColors ;
            } ) ;
            _Debugger_InterpreterLoop ( debugger ) ;
            SetState ( debugger, DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, false ) ;
            //Debugger_Off ( debugger, 0 ) ;
            // we just stepped this word and used it's arguments in the source code ; if we just return the interpreter will attempt to interpret the arguments
            Word * word = debugger->w_Word ;
            if ( ( ! word ) || GetState ( word, STEPPED ) )
            {
                siglongjmp ( _Context_->JmpBuf0, 1 ) ; //in Word_Run
            }
        }
    }
}

#endif

