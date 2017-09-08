
#include "../include/cfrtil.h"

void
_Debugger_DoStepOneInstruction ( Debugger * debugger )
{
    ( ( VoidFunction ) debugger->StepInstructionBA->BA_Data ) ( ) ;
}

void
Debugger_StepOneInstruction ( Debugger * debugger )
{
    if ( ! sigsetjmp ( debugger->JmpBuf0, 0 ) )
    {
        _Debugger_DoStepOneInstruction ( debugger ) ;
    }
}

// Debugger_CompileOneInstruction ::
// this function should not affect the C registers at all 
// we save them before we call our stuff and restore them after

byte *
_Debugger_CompileOneInstruction ( Debugger * debugger, byte * jcAddress )
{
    int64 showRegsFlag = 0 ;

    CfrTil_Compile_SaveCCompileTimeCpuState ( _CfrTil_, showRegsFlag && ( _Q_->Verbosity >= 3 ) ) ; // save our c compiler cpu register state

    Debugger_Compile_Restore_Runtime_DebuggerCpuState ( debugger, showRegsFlag && ( _Q_->Verbosity >= 3 ) ) ; // restore our runtime state before the current insn

    byte * nextInsn = __Debugger_CompileOneInstruction ( debugger, jcAddress ) ; // the single current stepping insn

    Debugger_Compile_Save_RunTime_DebuggerCpuState ( debugger, showRegsFlag && ( _Q_->Verbosity >= 3 ) ) ; // save our runtime state after the instruction : which we will restore before the next insn

    CfrTil_Compile_RestoreCCompileTimeCpuState ( _CfrTil_, showRegsFlag && ( _Q_->Verbosity >= 3 ) ) ; // finally restore our c compiler cpu register state

    _Compile_Return ( ) ;

    return nextInsn ;
}

byte *
Debugger_CompileOneInstruction ( Debugger * debugger, byte * jcAddress )
{
    int64 showExtraFlag = 0 ;

    ByteArray * svcs = _Q_CodeByteArray ;
    _ByteArray_ReInit ( debugger->StepInstructionBA ) ; // we are only compiling one insn here so clear our BA before each use
    Set_CompilerSpace ( debugger->StepInstructionBA ) ; // now compile to this space
    byte * svHere = Here ; // save 

    byte * nextInsn = _Debugger_CompileOneInstruction ( debugger, jcAddress ) ; // compile the insn here

    debugger->SaveDsp = Dsp ;
    debugger->PreHere = Here ;
    debugger->SaveTOS = TOS ;
    debugger->SaveStackDepth = DataStack_Depth ( ) ;
    byte * svHere2 = Here ;
    Set_CompilerSpace ( svcs ) ; // restore compiler space pointer before "do it" in case "do it" calls the compiler

    if ( showExtraFlag && ( _Q_->Verbosity >= 3 ) ) Debug_ExtraShow ( svHere2 - svHere, showExtraFlag ) ;
    return nextInsn ;
}

void
_Debugger_CompileAndStepOneInstruction ( Debugger * debugger, byte * jcAddress )
{
    byte * nextInsn = Debugger_CompileOneInstruction ( debugger, jcAddress ) ;
    Debugger_StepOneInstruction ( debugger ) ;

    DebugColors ;
    debugger->DebugAddress = nextInsn ;
}

int64
Debugger_CanWeStep ( Debugger * debugger, Word * word )
{
    if ( ( ! word ) || ( ! word->CodeStart ) || ( ! NamedByteArray_CheckAddress ( _Q_CodeSpace, word->CodeStart ) ) )
    {
        SetState ( debugger, DBG_CAN_STEP, false ) ; // debugger->State flag = false ;
        return false ;
    }
        //if ( word && ( Compiling || ( word->CProperty & ( ALIAS | IMMEDIATE | CPRIMITIVE | DLSYM_WORD ) ) || ( word->LProperty & T_LISP_DEFINE ) ) ) //|| ( CompileMode && ( ! ( word->CProperty & IMMEDIATE ) ) ) )
        //else if ( word && ( Compiling || ( word->CProperty & ( IMMEDIATE | CPRIMITIVE | DLSYM_WORD ) ) || ( word->LProperty & T_LISP_DEFINE ) ) ) //|| ( CompileMode && ( ! ( word->CProperty & IMMEDIATE ) ) ) )
    else if ( word && ( Compiling || ( word->CProperty & ( CPRIMITIVE | DLSYM_WORD ) ) || ( word->LProperty & T_LISP_DEFINE ) ) ) //|| ( CompileMode && ( ! ( word->CProperty & IMMEDIATE ) ) ) )
    {
        return false ;
    }
    else
    {
        SetState ( debugger, DBG_CAN_STEP, true ) ;
        return true ;
    }
}

byte *
__Debugger_CompileOneInstruction ( Debugger * debugger, byte * jcAddress )
{
    byte * newDebugAddress ;
    int64 size ;
    Word * word = 0 ;
start:
    size = Debugger_Udis_GetInstructionSize ( debugger ) ;
    if ( jcAddress ) // jump or call address
    {
        word = Word_GetFromCodeAddress ( jcAddress ) ; //Word_GetFromCodeAddress_NoAlias ( jcAddress ) ;
        if ( word && ( word != debugger->w_Word ) )
        {
            debugger->w_Word = word ;
            debugger->Token = word->Name ;
            //if ( * debugger->DebugAddress == CALLI32 ) _Word_ShowSourceCode ( word ) ;
        }

        if ( ( ! word ) || ( ! Debugger_CanWeStep ( debugger, word ) ) )//( jcAddress < ( byte* ) svcs->BA_Data ) || ( jcAddress > ( byte* ) svcs->bp_Last ) )
        {
            _Printf ( ( byte* ) "\ncalling thru - a subroutine : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
            Compile_Call_With32BitDisp ( jcAddress ) ; // this will call jcAddress subroutine and return to our code to be compiled next
            // so that newDebugAddress, below, will be our next stepping insn
            newDebugAddress = debugger->DebugAddress + size ;
        }
        else if ( debugger->Key == 't' ) // step '(t)hru' the native code like a non-native subroutine
        {
            _Printf ( ( byte* ) "\ncalling (t)hru - a subroutine : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
            Compile_Call_With32BitDisp ( jcAddress ) ;
            newDebugAddress = debugger->DebugAddress + size ;
        }
        else if ( debugger->Key == 'o' ) // step '(o)ver' the native code like a non-native subroutine
        {
            _Printf ( ( byte* ) "\nstepping '(o)ver' - a subroutine : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
            //Compile_Call ( jcAddress ) ;
            newDebugAddress = debugger->DebugAddress + size ;
        }
        else if ( debugger->Key == 'u' ) // step o(u)t of the native code like a non-native subroutine
        {
            _Printf ( ( byte* ) "\nstepping thru and 'o(u)t' of a \"native\" subroutine : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
            Compile_Call_With32BitDisp ( debugger->DebugAddress ) ;
            if ( Stack_Depth ( debugger->DebugStack ) ) newDebugAddress = ( byte* ) _Stack_Pop ( debugger->DebugStack ) ;
            else newDebugAddress = 0 ;
        }
        else // step into the the jmp/call insn
        {
            if ( word->CProperty & ( ALIAS ) ) word = word->W_AliasOf ;
            if ( * debugger->DebugAddress == CALLI32 )
            {
                if ( _Q_->Verbosity > 1 ) _Word_ShowSourceCode ( word ) ;
                _Printf ( ( byte* ) "\nstepping into a cfrtil compiled function : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
                _Stack_Push ( debugger->DebugStack, ( int64 ) ( debugger->DebugAddress + size ) ) ; // the return address
                // push the return address this time around; next time code at newDebugAddress will be processed
                // when ret is the insn Debugger_StepOneInstruction will handle it 
            }
            // for either jmp/call/jcc ...
            newDebugAddress = jcAddress ;
        }
    }
    else if ( debugger->Key == 'u' ) // step o(u)t of the native code like a non-native subroutine
    {
        _Printf ( ( byte* ) "\nstepping thru and out of a \"native\" subroutine : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
        debugger->Key = 's' ;
        SetState ( debugger, DBG_AUTO_MODE, true ) ;
        goto doRegular ;
    }
    else
    {
doRegular:
        newDebugAddress = debugger->DebugAddress + size ;
        if ( ! GetState ( debugger, DBG_JCC_INSN ) )
        {
            ByteArray_AppendCopy ( debugger->StepInstructionBA, size, debugger->DebugAddress ) ;
        }
    }
done:

    return newDebugAddress ;
}

void
Debugger_CompileAndStepOneInstruction ( Debugger * debugger )
{
start:
    if ( debugger->DebugAddress )
    {
        byte *jcAddress = 0 ;
        // special cases
        if ( * debugger->DebugAddress == _RET )
        {
            if ( Stack_Depth ( debugger->DebugStack ) )
            {
                debugger->DebugAddress = ( byte* ) _Stack_Pop ( debugger->DebugStack ) ;
                Debugger_GetWordFromAddress ( debugger ) ;
            }
            else
            {
                Debugger_SyncStackPointersFromCpuState ( debugger ) ;
                SetState ( debugger, DBG_STACK_OLD, true ) ;
                debugger->ReturnStackCopyPointer = 0 ;
                if ( GetState ( debugger, DBG_BRK_INIT ) )
                {
                    SetState_TrueFalse ( debugger, DBG_INTERPRET_LOOP_DONE | DBG_STEPPED, DBG_ACTIVE | DBG_BRK_INIT | DBG_STEPPING ) ;
                }
                else
                {
                    SetState_TrueFalse ( debugger, DBG_INTERPRET_LOOP_DONE | DBG_STEPPED, DBG_ACTIVE | DBG_STEPPING ) ;
                }
                if ( debugger->w_Word ) SetState ( debugger->w_Word, STEPPED, true ) ;
                debugger->DebugAddress = 0 ;
                if ( _Q_->Verbosity > 3 )
                {
                    _Debugger_CpuState_Show ( ) ;
                    Pause ( ) ;
                }
                debugger->cs_Cpu->State = 0 ;
            }
            goto end ;
        }
        else if ( ( * debugger->DebugAddress == JMPI32 ) || ( * debugger->DebugAddress == CALLI32 ) )
        {
            jcAddress = JumpCallInstructionAddress ( debugger->DebugAddress ) ;
            Word * word = Word_GetFromCodeAddress ( jcAddress ) ;
            //debugger->CurrentlyRunningWord = word ;
            if ( word && ( word->CProperty & ( DEBUG_WORD ) ) ) //&& ( ! GetState ( debugger, (DBG_CONTINUE_MODE|DBG_AUTO_MODE) ) ) )
            {
                SetState ( debugger, (DBG_CONTINUE_MODE|DBG_AUTO_MODE), false ) ;
                _Printf ( ( byte* ) "\nskipping over a debug word : %s : at 0x%-8x", word ? ( char* ) c_dd ( word->Name ) : "", debugger->DebugAddress ) ;
                debugger->DebugAddress += 5 ; // 5 : sizeof jmp/call insn // debugger->DebugAddress + size ; // skip the call insn to the next after it
                goto end ;
            }
        }
        else if ( ( * debugger->DebugAddress == CALL_JMP_MOD_RM ) && ( RM ( debugger->DebugAddress ) == 16 ) ) // inc/dec are also opcode == 0xff
        {
            //jcAddress = JumpCallInsnAddress_ModRm ( debugger->DebugAddress ) ;
            //-----------------------------------------------------------------------
            //   modRm byte ( bits )  mod 0 : no disp ; mod 1 : 1 byte disp : mod 2 : 4 byte disp ; mod 3 : just reg value
            //    mod     reg      rm
            //   7 - 6   5 - 3   2 - 0
            //-----------------------------------------------------------------------
            byte * address = debugger->DebugAddress ;
            byte modRm = * ( byte* ) ( address + 1 ) ; // 1 : 1 byte opCode
            if ( modRm & 32 ) SyntaxError ( 1 ) ; // we only currently compile call reg code 2/3, << 3 ; not jmp; jmp reg code == 4/5 : reg code 100/101 ; inc/dec 0/1 : << 3
            int64 mod = modRm & 192 ; // 192 : CALL_JMP_MOD_RM : RM not inc/dec
            if ( mod == 192 ) jcAddress = ( byte* ) _Debugger_->cs_Cpu->Eax ;
            // else it could be inc/dec
        }
        else if ( ( * debugger->DebugAddress == 0x0f ) && ( ( * ( debugger->DebugAddress + 1 ) >> 4 ) == 0x8 ) ) // jcc 
        {
            SetState ( debugger, DBG_JCC_INSN, true ) ;
            jcAddress = Debugger_DoJcc ( debugger, 2 ) ;
            _Debugger_CompileAndStepOneInstruction ( debugger, jcAddress ) ;
            SetState ( debugger, DBG_JCC_INSN, false ) ;
            goto end ;
        }
        else if ( ( ( ( byte* ) debugger->DebugAddress )[0] >> 4 ) == 7 ) // callcc ?
        {
            SetState ( debugger, DBG_JCC_INSN, true ) ;
            jcAddress = Debugger_DoJcc ( debugger, 1 ) ;
            _Debugger_CompileAndStepOneInstruction ( debugger, jcAddress ) ;
            SetState ( debugger, DBG_JCC_INSN, false ) ;
            goto end ;
        }
        _Debugger_CompileAndStepOneInstruction ( debugger, jcAddress ) ;
end:
        if ( debugger->DebugAddress )
        {

            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ; // the next instruction
            // keep eip - instruction pointer - up to date ..
            debugger->cs_Cpu->Eip = ( uint64 * ) debugger->DebugAddress ;
        }
    }
}

// simply : copy the current insn to a ByteArray buffer along with
// prefix and postfix instructions that restore and
// save the cpu state; then run that ByteArray code buffer

void
Debugger_PreStartStepping ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    if ( word )
    {
        debugger->WordDsp = Dsp ; // by 'eval' we stop debugger->Stepping and //continue thru this word as if we hadn't stepped
        Debugger_CanWeStep ( debugger, word ) ;
        if ( ! GetState ( debugger, DBG_CAN_STEP ) )
        {
            if ( Compiling )
            {
                _Printf ( "\nCompiling : Stepping is off in Compile mode." ) ;
            }
            else
            {
                _Printf ( "\nStepping turned off for this word : %s%s%s%s : debugger->DebugAddress = 0x%08x : (e)valuating",
                    c_ud ( word->S_ContainingNamespace ? word->S_ContainingNamespace->Name : ( byte* ) "<literal> " ),
                    word->S_ContainingNamespace ? ( byte* ) "." : ( byte* ) "", c_du ( word->Name ),
                    GetState ( debugger, DBG_AUTO_MODE ) ? " : automode turned off" : "",
                    debugger->DebugAddress ) ;
                //if ( debugger->DebugAddress ) Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ; // the next instruction
            }
            if ( GetState ( debugger, DBG_RUNTIME_BREAKPOINT ) && String_Equal ( debugger->w_Word->Name, "<dbg>" ) )
            {
                debugger->DebugAddress += 5 ;
            }
            else Debugger_Eval ( debugger ) ;
            SetState ( _Debugger_, DBG_AUTO_MODE, false ) ; //if ( GetState ( debugger, DBG_AUTO_MODE ) )
            return ;
        }
        else
        {
            Debugger_SetupStepping ( debugger, 1 ) ;
            SetState ( debugger, DBG_NEWLINE | DBG_PROMPT | DBG_INFO, false ) ;
        }
    }
    else SetState_TrueFalse ( debugger, DBG_NEWLINE, DBG_STEPPING ) ;
    return ;
}

void
Debugger_Step ( Debugger * debugger )
{
    if ( ! GetState ( debugger, DBG_STEPPING ) )
    {
        Debugger_PreStartStepping ( debugger ) ;
        return ;
    }
    else
    {
        Debugger_CompileAndStepOneInstruction ( debugger ) ;
        Debugger_AfterStep ( debugger ) ;
    }
}

void
Debugger_AfterStep ( Debugger * debugger )
{
    //if ( ( _Q_->Verbosity > 3 ) && ( debugger->cs_Cpu->Esp != debugger->LastEsp ) ) Debugger_PrintReturnStackWindow ( ) ;
    debugger->LastEsp = debugger->cs_Cpu->Esp ;

    _Debugger_SyncStackPointersFromCpuState ( debugger ) ;
    if ( ( int64 ) debugger->DebugAddress ) // set by StepOneInstruction
    {
        debugger->SteppedWord = debugger->w_Word ;
        debugger->w_Word = Debugger_GetWordFromAddress ( debugger ) ;
        SetState_TrueFalse ( debugger, DBG_STEPPING, ( DBG_INFO | DBG_MENU | DBG_PROMPT ) ) ;
    }
    else
    {
        SetState_TrueFalse ( debugger, DBG_PRE_DONE | DBG_STEPPED | DBG_NEWLINE | DBG_PROMPT | DBG_INFO, DBG_STEPPING ) ;
        //if ( GetState ( debugger, DBG_DONE ) ) SetState ( _CfrTil_, DEBUG_MODE, false ) ;
        return ;
    }
}

void
_Debugger_SetupStepping ( Debugger * debugger, Word * word, byte * address, byte *name, int64 iflag )
{
    _Printf ( ( byte* ) "\nSetting up stepping : location = %s : debugger->word = \'%s\' : ...", c_dd ( _Context_Location ( _Context_ ) ), word ? word->Name : ( name ? name : ( byte* ) "" ) ) ;
    if ( word )
    {
        _CfrTil_Source ( debugger->w_Word, 0 ) ;
        if ( ( ! address ) || ( ! GetState ( debugger, ( DBG_BRK_INIT ) ) ) ) address = ( byte* ) word->Definition ;
    }
    SetState_TrueFalse ( debugger, DBG_STEPPING, DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_MENU ) ;
    if ( iflag )
    {

        _Printf ( "\nNext stepping instruction" ) ; // necessary in some cases
        Debugger_UdisOneInstruction ( debugger, address, ( byte* ) "", ( byte* ) "" ) ;
    }
    //debugger->SaveDsp = Dsp ; // saved before we start stepping
    debugger->DebugAddress = address ;
    debugger->w_Word = word ;
}

void
Debugger_SetupStepping ( Debugger * debugger, int64 iflag )
{
    _Debugger_SetupStepping ( debugger, debugger->w_Word, debugger->DebugAddress, 0, iflag ) ;
    //if ( ! GetState ( debugger, ( DBG_BRK_INIT ) ) ) Debugger_CheckSaveCpuState ( debugger ) ;
    Debugger_CheckSaveCpuState ( debugger ) ;
    debugger->LevelBitNamespaceMap = 0 ;
    Stack_Init ( debugger->LocalsNamespacesStack ) ;
}

int64
Debugger_SetupReturnStackCopy ( Debugger * debugger, int64 size )
{
    if ( _Q_->Verbosity > 3 ) _CfrTil_PrintNReturnStack ( 4 ) ;
    uint64 * esp = ( uint64* ) & debugger->cs_Cpu->Esp [0] ; //debugger->DebugESP [- 1] ; //debugger->cs_Cpu->Esp [1] ; //debugger->cs_Cpu->Esp ;
    if ( esp )
    {
        uint64 rsc0 ;
        int64 pushedWindow = 32 ;
        if ( ! debugger->ReturnStackCopyPointer )
        {
            rsc0 = ( uint64 ) Mem_Allocate ( size, COMPILER_TEMP ) ;
            rsc0 = rsc0 & 0xfffffffe ;
            debugger->ReturnStackCopyPointer = ( byte* ) ( rsc0 + size - pushedWindow ) ;
            d0 ( _PrintNStackWindow ( ( int64* ) debugger->ReturnStackCopyPointer, "ReturnStackCopy", "RSCP", 4 ) ) ;
        }
        else rsc0 = ( uint64 ) ( debugger->ReturnStackCopyPointer - size + pushedWindow ) ;
        memcpy ( ( byte* ) rsc0, ( ( byte* ) esp ) - size + pushedWindow, size ) ; // 32 : account for useful current stack
        //memcpy ( ( byte* ) rsc0, ( ( byte* ) esp ), size ) ; // 32 : account for useful current stack
        d0 ( _PrintNStackWindow ( ( int64* ) esp, "ReturnStack", "ESP", 4 ) ) ; //pushedWindow ) ) ;
        d0 ( _PrintNStackWindow ( ( int64* ) debugger->ReturnStackCopyPointer, "ReturnStackCopy", "RSCP", 4 ) ) ; //pushedWindow ) ) ;
        SetState ( debugger, DBG_STACK_OLD, false ) ;
        return true ;
    }
    else return false ;
}

void
Debugger_PrintReturnStackWindow ( )
{
    _PrintNStackWindow ( ( int64* ) _Debugger_->cs_Cpu->Esp, "Debugger ReturnStack (ESP)", "ESP", 4 ) ;
    //_PrintNStackWindow ( ( uint64* ) _Debugger_->ReturnStackCopyPointer, ( byte * ) "ReturnStackCopy", ( byte * ) "RSCP", 4 ) ;
}

// restore the 'internal running cfrTil' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running cfrTil' cpu state

void
Debugger_Compile_Restore_Runtime_DebuggerCpuState ( Debugger * debugger, int64 showFlag ) // restore the running cfrTil cpu state
{
    int64 stackSetupFlag = 0 ;
    // restore the 'internal running cfrTil' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running cfrTil' cpu state
    // we don't have to worry so much about the compiler 'spoiling' our insn with restore 
    _Compile_CpuState_Restore ( debugger->cs_Cpu, 1 ) ;
    if ( ( ! debugger->ReturnStackCopyPointer ) || GetState ( debugger, DBG_STACK_OLD ) )
    {
        stackSetupFlag = Debugger_SetupReturnStackCopy ( debugger, 8 * K ) ;
    }
    // restore the running cfrTil esp/ebp : nb! : esp/ebp were not restored by debugger->RestoreCpuState and are being restore here in the proper context
    if ( stackSetupFlag )
    {
        _Compile_Get_FromCAddress_ToReg_ThruReg ( EBP, ( byte * ) & debugger->ReturnStackCopyPointer - ( ( ( int64 ) debugger->cs_Cpu->Ebp ) - ( ( int64 ) debugger->cs_Cpu->Esp ) ), ECX ) ;
        _Compile_Get_FromCAddress_ToReg_ThruReg ( ESP, ( byte * ) & debugger->ReturnStackCopyPointer, ECX ) ;
    }
    if ( showFlag )
    {
        Compile_Call_With32BitDisp ( ( byte* ) _Debugger_CpuState_Show ) ; // also dis insn
        _Compile_Get_FromCAddress_ToReg ( ECX, ( byte * ) & debugger->cs_Cpu->Ecx ) ; // restore our scratch reg
        _Compile_Get_FromCAddress_ToReg ( EAX, ( byte * ) & debugger->cs_Cpu->Eax ) ; //nb.!! :: somehow needed (eax gets clobbered somewhere with showFlag on)
        _Compile_Get_FromCAddress_ToReg ( EBX, ( byte * ) & debugger->cs_Cpu->Ebx ) ; // why not just in case future gcc use different regs
    }
}

void
CfrTil_Compile_RestoreCCompileTimeCpuState ( CfrTil * cfrtil, int64 showFlag )
{
    _Compile_CpuState_Restore ( cfrtil->cs_Cpu, 1 ) ;
    if ( showFlag )
    {
        Compile_Call_With32BitDisp ( ( byte* ) _CfrTil_CpuState_Show ) ;
        _Compile_Get_FromCAddress_ToReg ( ECX, ( byte * ) & cfrtil->cs_Cpu->Ecx ) ; // restore our scratch reg
        _Compile_Get_FromCAddress_ToReg ( EAX, ( byte * ) & cfrtil->cs_Cpu->Eax ) ; //nb.!! :: somehow needed (eax gets clobbered somewhere with showFlag on)
        _Compile_Get_FromCAddress_ToReg ( EBX, ( byte * ) & cfrtil->cs_Cpu->Ebx ) ; // why not just in case future gcc use different regs
    }
}

// restore the 'internal running cfrTil' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running cfrTil' cpu state

void
CfrTil_Compile_SaveCCompileTimeCpuState ( CfrTil * cfrtil, int64 showFlag )
{
    _Compile_CpuState_Save ( cfrtil->cs_Cpu ) ;
    if ( showFlag ) Compile_Call_With32BitDisp ( ( byte* ) _CfrTil_CpuState_Show ) ;
}

void
Debugger_Compile_Save_RunTime_DebuggerCpuState ( Debugger * debugger, int64 showFlag )
{
    _Compile_CpuState_Save ( debugger->cs_Cpu ) ;
    if ( showFlag )
    {
        Compile_Call_With32BitDisp ( ( byte* ) CfrTil_Debugger_UdisOneInsn ) ;
    }
    if ( ( _Q_->Verbosity > 3 ) && ( debugger->cs_Cpu->Esp != debugger->LastEsp ) ) Debugger_PrintReturnStackWindow ( ) ;

    if ( showFlag )
    {
        Compile_Call_With32BitDisp ( ( byte* ) CfrTil_Debugger_CheckSaveCpuStateShow ) ;
    }
}

void
Debugger_Stepping_Off ( Debugger * debugger )
{
    if ( Debugger_IsStepping ( debugger ) )
    {
        Debugger_SetStepping ( debugger, false ) ;
        debugger->DebugAddress = 0 ;
    }
}

Word *
Debugger_GetWordFromAddress ( Debugger * debugger )
{
    Word * word = 0 ;
    if ( debugger->DebugAddress )
    {
        word = Word_GetFromCodeAddress ( debugger->DebugAddress ) ;
        if ( word ) debugger->w_Word = word ;
    }
    if ( ( ! word ) && debugger->Token ) word = Finder_FindWord_UsedNamespaces ( _Finder_, debugger->Token ) ;
    return word ;
}

byte *
Debugger_DoJcc ( Debugger * debugger, int64 numOfBytes )
{
    byte * jcAddress = ( numOfBytes == 2 ) ? JccInstructionAddress_2Byte ( debugger->DebugAddress ) : JccInstructionAddress_1Byte ( debugger->DebugAddress ) ;
    int64 tttn, ttt, n ;
    tttn = ( numOfBytes == 2 ) ? ( debugger->DebugAddress[1] & 0xf ) : ( debugger->DebugAddress[0] & 0xf ) ;
    ttt = ( tttn & 0xe ) >> 1 ;
    n = tttn & 1 ;

    // cf. Intel Software Developers Manual v1. (253665), Appendix B
    // ?? nb. some of this may not be (thoroughly) tested as of 11-14-2011 -- but no known problems after months of usual testing ??
    // setting jcAddress to 0 means we don't branch and just continue with the next instruction
    if ( ttt == BELOW ) // ttt 001
    {
        if ( ( n == 0 ) && ! ( ( uint64 ) debugger->cs_Cpu->EFlags & CARRY_FLAG ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( uint64 ) debugger->cs_Cpu->EFlags & CARRY_FLAG ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == ZERO_TTT ) // ttt 010
    {
        if ( ( n == 0 ) && ! ( ( uint64 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( uint64 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == BE ) // ttt 011 :  below or equal
    {
        if ( ( n == 0 ) && ! ( ( ( uint64 ) debugger->cs_Cpu->EFlags & CARRY_FLAG ) | ( ( uint64 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( ( uint64 ) debugger->cs_Cpu->EFlags & CARRY_FLAG ) | ( ( uint64 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == LESS ) // ttt 110
    {
        if ( ( n == 0 ) && ! ( ( ( uint64 ) debugger->cs_Cpu->EFlags & SIGN_FLAG ) ^ ( ( uint64 ) debugger->cs_Cpu->EFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( ( uint64 ) debugger->cs_Cpu->EFlags & SIGN_FLAG ) ^ ( ( uint64 ) debugger->cs_Cpu->EFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == LE ) // ttt 111
    {
        if ( ( n == 0 ) &&
            ! ( ( ( ( uint64 ) debugger->cs_Cpu->EFlags & SIGN_FLAG ) ^ ( ( uint64 ) debugger->cs_Cpu->EFlags & OVERFLOW_FLAG ) ) | ( ( uint64 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        if ( ( n == 1 ) &&
            ( ( ( ( uint64 ) debugger->cs_Cpu->EFlags & SIGN_FLAG ) ^ ( ( uint64 ) debugger->cs_Cpu->EFlags & OVERFLOW_FLAG ) ) | ( ( uint64 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) ) )
        {

            jcAddress = 0 ;
        }
    }
    return jcAddress ;
}

void
Debug_ExtraShow ( int64 size, int64 force )
{
    Debugger * debugger = _Debugger_ ;
    if ( force || ( _Q_->Verbosity > 3 ) )
    {
        if ( force || ( _Q_->Verbosity > 4 ) )
        {
            _Printf ( "\n\ndebugger->SaveCpuState" ) ;
            _Debugger_Disassemble ( debugger, ( byte* ) debugger->SaveCpuState, 200, 1 ) ; //137, 1 ) ;
            _Printf ( "\n\ndebugger->RestoreCpuState" ) ;
            _Debugger_Disassemble ( debugger, ( byte* ) debugger->RestoreCpuState, 76, 0 ) ; //100, 0 ) ;
        }
        _Printf ( "\n\ndebugger->StepInstructionBA->BA_Data" ) ;
        _Debugger_Disassemble ( debugger, ( byte* ) debugger->StepInstructionBA->BA_Data, size, 0 ) ;
    }
}

