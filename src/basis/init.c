#include "../include/cfrtil32.h"

// all except namespaces and number base
// this is called by the main interpreter _CfrTil_Interpret

void
_CfrTil_Init_SessionCore ( CfrTil * cfrTil, int32 cntxDelFlag, int32 promptFlag )
{
    int i ;
    CfrTil_LogOff ( ) ;
    CfrTil_SourceCodeOff () ;
    _System_Init ( _Context_->System0 ) ;
    ReadLine_Init ( _Context_->ReadLiner0, _CfrTil_Key ) ;
    Lexer_Init ( _Context_->Lexer0, 0, 0, CONTEXT ) ;
    Finder_Init ( _Context_->Finder0 ) ;
    Compiler_Init ( _Context_->Compiler0, 0 ) ;
    Interpreter_Init ( _Context_->Interpreter0 ) ;
    CfrTil_ClearTokenList ( ) ;
    if ( cntxDelFlag )
    {
        int stackDepth = Stack_Depth ( cfrTil->ContextStack ) ;
        for ( i = 0 ; i < stackDepth ; i ++ ) CfrTil_Context_PopDelete ( cfrTil ) ;
    }
    OVT_MemListFree_TempObjects ( ) ;
    OVT_MemListFree_CompilerTempObjects ( ) ;
    OVT_MemListFree_LispTemp ( ) ; // more careful allocation accounting work needs to be done before something like this can be done now
    CfrTil_CheckInitDataStack ( ) ;
    if ( ! _Q_->Verbosity ) _Q_->Verbosity = 1 ;
    _OVT_Ok ( promptFlag ) ;
    cfrTil->SC_QuoteMode = 0 ;
    SC_Global_Off ;
    cfrTil->DebugWordList = 0 ;

    SetState_TrueFalse ( cfrTil, CFRTIL_RUN, DEBUG_MODE ) ;
    SetState ( cfrTil->Debugger0, DBG_ACTIVE, false ) ;
    DebugOff ;
    SetBuffersUnused ( 1 ) ;
    d0 ( Buffer_PrintBuffers ( ) ) ;
}

void
CfrTil_ContextInit ( )
{
    _CfrTil_Init_SessionCore ( _CfrTil_, 0, 1 ) ;
}

void
CfrTil_ResetAll_Init ( CfrTil * cfrTil )
{
    byte * startDirectory = ( byte* ) "namespaces" ;
    if ( ! GetState ( _Q_, OVT_IN_USEFUL_DIRECTORY ) ) startDirectory = ( byte* ) "/usr/local/lib/cfrtil32/namespaces" ;
    _DataObject_New ( NAMESPACE_VARIABLE, 0, ( byte* ) "_startDirectory_", NAMESPACE_VARIABLE, 0, 0, ( int32 ) startDirectory, 0 ) ;
    if ( ( _Q_->RestartCondition >= RESET_ALL ) ) // || ( _Q_->StartIncludeTries == 1 ) )
    {
        _Q_->StartIncludeTries = 0 ;
        _CfrTil_Init_SessionCore ( cfrTil, 1, 0 ) ;
        if ( _Q_->StartupFilename )
        {
            _Q_->Verbosity = 0 ;
            _CfrTil_ContextNew_IncludeFile ( ( byte* ) "./namespaces/.sinit.cft" ) ;

            d0
                (
                _Q_->Verbosity = 2 ;
                _Printf ( ( byte* ) "\nIncluding Startup File : %s", _Q_->StartupFilename ) ; ;
                OpenVmTil_Pause ( ) ;
                _Q_->Verbosity = 0 ;
                ) ;

            _CfrTil_ContextNew_IncludeFile ( _Q_->StartupFilename ) ;
        }
        else
        {
            if ( ! _Q_->StartIncludeTries ++ )
            {
                _CfrTil_ContextNew_InterpretString ( cfrTil, _Q_->InitString ) ;
                _CfrTil_ContextNew_InterpretString ( cfrTil, _Q_->StartupString ) ;
            }
            else if ( _Q_->StartIncludeTries < 3 )
            {
                AlertColors ;
                _CfrTil_ContextNew_IncludeFile ( ( byte* ) "./namespaces/.init.cft" ) ;
                if ( _Q_->ErrorFilename )
                {
                    if ( strcmp ( ( char* ) _Q_->ErrorFilename, "Debug Context" ) )
                    {
                        _Printf ( ( byte* ) "\nError : \"%s\" include error!\n", _Q_->SigLocation ? _Q_->SigLocation : _Q_->ErrorFilename ) ;
                    }
                }
                DefaultColors ;
            }
        }
    }
    if ( _Q_->Verbosity > 2 )
    {
        _Printf ( ( byte* ) " \nInternal Namespaces have been initialized.  " ) ;
        OVT_ShowMemoryAllocated ( ) ;
    }
    if ( ( _Q_->InitSessionCoreTimes == 1 ) || ( ! _Q_->Verbosity ) ) _Q_->Verbosity = 1 ;
}

void
_CfrTil_InitialAddWordToNamespace ( Word * word, byte * containingNamespaceName, byte * superNamespaceName )
// this is only called at startup where we want to add the namespace to the RootNamespace
{
    Namespace *ns, *sns = _CfrTil_->Namespaces ;
    if ( superNamespaceName )
    {
        sns = Namespace_FindOrNew_SetUsing ( superNamespaceName, sns, 1 ) ;
        sns->State |= USING ;
    }
    ns = Namespace_FindOrNew_SetUsing ( containingNamespaceName, sns, 1 ) ; // find new namespace or create anew
    Namespace_DoAddWord ( ns, word ) ; // add word to new namespace
}

void
_CfrTil_CPrimitiveNewAdd ( const char * name, block b, uint64 ctype, uint64 ltype, const char *nameSpace, const char * superNamespace )
{
    Word * word = _Word_New ( ( byte* ) name, CPRIMITIVE | ctype, ltype, EXISTING ) ; //DICTIONARY ) ;
    _DObject_ValueDefinition_Init ( word, ( int32 ) b, BLOCK, 0, 0 ) ;
    _CfrTil_InitialAddWordToNamespace ( word, ( byte* ) nameSpace, ( byte* ) superNamespace ) ;
    if ( ctype & INFIXABLE ) word->WProperty = WT_INFIXABLE ;
    else if ( ctype & PREFIX ) word->WProperty = WT_PREFIX ;
    else if ( ctype & C_PREFIX_RTL_ARGS ) word->WProperty = WT_C_PREFIX_RTL_ARGS ;
    else word->WProperty = WT_POSTFIX ;
}

void
CfrTil_AddCPrimitives ( )
{
    int i ;
    for ( i = 0 ; CPrimitives [ i ].ccp_Name ; i ++ )
    {
        CPrimitive p = CPrimitives [ i ] ;
        _CfrTil_CPrimitiveNewAdd ( p.ccp_Name, p.blk_Definition, p.ui64_CProperty, p.ui64_LProperty, ( char* ) p.NameSpace, ( char* ) p.SuperNamespace ) ;
    }
}

void
CfrTil_MachineCodePrimitive_AddWords ( )
{
    int32 i, functionArg ;
    block * callHook ;
    Debugger * debugger = _Debugger_ ;
    for ( i = 0 ; MachineCodePrimitives [ i ].ccp_Name ; i ++ )
    {
        MachineCodePrimitive p = MachineCodePrimitives [ i ] ;

        // initialize some values in MachineCodePrimitives that are variables and have to be calculated at run time
#if 0        
        if ( String_Equal ( p.ccp_Name, "getESP" ) )
        {
            functionArg = - 1 ; //0 ; //( int ) debugger->DebugESP ;
            callHook = & debugger->GetESP ;
        }
        //else
#endif            
        if ( ( String_Equal ( p.ccp_Name, "restoreCpuState" ) ) && ( String_Equal ( p.NameSpace, "Debug" ) ) )
        {
            functionArg = ( int ) debugger->cs_Cpu ;
            callHook = & debugger->RestoreCpuState ;
        }
        else if ( ( String_Equal ( p.ccp_Name, "saveCpuState" ) ) && ( String_Equal ( p.NameSpace, "Debug" ) ) )
        {
            functionArg = ( int ) debugger->cs_Cpu ;
            callHook = & debugger->SaveCpuState ;
        }
        else if ( ( String_Equal ( p.ccp_Name, "restoreCpuState" ) ) && ( String_Equal ( p.NameSpace, "System" ) ) )
        {
            functionArg = ( int ) _CfrTil_->cs_Cpu ;
            callHook = & _CfrTil_->RestoreCpuState ;
        }
        else if ( ( String_Equal ( p.ccp_Name, "saveCpuState" ) ) && ( String_Equal ( p.NameSpace, "System" ) ) )
        {
            functionArg = ( int ) _CfrTil_->cs_Cpu ;
            callHook = & _CfrTil_->SaveCpuState ;
        }
#if 1       
        else if ( ( String_Equal ( p.ccp_Name, "syncDspToEsi" ) ) && ( String_Equal ( p.NameSpace, "System" ) ) )
        {
            callHook = & _CfrTil_->SyncDspToEsi ;
        }
        else if ( ( String_Equal ( p.ccp_Name, "syncEsiToDsp" ) ) && ( String_Equal ( p.NameSpace, "System" ) ) )
        {
            callHook = & _CfrTil_->SyncEsiToDsp ;
        }
#endif        
        else
        {
            functionArg = - 1 ;
            callHook = 0 ;
        }
        _CfrTil_MachineCodePrimitive_NewAdd ( p.ccp_Name, p.ui64_CProperty, callHook, p.Function, functionArg, p.NameSpace, p.SuperNamespace ) ;
    }
}



