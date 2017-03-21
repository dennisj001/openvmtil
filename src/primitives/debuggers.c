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

#if 0
void
CfrTil_Debugger_Verbosity ( )
{
    _DataStack_Push ( ( int32 ) & _Debugger_->Verbosity ) ;
}
#endif
// put this '<dbg>' into cfrtil code for a runtime break into the debugger

void
CfrTil_DebugRuntimeBreakpoint ( )
{
    Debugger * debugger = _Debugger_ ;
    if ( ! CompileMode )
    {
        if ( GetState ( debugger, DBG_INTERPRET_LOOP_DONE ) )
        {
            // GetESP and debugger->SaveCpuState ( ) has been called by _Compile_Debug1 which calls this function
            _Debugger_Init ( debugger, 0, 0 ) ;
            SetState ( debugger, (DBG_BRK_INIT), true ) ;
            debugger->StartHere = Here ;
            Debugger_SetupStepping ( debugger, 1 ) ;
            SetState_TrueFalse ( debugger, DBG_STEPPING | DBG_RUNTIME | DBG_BRK_INIT | DBG_RESTORE_REGS | DBG_ACTIVE,
                DBG_INTERPRET_LOOP_DONE | DBG_PRE_DONE | DBG_CONTINUE | DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_MENU ) ;
            SetState ( debugger, DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, true ) ;
            if ( _Q_->Verbosity > 1 )
            {
                DebugColors ;
                //if ( _Q_->Verbosity > 2 ) 
                _Printf ( (byte*) "\nVerbosity == %d", _Q_->Verbosity ) ;
                Debugger_Registers ( debugger ) ;
                DefaultColors ;
            }
            _Debugger_InterpreterLoop ( debugger ) ;
            SetState ( _Debugger_, DBG_RUNTIME_BREAKPOINT | DEBUG_SHTL_OFF, false ) ;
            Word * word = debugger->w_Word ;
            debugger->w_Word = 0 ;
            if ( GetState ( word, STEPPED ) )
            {
                SetState ( word, STEPPED, false ) ;
                Debugger_DebugOff ( debugger ) ;
                siglongjmp ( _Context_->JmpBuf0, 1 ) ; //in Word_Run
            }
        }
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
    debugger->DebugESP = 0 ;
    _Debugger_Init ( debugger, 0, 0 ) ;
    SetState ( _CfrTil_, DEBUG_MODE, true ) ;
    byte * nextToken = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0 ) ;
    debugger->EntryWord = Finder_Word_FindUsing ( cntx->Interpreter0->Finder0, nextToken, 0 ) ;
    SetState ( _Debugger_, DBG_PRE_DONE | DBG_INTERPRET_LOOP_DONE, false ) ;
    debugger->StartHere = Here ;
    debugger->LastSetupWord = 0 ;
    DebugShow_On ;
}

void
CfrTil_DebugOff ( )
{
    SetState ( _CfrTil_, DEBUG_MODE, false ) ;
    DebugShow_On ;
}

void
CfrTil_SourceCodeBeginBlock ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, true ) ;
    if ( ! GetState ( _Context_, C_SYNTAX ) ) CfrTil_BeginBlock ( ) ;
}

void
CfrTil_SourceCodeEndBlock ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, false ) ;
    if ( ! GetState ( _Context_, C_SYNTAX ) ) CfrTil_EndBlock ( ) ;
}

void
CfrTil_SourceCode_Begin_C_Block ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, true ) ;
    Word * word = _Context_->Compiler0->CurrentWord ;
    //if ( ! word->DebugWordList ) word->DebugWordList = _dllist_New ( DICTIONARY ) ;
    if ( ! word->DebugWordList ) word->DebugWordList = _dllist_New ( TEMPORARY ) ;
}

void
CfrTil_SourceCode_End_C_Block ( )
{
    SetState ( _CfrTil_, DEBUG_SOURCE_CODE_MODE, false ) ;
    CfrTil_End_C_Block ( ) ;
}

