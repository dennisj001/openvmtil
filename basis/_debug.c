
#include "../include/cfrtil.h"

// we have the address of a jcc insn 
// get the address it jccs to

byte *
JccInstructionAddress_2Byte ( byte * address )
{
    int32 offset = * ( int32* ) ( address + 2 ) ; // 2 : 2 byte opCode
    byte * jcAddress = address + offset + 6 ; // 6 : sizeof 0f jcc insn - 0x0f8x - includes 2 byte opCode
    return jcAddress ;
}

byte *
JccInstructionAddress_1Byte ( byte * address )
{
    int32 offset = ( int32 ) * ( byte* ) ( address + 1 ) ; // 1 : 1 byte opCode
    byte * jcAddress = address + offset + 2 ; // 2 : sizeof 0f jcc insn - 0x7x - includes 1 byte opCode
    return jcAddress ;
}

// we have the address of a jmp/call insn 
// get the address it jmp/calls to

byte *
JumpCallInstructionAddress ( byte * address )
{
    // calculate jmp or call address
    int offset = * ( int32* ) ( address + 1 ) ; // 1 : 1 byte opCode
    byte * jcAddress = address + offset + 5 ; // 5 : sizeof jmp insn - includes 1 byte opcode
    return jcAddress ;
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
        if ( ( n == 0 ) && ! ( debugger->cs_CpuState->EFlags & CARRY_FLAG ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( debugger->cs_CpuState->EFlags & CARRY_FLAG ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == ZERO_CC ) // ttt 010
    {
        if ( ( n == 0 ) && ! ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == BE ) // ttt 011 :  below or equal
    {
        if ( ( n == 0 ) && ! ( ( debugger->cs_CpuState->EFlags & CARRY_FLAG ) | ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( debugger->cs_CpuState->EFlags & CARRY_FLAG ) | ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == LESS ) // ttt 110
    {
        if ( ( n == 0 ) && ! ( ( debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == LE ) // ttt 111
    {
        if ( ( n == 0 ) &&
            ! ( ( ( debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) | ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        if ( ( n == 1 ) &&
            ( ( ( debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) | ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    return jcAddress ;
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
    //debugger->w_Word = word ;
    return word ;
}

byte *
Debugger_CompileInstruction ( Debugger * debugger, byte * jcAddress )
{
    byte * newDebugAddress ;
    ByteArray * svcs = _Q_CodeByteArray ;
    int32 size = Debugger_Udis_GetInstructionSize ( debugger ) ;
    if ( jcAddress ) // jump or call address
    {
        Word * word = Word_GetFromCodeAddress_NoAlias ( jcAddress ) ;
        if ( word && ( word != debugger->w_Word ) )
        {
            debugger->w_Word = word ;
            debugger->Token = word->Name ;
            if ( * debugger->DebugAddress == CALLI32 ) _Word_ShowSourceCode ( word ) ;
        }
        if ( word && ( word->CProperty & DEBUG_WORD ) )// ( String_Equal ( word->Name, "<dbg>" ) ) )
        {
            if ( GetState ( debugger, DBG_AUTO_MODE ) )
            {
                Printf ( ( byte* ) "\nskipping over '<dbg>' and turning off autoMode : %s : .... :> \n", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
                SetState ( debugger, DBG_AUTO_MODE, false ) ;
            }
            debugger->DebugAddress += size ; // skip the call insn to the next after it
            newDebugAddress = debugger->DebugAddress ; //+ size ;
            debugger->w_Word = Debugger_GetWordFromAddress ( debugger ) ; // so we can have our debugger->w_Word->DebugWordList which is not in word <dbg>
            Set_CompilerSpace ( svcs ) ;
            //goto done ; //return ;
        }
        else if ( debugger->Key == 'o' ) // step thru ("over") the native code like a non-native subroutine
        {
            Printf ( ( byte* ) "\ncalling thru (over) a \"native\" subroutine : %s : .... :> \n", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
            Compile_Call ( jcAddress ) ; // 5 : sizeof call insn with offset
            // !?!? this may need work right in here ... !?!?
            newDebugAddress = debugger->DebugAddress + size ;
        }
        else if ( debugger->Key == 'i' ) // step thru ("over") the native code like a non-native subroutine
        {
            Printf ( ( byte* ) "\nstepping into a \"foreign\" C subroutine : %s : .... :> \n", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
            _Stack_Push ( debugger->DebugStack, ( int32 ) ( debugger->DebugAddress + size ) ) ; // the return address
            newDebugAddress = jcAddress ;
        }
        else if ( debugger->Key == 'u' ) // step o(u)t of the native code like a non-native subroutine
        {
            Printf ( ( byte* ) "\nstepping out of a \"native\" subroutine" ) ;
            Compile_Call ( debugger->DebugAddress ) ; // 5 : sizeof call insn with offset
            // !?!? this may need work !?!?
            if ( Stack_Depth ( debugger->DebugStack ) ) newDebugAddress = ( byte* ) _Stack_Pop ( debugger->DebugStack ) ;
            else
            {
                //siglongjmp 
                newDebugAddress = 0 ;
            }
        }
        else if ( ( word && ( word->CProperty & CPRIMITIVE ) ) && ( ( jcAddress < ( byte* ) svcs->BA_Data ) || ( jcAddress > ( byte* ) svcs->bp_Last ) ) ||
            ( word && ( word->CProperty & DLSYM_WORD ) ) )
        {
            if ( * debugger->DebugAddress == JMPI32 )
            {
                CfrTil_Exception ( MACHINE_CODE_ERROR, 1 ) ; // can't in the current debugger jmp into C compiler code
            }
            else
            {
#if 1               
                newDebugAddress = debugger->DebugAddress + size ;
                Printf ( ( byte* ) "\ncalling thru a \"foreign\" C subroutine : %s : .... :> \n", word ? ( char* ) c_dd ( word->Name ) : "" ) ;
                Compile_Call ( jcAddress ) ; // 5 : sizeof call insn with offset
#else
                _Stack_Push ( debugger->DebugStack, ( int32 ) ( debugger->DebugAddress + size ) ) ; // the return address
                newDebugAddress = jcAddress ;
#endif                
            }
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
#if 1               
                Compile_Call ( jcAddress ) ; //_ByteArray_Here ( debugger->StepInstructionBA ) + size ) ; // 5 : sizeof call insn with offset - call to immediately after this very instruction
                // emulate a call -- all we really needed was its address and to push (above) the return address if necessary - if it was a 'call' instruction
                //newDebugAddress = jcAddress + size ;
                newDebugAddress = debugger->DebugAddress + size ;
#else
                _Stack_Push ( debugger->DebugStack, ( int32 ) ( debugger->DebugAddress + size ) ) ; // the return address
                newDebugAddress = jcAddress ;
#endif                
            }
            else newDebugAddress = jcAddress ; //
        }
    }
    else
    {
        if ( debugger->Key == 'u' ) // step o(u)t of the native code like a non-native subroutine
        {
            Printf ( ( byte* ) "\nstepping thru and out of a \"native\" subroutine" ) ;
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

void
Debugger_fflush ( )
{
    fflush ( stdout ) ;
}

void
Debugger_SetupReturnStackCopy ( Debugger * debugger, int32 size )
{
    byte * rsc0 ;
    int32 pushedWindow = 32 ;
    if ( ! debugger->ReturnStackCopyPointer )
    {
        rsc0 = Mem_Allocate ( size, SESSION ) ;
        debugger->ReturnStackCopyPointer = rsc0 + size - pushedWindow ;
        d0 ( _PrintNStackWindow ( ( int32* ) debugger->ReturnStackCopyPointer, "ReturnStackCopy", "RSCP", 8 ) ) ;
    }
    else rsc0 = debugger->ReturnStackCopyPointer - size + pushedWindow ;
    memcpy ( rsc0, ( byte* ) debugger->cs_CpuState->Esp - size + pushedWindow, size ) ; // 32 : account for useful current stack
    d0 ( _PrintNStackWindow ( ( int32* ) debugger->cs_CpuState->Esp, "ReturnStack", "ESP", 8 ) ) ;
    d0 ( _PrintNStackWindow ( ( int32* ) debugger->ReturnStackCopyPointer, "ReturnStackCopy", "RSCP", 8 ) ) ;
    SetState ( debugger, DBG_STACK_OLD, false ) ;
}

void
Debugger_CompileAndDoInstruction ( Debugger * debugger, byte * jcAddress )
{
    byte * newDebugAddress ;
    int32 stackSetupFlag = 0 ;

    d0 ( debugger->Verbosity = 2 ; ) ; // turn on extra debugging 

    ByteArray * svcs = _Q_CodeByteArray ;
    _ByteArray_ReInit ( debugger->StepInstructionBA ) ; // we are only compiling one insn here so clear our BA before each use
    Set_CompilerSpace ( debugger->StepInstructionBA ) ;

    // save the incoming current C cpu state
    Compile_Call ( ( byte* ) _CfrTil_->SaveCpuState ) ; // save incoming current C cpu state

    // restore the 'internal running cfrTil' cpu state : debugger->cs_CpuState is the 'internal running cfrTil' cpu state
    if ( ( ! debugger->ReturnStackCopyPointer ) || GetState ( debugger, DBG_STACK_OLD ) )
    {
        Debugger_SetupReturnStackCopy ( debugger, 8 * K ) ;
        stackSetupFlag = 1 ;
    }
    // restore the running cfrTil esp/ebp : nb! : esp/ebp were not restored by debugger->RestoreCpuState and are being restore here in the proper context
    if ( stackSetupFlag )
    {
        _Compile_MoveImm_To_Reg ( ESP, ( int32 ) debugger->ReturnStackCopyPointer, CELL ) ;
        _Compile_Move_Reg_To_Reg ( EBP, ESP ) ;
        //Compile_ADDI ( REG, ESP, 0, 4, BYTE ) ; //adjust stack from the Compile_Call to debugger->SaveCpuState which pushed (subtracted from esp) the return address 
    }
    else
    {
        _Compile_MoveMem_To_Reg ( EBP, ( byte * ) & debugger->cs_CpuState->Ebp, EBX, CELL ) ;
        _Compile_MoveMem_To_Reg ( ESP, ( byte * ) & debugger->cs_CpuState->Esp, EBX, CELL ) ;
        //Compile_ADDI ( REG, ESP, 0, 4, BYTE ) ; //adjust stack from the Compile_Call to debugger->SaveCpuState which pushed (subtracted from esp) the return address 
    }
    Compile_ADDI ( REG, ESP, 0, 4, BYTE ) ; //adjust stack from the Compile_Call to debugger->SaveCpuState which pushed (subtracted from esp) the return address 
    Compile_Call ( ( byte* ) debugger->RestoreCpuState ) ; // restore the running cfrTil cpu state

    newDebugAddress = Debugger_CompileInstruction ( debugger, jcAddress ) ; // the single current stepping insn

    Compile_Call ( ( byte* ) debugger->SaveCpuState ) ; // save the running cfrTil word cpu state after the insn has executed

    d0 ( Compile_Call ( ( byte* ) Debugger_fflush ) ) ;

    // restore the incoming current C cpu state
    Compile_Call ( ( byte* ) _CfrTil_->RestoreCpuState ) ;
    _Compile_MoveMem_To_Reg ( EBP, ( byte * ) & _CfrTil_->cs_CpuState->Ebp, EBX, CELL ) ;
    _Compile_MoveMem_To_Reg ( ESP, ( byte * ) & _CfrTil_->cs_CpuState->Esp, EBX, CELL ) ;
    // restore the pre - saved incoming esp, ebp : nb! SaveCpuState saves esp/ebp but RestoreCpuState does not restore them so ...
    //_Compile_PushReg ( EBX ) ; // save the scratch reg
    //_Compile_PopToReg ( EBX ) ; // restore scratch reg
    Compile_ADDI ( REG, ESP, 0, 4, BYTE ) ; //adjust stack from the Compile_Call 

    _Compile_Return ( ) ;

    debugger->SaveDsp = Dsp ;
    debugger->PreHere = Here ;
    debugger->SaveTOS = TOS ;
    debugger->SaveStackDepth = DataStack_Depth ( ) ;
    Set_CompilerSpace ( svcs ) ; // before "do it" in case "do it" calls the compiler
    // do it : step the instruction ...
    d0
        (
    if ( debugger->Verbosity > 1 )
    {
        DebugColors ;
            Printf ( "\n\ndebugger->ReturnStackCopyPointer = " UINT_FRMT_0x08, debugger->ReturnStackCopyPointer ) ;
            //_CpuState_Show ( _CfrTil_->cs_CpuState ) ;
            _CpuState_Show ( debugger->cs_CpuState ) ;
            d0 ( CfrTil_PrintReturnStack ( ) ) ;
            d0 ( _PrintNStackWindow ( ( int32* ) debugger->cs_CpuState->Esp, "ReturnStack", "ESP", 8 ) ) ;
            d0 ( _PrintNStackWindow ( ( int32* ) debugger->ReturnStackCopyPointer, "ReturnStackCopy", "RSCP", 8 ) ) ;
            Printf ( "\n\nCurrentInstruction :: \n" ) ;
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "\n" ) ; // the next instruction
            _Debugger_Disassemble ( debugger, debugger->StepInstructionBA->BA_Data, 128, 1 ) ;
            DefaultColors ;
            Block_PtrCall ( debugger->StepInstructionBA->BA_Data ) ;
            DebugColors ;
            d0 ( CfrTil_PrintReturnStack ( ) ) ;
            d0 ( _PrintNStackWindow ( ( int32* ) debugger->cs_CpuState->Esp, "ReturnStack", "ESP", 8 ) ) ;
            d0 ( _PrintNStackWindow ( ( int32* ) debugger->ReturnStackCopyPointer, "ReturnStackCopy", "RSCP", 8 ) ) ;
            _CpuState_Show ( debugger->cs_CpuState ) ;
            //_CpuState_Show ( _CfrTil_->cs_CpuState ) ;
            Printf ( "\n\n" ) ;
    }
    else
        ) ;
    {
        NoticeColors ;
        Block_PtrCall ( debugger->StepInstructionBA->BA_Data ) ;
    }
    Printf ( "\n" ) ; 
done:
    DebugColors ;
    debugger->DebugAddress = newDebugAddress ;
}

void
Debugger_StepOneInstruction ( Debugger * debugger )
{
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
                if ( GetState ( debugger, DBG_BRK_INIT ) )
                {
                    SetState_TrueFalse ( debugger, DBG_INTERPRET_LOOP_DONE | DBG_STEPPED, DBG_ACTIVE | DBG_BRK_INIT | DBG_STEPPING | DBG_RESTORE_REGS ) ;
                }
                else
                {
                    SetState_TrueFalse ( debugger, DBG_INTERPRET_LOOP_DONE | DBG_STEPPED, DBG_ACTIVE | DBG_STEPPING ) ;
                }
                if ( debugger->w_Word ) SetState ( debugger->w_Word, STEPPED, true ) ;
                debugger->DebugAddress = 0 ;
            }
            goto end ;
        }
        else if ( ( * debugger->DebugAddress == JMPI32 ) || ( * debugger->DebugAddress == CALLI32 ) )
        {
            jcAddress = JumpCallInstructionAddress ( debugger->DebugAddress ) ;
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
            Debugger_CompileAndDoInstruction ( debugger, jcAddress ) ;
            SetState ( debugger, DBG_JCC_INSN, false ) ;
            goto end ;
        }
        else if ( ( ( ( byte* ) debugger->DebugAddress )[0] >> 4 ) == 7 ) // callcc ?
        {
            SetState ( debugger, DBG_JCC_INSN, true ) ;
            jcAddress = Debugger_DoJcc ( debugger, 1 ) ;
            Debugger_CompileAndDoInstruction ( debugger, jcAddress ) ;
            SetState ( debugger, DBG_JCC_INSN, false ) ;
            goto end ;
        }
        Debugger_CompileAndDoInstruction ( debugger, jcAddress ) ;
end:
        if ( debugger->DebugAddress )
        {
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ; // the next instruction
            // keep eip - instruction pointer - up to date ..
            debugger->cs_CpuState->Eip = ( int32 ) debugger->DebugAddress ;
        }
    }
}

void
_CfrTil_ACharacterDump ( char aChar )
{
    if ( isprint ( aChar ) )
    {
        Printf ( ( byte* ) "%c", aChar ) ;
    }
    else Printf ( ( byte* ) "." ) ;
}

void
CfrTil_CharacterDump ( byte * address, int32 number )
{
    int32 i ;
    for ( i = 0 ; i < number ; i ++ )
    {

        _CfrTil_ACharacterDump ( address [ i ] ) ;
    }
    Printf ( ( byte* ) " " ) ;
}

void
_CfrTil_AByteDump ( byte aByte )
{

    Printf ( ( byte* ) "%02x ", aByte ) ;
}

void
CfrTil_NByteDump ( byte * address, int32 number )
{
    int32 i ;
    for ( i = 0 ; i < number ; i ++ )
    {

        _CfrTil_AByteDump ( address [ i ] ) ;
    }
    Printf ( ( byte* ) " " ) ;
}

byte *
GetPostfix ( byte * address, byte* postfix, byte * buffer )
{
    byte * iaddress ;
    char * prePostfix = ( char* ) "\t" ;
    if ( ( * address == JMPI32 ) || ( * address == CALLI32 ) )
    {
        int32 offset = * ( ( int32 * ) ( address + 1 ) ) ;
        iaddress = address + offset + 1 + CELL ;
    }
    else if ( ( ( * address == 0x0f ) && ( ( * ( address + 1 ) >> 4 ) == 0x8 ) ) )
    {
        int32 offset = * ( ( int32 * ) ( address + 2 ) ) ;
        iaddress = address + offset + 2 + CELL ;
    }
    else return postfix ;
    {
        Word * word = Word_GetFromCodeAddress ( iaddress ) ;
        if ( word )
        {
            byte * name = ( byte* ) c_dd ( word->Name ) ; //, &_Q_->Default ) ;
            if ( ( byte* ) word->CodeStart == iaddress )
            {
                snprintf ( ( char* ) buffer, 128, "%s< %s.%s >%s", prePostfix, word->ContainingNamespace->Name, name, postfix ) ;
            }
            else
            {
                snprintf ( ( char* ) buffer, 128, "%s< %s.%s+%d >%s", prePostfix,
                    word->ContainingNamespace->Name, name, iaddress - ( byte* ) word->CodeStart, postfix ) ;
            }
        }
        else snprintf ( ( char* ) buffer, 128, "%s< %s >", prePostfix, ( char * ) "C compiler code" ) ;
        postfix = buffer ;
    }
    return postfix ;
}

#if 0

void
_Compile_Debug_GetESP ( int * where ) // where we want the acquired pointer
{
    // ! nb : x86 cant do rm offset with ESP reg directly so use EAX
    _Compile_Move_Reg_To_Reg ( EAX, ESP ) ;
    _Compile_MoveImm_To_Reg ( ECX, ( int32 ) where, CELL ) ;
    _Compile_Move_Reg_To_Rm ( ECX, EAX, 0 ) ;
}
#else

void
_Compile_Debug_GetESP ( int * where ) // where we want the acquired pointer
{
    // ! nb : x86 cant do rm offset with ESP reg directly so use EAX
    _Compile_MoveImm_To_Reg ( EAX, ( int32 ) where, CELL ) ;
    //_Compile_Move_Reg_To_Reg ( EAX, ESP ) ;
    _Compile_Move_Reg_To_Rm ( EAX, ESP, 0 ) ;
}
#endif

void
Compile_Debug_GetESP ( ) // where we want the acquired pointer
{
    _Compile_Debug_GetESP ( ( int* ) & _Debugger_->DebugESP ) ;
}

void
_Compile_Debug1 ( ) // where we want the acquired pointer
{
    _Compile_Debug_GetESP ( ( int* ) & _Debugger_->DebugESP ) ;
    Compile_Call ( ( byte* ) _Debugger_->SaveCpuState ) ;
    Compile_Call ( ( byte* ) _CfrTil_->SaveCpuState ) ;
    Compile_Call ( ( byte* ) CfrTil_DebugRuntimeBreakpoint ) ;
}

#if 1

void
_Compile_Pause ( ) // where we want the acquired pointer
{
    _Compile_Debug_GetESP ( ( int* ) & _Debugger_->DebugESP ) ;
    Compile_Call ( ( byte* ) _Debugger_->SaveCpuState ) ;
    Compile_Call ( ( byte* ) _CfrTil_->SaveCpuState ) ;
    Compile_Call ( ( byte* ) _OpenVmTil_Pause ) ;
}
#endif
