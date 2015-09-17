
#include "../includes/cfrtil.h"
#define VERSION ((byte*) "0.762.054" ) 

// the only major extern variable but there are two global structures in primitives.c
OpenVmTil * _Q_ ;

int
main ( int argc, char * argv [ ] )
{
    struct termios savedTerminalAttributes ;
    LinuxInit ( &savedTerminalAttributes ) ;
    _OpenVmTil ( argc, argv, &savedTerminalAttributes ) ;
}

void
_OpenVmTil ( int argc, char * argv [ ], struct termios * sta )
{
    OpenVmTil * ovt ;
    while ( 1 )
    {
        _Q_ = ovt = _OpenVmTil_New ( _Q_ ) ;
        ovt->Argc = argc ;
        ovt->Argv = argv ;
        ovt->SavedTerminalAttributes = sta ;
        if ( ! setjmp ( ovt->JmpBuf0 ) )
        {
            _OpenVmTil_Run ( ovt ) ;
        }
    }
}

void
_OpenVmTil_Run ( OpenVmTil * ovt )
{
    _CfrTil_Run ( ovt->OVT_CfrTil ) ;
}

DLList *
MemList_Init ()
{
    DLList * ml = (DLList *) _MemChunk_Allocate ( sizeof (DLList), OPENVMTIL ) ;
    DLNode * ml_head = (DLNode*) _MemChunk_Allocate ( sizeof (DLNode), OPENVMTIL ) ;
    DLNode * ml_tail = (DLNode *) _MemChunk_Allocate ( sizeof (DLNode), OPENVMTIL ) ;
    DLList_Init ( ml, ml_head, ml_tail ) ;
    return ml ;
}

OpenVmTil *
_OpenVmTil_Allocate ( OpenVmTil * ovt )
{
    DLList * ml  ;
    if ( ! ovt ) ml = MemList_Init () ;
    else ml = ovt->PermanentMemList ;
    OpenVmTil_Delete ( ovt ) ;
    _Q_ = ovt = ( OpenVmTil* ) MemList_AllocateChunk ( ml, sizeof ( OpenVmTil ), OPENVMTIL ) ;
    ovt->MmapMemoryAllocated = ( sizeof (MemChunk ) + sizeof (OpenVmTil ) ) ; // needed here because '_Q_' was not initialized yet for _MemChunk_CheckAndInit accounting
    ovt->PermanentMemList = ml ;
    return ovt ;
}

void
_OpenVmTil_Init ( OpenVmTil * ovt, int resetHistory )
{
    MemorySpace_New ( ovt ) ; // nb : memory must be after we set Size values and before lists; list are allocated from memory
    _HistorySpace_New ( ovt, resetHistory ) ;
    ovt->psi_PrintStateInfo = PrintStateInfo_New ( ) ; // variable init needed by any allocation which call Printf
    //ovt->ExceptionStack = Stack_New ( 1 * K, OPENVMTIL ) ;
    ovt->VersionString = VERSION ;
    // ? where do we want the init file ?
    if ( _File_Exists ( "./.init.cft" ) )
    {
        ovt->InitString = ( byte* ) "\"./.init.cft\" include" ; // could allow override with a startup parameter
        SetState ( ovt, OVT_IN_USEFUL_DIRECTORY, true ) ;
    }
    else 
    {
        ovt->InitString = ( byte* ) "\"/usr/local/lib/cfrTil/.init.cft\" include" ; // could allow override with a startup parameter
        SetState ( ovt, OVT_IN_USEFUL_DIRECTORY, false ) ;
    }
    if ( ovt->Verbosity > 1 )
    {
        Printf ( ( byte* ) "\nRestart : All memory freed, allocated and initialized as at startup. "
            "termios, verbosity and memory category allocation sizes preserved. verbosity = %d.", ovt->Verbosity ) ;
        OpenVmTil_Print_DataSizeofInfo ( 0 ) ;
    }
    _OpenVmTil_ColorsInit ( ovt ) ;
}

void
Ovt_RunInit ( OpenVmTil * ovt )
{
    ovt->SignalExceptionsHandled = 0 ;
    ovt->StartedTimes ++ ;
    ovt->RestartCondition = STOP ;
}

void
OpenVmTil_Delete ( OpenVmTil * ovt )
{
    if ( ovt )
    {
        if ( ovt->Verbosity > 2 ) Printf ( ( byte* ) "\nAll allocated memory is being freed.\nRestart : verbosity = %d.", ovt->Verbosity ) ;
        NBAsMemList_FreeVariousTypes ( - 1 ) ;
        FreeChunkList ( ovt->PermanentMemList ) ;
        //if ( ovt->OVT_CfrTil ) ovt->OVT_CfrTil = 0 ;
    }
    _Q_ = 0 ;
}

OpenVmTil *
_OpenVmTil_New ( OpenVmTil * ovt )
{
    char errorFilename [256] ;
    int32 fullRestart, restartCondition, startIncludeTries, verbosity, objectsSize, tempObjectsSize, codeSize, dictionarySize,
        sessionObjectsSize, dataStackSize, historySize, lispTempSize, compilerTempObjectsSize, exceptionsHandled, contextSize ; // inlining, optimize ;
    if ( ! ovt )
    {
        fullRestart = INITIAL_START ;
    }
    else fullRestart = ( ovt->RestartCondition == INITIAL_START ) ;

    ovt = _OpenVmTil_Allocate ( ovt ) ;

    startIncludeTries = ( ovt && ( ! fullRestart ) ) ? ovt->StartIncludeTries : 0 ;
    if ( startIncludeTries )
    {
        if ( _Q_->OVT_Context->ReadLiner0->bp_Filename ) strcpy ( errorFilename, ( char* ) _Q_->OVT_Context->ReadLiner0->bp_Filename ) ;
        else strcpy ( errorFilename, "Debug Context" ) ;
    }
    else errorFilename [ 0 ] = 0 ;
    restartCondition = ( ! fullRestart ) && ( startIncludeTries < 2 ) ? ovt->RestartCondition : RESTART ;

    if ( restartCondition != RESTART ) //< RESTART )
    {
        verbosity = ovt->Verbosity ;
        // preserve values across partial restarts
        sessionObjectsSize = ovt->SessionObjectsSize ;
        dictionarySize = ovt->DictionarySize ;
        lispTempSize = ovt->LispTempSize ;
        codeSize = ovt->MachineCodeSize ;
        objectsSize = ovt->ObjectsSize ;
        tempObjectsSize = ovt->TempObjectsSize ;
        compilerTempObjectsSize = ovt->CompilerTempObjectsSize ;
        dataStackSize = ovt->DataStackSize ;
        historySize = ovt->HistorySize ;
        contextSize = ovt->ContextSize ;
        exceptionsHandled = ovt->SignalExceptionsHandled ;
    }
    else // default values
    {
        verbosity = 1 ;
        objectsSize = OBJECTS_SIZE ;
        tempObjectsSize = TEMP_OBJECTS_SIZE ;
        sessionObjectsSize = SESSION_OBJECTS_SIZE ;
        codeSize = CODE_SIZE ;
        dictionarySize = DICTIONARY_SIZE ;
        dataStackSize = STACK_SIZE ;
        historySize = HISTORY_SIZE ;
        lispTempSize = LISP_TEMP_SIZE ;
        compilerTempObjectsSize = COMPILER_TEMP_OBJECTS_SIZE ;
        contextSize = CONTEXT_SIZE ;
        exceptionsHandled = 0 ;
    }

    ovt->RestartCondition = FULL_RESTART ; //restartCondition ;
    ovt->StartIncludeTries = startIncludeTries ;
    ovt->SignalExceptionsHandled = exceptionsHandled ;
    ovt->Verbosity = verbosity ;
    ovt->MachineCodeSize = codeSize ;
    ovt->DictionarySize = dictionarySize ;
    ovt->ObjectsSize = objectsSize ;
    ovt->TempObjectsSize = tempObjectsSize ;
    ovt->SessionObjectsSize = sessionObjectsSize ;
    ovt->DataStackSize = dataStackSize ;
    ovt->HistorySize = historySize ;
    ovt->LispTempSize = lispTempSize ;
    ovt->ContextSize = contextSize ;
    ovt->CompilerTempObjectsSize = compilerTempObjectsSize ;

    _Q_ = ovt ;
    _OpenVmTil_Init ( ovt, exceptionsHandled > 1 ) ; // try to keep history if we can
    if ( startIncludeTries ) ovt->ErrorFilename = String_New ( ( byte* ) errorFilename, DICTIONARY ) ;
    return ovt ;
}

void
OpenVmTil_Verbosity ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->Verbosity ) ;
}

void
Ovt_Optimize ( )
{
    _DataStack_Push ( ( int32 ) GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) ? 1 : 0 ) ;
}

void
Ovt_Inlining ( )
{
    _DataStack_Push ( ( int32 ) GetState ( _Q_->OVT_CfrTil, INLINE_ON ) ? 1 : 0 ) ;
}

// allows variables to be created on first use without a "var" declaration

void
Ovt_AutoVar ( )
{
    _DataStack_Push ( ( int32 ) GetState ( _Q_, AUTO_VAR ) ? 1 : 0 ) ;
}

void
Ovt_AutoVarOff ( )
{
    SetState ( _Q_, AUTO_VAR, false ) ;
}

// allows variables to be created on first use without a "var" declaration

void
Ovt_AutoVarOn ( )
{
    SetState ( _Q_, AUTO_VAR, true ) ;
}

void
OpenVmTil_HistorySize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->HistorySize ) ;
}

void
OpenVmTil_DataStackSize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->DataStackSize ) ;
}

void
OpenVmTil_CodeSize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->MachineCodeSize ) ;
}

void
OpenVmTil_SessionObjectsSize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->SessionObjectsSize ) ;
}

void
OpenVmTil_CompilerTempObjectsSize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->CompilerTempObjectsSize ) ;
}

void
OpenVmTil_ObjectsSize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->ObjectsSize ) ;
}

void
OpenVmTil_DictionarySize ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->DictionarySize ) ;
}

void
OpenVmTil_Print_DataSizeofInfo ( int flag )
{
    if ( flag || ( _Q_->Verbosity > 1 ) )
    {
        Printf ( ( byte* ) "\nOpenVimTil size : %d bytes, ", sizeof (OpenVmTil ) ) ;
        Printf ( ( byte* ) "Object size : %d bytes, ", sizeof (Object ) ) ;
        Printf ( ( byte* ) "Type size : %d bytes, ", sizeof (Type ) ) ;
        Printf ( ( byte* ) "\nCfrTil size : %d bytes, ", sizeof (CfrTil ) ) ;
        Printf ( ( byte* ) "Context size : %d bytes, ", sizeof (Context ) ) ;
        Printf ( ( byte* ) "System size : %d bytes, ", sizeof (System ) ) ;
        Printf ( ( byte* ) "Debugger size : %d bytes, ", sizeof (Debugger ) ) ;
        Printf ( ( byte* ) "\nMemorySpace size : %d bytes, ", sizeof (MemorySpace ) ) ;
        Printf ( ( byte* ) "ReadLiner size : %d bytes, ", sizeof (ReadLiner ) ) ;
        Printf ( ( byte* ) "Lexer size : %d bytes, ", sizeof (Lexer ) ) ;
        Printf ( ( byte* ) "Interpreter size : %d bytes, ", sizeof (Interpreter ) ) ;
        Printf ( ( byte* ) "\nFinder size : %d bytes, ", sizeof (Finder ) ) ;
        Printf ( ( byte* ) "Compiler size : %d bytes, ", sizeof (Compiler ) ) ;
        Printf ( ( byte* ) "Word size : %d bytes, ", sizeof (Word ) ) ;
        Printf ( ( byte* ) "Symbol size : %d bytes, ", sizeof (Symbol ) ) ;
        Printf ( ( byte* ) "\nDLNode size : %d bytes, ", sizeof (DLNode ) ) ;
        Printf ( ( byte* ) "DLList size : %d bytes, ", sizeof (DLList ) ) ;
        Printf ( ( byte* ) "WordData size : %d bytes", sizeof (WordData ) ) ;
        Printf ( ( byte* ) "ListObject size : %d bytes, ", sizeof ( ListObject ) ) ;
        Printf ( ( byte* ) "\nByteArray size : %d bytes, ", sizeof (ByteArray ) ) ;
        Printf ( ( byte* ) "NamedByteArray size : %d bytes", sizeof (NamedByteArray ) ) ;
    }
}

