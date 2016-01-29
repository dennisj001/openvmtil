#include "../includes/cfrtil.h"

// all except namespaces and number base
// this is called by the main interpreter _CfrTil_Interpret

void
_CfrTil_Init_SessionCore ( CfrTil * cfrTil, int32 cntxDelFlag, int32 promptFlag )
{
    int i ;
    CfrTil_LogOff ( ) ;
    _System_Init ( _Q_->OVT_Context->System0 ) ;
    ReadLine_Init ( _Q_->OVT_Context->ReadLiner0, _CfrTil_GetC, SESSION ) ;
    Lexer_Init ( _Q_->OVT_Context->Lexer0, 0, 0, SESSION ) ;
    Finder_Init ( _Q_->OVT_Context->Finder0 ) ;
    Compiler_Init ( _Q_->OVT_Context->Compiler0, 0 ) ;
    _DLList_Init ( _Q_->OVT_CfrTil->TokenList ) ;
    Interpreter_Init ( _Q_->OVT_Context->Interpreter0 ) ;
    Debugger_SetState ( cfrTil->Debugger0, DBG_ACTIVE, false ) ;
    if ( cntxDelFlag )
    {
        int stackDepth = Stack_Depth ( cfrTil->ContextStack ) ;
        for ( i = 0 ; i < stackDepth ; i ++ ) CfrTil_Context_PopDelete ( cfrTil ) ;
    }
    SetState_TrueFalse ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, PSI_PROMPT ) ;
    OVT_MemListFree_TempObjects ( ) ;
    OVT_MemListFree_CompilerTempObjects ( ) ;
    OVT_MemListFree_LispTemp ( ) ;
    OVT_MemListFree_ContextMemory ( ) ;
    Buffers_SetAsUnused ( ) ;
    //_Q_->OVT_LC = 0 ;
    Ovt_AutoVarOff ( ) ;
    SetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, true ) ;
    CfrTil_CheckInitDataStack ( ) ;
    _CfrTil_Ok ( promptFlag ) ;
    SetState_TrueFalse ( cfrTil, CFRTIL_RUN, DEBUG_MODE ) ;
    SetBuffersUnused ;
}

void
CfrTil_ContextInit ( )
{
    _CfrTil_Init_SessionCore ( _Q_->OVT_CfrTil, 0, 1 ) ;
}

void
CfrTil_ResetAll_Init ( CfrTil * cfrTil )
{
    byte * startDirectory = (byte*) "namespaces" ;
    if ( ! GetState ( _Q_, OVT_IN_USEFUL_DIRECTORY ) ) startDirectory = (byte*) "/usr/local/lib/cfrTil/namespaces" ;
    _CfrTil_Variable ( ( byte* ) "_startDirectory_", ( int32 ) startDirectory ) ;
    if ( ( _Q_->RestartCondition >= RESTART ) ) // || ( _Q_->StartIncludeTries == 1 ) )
    {
        _CfrTil_Init_SessionCore ( cfrTil, 1, 0 ) ;
        if ( ( ! _Q_->StartIncludeTries ) && ( _Q_->Argc > 1 ) )
        {
            // -s : a script file with "#! cfrTil -s" -- as first line includes the script file, the #! whole line is treated as a comment
            if ( String_Equal ( "-f", _Q_->Argv [1] ) ) _Q_->StartupFilename = ( byte* ) _Q_->Argv [ 2 ] ;
            else if ( String_Equal ( "-s", _Q_->Argv [1] ) )
            {
                _Q_->StartupFilename = ( byte* ) _Q_->Argv [ 2 ] ;
            }
            else if ( String_Equal ( "-e", _Q_->Argv [1] ) ) _Q_->StartupString = ( byte* ) _Q_->Argv [ 2 ] ;
        }
        else
        {
            _Q_->StartupString = 0 ;
        }
        if ( _Q_->StartupFilename )
        {
            _Q_->Verbosity = 0 ;
            _CfrTil_ContextNew_IncludeFile ( ( byte* ) "./namespaces/.init.cft" ) ;
            _CfrTil_ContextNew_IncludeFile ( _Q_->StartupFilename ) ;
        }
        else
        {
            if ( ! _Q_->StartIncludeTries ++ )
            {
                //_CfrTil_ContextNew_IncludeFile ( "./.init.cft" ) ;
                _CfrTil_ContextNew_InterpretString ( cfrTil, _Q_->InitString, SESSION ) ;
                _CfrTil_ContextNew_InterpretString ( cfrTil, _Q_->StartupString, SESSION ) ;
            }
            else if ( _Q_->StartIncludeTries < 3 )
            {
                AlertColors ;
                _CfrTil_ContextNew_IncludeFile ( ( byte* ) "./namespaces/.init.cft" ) ;
                if ( _Q_->ErrorFilename )
                {
                    if ( strcmp ( ( char* ) _Q_->ErrorFilename, "Debug Context" ) )
                    {
                        Printf ( ( byte* ) "\nError : \"%s\" include error!\n", _Q_->SigLocation ? _Q_->SigLocation : _Q_->ErrorFilename ) ;
                    }
#if 0                    
                    else
                    {
                        DebugColors ;
                        Printf ( ( byte* ) "\nComing from Debug Context\n" ) ;
                    }
#endif                    
                }
                DefaultColors ;
            }
        }
    }
    if ( _Q_->Verbosity > 2 )
    {
        Printf ( ( byte* ) " \nInternal Namespaces have been initialized.  " ) ;
        OVT_MemoryAllocated ( ) ;
    }
    _Q_->Verbosity = 1 ;
}

void
_CfrTil_InitialAddWordToNamespace ( Word * word, byte * containingNamespaceName, byte * superNamespaceName )
// this is only called at startup where we want to add the namespace to the RootNamespace
{
    Namespace *ns, *sns = _Q_->OVT_CfrTil->Namespaces ;
    if ( superNamespaceName )
    {
        sns = Namespace_FindOrNew_SetUsing ( superNamespaceName, sns, 1 ) ;
        sns->State = USING ;
    }
    ns = Namespace_FindOrNew_SetUsing ( containingNamespaceName, sns, 1 ) ; // find new namespace or create anew
    _Namespace_DoAddWord ( ns, word ) ; // add word to new namespace
}

void
_CfrTil_CPrimitiveNewAdd ( const char * name, block b, uint64 ctype, uint64 ltype, const char *nameSpace, const char * superNamespace )
{
    Word * word = _Word_New ( ( byte* ) name, CPRIMITIVE | ctype, ltype, EXISTING ) ; //DICTIONARY ) ;
    //if ( ctype & C_PREFIX_RTL_ARGS ) _DObject_ValueDefinition_Init ( word, ( int32 ) b, ( CPRIMITIVE | ctype ), BLOCK, ( byte* ) CfrTil_Eval_C_Rtl_ArgList, 0 ) ;
    //else 
    _DObject_ValueDefinition_Init ( word, ( int32 ) b, CPRIMITIVE | ctype, BLOCK, 0, 0 ) ;
    _CfrTil_InitialAddWordToNamespace ( word, ( byte* ) nameSpace, ( byte* ) superNamespace ) ;
    if ( ctype & INFIXABLE ) word->WType = WT_INFIXABLE ;
    else if ( ctype & PREFIX ) word->WType = WT_PREFIX ;
    else if ( ctype & C_PREFIX_RTL_ARGS ) word->WType = WT_C_PREFIX_RTL_ARGS ;
}

void
CfrTil_AddCPrimitives ( )
{
    int i ;
    for ( i = 0 ; CPrimitives [ i ].ccp_Name ; i ++ )
    {
        CPrimitive p = CPrimitives [ i ] ;
        _CfrTil_CPrimitiveNewAdd ( p.ccp_Name, p.blk_Definition, p.ui64_CType, p.ui64_LType, ( char* ) p.NameSpace, ( char* ) p.SuperNamespace ) ;
    }
}

void
CfrTil_MachineCodePrimitive_AddWords ( )
{
    int32 i, functionArg ;
    block * callHook ;
    Debugger * debugger = _Q_->OVT_CfrTil->Debugger0 ;
    for ( i = 0 ; MachineCodePrimitives [ i ].ccp_Name ; i ++ )
    {
        MachineCodePrimitive p = MachineCodePrimitives [ i ] ;

        // initialize some values in MachineCodePrimitives that are variables and have to be calculated at run time
        if ( String_Equal ( p.ccp_Name, "getESP" ) )
        {
            functionArg = ( int ) &debugger->DebugESP ;
            callHook = & debugger->GetESP ;
        }
        else if ( String_Equal ( p.ccp_Name, "restoreCpuState" ) )
        {
            functionArg = ( int ) debugger->cs_CpuState ;
            callHook = & debugger->RestoreCpuState ;
        }
        else if ( ( String_Equal ( p.ccp_Name, "saveCpuState" ) ) && ( String_Equal ( p.NameSpace, "System" ) ) )
        {
            functionArg = ( int ) _Q_->OVT_CfrTil->cs_CpuState ;
            callHook = & _Q_->OVT_CfrTil->SaveCpuState ;
        }
        else if ( ( String_Equal ( p.ccp_Name, "saveCpuState" ) ) && ( String_Equal ( p.NameSpace, "Debug" ) ) )
        {
            functionArg = ( int ) debugger->cs_CpuState ;
            callHook = & debugger->SaveCpuState ;
        }
        else
        {
            functionArg = - 1 ;
            callHook = 0 ;
        }
        _CfrTil_MachineCodePrimitive_NewAdd ( p.ccp_Name, p.ui64_CType, callHook, p.Function, functionArg, p.NameSpace, p.SuperNamespace ) ;
    }
}



