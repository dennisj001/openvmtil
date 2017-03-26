
#include "../include/cfrtil.h"

void
_Debugger_CompileAndStepOneInstruction ( Debugger * debugger, byte * jcAddress )
{
    byte * newDebugAddress ;

    ByteArray * svcs = _Q_CodeByteArray ;
    _ByteArray_ReInit ( debugger->StepInstructionBA ) ; // we are only compiling one insn here so clear our BA before each use
    Set_CompilerSpace ( debugger->StepInstructionBA ) ;

    byte * svHere = Here ;
    if ( _Q_->Verbosity > 3 )
    {
        Compile_Call ( ( byte* ) CfrTil_Debugger_Registers ) ;
    }
    CfrTil_Compile_SaveCompileTimeDebuggerCpuState ( _CfrTil_ ) ;
    Debugger_Compile_Restore_Runtime_DebuggerCpuState ( debugger, 1 ) ;

    newDebugAddress = Debugger_CompileInstruction ( debugger, jcAddress ) ; // the single current stepping insn

    Debugger_Compile_Save_RunTime_DebuggerCpuState ( debugger ) ;
    CfrTil_Compile_RestoreCompileTimeDebuggerCpuState ( _CfrTil_ ) ;
    if ( _Q_->Verbosity > 3 )
    {
        Compile_Call ( ( byte* ) CfrTil_Debugger_ShowCpuState ) ;
    }

    _Compile_Return ( ) ;
    debugger->SaveDsp = Dsp ;
    //debugger->PreHere = Here ;
    debugger->SaveTOS = TOS ;
    debugger->SaveStackDepth = DataStack_Depth ( ) ;
    byte * svHere2 = Here ;
    Set_CompilerSpace ( svcs ) ; // before "do it" in case "do it" calls the compiler

    if ( _Q_->Verbosity > 3 ) Debug_ExtraShow ( svHere2 - svHere ) ;
    
    if ( ! sigsetjmp ( debugger->JmpBuf0, 0 ) )
    {
        _Debugger_DoStepOneInstruction ( debugger ) ;
        //( ( VoidFunction ) debugger->StepInstructionBA->BA_Data ) ( ) ;
    }
    DebugColors ;
    debugger->DebugAddress = newDebugAddress ;
}

byte *
Debugger_CompileInstruction ( Debugger * debugger, byte * jcAddress )
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
        if ( ! Debugger_CanWeStep ( debugger ) ) //( jcAddress < ( byte* ) svcs->BA_Data ) || ( jcAddress > ( byte* ) svcs->bp_Last ) )
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

void
Debugger_StepOneInstruction ( Debugger * debugger )
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
                    CfrTil_Debugger_ShowCpuState ( ) ;
                    Pause ( ) ;
                }
            }
            goto end ;
        }
        else if ( ( * debugger->DebugAddress == JMPI32 ) || ( * debugger->DebugAddress == CALLI32 ) )
        {
            jcAddress = JumpCallInstructionAddress ( debugger->DebugAddress ) ;
            Word * word = Word_GetFromCodeAddress_NoAlias ( jcAddress ) ;
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
            if ( mod == 192 ) jcAddress = ( byte* ) _Debugger_->cs_CpuState->Eax ;
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
            debugger->cs_CpuState->Eip = ( uint32 * ) debugger->DebugAddress ;
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
            Debugger_CanWeStep ( debugger ) ;
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
        Debugger_StepOneInstruction ( debugger ) ;

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
Debugger_SetupStepping ( Debugger * debugger, int32 iflag )
{
    _Printf ( ( byte* ) "\nSetting up stepping : location = %s : debugger->word = \'%s\' : ...", _Context_Location ( _Context_ ), debugger->w_Word->Name ) ;
    _CfrTil_Source ( debugger->w_Word, 0 ) ;
    if ( ( ! debugger->DebugAddress ) || ( ! GetState ( debugger, ( DBG_BRK_INIT ) ) ) ) debugger->DebugAddress = ( byte* ) debugger->w_Word->Definition ;
    SetState_TrueFalse ( debugger, DBG_STEPPING, DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_MENU ) ;
    if ( iflag )
    {
        _Printf ( "\nNext stepping instruction" ) ; // necessary in some cases
        Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ;
    }
    debugger->SaveDsp = Dsp ; // saved before we start stepping
}

// restore the 'internal running cfrTil' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running cfrTil' cpu state

void
Debugger_Compile_Restore_Runtime_DebuggerCpuState ( Debugger * debugger, int32 setupOff ) // restore the running cfrTil cpu state
{
    // restore the 'internal running cfrTil' cpu state which was saved after the last instruction : debugger->cs_CpuState is the 'internal running cfrTil' cpu state
    Compile_Call ( ( byte* ) debugger->RestoreCpuState ) ; // restore the running cfrTil cpu state
    // we don't have to worry so much about the compiler 'spoiling' our insn with restore 
    //_Compile_CpuState_Restore ( debugger->cs_CpuState ) ;
#if 1   
    {
        int32 stackSetupFlag = 0 ;
        if ( setupOff && ( ( ! debugger->ReturnStackCopyPointer ) || GetState ( debugger, DBG_STACK_OLD ) ) )
        {
            stackSetupFlag = Debugger_SetupReturnStackCopy ( debugger, 8 * K ) ;
        }
        // restore the running cfrTil esp/ebp : nb! : esp/ebp were not restored by debugger->RestoreCpuState and are being restore here in the proper context
        if ( stackSetupFlag )
        {
            _Compile_MoveImm_To_Reg ( ESP, ( int32 ) debugger->ReturnStackCopyPointer, CELL ) ;
            _Compile_MoveImm_To_Reg ( EBP, ( int32 ) debugger->ReturnStackCopyPointer - ( debugger->cs_CpuState->Ebp - debugger->cs_CpuState->Esp ), CELL ) ; // ??
        }
        else if ( debugger->cs_CpuState->Esp )
        {
            _Compile_MoveMem_To_Reg ( EBP, ( byte * ) & debugger->cs_CpuState->Ebp, EBX, CELL ) ;
            _Compile_MoveMem_To_Reg ( ESP, ( byte * ) & debugger->cs_CpuState->Esp, EBX, CELL ) ;
        }
    }
#endif    
    //_Compile_MoveAddressValueToReg_ThruReg ( ESI, ( byte* ) & debugger->Esi, EBX ) ;
    //_Compile_MoveAddressValueToReg_ThruReg ( EDI, ( byte* ) & debugger->Edi, EBX ) ;
#if 0   
    if ( ( debugger->cs_CpuState->State ) && ( debugger->cs_CpuState->Edi < debugger->cs_CpuState->Esi ) ) //0xf7fc96ac ) )
    {
        if ( _Q_->Verbosity > 2 )
        {
            _Printf ( "\nSave_RunTime_DebuggerCpuState : Esi = 0x%08x : Dsp = 0x%08x : Edi = 0x%08x : at :: ", debugger->cs_CpuState->Esi, Dsp, debugger->cs_CpuState->Edi ) ;
            if ( debugger->DebugAddress )
            {
                Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\t", ( byte* ) "" ) ; // the next instruction
            }
        }
        if ( ( ! debugger->SaveEdi ) )
        {
            debugger->cs_CpuState->Edi = debugger->cs_CpuState->Esi ;
            debugger->SaveEdi = debugger->cs_CpuState->Edi ;
        }
        else debugger->cs_CpuState->Edi = debugger->SaveEdi ; // nb. pointer arithmetic
        //debugger->cs_CpuState->Edi = debugger->SaveEdi ; //debugger->cs_CpuState->Esi + 1 ; // nb. pointer arithmetic
        if ( _Q_->Verbosity > 2 )
        {
            _Printf ( "\nSave_RunTime_DebuggerCpuState : Esi = 0x%08x : Dsp = 0x%08x : Edi = 0x%08x : after fix?!?", debugger->cs_CpuState->Esi, Dsp, debugger->cs_CpuState->Edi ) ;
        }
    }
    //#else    
    if ( ( ( uint32 ) debugger->cs_CpuState->Esi < 0xf7fc96ac ) || ( ( uint32 ) debugger->cs_CpuState->Edi < 0xf7fc96ac ) )
    {
        _Printf ( "\nSave_RunTime_DebuggerCpuState : Esi = 0x%08x : Dsp = 0x%08x : Edi = 0x%08x", debugger->cs_CpuState->Esi, Dsp, debugger->cs_CpuState->Edi ) ;
        if ( debugger->DebugAddress )
        {
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ; // the next instruction
        }
    }
    if ( ( ( uint32 ) debugger->cs_CpuState->Edi < 0xf7fc96ac ) ) debugger->cs_CpuState->Edi = debugger->cs_CpuState->Esi + 1 ; // nb. pointer arithmetic
#endif    
}

void
Debugger_Compile_Save_RunTime_DebuggerCpuState ( Debugger * debugger ) // restore the running cfrTil cpu state
{
    Compile_Call ( ( byte* ) debugger->SaveCpuState ) ; // save the running cfrTil word cpu state after the insn has executed
    //_Compile_CpuState_Save ( debugger->cs_CpuState ) ;
}

void
CfrTil_Compile_SaveCompileTimeDebuggerCpuState ( CfrTil * cfrtil )
{
    // save the incoming current C cpu state
    Compile_Call ( ( byte* ) cfrtil->SaveCpuState ) ; // save incoming current C cpu state
    //_Compile_CpuState_Save ( cfrtil->cs_CpuState ) ; // 
    _Compile_PushReg ( EBX ) ; // scratch reg0
    _Compile_MoveRegToAddress_ThruReg ( ( byte* ) & cfrtil->cs_CpuState->Ebp, EBP, EBX ) ;
    _Compile_MoveRegToAddress_ThruReg ( ( byte* ) & cfrtil->cs_CpuState->Esp, ESP, EBX ) ;
    _Compile_PopToReg ( EBX ) ; // restore scratch reg0
}

void
CfrTil_Compile_RestoreCompileTimeDebuggerCpuState ( CfrTil * cfrtil )
{
    // restore the incoming current C cpu state
    Compile_Call ( ( byte* ) cfrtil->RestoreCpuState ) ;
    //_Compile_CpuState_Restore ( cfrtil->cs_CpuState ) ;
    _Compile_PushReg ( EBX ) ; // scratch reg0
    _Compile_MoveAddressValueToReg_ThruReg ( EBP, ( byte* ) & cfrtil->cs_CpuState->Ebp, EBX ) ;
    _Compile_MoveAddressValueToReg_ThruReg ( ESP, ( byte* ) & cfrtil->cs_CpuState->Esp, EBX ) ;
    _Compile_PopToReg ( EBX ) ; // restore scratch reg0
}

int32
Debugger_CanWeStep ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    if ( ( ! word ) || ( Compiling || ( word->CProperty & ( IMMEDIATE | CPRIMITIVE | DLSYM_WORD ) ) || ( word->LProperty & T_LISP_DEFINE ) ) ) //|| ( CompileMode && ( ! ( word->CProperty & IMMEDIATE ) ) ) )
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
        word = Word_GetFromCodeAddress_NoAlias ( debugger->DebugAddress ) ;
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
        if ( ( n == 0 ) && ! ( ( uint32 ) debugger->cs_CpuState->EFlags & CARRY_FLAG ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( uint32 ) debugger->cs_CpuState->EFlags & CARRY_FLAG ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == ZERO_TTT ) // ttt 010
    {
        if ( ( n == 0 ) && ! ( ( uint32 ) debugger->cs_CpuState->EFlags & ZERO_FLAG ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( uint32 ) debugger->cs_CpuState->EFlags & ZERO_FLAG ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == BE ) // ttt 011 :  below or equal
    {
        if ( ( n == 0 ) && ! ( ( ( uint32 ) debugger->cs_CpuState->EFlags & CARRY_FLAG ) | ( ( uint32 ) debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( ( uint32 ) debugger->cs_CpuState->EFlags & CARRY_FLAG ) | ( ( uint32 ) debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == LESS ) // ttt 110
    {
        if ( ( n == 0 ) && ! ( ( ( uint32 ) debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( ( uint32 ) debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( ( uint32 ) debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( ( uint32 ) debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == LE ) // ttt 111
    {
        if ( ( n == 0 ) &&
            ! ( ( ( ( uint32 ) debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( ( uint32 ) debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) | ( ( uint32 ) debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        if ( ( n == 1 ) &&
            ( ( ( ( uint32 ) debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( ( uint32 ) debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) | ( ( uint32 ) debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    return jcAddress ;
}

void
_Debugger_DoStepOneInstruction ( Debugger * debugger )
{
    ( ( VoidFunction ) debugger->StepInstructionBA->BA_Data ) ( ) ;
}

int32
Debugger_SetupReturnStackCopy ( Debugger * debugger, int32 size )
{
    if ( debugger->cs_CpuState->Esp )
    {
        byte * rsc0 ;
        int32 pushedWindow = 32 ;
        if ( ! debugger->ReturnStackCopyPointer )
        {
            rsc0 = Mem_Allocate ( size, COMPILER_TEMP ) ;
            debugger->ReturnStackCopyPointer = rsc0 + size - pushedWindow ;
            d0 ( _PrintNStackWindow ( ( int32* ) debugger->ReturnStackCopyPointer, "ReturnStackCopy", "RSCP", 8 ) ) ;
        }
        else rsc0 = debugger->ReturnStackCopyPointer - size + pushedWindow ;
        memcpy ( rsc0, ( byte* ) debugger->cs_CpuState->Esp - size + pushedWindow, size ) ; // 32 : account for useful current stack
        d0 ( _PrintNStackWindow ( ( int32* ) debugger->cs_CpuState->Esp, "ReturnStack", "ESP", 8 ) ) ;
        d0 ( _PrintNStackWindow ( ( int32* ) debugger->ReturnStackCopyPointer, "ReturnStackCopy", "RSCP", 8 ) ) ;
        SetState ( debugger, DBG_STACK_OLD, false ) ;
        return true ;
    }
    else return false ;
}

void
Debug_ExtraShow ( int32 size )
{
    Debugger * debugger = _Debugger_ ;
    //if ( _Q_->Verbosity > 3 )
    {
        if ( _Q_->Verbosity > 4 )
        {
            _Printf ( "\n\ndebugger->RestoreCpuState" ) ;
            _Debugger_Disassemble ( debugger, ( byte* ) debugger->RestoreCpuState, 88, 0 ) ;
            _Printf ( "\n\ndebugger->SaveCpuState" ) ;
            _Debugger_Disassemble ( debugger, ( byte* ) debugger->SaveCpuState, 101, 0 ) ;
        }
        if ( _Q_->Verbosity > 3 )
        {
            _Printf ( "\n\ndebugger->StepInstructionBA->BA_Data" ) ;
            _Debugger_Disassemble ( debugger, ( byte* ) debugger->StepInstructionBA->BA_Data, size, 0 ) ;
        }
    }
}
