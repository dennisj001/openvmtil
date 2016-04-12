
#include "../includes/cfrtil.h"

void
Debugger_Menu ( )
{
    Printf ( ( byte* )
        "\n(m)enu, so(u)rce, dum(p), (e)val, (d)is, dis(a)ccum dis(A)ccum (r)egisters, (l)ocals, (v)ariables, (I)nfo, (w)dis, s(h)ow"
        "\nsto(P), (S)tate, (c)ontinue, (s)tep, (o)ver, (i)nto, s(t)ack, (z)auto, (V)erbosity, (q)uit, a(B)ort, (U)sing, '\\\' - escape" ) ;
}

void
Debugger_Locals_Show ( Debugger * debugger )
{
    byte *address ;
    Word * word, * word2 ;
    if ( CompileMode )
    {
        Printf ( ( byte* ) c_ad ( "\nLocal variables can be shown only at run time not at Compile time!" ) ) ;
        return ;
    }
    ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
    if ( debugger->Locals ) _Namespace_Clear ( debugger->Locals ) ;
    if ( word = debugger->w_Word )
    {
        Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
        DLNode * node ;
        int32 s, e ;
        byte buffer [ 256 ], * start, * sc = word->SourceCode ;
        compiler->NumberOfParameterVariables = 0 ;
        compiler->NumberOfLocals = 0 ;
        compiler->NumberOfRegisterVariables = 0 ;
        SetState ( debugger, DBG_SKIP_INNER_SHOW, true ) ;
        if ( sc )
        {
            // reconstruct locals code 
            for ( s = 0 ; sc [ s ] && sc [ s ] != '(' ; s ++ ) ;
            if ( sc [ s ] )
            {
                start = & sc [ s + 1 ] ; // right after '(' is how _CfrTil_Parse_LocalsAndStackVariables is set up
                for ( e = s ; sc [ e ] && sc [ e ] != ')' ; e ++ ) ; // end = & sc [ e ] ;
                if ( sc [ e ] )
                {
                    strncpy ( ( char* ) buffer, ( char* ) start, e - s + 1 ) ;
                    buffer [ e - s + 1 ] = 0 ;
                    String_InsertDataIntoStringSlot ( rl->InputLine, rl->ReadIndex, rl->ReadIndex, buffer ) ;
                    debugger->Locals = _CfrTil_Parse_LocalsAndStackVariables ( 1, 1, 0, 0 ) ; // stack variables & debug flags
                }
            }
        }
        SetState ( debugger, DBG_SKIP_INNER_SHOW, false ) ;
        // show value of each local var on Locals list
        char * registerNames [ 8 ] = { ( char* ) "EAX", ( char* ) "ECX", ( char* ) "EDX", ( char* ) "EBX", ( char* ) "ESP", ( char* ) "EBP", ( char* ) "ESI", ( char* ) "EDI" } ;
        debugger->RestoreCpuState ( ) ;
        int32 * fp = ( int32* ) debugger->cs_CpuState->Edi, * dsp = ( int32* ) debugger->cs_CpuState->Esi ;
#if 0        
        if ( ( uint32 ) fp < 0xf0000000 )
        {
            // this is an ad hoc for now : fix me ??
            Debugger_Step ( debugger ) ;
            Debugger_Step ( debugger ) ;
        }
        fp = ( int32* ) debugger->cs_CpuState->Edi, dsp = ( int32* ) debugger->cs_CpuState->Esi ;
#endif        
        if ( ( uint32 ) fp > 0xf0000000 )
        {
            debugger->RestoreCpuState ( ) ;
            Debugger_Registers ( debugger ) ;
            Printf ( ( byte* ) "\nLocal Variables for %s.%s : Frame Pointer = EDI = <0x%08x> = 0x%08x : Stack Pointer = ESI <0x%08x> = 0x%08x",
                c_dd ( word->ContainingNamespace->Name ), c_dd ( word->Name ), ( uint ) fp, fp ? *fp : 0, ( uint ) dsp, dsp ? *dsp : 0 ) ;
            for ( node = DLList_Last ( debugger->Locals->W_List ) ; node ; node = DLNode_Previous ( node ) )
            {
                word = ( Word * ) node ;
                int32 wi = word->RegToUse ;
                if ( word->CType & REGISTER_VARIABLE ) Printf ( ( byte* ) "\nReg   Variable : %-12s : %s : 0x%x", word->Name, registerNames [ word->RegToUse ], _Q_->OVT_CfrTil->cs_CpuState->Registers [ word->RegToUse ] ) ;
                else if ( word->CType & LOCAL_VARIABLE )
                {
                    wi = LocalVarOffset ( word ) ; // 1 ??
                    address = ( byte* ) fp [ wi ] ;
                    word2 = Word_GetFromCodeAddress ( ( byte* ) ( address ) ) ;
                    if ( word2 ) sprintf ( ( char* ) buffer, "< %s.%s >", word2->ContainingNamespace->Name, word2->Name ) ;
                    Printf ( ( byte* ) "\n%-018s : index = +%-2d : <0x%08x> = 0x%08x\t\t%s%s", "Local Variable", wi, fp + wi, fp [ wi ], word->Name, word2 ? ( char* ) buffer : "" ) ;
                }
                else if ( word->CType & PARAMETER_VARIABLE )
                {
                    wi = ParameterVarOffset ( word ) ; // 1 ??
                    address = ( byte* ) fp [ wi ] ;
                    word2 = Word_GetFromCodeAddress ( ( byte* ) ( address ) ) ;
                    if ( word2 ) sprintf ( ( char* ) buffer, "< %s.%s >", word2->ContainingNamespace->Name, word2->Name ) ;
                    Printf ( ( byte* ) "\n%-018s : index = %-2d  : <0x%08x> = 0x%08x\t\t%s%s", "Parameter Variable", wi, fp + wi, fp [ wi ], word->Name, word2 ? ( char* ) buffer : "" ) ;
                }
            }
            Printf ( ( byte * ) "\n" ) ;
        }
        else Printf ( ( byte* ) "\nTry stepping a couple of instructions and try again.\n" ) ;
    }
}

void
Debugger_ShowWrittenCode ( Debugger * debugger, int32 stepFlag )
{
    Context * cntx = _Q_->OVT_Context ;
    ReadLiner * rl = cntx->ReadLiner0 ;
    Word * word = debugger->w_Word ;
    int32 ts = debugger->TokenStart_ReadLineIndex, ln = rl->LineNumber ;
    byte * fn = rl->Filename ;
    if ( IsDebugDontShow ) return ;
    if ( word )
    {
        NoticeColors ;
        if ( ( word->CType & OBJECT_FIELD ) && ( ! ( word->CType & DOT ) ) )
        {
            if ( strcmp ( ( char* ) word->Name, "[" ) && strcmp ( ( char* ) word->Name, "]" ) ) // this block is repeated in arrays.c : make it into a function - TODO
            {
                Word_PrintOffset ( word, 0, 0 ) ;
            }
        }
        //else //if ( GetState ( debugger, DBG_COMPILE_MODE | DBG_FORCE_SHOW_WRITTEN_CODE ) )
        {
            char * b = ( char* ) Buffer_Data ( _Q_->OVT_CfrTil->DebugB ) ;
            char * c = ( char* ) Buffer_Data ( _Q_->OVT_CfrTil->DebugB2 ) ;
            const char * insert ;
            byte * name ;
            int32 change, depthChange ;
            if ( GetState ( debugger, DBG_COMPILE_MODE | DBG_FORCE_SHOW_WRITTEN_CODE ) ) _Debugger_DisassembleWrittenCode ( debugger ) ;
            if ( Debugger_IsStepping ( debugger ) ) change = Dsp - debugger->SaveDsp ;
            else change = Dsp - debugger->WordDsp ;
            depthChange = DataStack_Depth ( ) - debugger->SaveStackDepth ;
            if ( GetState ( debugger, DBG_STACK_CHANGE ) || ( change ) || ( debugger->SaveTOS != TOS ) || ( depthChange ) )
            {
                byte pb_change [ 256 ] ;
                pb_change [ 0 ] = 0 ;

                if ( GetState ( debugger, DBG_STACK_CHANGE ) ) SetState ( debugger, DBG_STACK_CHANGE, false ) ;
                if ( depthChange > 0 ) sprintf ( ( char* ) pb_change, "%d %s%s", depthChange, ( depthChange > 1 ) ? "cells" : "cell", " pushed onto to the stack. " ) ;
                else if ( depthChange ) sprintf ( ( char* ) pb_change, "%d %s%s", - depthChange, ( depthChange < - 1 ) ? "cells" : "cell", " popped off the stack. " ) ;
                if ( debugger->SaveTOS != TOS )
                {
                    sprintf ( ( char* ) c, ( char* ) "0x%x", TOS ) ;
                    sprintf ( ( char* ) b, ( char* ) "TOS at : <0x%08x> : changed to %s.", ( uint ) Dsp, c_dd ( c ) ) ;
                    strcat ( ( char* ) pb_change, ( char* ) b ) ; // strcat ( (char*) _change, cc ( ( char* ) c, &_Q_->Default ) ) ;
                }
                name = word->Name ;
                if ( name ) name = String_ConvertToBackSlash ( name ) ;
                char * achange = ( char* ) pb_change ;
                if ( stepFlag )
                {
                    Word * word = Word_GetFromCodeAddress ( debugger->DebugAddress ) ;
                    if ( ( word ) && ( ( byte* ) word->Definition == debugger->DebugAddress ) )
                    {
                        //SetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, true ) ;
                        insert = "function call" ;
                        if ( achange [0] )
                        {
                            if ( GetState ( debugger, DBG_STEPPING ) ) Printf ( ( byte* ) "\nStack changed by %s at %s %d.%d :> %s <: %s ...\n", insert, fn, ln, ts, c_dd (word->Name), achange ) ;
                            else Printf ( ( byte* ) "\nStack changed by %s at %s %d.%d :> %s <: %s ...\n", insert, fn, ln, ts, c_dd (word->Name), achange ) ;
                        }
                    }
                    else
                    {
                        if ( ( ( * debugger->DebugAddress ) != 0x83 ) && ( ( * debugger->DebugAddress ) != 0x81 ) )// add esi
                        {
                            insert = "instruction" ;
                            if ( achange [0] )
                            {
                                if ( GetState ( debugger, DBG_STEPPING ) ) Printf ( ( byte* ) "\nStack changed by %s at %s %d.%d :> 0x%x <: %s ...\n", insert, fn, ln, ts, ( uint ) debugger->DebugAddress, achange ) ;
                                else Printf ( ( byte* ) "\nStack changed by %s at %s %d.%d :> 0x%x <: %s ...\n", insert, fn, ln, ts, ( uint ) debugger->DebugAddress, achange ) ;
                            }
                        }
                        else SetState ( debugger, DBG_STACK_CHANGE, true ) ;
                    }
                }
                else
                {
                    if ( word )
                    {
                        insert = "word" ;
                    }
                    else
                    {
                        insert = "token" ;
                    }
                    if ( achange [0] )
                    {
                        if ( GetState ( debugger, DBG_STEPPING ) ) Printf ( ( byte* ) "Stack changed by %s at %s %d.%d :> %s <: %s ...\n", insert, fn, ln, ts, c_dd ( name ), achange ) ;
                        else Printf ( ( byte* ) "\nStack changed by %s at %s %d.%d :> %s <: %s ...\n", insert, fn, ln, ts, c_dd ( name ), achange ) ;
                        if ( _Q_->Verbosity > 1 )
                        {
                            Stack ( ) ;
                            //DEBUG_START ;
                        }
                    }
                }
                if ( Lexer_GetState ( _Q_->OVT_Context->Lexer0, KNOWN_OBJECT ) )
                {
                    if ( Dsp > debugger->SaveDsp )
                    {
                        Printf ( ( byte* ) "\nLiteral :> 0x%x <: was pushed onto the stack ...\n", TOS ) ;
                    }
                    else if ( Dsp < debugger->SaveDsp )
                    {
                        Printf ( ( byte* ) "\n%s popped %d value off the stack.\n", insert, ( debugger->SaveDsp - Dsp ) ) ;
                    }
                }
                //if ( ( change > 1 ) || ( change < - 1 ) && 
                if ( debugger->Verbosity > 1 ) CfrTil_PrintDataStack ( ) ;
            }
            //debugger->LastShowWord = word ;
            DebugColors ;
        }
    }
}

char *
_highlightTokenInputLine ( Word * word, byte *token, int32 tokenStart )
{
    char * cc_line = ( char* ) "", *b2 ;

    ReadLiner *rl = _Q_->OVT_Context->ReadLiner0 ;
    if ( rl->InputLine [0] ) // this happens at the end of a file with no newline
    {
        int32 dot = String_Equal ( token, "." ) ;
        if ( word ) word->W_StartCharRlIndex = tokenStart ;
        //if ( word ) word->W_StartCharRlIndex = 0 ;
        byte * b = Buffer_Data ( _Q_->OVT_CfrTil->DebugB ) ;
        byte * b1 = Buffer_Data ( _Q_->OVT_CfrTil->Scratch1B ) ;
        strcpy ( ( char* ) b, ( char* ) rl->InputLine ) ;
        String_RemoveFinalNewline ( b ) ;
        if ( dot ) // why is this necessary?
        {
            if ( b [ tokenStart - 1 ] == '.' ) tokenStart -- ;
            else if ( b [ tokenStart + 1 ] == '.' ) tokenStart ++ ;
        }
        b [ tokenStart ] = 0 ; //- ( dot ? 1 : 0 ) ] = 0 ; // dot ?? what? - ad hoc
        strcpy ( ( char* ) b1, ( char* ) cc ( b, &_Q_->Debug ) ) ;
        strcat ( ( char* ) b1, ( char* ) cc ( token, &_Q_->Notice ) ) ;
        b2 = ( char* ) &b [ tokenStart + strlen ( ( char* ) token ) ] ; // - ( dot ? 1 : 0 ) ) ] ;
        strcat ( ( char* ) b1, ( char* ) cc ( b2, &_Q_->Debug ) ) ; // + strlen ( ( char* ) token ) ] ) ;
        if ( *( b2 + 1 ) < ' ' ) strcat ( ( char* ) b1, ( char* ) cc ( " ", &_Q_->Debug ) ) ;
        cc_line = ( char* ) b1 ;
    }
    return cc_line ;
}

void
_CfrTil_ShowInfo ( Debugger * debugger, byte * prompt, int32 signal )
{
    if ( ( ! debugger->LastShowWord ) || ( debugger->w_Word && ( ! String_Equal ( debugger->w_Word->Name, debugger->LastShowWord->Name ) ) ) )
    {
        byte *location ;
        byte signalAscii [ 128 ] ;
        ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
        Lexer * lexer = _Q_->OVT_Context->Lexer0 ;
        int32 tokenStart = 0 ;

        DebugColors ;
        ConserveNewlines ;
        if ( ! ( _Q_->OVT_Context && _Q_->OVT_Context->Lexer0 ) )
        {
            Throw ( ( byte* ) "\nNo token at _CfrTil_ShowInfo\n", QUIT ) ;
        }
        if ( rl->Filename ) location = rl->Filename ;
        else location = ( byte* ) "<command line>" ;
        if ( ( signal == 11 ) || _Q_->SigAddress ) sprintf ( ( char* ) signalAscii, "\nError : signal " INT_FRMT ":: attempting address : " UINT_FRMT_0x08, signal, ( uint ) _Q_->SigAddress ) ;
        else if ( signal ) sprintf ( ( char* ) signalAscii, "\nError : signal " INT_FRMT " ", signal ) ;

        Word * word = debugger->w_Word ;
        byte * token = word ? word->Name : debugger->Token ;
        //->_Q_->OVT_Context->CurrentRunWord ? 
        //    _Q_->OVT_Context->CurrentRunWord->W_StartCharRlIndex : debugger->w_Word->W_StartCharRlIndex ; //word->W_StartCharRlIndex ;
        if ( word && ( ! token ) )
        {
            token = word->Name ;
        }
        if ( token )
        {
            token = String_ConvertToBackSlash ( token ) ;
            char * cc_Token = ( char* ) cc ( token, &_Q_->Notice ) ;
            char * cc_location = ( char* ) cc ( location, &_Q_->Debug ) ;
            if ( word && ( ! String_Equal ( token, lexer->OriginalToken ) ) )
            {
                if ( GetState ( _Q_->OVT_Context->Compiler0, LC_ARG_PARSING ) )
                {
                    tokenStart = word->W_StartCharRlIndex ; // for _Interpret_PrefixFunction_Until_Token
                    debugger->TokenStart_ReadLineIndex = tokenStart ;
                }
                //else goto next ;
            }
            else tokenStart = debugger->TokenStart_ReadLineIndex ;
            char * cc_line = _highlightTokenInputLine ( word, token, tokenStart ) ;
            next :
            prompt = prompt ? prompt : ( byte* ) "" ;
            DefaultColors ;
            if ( word )
            {
                if ( word->CType & CPRIMITIVE )
                {
                    Printf ( ( byte* ) "\n%s%s:: %s : %03d.%03d : %s :> %s <: cprimitive :> %s <:: " INT_FRMT "." INT_FRMT " ",
                        prompt, signal ? signalAscii : ( byte* ) " ", cc_location, rl->LineNumber, rl->ReadIndex,
                        word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : "no namespace",
                        cc_Token, cc_line, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
                }
                else
                {
                    Printf ( ( byte* ) "\n%s%s:: %s : %03d.%03d : %s :> %s <: 0x%08x :> %s <:: " INT_FRMT "." INT_FRMT " ",
                        prompt, signal ? signalAscii : ( byte* ) " ", cc_location, rl->LineNumber, rl->ReadIndex,
                        word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : ( char* ) "no namespace",
                        cc_Token, ( uint ) word->Definition, cc_line, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
                }
            }
            else
            {
                Printf ( ( byte* ) "\n%s%s:: %s : %03d.%03d : %s :> %s <::> %s <:: " INT_FRMT "." INT_FRMT " ",
                    prompt, signal ? signalAscii : ( byte* ) " ", cc_location, rl->LineNumber, rl->ReadIndex,
                    "<literal>", cc_Token, cc_line, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
            }
        }
        else
        {
            byte * b = Buffer_Data ( _Q_->OVT_CfrTil->DebugB ) ;
            strcpy ( ( char* ) b, ( char* ) rl->InputLine ) ;
            //char * cc_line = ( char* ) c_dd ( ( char* ) String_RemoveFinalNewline ( b ) ) ;
            char * cc_line = ( char* ) String_RemoveFinalNewline ( b ) ;

            Printf ( ( byte* ) "\n%s %s:: %s : %03d.%03d :> %s <:: " INT_FRMT "." INT_FRMT,
                prompt, signal ? signalAscii : ( byte* ) "", location, rl->LineNumber, rl->ReadIndex,
                cc_line, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
        }
        DefaultColors ;
    }
}

void
Debugger_ShowInfo ( Debugger * debugger, byte * prompt, int32 signal )
{
    {
        int32 sif = 0 ;
        ConserveNewlines ;
        if ( ( GetState ( debugger, DBG_INFO ) ) && GetState ( debugger, DBG_STEPPING ) )
        {
            _CfrTil_ShowInfo ( debugger, prompt, signal ) ;
            sif = 1 ;
        }
        if ( ! ( _Q_->OVT_Context && _Q_->OVT_Context->Lexer0 ) )
        {
            Printf ( ( byte* ) "\nSignal Error : signal = %d\n", signal ) ;
            return ;
        }
        if ( ! Debugger_GetState ( _Q_->OVT_CfrTil->Debugger0, DBG_ACTIVE ) )
        {
            debugger->Token = _Q_->OVT_Context->Lexer0->OriginalToken ;
            if ( signal > SIGSEGV ) Debugger_FindUsing ( debugger ) ;
        }
        if ( debugger->w_Word ) debugger->Token = debugger->w_Word->Name ;
        if ( GetState ( debugger, DBG_STEPPING ) )
        {
            Printf ( ( byte* ) "\nDebug Stepping Address : 0x%08x\n", ( uint ) debugger->DebugAddress ) ;
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\n", ( byte* ) "\r" ) ; // the next instruction
        }
        if ( ( ! sif ) && ( ! GetState ( debugger, DBG_STEPPING ) ) && ( GetState ( debugger, DBG_INFO ) ) ) _CfrTil_ShowInfo ( debugger, prompt, signal ) ;
    }
}

void
Debugger_ShowState ( Debugger * debugger, byte * prompt )
{
    ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;
    Word * word = debugger->w_Word ;
    int cflag = 0 ;
    if ( word )
    {
        if ( word->CType & CONSTANT ) cflag = 1 ;
    }
    DebugColors ;
    ConserveNewlines ;
    byte * token = debugger->Token ;
    token = String_ConvertToBackSlash ( token ) ;
    if ( word )
    {
        Printf ( ( byte* ) ( cflag ? "\n%s :: %03d.%03d : %s : <constant> : %s%s%s " : word->ContainingNamespace ? "\n%s :: %03d.%03d : %s : <word> : %s%s%s " : "\n%s :: %03d.%03d : %s : <word?> : %s%s%s " ),
            prompt, rl->LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ),
            // _Q_->CfrTil->Namespaces doesn't have a ContainingNamespace
            word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "",
            word->ContainingNamespace ? ( byte* ) "." : ( byte* ) "", // the dot between
            c_dd ( word->Name ) ) ;
    }
    else if ( token )
    {
        Printf ( ( byte* ) ( cflag ? "\n%s :: %03d.%03d : %s : <constant> :> %s " : "\n%s :: %03d.%03d : %s : <literal> :> %s " ),
            prompt, rl->LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ), c_dd ( token ) ) ;
    }
    else Printf ( ( byte* ) "\n%s :: %03d.%03d : %s : ", prompt, rl->LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ) ) ;
    if ( ! debugger->Key )
    {
        if ( word ) _CfrTil_Source ( word, 0 ) ;
        if ( Debugger_GetState ( debugger, DBG_STEPPING ) )
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r", ( byte* ) "\r" ) ; // current insn
    }
}

void
Debugger_ConsiderAndShowWord ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    SetState ( debugger, DBG_CAN_STEP, false ) ; // debugger->State flag = false ;
    AllowNewlines ;
    if ( word ) // then it wasn't a literal
    {
        byte * name = c_dd ( word->Name ) ;
        if ( ( word->CType & ( CPRIMITIVE | DLSYM_WORD ) ) && ( ! ( CompileMode ) ) )
        {
            if ( word->ContainingNamespace ) Printf ( ( byte* ) "\ncprimitive :> %s.%s <:> 0x%08x <: => evaluating ...", word->ContainingNamespace->Name, name, ( uint ) word->Definition ) ;
        }
        else
        {
            SetState ( debugger, DBG_CAN_STEP, true ) ;
            if ( Debugger_GetState ( debugger, DBG_INTERNAL ) )
            {
                Debugger_Info ( debugger ) ;
                //Printf ( ( byte* ) "\nInternal DebugAddress = :> 0x%08x <: ", ( unsigned int ) debugger->DebugAddress ) ;
            }
            else if ( word->CType & VARIABLE )
            {
                Printf ( ( byte* ) "\nVariable :> %s.%s <: => evaluating ... :> ", word->ContainingNamespace->Name, name ) ;
                SetState ( debugger, DBG_CAN_STEP, false ) ;
            }
            else if ( word->CType & TEXT_MACRO )
            {
                Printf ( ( byte* ) "\nMacro :> %s.%s <: => evaluating ... :> ", word->ContainingNamespace->Name, name ) ;
                SetState ( debugger, DBG_CAN_STEP, false ) ;
            }
            else if ( word->CType & IMMEDIATE )
            {
                Printf ( ( byte* ) "\nImmediate word :> %s.%s <:> 0x%08x <: => evaluating ...",
                    word->ContainingNamespace->Name, name, ( uint ) word->Definition ) ;
            }
            else if ( CompileMode )
            {
                Printf ( ( byte* ) "\nCompileMode :> %s.%s : %s <: not a macro => compiling ...",
                    word->ContainingNamespace->Name, name, Debugger_GetStateString ( debugger ) ) ;
            }
            else
            {
                Printf ( ( byte* ) "\nCompiled word :> %s.%s <:> 0x%08x <: => evaluating ...",
                    word->ContainingNamespace->Name, name, ( uint ) word->Definition ) ;
            }
            _Word_ShowSourceCode ( word ) ;
            DefaultColors ;
        }
    }
    else
    {
        if ( debugger->Token )
        {
            Lexer_ParseObject ( _Q_->OVT_Context->Lexer0, debugger->Token ) ;
            if ( ( Lexer_GetState ( _Q_->OVT_Context->Lexer0, KNOWN_OBJECT ) ) )
            {
                if ( CompileMode )
                {
                    Printf ( ( byte* ) "\nCompileMode :> %s <: literal stack push will be compiled ...", _Q_->OVT_Context->Lexer0->OriginalToken ) ;
                }
                else Printf ( ( byte* ) "\nLiteral :> %s <: will be pushed onto the stack ...", _Q_->OVT_Context->Lexer0->OriginalToken ) ;
            }
        }
    }
}

