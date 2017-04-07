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
            if ( _Q_->Verbosity > 1 ) _Printf ( "\nCfrTil_DebugRuntimeBreakpoint : at %s", Context_Location ( ) ) ;
            // GetESP has been called by _Compile_Debug1 which calls this function
            Debugger_On ( debugger ) ;
            //Debugger_AdjustEdi ( debugger, _Context_->CurrentlyRunningWord->W_InitialRuntimeDsp ) ;

            SetState ( debugger, DBG_NEWLINE | DBG_PROMPT | DBG_INFO, false ) ;
            SetState_TrueFalse ( debugger, DBG_RUNTIME | DBG_BRK_INIT | DBG_ACTIVE | DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, DBG_INTERPRET_LOOP_DONE ) ;
            _Debugger_SetupStepping ( debugger, 1 ) ; // nb. after setting DBG_BRK_INIT true
            if ( _Q_->Verbosity > 1 )
            {
                DebugColors ;
                Debugger_CpuStateShow ( debugger ) ;
                DefaultColors ;
            }
            _Debugger_InterpreterLoop ( debugger ) ;
            SetState ( debugger, DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, false ) ;
            Debugger_Off ( debugger, 0 ) ;
            Word * word = debugger->w_Word ;
            // we just stepped this word and used it's arguments in the source code ; if we just return the interpreter will attempt to interpret the arguments
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
#else

void
CfrTil_DebugRuntimeBreakpoint ( )
{
    Debugger * debugger = _Debugger_ ;
    d1 ( CfrTil_PrintDataStack ( ) ) ;
    if ( ! CompileMode )
    {
        if ( ! GetState ( debugger, DBG_ACTIVE ) ) // ?? necessary/correct
        {
            if ( _Q_->Verbosity > 1 ) _Printf ( "\nCfrTil_DebugRuntimeBreakpoint : at %s", Context_Location ( ) ) ;
            // GetESP has been called by _Compile_Debug1 which calls this function
            Debugger_On ( debugger ) ;
            byte * stopAddress = debugger->DebugAddress - 10 ;
            debugger->DebugAddress = ( byte* ) debugger->w_Word->Definition ;
            d1 ( Debugger_CpuStateShow ( debugger ) ) ;
            //Debugger_AdjustEdi ( debugger, 0, _Context_->CurrentlyRunningWord ) ;
            //d0 ( CfrTil_PrintDataStack ( ) ) ;
#if 0            
            Debugger_SetupStepping ( debugger, 1 ) ; // nb. after setting DBG_BRK_INIT true
            debugger->cs_Cpu->State = 0 ;
            Debugger_SaveCpuState ( debugger ) ;
            Debugger_AdjustEdi ( debugger, 0, _Context_->CurrentlyRunningWord ) ;
            d1 ( CfrTil_PrintDataStack ( ) ) ;
#endif
            while ( debugger->DebugAddress <= stopAddress )
            {
                Debugger_Step ( debugger ) ;
            }
            debugger->DebugAddress += 5 ; // skip over the <dbg> call
            SetState ( debugger, DBG_NEWLINE | DBG_PROMPT | DBG_INFO, false ) ;
            SetState_TrueFalse ( debugger, DBG_RUNTIME | DBG_BRK_INIT | DBG_ACTIVE | DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, DBG_INTERPRET_LOOP_DONE ) ;
            Debugger_SetupStepping ( debugger, 1 ) ; // nb. after setting DBG_BRK_INIT true
            if ( _Q_->Verbosity > 1 )
            {
                DebugColors ;
                Debugger_CpuStateShow ( debugger ) ;
                DefaultColors ;
            }
            _Debugger_InterpreterLoop ( debugger ) ;
            SetState ( debugger, DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, false ) ;
            Debugger_Off ( debugger, 0 ) ;
            Word * word = debugger->w_Word ;
            // we just stepped this word and used it's arguments in the source code ; if we just return the interpreter will attempt to interpret the arguments
            //d1 ( Debugger_CpuStateShow ( debugger ) ) ;
            //d1 ( CfrTil_PrintDataStack ( ) ) ;
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
#endif

#if 0

void
CfrTil_Debugger_Verbosity ( )
{
    _DataStack_Push ( ( int32 ) & _Debugger_->Verbosity ) ;
}
#endif

