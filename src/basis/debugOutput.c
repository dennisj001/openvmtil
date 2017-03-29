
#include "../include/cfrtil.h"

void
Debugger_Menu ( Debugger * debugger )
{
    _Printf ( ( byte* )
        "\nDebug Menu :\n(m)enu, so(u)rce, dum(p), (e)val, (d)is, dis(a)ccum, dis(A)ccum, (r)egisters, (l)ocals, (v)ariables, (I)nfo, (w)dis, s(h)ow"
        "\n(R)eturnStack, sto(P), (S)tate, (c)ontinue, (s)tep, (o)ver, (i)nto, s(t)ack, auto(z), (V)erbosity, (q)uit, a(B)ort, (U)sing"
        "\n'\\n' - escape, , '\\\' - <esc> - escape, ' ' - <space> - continue" ) ;
    SetState ( debugger, DBG_MENU, false ) ;
}

void
Debugger_Locals_Show ( Debugger * debugger )
{
    byte *address ;
    Word * word, * word2 ;
    if ( CompileMode )
    {
        _Printf ( ( byte* ) c_ad ( "\nLocal variables values can be shown only at run time not when compiling - at compile time." ) ) ;
        //return ;
    }
    //if ( debugger->Locals )
    {
        ReadLiner * rl = _Context_->ReadLiner0 ;
        _Namespace_Clear ( debugger->Locals ) ;
        word = debugger->w_Word ;
        if ( ! word ) word = debugger->DebugWordListWord ;
        if ( word ) //= debugger->w_Word )
        {
            Compiler * compiler = _Context_->Compiler0 ;
            dlnode * node, *nextNode ;
            int32 s, e ;
            byte localsScBuffer [ 256 ], * start, * sc = word->SourceCode ;
            if ( sc )
            {
                compiler->NumberOfArgs = 0 ;
                compiler->NumberOfLocals = 0 ;
                compiler->NumberOfRegisterVariables = 0 ;
                // reconstruct locals code 
                for ( s = 0 ; sc [ s ] && sc [ s ] != '(' ; s ++ ) ;
                if ( sc [ s ] )
                {
                    start = & sc [ s + 1 ] ; // right after '(' is how _CfrTil_Parse_LocalsAndStackVariables is set up
                    for ( e = s ; sc [ e ] && sc [ e ] != ')' ; e ++ ) ; // end = & sc [ e ] ;
                    if ( sc [ e ] )
                    {
                        Strncpy ( ( char* ) localsScBuffer, ( char* ) start, e - s + 1 ) ;
                        localsScBuffer [ e - s + 1 ] = 0 ;
                        String_InsertDataIntoStringSlot ( rl->InputLine, rl->ReadIndex, rl->ReadIndex, localsScBuffer ) ;
                        debugger->Locals = _CfrTil_Parse_LocalsAndStackVariables ( 1, 0, 0, 1 ) ; // stack variables & debug flags
                    }
                }
            }
            // show value of each local var on Locals list
            char * registerNames [ 8 ] = { ( char* ) "EAX", ( char* ) "ECX", ( char* ) "EDX", ( char* ) "EBX", ( char* ) "ESP", ( char* ) "EBP", ( char* ) "ESI", ( char* ) "EDI" } ;
            int32 * fp = ( int32* ) debugger->cs_CpuState->Edi, * dsp = ( int32* ) debugger->cs_CpuState->Esi ;
            if ( sc && debugger->Locals && (( uint32 ) fp > 0xf0000000 ) )
            {
                _Debugger_CpuState_Show ( ) ; // Debugger_Registers is included in Debugger_CpuState_Show
                _Printf ( ( byte* ) "Local Variables for %s.%s %s%s : \nFrame Pointer = EDI = <0x%08x> = 0x%08x : Stack Pointer = ESI <0x%08x> = 0x%08x",
                    c_dd ( word->ContainingNamespace->Name ), c_dd ( word->Name ), c_dd ( "(" ), c_dd ( localsScBuffer ), 
                    ( uint ) fp, fp ? *fp : 0, ( uint ) dsp, dsp ? *dsp : 0 ) ;
                byte buffer [64] ;
                Strncpy ( buffer, word->Name, 64) ; Strncat ( buffer, ".locals", 64 ) ;
                Namespace * ns = Namespace_FindOrNew_SetUsing ( buffer, _CfrTil_->Namespaces, 1 ) ;
                for ( node = dllist_Last ( debugger->Locals->W_List ) ; node ; node = nextNode )
                {
                    nextNode = dlnode_Previous ( node ) ;
                    word = ( Word * ) node ;
                    
                    int32 wi = word->RegToUse ;
                    if ( word->CProperty & REGISTER_VARIABLE ) _Printf ( ( byte* ) "\nReg   Variable : %-12s : %s : 0x%x", word->Name, registerNames [ word->RegToUse ], _CfrTil_->cs_CpuState->Registers [ word->RegToUse ] ) ;
                    else if ( word->CProperty & LOCAL_VARIABLE )
                    {
                        wi = LocalVarOffset ( word ) ;
                        address = ( byte* ) fp [ wi ] ;
                        word2 = Word_GetFromCodeAddress ( ( byte* ) ( address ) ) ; // Finder_Address_FindInOneNamespace ( _Context_->Finder0, debugger->Locals, address ) ; 
                        if ( word2 ) sprintf ( ( char* ) localsScBuffer, "< %s.%s >", word2->ContainingNamespace->Name, word2->Name ) ;
                        //_Printf ( ( byte* ) "\n%-018s : index = EDI [ %-2d ] : <0x%08x> = 0x%08x\t\t%s%s", "Local Variable", wi * ( sizeof (int ) ), fp + wi, fp [ wi ], word->Name, word2 ? ( char* ) localsScBuffer : "" ) ;
                        _Printf ( ( byte* ) "\n%-018s : index = EDI [ %-2d ] : <0x%08x> = 0x%08x\t\t%s%s", "Local Variable", wi, fp + wi, fp [ wi ], word->Name, word2 ? ( char* ) localsScBuffer : "" ) ;
                    }
                    else if ( word->CProperty & PARAMETER_VARIABLE )
                    {
                        wi = ParameterVarOffset ( word ) ;
                        address = ( byte* ) fp [ wi ] ;
                        word2 = Word_GetFromCodeAddress ( ( byte* ) ( address ) ) ; //Finder_Address_FindInOneNamespace ( _Context_->Finder0, debugger->Locals, address ) ; 
                        if ( word2 ) sprintf ( ( char* ) localsScBuffer, "< %s.%s >", word2->ContainingNamespace->Name, word2->Name ) ;
                        //_Printf ( ( byte* ) "\n%-018s : index = EDI [ -%-2d ]  : <0x%08x> = 0x%08x\t\t%s%s", "Parameter Variable", wi * ( sizeof (int ) ), fp + wi, fp [ wi ], word->Name, word2 ? ( char* ) localsScBuffer : "" ) ;
                        _Printf ( ( byte* ) "\n%-018s : index = EDI [ -%-2d ]  : <0x%08x> = 0x%08x\t\t%s%s", "Parameter Variable", wi, fp + wi, fp [ wi ], word->Name, word2 ? ( char* ) localsScBuffer : "" ) ;
                    }
                    dlnode_Remove ( (dlnode*) word ) ;
                    _Namespace_DoAddWord ( ns, word ) ;
                }
                
                _Printf ( ( byte * ) "\n" ) ;
            }
            else if (sc && debugger->Locals) _Printf ( ( byte* ) "\nTry stepping a couple of instructions and try again." ) ;
        }
    }
}

void
Debugger_ShowEffects ( Debugger * debugger, int32 stepFlag )
{
    if ( Is_DebugShow && ( debugger->w_Word != debugger->LastEffectsWord ) )
    {
        Word * word = debugger->w_Word ;
        if ( ( stepFlag ) || ( word ) && ( word != debugger->LastEffectsWord ) )
        {
            Context * cntx = _Context_ ;
            ReadLiner * rl = cntx->ReadLiner0 ;
            int32 ts = debugger->TokenStart_ReadLineIndex, ln = rl->LineNumber ;
            byte * fn = rl->Filename ;
            NoticeColors ;
            if ( ( word->CProperty & OBJECT_FIELD ) && ( ! ( word->CProperty & DOT ) ) )
            {
                if ( strcmp ( ( char* ) word->Name, "[" ) && strcmp ( ( char* ) word->Name, "]" ) ) // this block is repeated in arrays.c : make it into a function - TODO
                {
                    Word_PrintOffset ( word, 0, 0 ) ;
                }
            }
            char * b = ( char* ) Buffer_Data ( _CfrTil_->DebugB ) ;
            char * c = ( char* ) Buffer_Data ( _CfrTil_->DebugB2 ) ;
            const char * insert ;
            byte * name ;
            int32 change, depthChange ;
            //if ( GetState ( debugger, DBG_COMPILE_MODE | DBG_FORCE_SHOW_WRITTEN_CODE ) ) 
            _Debugger_DisassembleWrittenCode ( debugger ) ;
            if ( Debugger_IsStepping ( debugger ) ) change = Dsp - debugger->SaveDsp ;
            else change = Dsp - debugger->WordDsp ;
            depthChange = DataStack_Depth ( ) - debugger->SaveStackDepth ;
            if ( debugger->WordDsp && ( GetState ( debugger, DBG_STACK_CHANGE ) || ( change ) || ( debugger->SaveTOS != TOS ) || ( depthChange ) ) )
            {
                byte pb_change [ 256 ] ;
                pb_change [ 0 ] = 0 ;

                if ( GetState ( debugger, DBG_STACK_CHANGE ) ) SetState ( debugger, DBG_STACK_CHANGE, false ) ;
                if ( depthChange > 0 ) sprintf ( ( char* ) pb_change, "%d %s%s", depthChange, ( depthChange > 1 ) ? "cells" : "cell", " pushed onto to the stack. " ) ;
                else if ( depthChange ) sprintf ( ( char* ) pb_change, "%d %s%s", - depthChange, ( depthChange < - 1 ) ? "cells" : "cell", " popped off the stack. " ) ;
                if ( debugger->SaveTOS != TOS )
                {
                    sprintf ( ( char* ) c, ( char* ) "0x%x", ( uint ) TOS ) ;
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
                        insert = "function call" ;
                        if ( achange [0] )
                        {
                            if ( GetState ( debugger, DBG_STEPPING ) ) _Printf ( ( byte* ) "\nStack changed by %s at %s %d.%d :> %s <: %s ...", insert, fn, ln, ts, c_dd ( word->Name ), achange ) ;
                            else _Printf ( ( byte* ) "\nStack changed by %s at %s %d.%d :> %s <: %s ...\n", insert, fn, ln, ts, c_dd ( word->Name ), achange ) ;
                        }
                    }
                    else
                    {
                        if ( ( ( * debugger->DebugAddress ) != 0x83 ) && ( ( * debugger->DebugAddress ) != 0x81 ) )// add esi
                        {
                            insert = "instruction" ;
                            if ( achange [0] )
                            {
                                if ( GetState ( debugger, DBG_STEPPING ) ) _Printf ( ( byte* ) "\nStack changed by %s at %s %d.%d :> 0x%x <: %s ...", insert, fn, ln, ts, ( uint ) debugger->DebugAddress, achange ) ;
                                else _Printf ( ( byte* ) "\nStack changed by %s at %s %d.%d :> 0x%x <: %s ...\n", insert, fn, ln, ts, ( uint ) debugger->DebugAddress, achange ) ;
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
                        if ( GetState ( debugger, DBG_STEPPING ) ) _Printf ( ( byte* ) "Stack changed by %s at %s %d.%d :> %s <: %s ...", insert, fn, ln, ts, c_dd ( name ), achange ) ;
                        else _Printf ( ( byte* ) "\nStack changed by %s at %s %d.%d :> %s <: %s ...", insert, fn, ln, ts, c_dd ( name ), achange ) ;
                        if ( _Q_->Verbosity > 1 )
                        {
                            Stack ( ) ;
                        }
                    }
                }
                if ( GetState ( _Context_->Lexer0, KNOWN_OBJECT ) )
                {
                    if ( Dsp > debugger->SaveDsp )
                    {
                        _Printf ( ( byte* ) "\nLiteral :> 0x%x <: was pushed onto the stack ...", TOS ) ;
                    }
                    else if ( Dsp < debugger->SaveDsp )
                    {
                        _Printf ( ( byte* ) "\n%s popped %d value off the stack.", insert, ( debugger->SaveDsp - Dsp ) ) ;
                    }
                }
                if ( ( change > 1 ) || ( change < - 1 ) || ( _Q_->Verbosity > 1 ) )
                {
                    CfrTil_PrintDataStack ( ) ;
                }
                debugger->LastEffectsWord = word ;
            }
            else debugger->LastEffectsWord = 0 ;
            DebugColors ;
            debugger->LastShowWord = debugger->w_Word ;
        }
    }
    debugger->ShowLine = 0 ;
}

char *
_String_HighlightTokenInputLine ( Word * word, byte *token )
{
    ReadLiner *rl = _Context_->ReadLiner0 ;
    int32 tokenStart = word->W_StartCharRlIndex, inc = 20 ;
    char * b = ( char* ) Buffer_Data ( _CfrTil_->DebugB ) ;
    Strncpy ( b, rl->InputLine, BUFFER_SIZE ) ;
    String_RemoveFinalNewline ( ( byte * ) b ) ;
    char * cc_line = b, *b2 ;
    if ( ! GetState ( _Debugger_, DEBUG_SHTL_OFF ) )
    {
        if ( rl->InputLine [0] ) // this happens at the end of a file with no newline
        {
            byte * b1 = Buffer_Data ( _CfrTil_->DebugB2 ) ;
#if 1            
            // this code is also used in PrepareSourceCodeString in cfrtil.c 
            // it makes or attempts to make sure that that tokenStart is correct for any string
            int32 i = 0, wl0 = Strlen ( token ) ;
            int32 index = String_FindStrnCmpIndex ( b, token, &i, tokenStart, wl0, inc ) ;
            if ( i < ( wl0 + inc ) ) tokenStart = index ;
#endif            
#if 1            
            else //the old method ( 806.270 ) used here as a fall back ; this block may not be necessary at this point
            {
                if ( String_Equal ( token, "." ) ) // why is this necessary?
                {
                    if ( b [ tokenStart - 1 ] == '.' ) tokenStart -- ;
                    else if ( b [ tokenStart + 1 ] == '.' ) tokenStart ++ ;
                }
            }
#endif    
            b [ tokenStart ] = 0 ;
            Strncpy ( ( char* ) b1, ( char* ) cc ( b, &_Q_->Debug ), BUFFER_SIZE ) ;
            Strncat ( ( char* ) b1, ( char* ) cc ( token, &_Q_->Notice ), BUFFER_SIZE ) ;
            b2 = ( char* ) &b [ tokenStart + Strlen ( token ) ] ;
            Strncat ( ( char* ) b1, ( char* ) cc ( b2, &_Q_->Debug ), BUFFER_SIZE ) ;
            if ( *( b2 + 1 ) < ' ' ) Strncat ( ( char* ) b1, ( char* ) cc ( " ", &_Q_->Debug ), BUFFER_SIZE ) ;
            cc_line = ( char* ) b1 ;
        }
    }
    return cc_line ;
}

void
_CfrTil_ShowInfo ( Debugger * debugger, byte * prompt, int32 signal, int32 force )
{
    if ( force || ( ! debugger->LastShowWord ) || ( debugger->w_Word != debugger->LastShowWord ) )
    {
        Context * cntx = _Context_ ;
        byte *location ;
        byte signalAscii [ 128 ] ;
        ReadLiner * rl = cntx->ReadLiner0 ;
        char * compileOrInterpret = ( char* ) ( CompileMode ? "[c]" : "[i]" ), buffer [32] ;

        DebugColors ;
        if ( ! ( cntx && cntx->Lexer0 ) )
        {
            Throw ( ( byte* ) "\nNo token at _CfrTil_ShowInfo\n", QUIT ) ;
        }
        if ( rl->Filename ) location = rl->Filename ;
        else location = ( byte* ) "<command line>" ;
        if ( ( location == debugger->Filename ) && ( GetState ( debugger, DBG_FILENAME_LOCATION_SHOWN ) ) ) location = ( byte * ) "..." ;
        SetState ( debugger, DBG_FILENAME_LOCATION_SHOWN, true ) ;
        Word * word = debugger->w_Word ;
        byte * token0 = word ? word->Name : debugger->Token, *token1 ;
        if ( ( signal == 11 ) || _Q_->SigAddress )
        {
            sprintf ( ( char* ) signalAscii, ( char * ) "\nError : signal " INT_FRMT ":: attempting address : " UINT_FRMT_0x08, signal, ( uint ) _Q_->SigAddress ) ;
            debugger->DebugAddress = _Q_->SigAddress ;
            //Debugger_Dis ( debugger ) ;
        }
        else if ( signal ) sprintf ( ( char* ) signalAscii, ( char * ) "\nError : signal " INT_FRMT " ", signal ) ;

        if ( token0 )
        {
            token1 = String_ConvertToBackSlash ( token0 ) ;
            token0 = token1 ;
            debugger->ShowLine = ( byte* ) ( word ? _String_HighlightTokenInputLine ( word, token0 ) : "" ) ;
            char * cc_Token = ( char* ) cc ( token0, &_Q_->Notice ) ;
            char * cc_location = ( char* ) cc ( location, &_Q_->Debug ) ;
            char * cc_line = ( char* ) String_RemoveFinalNewline ( debugger->ShowLine ) ;
next:
            if ( signal ) AlertColors ;
            else DebugColors ;
            prompt = prompt ? prompt : ( byte* ) "" ;
            strcpy ( ( char* ) buffer, ( char* ) prompt ) ; //, BUFFER_SIZE ) ;
            strcat ( buffer, compileOrInterpret ) ; //, BUFFER_SIZE ) ;
            prompt = ( byte* ) buffer ;
            if ( word )
            {
                if ( word->CProperty & CPRIMITIVE )
                {
                    _Printf ( ( byte* ) "\n%s%s:: %s : %03d.%03d : %s :> %s <: cprimitive :> %s", // <:: " INT_FRMT "." INT_FRMT " ",
                        prompt, signal ? signalAscii : ( byte* ) " ", cc_location, rl->LineNumber, rl->ReadIndex,
                        word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : "<literal>",
                        cc_Token, cc_line ) ; //, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
                }
                else
                {
                    _Printf ( ( byte* ) "\n%s%s:: %s : %03d.%03d : %s :> %s <: 0x%08x :> %s", // <:: " INT_FRMT "." INT_FRMT " ",
                        prompt, signal ? signalAscii : ( byte* ) " ", cc_location, rl->LineNumber, rl->ReadIndex,
                        word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : ( char* ) "<literal>",
                        cc_Token, ( uint ) word->Definition, cc_line ) ; //, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
                }
            }
            else
            {
                _Printf ( ( byte* ) "\n%s%s:: %s : %03d.%03d : %s :> %s <::> %s", // <:: " INT_FRMT "." INT_FRMT " ",
                    prompt, signal ? signalAscii : ( byte* ) " ", cc_location, rl->LineNumber, rl->ReadIndex,
                    "<literal>", cc_Token, cc_line ) ; //, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
            }
        }
        else
        {
            byte * b = Buffer_Data ( _CfrTil_->DebugB ) ;
            strcpy ( ( char* ) b, ( char* ) rl->InputLine ) ;
            char * cc_line = ( char* ) String_RemoveFinalNewline ( b ) ;

            _Printf ( ( byte* ) "\n%s %s:: %s : %03d.%03d :> %s", // <:: " INT_FRMT "." INT_FRMT,
                prompt, signal ? signalAscii : ( byte* ) "", location, rl->LineNumber, rl->ReadIndex,
                cc_line ) ; //, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
        }
        DefaultColors ;
        if ( ! String_Equal ( "...", location ) ) debugger->Filename = location ;
    }
    else SetState ( _Debugger_, DBG_AUTO_MODE_ONCE, true ) ;
}

void
Debugger_ShowInfo ( Debugger * debugger, byte * prompt, int32 signal )
{
    if ( ( ! debugger->w_Word ) || debugger->w_Word != debugger->LastShowWord )
    {
        Context * cntx = _Context_ ;
        int32 sif = 0 ;
        if ( ( GetState ( debugger, DBG_INFO ) ) && GetState ( debugger, DBG_STEPPING ) )
        {
            _CfrTil_ShowInfo ( debugger, prompt, signal, 0 ) ;
            sif = 1 ;
        }
        if ( ! ( cntx && cntx->Lexer0 ) )
        {
            _Printf ( ( byte* ) "\nSignal Error : signal = %d\n", signal ) ;
            return ;
        }
        if ( ! GetState ( _Debugger_, DBG_ACTIVE ) )
        {
            debugger->Token = cntx->Lexer0->OriginalToken ;
            Debugger_FindUsing ( debugger ) ;
        }
        else if ( debugger->w_Word ) debugger->Token = debugger->w_Word->Name ;
        if ( ( signal != SIGSEGV ) && GetState ( debugger, DBG_STEPPING ) )
        {
            _Printf ( ( byte* ) "\nDebug Stepping Address : 0x%08x", ( uint ) debugger->DebugAddress ) ;
            //if ( _Q_->RestartCondition && (_Q_->RestartCondition < INITIAL_START) ) 
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ; // the next instruction
        }
        if ( ( ! sif ) && ( ! GetState ( debugger, DBG_STEPPING ) ) && ( GetState ( debugger, DBG_INFO ) ) ) _CfrTil_ShowInfo ( debugger, prompt, signal, 0 ) ;
        if ( prompt == _Q_->ExceptionMessage ) _Q_->ExceptionMessage = 0 ;
    }
}

void
Debugger_ShowState ( Debugger * debugger, byte * prompt )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    Word * word = debugger->w_Word ;
    int cflag = 0 ;
    if ( word )
    {
        if ( word->CProperty & CONSTANT ) cflag = 1 ;
    }
    DebugColors ;
    byte * token = debugger->Token ;
    token = String_ConvertToBackSlash ( token ) ;
    if ( word )
    {
        _Printf ( ( byte* ) ( cflag ? "\n%s :: %03d.%03d : %s : <constant> : %s%s%s " : word->ContainingNamespace ? "\n%s :: %03d.%03d : %s : <word> : %s%s%s " : "\n%s :: %03d.%03d : %s : <word?> : %s%s%s " ),
            prompt, rl->LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ),
            // _Q_->CfrTil->Namespaces doesn't have a ContainingNamespace
            word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "",
            word->ContainingNamespace ? ( byte* ) "." : ( byte* ) "", // the dot between
            c_dd ( word->Name ) ) ;
    }
    else if ( token )
    {
        _Printf ( ( byte* ) ( cflag ? "\n%s :: %03d.%03d : %s : <constant> :> %s " : "\n%s :: %03d.%03d : %s : <literal> :> %s " ),
            prompt, rl->LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ), c_dd ( token ) ) ;
    }
    else _Printf ( ( byte* ) "\n%s :: %03d.%03d : %s : ", prompt, rl->LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ) ) ;
    if ( ! debugger->Key )
    {
        if ( word ) _CfrTil_Source ( word, 0 ) ;
        if ( GetState ( debugger, DBG_STEPPING ) )
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r", ( byte* ) "" ) ; // current insn
    }
}

void
Debugger_ConsiderAndShowWord ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    SetState ( debugger, DBG_CAN_STEP, false ) ; // debugger->State flag = false ;
    if ( word ) // then it wasn't a literal
    {
        byte * name = c_dd ( word->Name ) ;
        if ( ( word->CProperty & ( CPRIMITIVE | DLSYM_WORD ) ) && ( ! ( CompileMode ) ) )
        {
            if ( word->ContainingNamespace ) _Printf ( ( byte* ) "\ncprimitive :> %s.%s <:> 0x%08x <: => evaluating ...", word->ContainingNamespace->Name, name, ( uint ) word->Definition ) ;
        }
        else
        {
            SetState ( debugger, DBG_CAN_STEP, true ) ;
            if ( GetState ( debugger, DBG_INTERNAL ) )
            {
                Debugger_Info ( debugger ) ;
                //_Printf ( ( byte* ) "\nInternal DebugAddress = :> 0x%08x <: ", ( unsigned int ) debugger->DebugAddress ) ;
            }
            else if ( word->CProperty & NAMESPACE_VARIABLE )
            {
                _Printf ( ( byte* ) "\nVariable :> %s.%s <: => evaluating ... :> ", word->ContainingNamespace->Name, name ) ;
                SetState ( debugger, DBG_CAN_STEP, false ) ;
            }
            else if ( word->CProperty & TEXT_MACRO )
            {
                _Printf ( ( byte* ) "\nMacro :> %s.%s <: => evaluating ... :> ", word->ContainingNamespace->Name, name ) ;
                SetState ( debugger, DBG_CAN_STEP, false ) ;
            }
            else if ( word->CProperty & IMMEDIATE )
            {
                _Printf ( ( byte* ) "\nImmediate word :> %s.%s <:> 0x%08x <: => evaluating ...",
                    word->ContainingNamespace->Name, name, ( uint ) word->Definition ) ;
            }
            else if ( CompileMode )
            {
                _Printf ( ( byte* ) "\nCompileMode :> %s.%s : %s <: not a macro => compiling ...",
                    word->ContainingNamespace->Name, name, Debugger_GetStateString ( debugger ) ) ;
            }
            else
            {
                _Printf ( ( byte* ) "\nCompiled word :> %s.%s <:> 0x%08x <: => evaluating ...",
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
            Lexer_ParseObject ( _Context_->Lexer0, debugger->Token ) ;
            if ( ( GetState ( _Context_->Lexer0, KNOWN_OBJECT ) ) )
            {
                if ( CompileMode )
                {
                    _Printf ( ( byte* ) "\nCompileMode :> %s <: literal stack push will be compiled ...", _Context_->Lexer0->OriginalToken ) ;
                }
                else _Printf ( ( byte* ) "\nLiteral :> %s <: will be pushed onto the stack ...", _Context_->Lexer0->OriginalToken ) ;
            }
        }
    }
}

void
_Debugger_DoNewlinePrompt ( Debugger * debugger )
{
    _Printf ( ( byte* ) "\n" ) ; //%s=> ", GetState ( debugger, DBG_RUNTIME ) ? ( byte* ) "<dbg>" : ( byte* ) "dbg" ) ; //, (char*) ReadLine_GetPrompt ( _Context_->ReadLiner0 ) ) ;
    Debugger_SetNewLine ( debugger, false ) ;
}

void
_Debugger_DoState ( Debugger * debugger )
{
    if ( GetState ( debugger, DBG_RETURN ) )
    {
        _Printf ( ( byte* ) "\r" ) ;
        SetState ( debugger, DBG_RETURN, false ) ;
    }
    if ( GetState ( debugger, DBG_MENU ) )
    {
        Debugger_Menu ( debugger ) ;
        SetState ( debugger, DBG_FILENAME_LOCATION_SHOWN, false ) ;
    }
    if ( GetState ( debugger, DBG_INFO ) ) Debugger_ShowInfo ( debugger, GetState ( debugger, DBG_RUNTIME ) ? ( byte* ) "<dbg>" : ( byte* ) "dbg", 0 ) ;
    else if ( GetState ( debugger, DBG_PROMPT ) ) Debugger_ShowState ( debugger, GetState ( debugger, DBG_RUNTIME ) ? ( byte* ) "<dbg>" : ( byte* ) "dbg" ) ;
    if ( GetState ( debugger, DBG_NEWLINE ) ) _Debugger_DoNewlinePrompt ( debugger ) ;
    Debugger_DebugWordListLogic ( debugger ) ;
}

void
_Debug_ExtraShow ( int32 showStackFlag, int32 verbosity, int32 wordList, byte *format, ... )
{
    if ( GetState ( _CfrTil_, DEBUG_MODE ) )
    {
        if ( _Q_->Verbosity > verbosity )
        {
            va_list args ;
            va_start ( args, ( char* ) format ) ;
            char * out = ( char* ) Buffer_Data ( _CfrTil_->DebugB ) ;
            vsprintf ( ( char* ) out, ( char* ) format, args ) ;
            va_end ( args ) ;
            DebugColors ;
            if ( wordList ) Compiler_Show_WordList ( ( byte* ) out ) ;
            else
            {
                printf ( "%s", out ) ;
                fflush ( stdout ) ;
            }
            if ( showStackFlag && _Q_->Verbosity > verbosity ) Stack ( ) ;
            DefaultColors ;
        }
    }
}


