
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
        //( ( VoidFunction ) debugger->StepInstructionBA->BA_Data ) ( ) ;
    }
}

// Debugger_CompileOneInstruction ::
// this function should not affect the C registers at all 
// we save them before we call our stuff and restore them after

byte *
Debugger_CompileOneInstruction ( Debugger * debugger, byte * jcAddress )
{
    int32 showRegsFlag = 0 ;

    CfrTil_Compile_SaveCCompileTimeCpuState ( _CfrTil_, showRegsFlag || ( _Q_->Verbosity > 3 ) ) ; // save our c compiler cpu register state

    Debugger_Compile_Restore_Runtime_DebuggerCpuState ( debugger, 1, showRegsFlag || ( _Q_->Verbosity > 3 ) ) ; // restore our runtime state before the current insn

    byte * nextInsn = _Debugger_CompileOneInstruction ( debugger, jcAddress ) ; // the single current stepping insn

    Debugger_Compile_Save_RunTime_DebuggerCpuState ( debugger, showRegsFlag || ( _Q_->Verbosity > 3 ) ) ; // save our runtime state after the instruction : which we will restore before the next insn

    CfrTil_Compile_RestoreCCompileTimeCpuState ( _CfrTil_, showRegsFlag || ( _Q_->Verbosity > 3 ) ) ; // finally restore our c compiler cpu register state

    _Compile_Return ( ) ;

    return nextInsn ;
}

void
_Debugger_CompileAndStepOneInstruction ( Debugger * debugger, byte * jcAddress )
{
    int32 showExtraFlag = 0 ;

    ByteArray * svcs = _Q_CodeByteArray ;
    _ByteArray_ReInit ( debugger->StepInstructionBA ) ; // we are only compiling one insn here so clear our BA before each use
    Set_CompilerSpace ( debugger->StepInstructionBA ) ; // now compile to this space
    byte * svHere = Here ; // save 

    byte * nextInsn = Debugger_CompileOneInstruction ( debugger, jcAddress ) ; // compile the insn here

    debugger->SaveDsp = Dsp ;
    debugger->PreHere = Here ;
    debugger->SaveTOS = TOS ;
    debugger->SaveStackDepth = DataStack_Depth ( ) ;
    byte * svHere2 = Here ;
    Set_CompilerSpace ( svcs ) ; // restore compiler space pointer before "do it" in case "do it" calls the compiler

    if ( showExtraFlag || ( _Q_->Verbosity > 3 ) ) Debug_ExtraShow ( svHere2 - svHere, showExtraFlag ) ;

    Debugger_StepOneInstruction ( debugger ) ;

    DebugColors ;
    debugger->DebugAddress = nextInsn ;
}

int32
Debugger_CanWeStep ( Debugger * debugger, Word * word )
{
    //Word * word = debugger->w_Word ;
    //if ( ( ! word ) || ( Compiling || ( word->CProperty & ( IMMEDIATE | CPRIMITIVE | DLSYM_WORD ) ) || ( word->LProperty & T_LISP_DEFINE ) ) ) //|| ( CompileMode && ( ! ( word->CProperty & IMMEDIATE ) ) ) )
    //if ( word && ( Compiling || ( word->CProperty & ( ALIAS | IMMEDIATE | CPRIMITIVE | DLSYM_WORD ) ) || ( word->LProperty & T_LISP_DEFINE ) ) ) //|| ( CompileMode && ( ! ( word->CProperty & IMMEDIATE ) ) ) )
    if ( ( ! word ) || ( ! word->CodeStart ) || ( ! NamedByteArray_CheckAddress ( _Q_CodeSpace, word->CodeStart ) ) ) return false ;
    else if ( word && ( Compiling || ( word->CProperty & ( IMMEDIATE | CPRIMITIVE | DLSYM_WORD ) ) || ( word->LProperty & T_LISP_DEFINE ) ) ) //|| ( CompileMode && ( ! ( word->CProperty & IMMEDIATE ) ) ) )
    {
        SetState ( debugger, DBG_CAN_STEP, false ) ; // debugger->State flag = false ;
        return false ;
    }
    else
    {
        SetState ( debugger, DBG_CAN_STEP, true ) ;
        return true ;
    }
}

#if 0

byte *
_Debugger_CompileOneInstruction ( Debugger * debugger, byte * jcAddress )
{
    byte * newDebugAddress ;
    int32 size ;
start:
    size = Debugger_Udis_GetInstructionSize ( debugger ) ;
    if ( jcAddress ) // jump or call address
    {
        Word * word = Word_GetFromCodeAddress_NoAlias ( jcAddress ) ;
        if ( word && ( word != debugger->w_Word ) )
        {
            debugger->w_Word = word ;
            debugger->Token = word->Name ;
            //if ( * debugger->DebugAddress == CALLI32 ) _Word_ShowSourceCode ( word ) ;
        }
#if 1       
        if ( word && ( word->CProperty & DEBUG_WORD ) )// ( String_Equal ( word->Name, "<dbg>" ) ) )
        {
            if ( GetState ( debugger, DBG_AUTO_MODE ) )
            {
                //_Printf ( ( byte* ) "\nskipping over '<dbg>' and turning off autoMode : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
                _Printf ( ( byte* ) "\nskipping over %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
                //SetState ( debugger, DBG_AUTO_MODE, false ) ;
            }
            jcAddress += size ; // debugger->DebugAddress + size ; // skip the call insn to the next after it
            goto start ;
        }
        //else 
#endif        
        if ( ! Debugger_CanWeStep ( debugger ) ) //( jcAddress < ( byte* ) svcs->BA_Data ) || ( jcAddress > ( byte* ) svcs->bp_Last ) )
        {
            if ( * debugger->DebugAddress == JMPI32 )
            {
                CfrTil_Exception ( MACHINE_CODE_ERROR, 1 ) ; // can't in the current debugger jmp into C compiler code
            }
            else
            {
                newDebugAddress = debugger->DebugAddress + size ;
                //                _Printf ( ( byte* ) "\ncalling thru a \"foreign\" C subroutine : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
                Compile_Call ( jcAddress ) ; // 5 : sizeof call insn with offset
            }
        }
        else if ( debugger->Key == 'o' ) // step thru ("over") the native code like a non-native subroutine
        {
            _Printf ( ( byte* ) "calling thru (over) a \"native\" subroutine : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
            Compile_Call ( jcAddress ) ; // 5 : sizeof call insn with offset
            // !?!? this may need work right in here ... !?!?
            newDebugAddress = debugger->DebugAddress + size ;
        }
        else if ( debugger->Key == 'u' ) // step o(u)t of the native code like a non-native subroutine
        {
            _Printf ( ( byte* ) "\nstepping out of a \"native\" subroutine" ) ;
            Compile_Call ( debugger->DebugAddress ) ; // 5 : sizeof call insn with offset
            // !?!? this may need work !?!?
            if ( Stack_Depth ( debugger->DebugStack ) ) newDebugAddress = ( byte* ) _Stack_Pop ( debugger->DebugStack ) ;
            else
            {
                newDebugAddress = 0 ;
            }
        }
            //else if ( ( word && ( word->CProperty & CPRIMITIVE ) ) && ( ( jcAddress < ( byte* ) svcs->BA_Data ) || ( jcAddress > ( byte* ) svcs->bp_Last ) ) || ( word && ( word->CProperty & DLSYM_WORD ) ) )
        else if ( debugger->Key == 'i' ) // step (i)nto native code 
        {
            _Printf ( ( byte* ) "\nstepping into a \"native\" C subroutine : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
            _Stack_Push ( debugger->DebugStack, ( int32 ) ( debugger->DebugAddress + size ) ) ; // the return address
            newDebugAddress = jcAddress ;
        }
        else
        {
            if ( GetState ( debugger, DBG_JCC_INSN ) ) newDebugAddress = jcAddress ;
            else if ( ( * debugger->DebugAddress == JMPI32 ) ) //|| GetState ( debugger, DBG_JCC_INSN ) )
            {
                //_Compile_JumpToAddress ( jcAddress ) ; //_ByteArray_Here ( debugger->StepInstructionBA ) + size ) ; // 5 : sizeof call insn with offset - call to immediately after this very instruction
                newDebugAddress = jcAddress ; //skip an insn this time 
            }
            else if ( * debugger->DebugAddress == CALLI32 )
            {
                Compile_Call ( jcAddress ) ; //_ByteArray_Here ( debugger->StepInstructionBA ) + size ) ; // 5 : sizeof call insn with offset - call to immediately after this very instruction
                // emulate a call -- all we really needed was its address and to push (above) the return address if necessary - if it was a 'call' instruction
                newDebugAddress = debugger->DebugAddress + size ;
            }
            else newDebugAddress = jcAddress ; //
        }
    }
    else
    {
        if ( debugger->Key == 'u' ) // step o(u)t of the native code like a non-native subroutine
        {
            _Printf ( ( byte* ) "\nstepping thru and out of a \"native\" subroutine" ) ;
            Compile_Call ( debugger->DebugAddress ) ; // 5 : sizeof call insn with offset
            // !?!? this may need work !?!?
            if ( Stack_Depth ( debugger->DebugStack ) ) newDebugAddress = ( byte* ) _Stack_Pop ( debugger->DebugStack ) ;
            else
            {
                newDebugAddress = 0 ;
            }
        }
        else
        {
            newDebugAddress = debugger->DebugAddress + size ;
            if ( ! GetState ( debugger, DBG_JCC_INSN ) )
            {
                ByteArray_AppendCopy ( debugger->StepInstructionBA, size, debugger->DebugAddress ) ;
            }
            else
            {
                SetState ( debugger, DBG_JCC_INSN, false ) ;
            }
        }
    }
done:
    return newDebugAddress ;
}

#else

byte *
_Debugger_CompileOneInstruction ( Debugger * debugger, byte * jcAddress )
{
    byte * newDebugAddress ;
    int32 size ;
start:
    size = Debugger_Udis_GetInstructionSize ( debugger ) ;
    if ( jcAddress ) // jump or call address
    {
        Word * word = Word_GetFromCodeAddress ( jcAddress ) ; //Word_GetFromCodeAddress_NoAlias ( jcAddress ) ;
        if ( word && ( word != debugger->w_Word ) )
        {
            debugger->w_Word = word ;
            debugger->Token = word->Name ;
            //if ( * debugger->DebugAddress == CALLI32 ) _Word_ShowSourceCode ( word ) ;
        }
        if ( ( ! word ) || ( ! Debugger_CanWeStep ( debugger, word ) ) )//( jcAddress < ( byte* ) svcs->BA_Data ) || ( jcAddress > ( byte* ) svcs->bp_Last ) )
        {
            _Printf ( ( byte* ) "\ncalling thru a non-cfrtil compiled function : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
            Compile_Call ( jcAddress ) ; // this will call jcAddress subroutine and return to our code to be compiled next
            // so that newDebugAddress, below, will be our next stepping insn
            newDebugAddress = debugger->DebugAddress + size ;
        }
        else // step into the the jmp/call insn
        {
            if ( * debugger->DebugAddress == CALLI32 )
            {
                if ( _Q_->Verbosity > 1 ) _Word_ShowSourceCode ( word ) ;
                _Printf ( ( byte* ) "\nstepping into a cfrtil compiled function : %s : .... :>", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
                _Stack_Push ( debugger->DebugStack, ( int32 ) ( debugger->DebugAddress + size ) ) ; // the return address
                // push the return address this time around; next time code at newDebugAddress will be processed
                // when ret is the insn Debugger_StepOneInstruction will handle it 
            }
            // for either jmp/call/jcc ...
            newDebugAddress = jcAddress ;
        }
    }
    else
    {
        newDebugAddress = debugger->DebugAddress + size ;
        if ( ! GetState ( debugger, DBG_JCC_INSN ) )
        {
            ByteArray_AppendCopy ( debugger->StepInstructionBA, size, debugger->DebugAddress ) ;
        }
    }
done:

    return newDebugAddress ;
}
#endif

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
                CfrTil_SyncStackPointers ( ) ;
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
            }
            goto end ;
        }
        else if ( ( * debugger->DebugAddress == JMPI32 ) || ( * debugger->DebugAddress == CALLI32 ) )
        {
            jcAddress = JumpCallInstructionAddress ( debugger->DebugAddress ) ;
            Word * word = Word_GetFromCodeAddress ( jcAddress ) ;
            debugger->CurrentlyRunningWord = word ;
            if ( word && ( word->CProperty & ( DEBUG_WORD ) ) )
            {
                _Printf ( ( byte* ) "\nskipping over %s : at 0x%-8x", word ? ( char* ) c_dd ( word->Name ) : "", debugger->DebugAddress ) ;
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
            int mod = modRm & 192 ; // 192 : CALL_JMP_MOD_RM : RM not inc/dec
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
            debugger->cs_Cpu->Eip = ( uint32 * ) debugger->DebugAddress ;
        }
    }
}

// simply : copy the current insn to a ByteArray buffer along with
// prefix and postfix instructions that restore and
// save the cpu state; then run that ByteArray code buffer

void
Debugger_Step ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    if ( ! GetState ( debugger, DBG_STEPPING ) )
    {
        //debugger->cs_CpuState->State = 0 ;
        if ( word )
        {
            debugger->WordDsp = Dsp ; // by 'eval' we stop debugger->Stepping and //continue thru this word as if we hadn't stepped
            Debugger_CanWeStep ( debugger, word ) ;
            if ( ! GetState ( debugger, DBG_CAN_STEP ) )
            {
                Debugger_Eval ( debugger ) ;
                if ( Compiling )
                {
                    _Printf ( "\nCompiling : Stepping turned off" ) ;
                }
                else
                {
                    _Printf ( "\nStepping turned off for this word : %s%s%s%s",
                        c_ud ( word->S_ContainingNamespace ? word->S_ContainingNamespace->Name : ( byte* ) "<literal> " ),
                        word->S_ContainingNamespace ? ( byte* ) "." : ( byte* ) "", c_du ( word->Name ), GetState ( debugger, DBG_AUTO_MODE ) ? " : automode turned off" : "" ) ;
                }
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
    else
    {
        Debugger_SaveCpuState ( debugger ) ;
        Debugger_CompileAndStepOneInstruction ( debugger ) ;
        //Debugger_AdjustEdi ( debugger, 0, debugger->CurrentlyRunningWord ) ;

        CfrTil_SyncStackPointers ( ) ;

        if ( ( int32 ) debugger->DebugAddress ) // set by StepOneInstruction
        {
            debugger->w_Word = Debugger_GetWordFromAddress ( debugger ) ;
            SetState_TrueFalse ( debugger, DBG_STEPPING, ( DBG_INFO | DBG_MENU | DBG_PROMPT ) ) ;
            debugger->SteppedWord = word ;
        }
        else
        {
            SetState_TrueFalse ( debugger, DBG_PRE_DONE | DBG_STEPPED | DBG_NEWLINE | DBG_PROMPT | DBG_INFO, DBG_STEPPING ) ;
            if ( GetState ( debugger, DBG_DONE ) ) SetState ( _CfrTil_, DEBUG_MODE, false ) ;

            return ;
        }
    }
}

void
_Debugger_SetupStepping ( Debugger * debugger, Word * word, byte * address, byte *name, int32 iflag )
{
    _Printf ( ( byte* ) "\nSetting up stepping : location = %s : debugger->word = \'%s\' : ...", _Context_Location ( _Context_ ), word ? word->Name : ( name ? name : ( byte* ) "" ) ) ;
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
Debugger_SetupStepping ( Debugger * debugger, int32 iflag )
{

    _Debugger_SetupStepping ( debugger, debugger->w_Word, debugger->DebugAddress, 0, iflag ) ;
}

void
CpuState_Compile_SaveStackRegs ( Cpu * cpu )
{

    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Ebp, EBP, ECX ) ;
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Esp, ESP, ECX ) ;
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Esi, ESI, ECX ) ;
    _Compile_Set_CAddress_WithRegValue_ThruReg ( ( byte* ) & cpu->Edi, EDI, ECX ) ;
    _Compile_Get_FromCAddress_ToReg ( ECX, ( byte * ) & cpu->Ecx ) ; // restore our scratch reg
}

void
CpuState_Compile_RestoreStackRegs ( Cpu * cpu, int32 esiEdiFlag )
{
    _Compile_Get_FromCAddress_ToReg_ThruReg ( ESP, ( byte * ) & cpu->Esp, ECX ) ;
    _Compile_Get_FromCAddress_ToReg_ThruReg ( EBP, ( byte * ) & cpu->Ebp, ECX ) ;
    if ( esiEdiFlag )
    {

        _Compile_Get_FromCAddress_ToReg_ThruReg ( ESI, ( byte * ) & cpu->Esi, ECX ) ;
        _Compile_Get_FromCAddress_ToReg_ThruReg ( EDI, ( byte * ) & cpu->Edi, ECX ) ;
    }
    _Compile_Get_FromCAddress_ToReg ( ECX, ( byte * ) & cpu->Ecx ) ; // restore our scratch reg
}

int32
Debugger_SetupReturnStackCopy ( Debugger * debugger, int32 size )
{
    uint32 * esp = debugger->cs_Cpu->Esp ;
    if ( esp )
    {
        uint32 rsc0 ;
        int32 pushedWindow = 32 ;
        if ( ! debugger->ReturnStackCopyPointer )
        {
            rsc0 = ( uint32 ) Mem_Allocate ( size, COMPILER_TEMP ) ;
            rsc0 = rsc0 & 0xfffffffe ;
            debugger->ReturnStackCopyPointer = ( byte* ) ( rsc0 + size - pushedWindow ) ;
            d0 ( _PrintNStackWindow ( ( int32* ) debugger->ReturnStackCopyPointer, "ReturnStackCopy", "RSCP", 8 ) ) ;
        }
        else rsc0 = ( uint32 ) ( debugger->ReturnStackCopyPointer - size + pushedWindow ) ;
        memcpy ( ( byte* ) rsc0, ( ( byte* ) esp ) - size + pushedWindow, size ) ; // 32 : account for useful current stack
        d0 ( _PrintNStackWindow ( ( int32* ) esp, "ReturnStack", "ESP", 32 ) ) ;
        d0 ( _PrintNStackWindow ( ( int32* ) debugger->ReturnStackCopyPointer, "ReturnStackCopy", "RSCP", 64 ) ) ;
        SetState ( debugger, DBG_STACK_OLD, false ) ;
        return true ;
    }

    else return false ;
}

// restore the 'internal running cfrTil' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running cfrTil' cpu state

void
Debugger_Compile_Restore_Runtime_DebuggerCpuState ( Debugger * debugger, int32 setupOff, int32 showFlag ) // restore the running cfrTil cpu state
{
    int32 stackSetupFlag = 0 ;
    //if ( debugger->cs_CpuState->State != CPU_STATE_SAVED ) Compile_Call ( ( byte* ) debugger->SaveCpuState ) ; // save the running cfrTil word cpu state after the insn has executed
    // restore the 'internal running cfrTil' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running cfrTil' cpu state
    Compile_Call ( ( byte* ) debugger->RestoreCpuState ) ; // restore the running cfrTil cpu state
    // we don't have to worry so much about the compiler 'spoiling' our insn with restore 
    if ( setupOff && ( ( ! debugger->ReturnStackCopyPointer ) || GetState ( debugger, DBG_STACK_OLD ) ) )
    {
        stackSetupFlag = Debugger_SetupReturnStackCopy ( debugger, 8 * K ) ;
    }
    // restore the running cfrTil esp/ebp : nb! : esp/ebp were not restored by debugger->RestoreCpuState and are being restore here in the proper context
    if ( stackSetupFlag )
    {
        _Compile_Get_FromCAddress_ToReg_ThruReg ( EBP, ( byte * ) & debugger->ReturnStackCopyPointer - ( ( ( int32 ) debugger->cs_Cpu->Ebp ) - ( ( int32 ) debugger->cs_Cpu->Esp ) ), ECX ) ;
        _Compile_Get_FromCAddress_ToReg_ThruReg ( ESP, ( byte * ) & debugger->ReturnStackCopyPointer, ECX ) ;
        _Compile_Get_FromCAddress_ToReg_ThruReg ( ESI, ( byte * ) & debugger->cs_Cpu->Esi, ECX ) ;
        _Compile_Get_FromCAddress_ToReg_ThruReg ( EDI, ( byte * ) & debugger->cs_Cpu->Edi, ECX ) ;
        _Compile_Get_FromCAddress_ToReg ( ECX, ( byte * ) & debugger->cs_Cpu->Ecx ) ; // restore our scratch reg
    }
#if 1   
    else if ( debugger->cs_Cpu->State && debugger->cs_Cpu->Esp )
    {
        CpuState_Compile_RestoreStackRegs ( debugger->cs_Cpu, 0 ) ;
    }
#endif

    if ( showFlag ) Compile_Call ( ( byte* ) CfrTil_Debugger_State_Show ) ;
}

void
CfrTil_Compile_RestoreCCompileTimeCpuState ( CfrTil * cfrtil, int32 showFlag )
{
    // restore the incoming current C cpu state
    //if ( cfrtil->cs_CpuState->State != CPU_STATE_SAVED ) Compile_Call ( ( byte* ) cfrtil->SaveCpuState ) ; // save incoming current C cpu state
    Compile_Call ( ( byte* ) cfrtil->RestoreCpuState ) ;
    CpuState_Compile_RestoreStackRegs ( cfrtil->cs_Cpu, 1 ) ;

    if ( showFlag ) Compile_Call ( ( byte* ) _CfrTil_CpuState_Show ) ;
}

// restore the 'internal running cfrTil' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running cfrTil' cpu state

void
CfrTil_Compile_SaveCCompileTimeCpuState ( CfrTil * cfrtil, int32 showFlag )
{
    // save the incoming current C cpu state
    Compile_Call ( ( byte* ) cfrtil->SaveCpuState ) ; // save incoming current C cpu state
    CpuState_Compile_SaveStackRegs ( cfrtil->cs_Cpu ) ;

    if ( showFlag ) Compile_Call ( ( byte* ) _CfrTil_CpuState_Show ) ;
}

void
Debugger_Compile_Save_RunTime_DebuggerCpuState ( Debugger * debugger, int32 showFlag )
{
    Compile_Call ( ( byte* ) debugger->SaveCpuState ) ; // save the running cfrTil word cpu state after the insn has executed
    //if ( ! debugger->cs_Cpu->Edi ) Debugger_AdjustEdi ( debugger, _Context_->CurrentlyRunningWord->W_InitialRuntimeDsp ) ;
    CpuState_Compile_SaveStackRegs ( debugger->cs_Cpu ) ;

    if ( showFlag ) Compile_Call ( ( byte* ) CfrTil_Debugger_State_Show ) ;
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
    }
    //if ( ( ! word ) && debugger->Token ) word = Finder_Word_FindUsing ( _Context_->Finder0, debugger->Token, 0 ) ;
    debugger->w_Word = word ;

    return word ;
}

byte *
Debugger_DoJcc ( Debugger * debugger, int32 numOfBytes )
{
    byte * jcAddress = ( numOfBytes == 2 ) ? JccInstructionAddress_2Byte ( debugger->DebugAddress ) : JccInstructionAddress_1Byte ( debugger->DebugAddress ) ;
    int tttn, ttt, n ;
    tttn = ( numOfBytes == 2 ) ? ( debugger->DebugAddress[1] & 0xf ) : ( debugger->DebugAddress[0] & 0xf ) ;
    ttt = ( tttn & 0xe ) >> 1 ;
    n = tttn & 1 ;

    // cf. Intel Software Developers Manual v1. (253665), Appendix B
    // ?? nb. some of this may not be (thoroughly) tested as of 11-14-2011 -- but no known problems after months of usual testing ??
    // setting jcAddress to 0 means we don't branch and just continue with the next instruction
    if ( ttt == BELOW ) // ttt 001
    {
        if ( ( n == 0 ) && ! ( ( uint32 ) debugger->cs_Cpu->EFlags & CARRY_FLAG ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( uint32 ) debugger->cs_Cpu->EFlags & CARRY_FLAG ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == ZERO_TTT ) // ttt 010
    {
        if ( ( n == 0 ) && ! ( ( uint32 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( uint32 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == BE ) // ttt 011 :  below or equal
    {
        if ( ( n == 0 ) && ! ( ( ( uint32 ) debugger->cs_Cpu->EFlags & CARRY_FLAG ) | ( ( uint32 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( ( uint32 ) debugger->cs_Cpu->EFlags & CARRY_FLAG ) | ( ( uint32 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == LESS ) // ttt 110
    {
        if ( ( n == 0 ) && ! ( ( ( uint32 ) debugger->cs_Cpu->EFlags & SIGN_FLAG ) ^ ( ( uint32 ) debugger->cs_Cpu->EFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( ( uint32 ) debugger->cs_Cpu->EFlags & SIGN_FLAG ) ^ ( ( uint32 ) debugger->cs_Cpu->EFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == LE ) // ttt 111
    {
        if ( ( n == 0 ) &&
            ! ( ( ( ( uint32 ) debugger->cs_Cpu->EFlags & SIGN_FLAG ) ^ ( ( uint32 ) debugger->cs_Cpu->EFlags & OVERFLOW_FLAG ) ) | ( ( uint32 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        if ( ( n == 1 ) &&
            ( ( ( ( uint32 ) debugger->cs_Cpu->EFlags & SIGN_FLAG ) ^ ( ( uint32 ) debugger->cs_Cpu->EFlags & OVERFLOW_FLAG ) ) | ( ( uint32 ) debugger->cs_Cpu->EFlags & ZERO_FLAG ) ) )
        {

            jcAddress = 0 ;
        }
    }
    return jcAddress ;
}

void
Debug_ExtraShow ( int32 size, int32 force )
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

