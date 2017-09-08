
#include "../include/cfrtil.h"

void
Debugger_Menu ( Debugger * debugger )
{
    _Printf ( ( byte* )
        "\nDebug Menu at : %s :\n(m)enu, so(U)rce, dum(p), (e)val, (d)is, dis(a)ccum, dis(A)ccum, (r)egisters, (l)ocals, (v)ariables, (I)nfo, (w)dis, s(h)ow"
        "\n(R)eturnStack, sto(P), (S)tate, (c)ontinue, (s)tep, (o)ver, (i)nto, o(u)t, (t)hru, s(T)ack, auto(z), (V)erbosity, (q)uit, a(B)ort"
        "\nusi(N)g, s(H)ow DebugWordList, sh(O)w CompilerWordList"
        "\n'\\n' - escape, , '\\\' - <esc> - escape, ' ' - <space> - continue", c_dd ( Context_Location ( ) ) ) ;
    SetState ( debugger, DBG_MENU, false ) ;
}

// find and reconstruct locals source code in a buffer and parse it with the regular locals parse code

void
Debugger_ParseFunctionLocalVariables ( Debugger * debugger, Lexer * lexer )
{
    int64 levelBit = 0 ;
    byte * token, *prevToken = 0 ;
    _Compiler_->NumberOfLocals = 0 ;
    levelBit = 0 ;
    while ( token = _Lexer_ReadToken ( lexer, ( byte* ) " ,\n\r\t" ) )
    {
        if ( String_Equal ( token, "{" ) ) levelBit ++ ;
        else if ( String_Equal ( token, "{" ) ) levelBit -- ;
        else if ( String_Equal ( token, "(" ) )
        {
            if ( ! ( debugger->LevelBitNamespaceMap & ( ( uint64 ) 1 << ( levelBit ) ) ) )
            {
                _CfrTil_Parse_LocalsAndStackVariables ( 1, 0, 0, debugger->LocalsNamespacesStack, 0 ) ;
                debugger->LevelBitNamespaceMap |= ( ( uint64 ) 1 << ( levelBit ) ) ;
            }
        }
        else if ( String_Equal ( token, "var" ) )
        {
            if ( ! ( debugger->LevelBitNamespaceMap & ( ( uint64 ) 1 << ( levelBit ) ) ) )
            {
                _Namespace_FindOrNew_Local ( debugger->LocalsNamespacesStack ) ;
                debugger->LevelBitNamespaceMap |= ( ( uint64 ) 1 << ( levelBit ) ) ;
            }
            _CfrTil_LocalWord ( prevToken, ++ _Compiler_->NumberOfLocals, LOCAL_VARIABLE ) ;
        }
        else if ( String_Equal ( token, ";" ) || String_Equal ( token, ")" ) ) return ;
        prevToken = token ;
    }
}

void
_Debugger_Locals_ShowALocal ( Debugger * debugger, Word * localsWord, byte * buffer ) // use a debugger buffer instead ??
{
    int64 varOffset ;
    if ( localsWord->CProperty & LOCAL_VARIABLE ) varOffset = LocalVarOffset ( localsWord ) ;
    else if ( localsWord->CProperty & PARAMETER_VARIABLE ) varOffset = ParameterVarOffset ( localsWord ) ;
    int64 * fp = ( int64* ) debugger->cs_Cpu->Edi ;
    byte * address = ( byte* ) fp [ varOffset ] ;
    byte * stringValue = String_CheckForAtAdddress ( address ) ;
    Word * word2 = Word_GetFromCodeAddress ( ( byte* ) ( address ) ) ;

    if ( word2 ) sprintf ( ( char* ) buffer, "< %s.%s >", word2->ContainingNamespace->Name, word2->Name ) ;
    _Printf ( ( byte* ) "\n%-018s : index = [edi%s0x%-2x]  : <0x%08x> = 0x%08x : %16s.%-16s : %s",
        ( localsWord->CProperty & LOCAL_VARIABLE ) ? "LocalVariable" : "Parameter Variable", ( localsWord->CProperty & LOCAL_VARIABLE ) ? "+" : "-",
        abs ( varOffset * ( sizeof (int64 ) ) ), fp + varOffset, fp [ varOffset ], localsWord->S_ContainingNamespace->Name, localsWord->Name, word2 ? buffer : stringValue ? stringValue : ( byte* ) "" ) ;
}

void
Debugger_Locals_Show ( Debugger * debugger )
{
    Word * scWord = debugger->w_Word ? debugger->w_Word : _CfrTil_->DebugWordListWord ;
    if ( ( ! CompileMode ) && scWord && scWord->W_SourceCode )
    {
        int64 i ;
        Compiler * compiler = _Context_->Compiler0 ;
        Lexer * lexer = Lexer_New ( COMPILER_TEMP ) ;
        ReadLiner * rl = lexer->ReadLiner0 ; //ReadLine_New ( allocType ) ; //_Context_->ReadLiner0 ;
        compiler->NumberOfArgs = 0 ;
        compiler->NumberOfLocals = 0 ;
        compiler->NumberOfRegisterVariables = 0 ; //nb. prevent increasing the locals offset by adding in repeated calls to this function
        //Stack_Init ( debugger->LocalsNamespacesStack ) ;
        byte buffer [ 256 ], * sc = scWord->W_SourceCode ; //, * localScString ; // use a debugger buffer instead ??
        char * registerNames [ 8 ] = { ( char* ) "EAX", ( char* ) "ECX", ( char* ) "EDX", ( char* ) "EBX", ( char* ) "ESP", ( char* ) "EBP", ( char* ) "ESI", ( char* ) "EDI" } ;
        // show value of each local var on Locals list
        int64 * fp = ( int64* ) debugger->cs_Cpu->Edi, * dsp = ( int64* ) debugger->cs_Cpu->Esi ;
        byte * b = Buffer_New_pbyte ( BUFFER_SIZE ) ;
        strcpy ( b, sc ) ;
        sc = b ;
        sc = DelimitSourceCodeStartForLispCfrTil ( sc ) ;
        strcpy ( rl->InputLine, sc ) ;
        //if ( ! debugger->LevelBitNamespaceMap ) 
        debugger->LevelBitNamespaceMap = 0 ;
        Lexer * svLexer = _Context_->Lexer0 ;
        _Context_->Lexer0 = lexer ;
        Debugger_ParseFunctionLocalVariables ( debugger, lexer ) ;
        _Context_->Lexer0 = svLexer ;
        if ( sc && debugger->LocalsNamespacesStack && ( ( uint64 ) fp > 0xf0000000 ) )
        {
            //_Debugger_CpuState_Show ( ) ; // Debugger_Registers is included in Debugger_CpuState_Show
            Debugger_CheckSaveCpuStateShow ( debugger ) ;
            _Printf ( ( byte* ) "\n%s.%s.%s : \nFrame Pointer = EDI = <0x%08lx> = 0x%08lx : Stack Pointer = ESI <0x%08lx> = 0x%08lx",
                c_dd ( scWord->ContainingNamespace->Name ), c_dd ( scWord->Name ), c_dd ( "locals" ),
                ( uint64 ) fp, fp ? *fp : 0, ( uint64 ) dsp, dsp ? *dsp : 0 ) ;
            for ( i = Stack_Depth ( debugger->LocalsNamespacesStack ) ; i >= 0 ; i -- )
            {
                dlnode * node, *nextNode ;
                Namespace * ns = ( Namespace* ) _Stack_Pick ( debugger->LocalsNamespacesStack, i ) ;

                for ( node = dllist_Last ( ns->W_List ) ; node ; node = nextNode )
                {
                    nextNode = dlnode_Previous ( node ) ;
                    scWord = ( Word * ) node ;
                    if ( scWord->CProperty & REGISTER_VARIABLE ) _Printf ( ( byte* ) "\nReg   Variable : %-12s : %s : 0x%lx", scWord->Name,
                        registerNames [ scWord->RegToUse ], _CfrTil_->cs_Cpu->Registers [ scWord->RegToUse ] ) ;
                    else _Debugger_Locals_ShowALocal ( debugger, scWord, buffer ) ;
                }
            }
            _Printf ( "\nNext stepping instruction" ) ; // necessary in some cases
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ;
            _Namespace_FreeNamespacesStack ( debugger->LocalsNamespacesStack ) ;
        }
        else if ( sc && debugger->LocalsNamespacesStack ) _Printf ( ( byte* ) "\nTry stepping a couple of instructions and try again." ) ;
    }
    else
    {
        _Printf ( ( byte* ) c_ad ( "\nLocal variables values can be shown only at run time not when at compile time." ) ) ;
    }
}

int64
Debugger_TerminalLineWidth ( Debugger * debugger )
{
    int64 tw = GetTerminalWidth ( ) ;
    if ( tw > debugger->TerminalLineWidth ) debugger->TerminalLineWidth = tw ;

    return debugger->TerminalLineWidth ;
}

void
Debugger_ShowStackChange ( Debugger * debugger, Word * word, byte * insert, byte * achange )
{
    int64 sl, i = 0 ;
    char *location = ( char* ) Context_Location ( ), * name = ( char* ) c_dd ( word->Name ), *b ;
start:
    b = ( char* ) Buffer_Data_Cleared ( _CfrTil_->DebugB1 ) ;

    if ( GetState ( debugger, DBG_STEPPING ) ) sprintf ( ( char* ) b, "\nStack : %s at %s :> %s <: %s", insert, location, ( char* ) c_dd ( word->Name ), ( char* ) achange ) ;
    else sprintf ( ( char* ) b, "\nStack : %s at %s :> %s <: %s", insert, ( char* ) location, name, ( char* ) achange ) ;
    if ( ( sl = strlen ( ( char* ) b ) ) > Debugger_TerminalLineWidth ( debugger ) )
    {
        location = ( char* ) "..." ;
        if ( ++ i > 1 ) name = ( char* ) "" ;
        if ( i > 2 ) insert = ( byte* ) "" ;
        if ( i > 3 ) achange = ( byte* ) "" ;
        if ( i > 4 ) goto done ;
        else goto start ;
    }
done:
    _Printf ( ( byte* ) "%s", b ) ;

    if ( _Q_->Verbosity > 1 ) Stack ( ) ;
}

void
_Debugger_ShowEffects ( Debugger * debugger, Word * word, int64 stepFlag )
{
    debugger->w_Word = word ;
    //int64 * dsp = DSP ;
    if ( Is_DebugShowOn && ( debugger->w_Word != debugger->LastEffectsWord ) )
    {
        Word * word = debugger->w_Word ;
        if ( ( stepFlag ) || ( word ) && ( word != debugger->LastEffectsWord ) )
        {
            NoticeColors ;
            if ( ( word->CProperty & OBJECT_FIELD ) && ( ! ( word->CProperty & DOT ) ) )
            {
                if ( strcmp ( ( char* ) word->Name, "[" ) && strcmp ( ( char* ) word->Name, "]" ) ) // this block is repeated in arrays.c : make it into a function - TODO
                {
                    Word_PrintOffset ( word, 0, 0 ) ;
                }
            }
            _Debugger_DisassembleWrittenCode ( debugger ) ;
            const char * insert ;
            int64 change, depthChange ;
            if ( Debugger_IsStepping ( debugger ) ) change = Dsp - debugger->SaveDsp ;
            else change = Dsp - debugger->WordDsp ;
            depthChange = DataStack_Depth ( ) - debugger->SaveStackDepth ;
            if ( debugger->WordDsp && ( GetState ( debugger, DBG_STACK_CHANGE ) || ( change ) || ( debugger->SaveTOS != TOS ) || ( depthChange ) ) )
            {
                byte * name, pb_change [ 256 ] ;
                char * b = ( char* ) Buffer_Data ( _CfrTil_->DebugB ) ;
                char * c = ( char* ) Buffer_Data ( _CfrTil_->DebugB2 ) ;
                pb_change [ 0 ] = 0 ;

                if ( GetState ( debugger, DBG_STACK_CHANGE ) ) SetState ( debugger, DBG_STACK_CHANGE, false ) ;
                if ( depthChange > 0 ) sprintf ( ( char* ) pb_change, "%ld %s%s", depthChange, ( depthChange > 1 ) ? "cells" : "cell", " pushed onto to the stack. " ) ;
                else if ( depthChange ) sprintf ( ( char* ) pb_change, "%ld %s%s", - depthChange, ( depthChange < - 1 ) ? "cells" : "cell", " popped off the stack. " ) ;
                if ( debugger->SaveTOS != TOS )
                {
                    sprintf ( ( char* ) c, ( char* ) "0x%lx", ( uint64 ) TOS ) ;
                    sprintf ( ( char* ) b, ( char* ) "TOS at : <0x%08lx> : changed to %s.", ( uint64 ) Dsp, c_dd ( c ) ) ;
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
                        if ( achange [0] ) Debugger_ShowStackChange ( debugger, word, ( byte* ) insert, ( byte* ) achange ) ;
                    }
                    else
                    {
                        if ( ( ( * debugger->DebugAddress ) != 0x83 ) && ( ( * debugger->DebugAddress ) != 0x81 ) )// add esi
                        {
                            insert = "instruction" ;
                            if ( achange [0] ) Debugger_ShowStackChange ( debugger, word, ( byte* ) insert, ( byte* ) achange ) ;
                        }
                        else SetState ( debugger, DBG_STACK_CHANGE, true ) ;
                    }
                }
                else
                {
                    if ( word ) insert = "word" ;
                    else insert = "token" ;
                    if ( achange [0] ) Debugger_ShowStackChange ( debugger, word, ( byte* ) insert, ( byte* ) achange ) ;
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
                //debugger->LastEffectsWord = word ;
            }
            //else debugger->LastEffectsWord = 0 ;
            DebugColors ;
            debugger->LastEffectsWord = word ;
            debugger->LastShowWord = debugger->w_Word ;
        }
    }
    debugger->ShowLine = 0 ;
}

void
Debugger_ShowEffects ( Debugger * debugger, int64 stepFlag )
{

    _Debugger_ShowEffects ( debugger, debugger->w_Word, stepFlag ) ;
}

// the border (aesthetically) surrounds (equally or sliding) on either side of a token, it's string length, in the tvw - available terminal view window space; 
// token slides in the window which is 2 * border + token length 
// |ilw...------ inputLine  ----- |--- border ---|---token---|---  border  ---|------ inputLine -----...ilw| -- ilw : inputLine window

byte * // nvw, lef, leftBorder, nts, token0, rightBorder, ref
_String_HighlightTokenInputLine ( byte * nvw, int64 lef, int64 leftBorder, int64 tokenStart, byte *token, int64 rightBorder, int64 ref, int64 dl )
{
    int64 slt = Strlen ( token ) ; //, slilw = Strlen ( nvw ) ;
    if ( ! GetState ( _Debugger_, DEBUG_SHTL_OFF ) )
    {
        byte * b2 = Buffer_Data_Cleared ( _CfrTil_->DebugB2 ) ;
        byte * b3 = Buffer_Data_Cleared ( _CfrTil_->ScratchB3 ) ;
        // inputLineW is the inputLine line (window) start that we use here
        // we are building our output in b2
        // our scratch buffer is b3
        if ( leftBorder < 0 ) leftBorder = 0 ; // something more precise here with C syntax is needed !?!?
        if ( ! lef )
        {
            strncpy ( ( char* ) b3, ( char* ) nvw, leftBorder ) ;
        }
        else
        {
            strncpy ( ( char* ) b3, " .. ", 4 ) ;
            if ( leftBorder > 4 ) strncat ( ( char* ) b3, ( char* ) &nvw[4], leftBorder - 4 ) ; // 3 : [0 1 2 3]  0 indexed array
        }

        strcpy ( ( char* ) b2, ( char* ) cc ( b3, &_Q_->Debug ) ) ;
        char * ccToken = ( char* ) cc ( token, &_Q_->Notice ) ;
        strcat ( ( char* ) b2, ccToken ) ;

        if ( ! ref ) strcpy ( ( char* ) b3, ( char* ) &nvw[tokenStart + slt - dl] ) ; //, BUFFER_SIZE ) ; // 3 : [0 1 2 3]  0 indexed array
        else
        {

            if ( rightBorder > 4 ) strncpy ( ( char* ) b3, ( char* ) &nvw[tokenStart + slt - dl], rightBorder - 4 ) ; // 4 : strlen " .. " 
            strcat ( ( char* ) b3, " .. " ) ;
        }
        char * ccR = ( char* ) cc ( b3, &_Q_->Debug ) ;
        strcat ( ( char* ) b2, ccR ) ;

        nvw = b2 ;
    }
    return nvw ;
}

byte *
Debugger_ShowSourceCodeLine ( Debugger * debugger, Word * word, byte * token0, int64 twAlreayUsed )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    int64 slt = Strlen ( token0 ) ;

    // NB!! : remember the highlighting formatting characters don't add any additional *length* to *visible* the output line
    char * nvw = ( char* ) Buffer_Data_Cleared ( _CfrTil_->DebugB ) ; // nvw : new view window
    char * il = ( char* ) String_New ( rl->InputLineString, TEMPORARY ) ; //nb! dont tamper with the input line. eg. removing its newline will affect other code which depends on newline
    int64 totalBorder, idealBorder, leftBorder, rightBorder, lef, ref, tvw, nws, ots = word->W_StartCharRlIndex, nts ;
    // ots : original token start (index into the source code), nws : new window start ; tvw: targetViewWidth ; nts : new token start
    // lef : left ellipsis flag, ref : right ellipsis flag
    const int64 fel = 32 - 1 ; //fe : formatingEstimate length : 2 formats with 8/12 chars on each sude - 32/48 :: 1 : a litte leave way
    int64 tw = Debugger_TerminalLineWidth ( debugger ) ; // 139 ; //139 : nice width :: Debugger_TerminalLineWidth ( debugger ) ; 
    d0 ( if ( _Q_->Verbosity > 2 ) _Printf ( ( byte* ) "\nTerminal Width = %d\n", tw ) ) ;
    tvw = tw - ( twAlreayUsed - fel ) ; //subtract the formatting chars which don't add to visible length
    int64 i = 0, slil = Strlen ( String_RemoveEndWhitespace ( il ) ) ;
    ots = String_FindStrnCmpIndex ( il, token0, &i, ots, slt, 20 ) ;
    totalBorder = ( tvw - slt ) ; // the borders allow us to slide token within the window of tvw
    // try to get nts relatively the same as ots
    idealBorder = ( totalBorder / 2 ) ;
    leftBorder = rightBorder = idealBorder ; // tentatively set leftBorder/rightBorder as ideally equal
    nws = ots - idealBorder ;
    nts = idealBorder ;
    if ( nws < 0 )
    {
        nws = 0 ;
        nts = leftBorder = ots ;
        rightBorder = totalBorder - leftBorder ;
    }
    else if ( ( ots + slt + idealBorder ) > slil )
    {
        nws = slil - tvw ;
        rightBorder = slil - ( ots + slt ) ; // keep all on right beyond token - the cutoff is on the left side
        if ( nws < 0 )
        {
            nws = 0 ;
            rightBorder += ( tvw - slil ) ;
        }
        leftBorder = totalBorder - rightBorder ;
        nts = leftBorder ;
    }
    Strncpy ( nvw, &il[nws], tvw ) ; // copy the the new view window to buffer nvw
    int64 slb = Strlen ( nvw ) ;
    if ( slb > ( tvw + 8 ) ) // is there a need for ellipsis
    {
        if ( ( ots - leftBorder ) < 4 ) lef = 0, ref = 1 ;
        else lef = ref = 1 ;
    }
    else if ( slb > ( tvw + 4 ) ) // is there a need for one ellipsis
    {
        if ( ( ots - leftBorder ) < 4 ) lef = 0, ref = 1 ;
        else lef = 1, ref = 0 ; // choose lef as preferable
    }
    else lef = ref = 0 ;
    byte * cc_line = ( word ? _String_HighlightTokenInputLine ( nvw, lef, leftBorder, nts, token0, rightBorder, ref, 0 ) : ( byte* ) "" ) ; // nts : new token start is a index into b - the nwv buffer

    return cc_line ;
}

void
_CfrTil_ShowInfo ( Debugger * debugger, byte * prompt, int64 signal, int64 force )
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
            sprintf ( ( char* ) signalAscii, ( char * ) "\nError : signal " INT_FRMT ":: attempting address : " UINT_FRMT_0x08, signal, ( uint64 ) _Q_->SigAddress ) ;
            debugger->DebugAddress = _Q_->SigAddress ;
            //Debugger_Dis ( debugger ) ;
        }
        else if ( signal ) sprintf ( ( char* ) signalAscii, ( char * ) "\nError : signal " INT_FRMT " ", signal ) ;

        if ( token0 )
        {
            byte * obuffer = Buffer_Data_Cleared ( _CfrTil_->DebugB1 ) ;
            token1 = String_ConvertToBackSlash ( token0 ) ;
            token0 = token1 ;
            char * cc_Token = ( char* ) cc ( token0, &_Q_->Notice ) ;
            char * cc_location = ( char* ) cc ( location, &_Q_->Debug ) ;
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
                    sprintf ( obuffer, "\n%s%s:: %s : %03ld.%03ld : %s :> %s <: cprimitive :> ", // <:: " INT_FRMT "." INT_FRMT " ",
                        prompt, signal ? ( char* ) signalAscii : " ", cc_location, rl->LineNumber, rl->ReadIndex,
                        word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : "<literal>",
                        cc_Token ) ; //, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
                }
                else
                {
                    sprintf ( obuffer, "\n%s%s:: %s : %03ld.%03ld : %s :> %s <: 0x%08lx :> ", // <:: " INT_FRMT "." INT_FRMT " ",
                        prompt, signal ? ( char* ) signalAscii : " ", cc_location, rl->LineNumber, rl->ReadIndex,
                        word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : ( char* ) "<literal>",
                        ( char* ) cc_Token, ( uint64 ) word->Definition ) ; //, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
                }
                byte * cc_line = Debugger_ShowSourceCodeLine ( debugger, word, token0, ( int64 ) Strlen ( obuffer ) ) ;
                String_RemoveEndWhitespace ( cc_line ) ;
                Strncat ( obuffer, cc_line, BUFFER_SIZE ) ;
                _Printf ( ( byte* ) "%s", obuffer ) ;
            }
            else
            {
                _Printf ( ( byte* ) "\n%s%s:: %s : %03ld.%03ld : %s :> %s <::> ", // <:: " INT_FRMT "." INT_FRMT " ",
                    prompt, signal ? signalAscii : ( byte* ) " ", cc_location, rl->LineNumber, rl->ReadIndex,
                    "<literal>", cc_Token ) ; //, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
            }
        }
        else
        {
            char * cc_line = ( char* ) Buffer_Data ( _CfrTil_->DebugB ) ;
            strcpy ( cc_line, ( char* ) rl->InputLine ) ;
            String_RemoveEndWhitespace ( ( byte* ) cc_line ) ;

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
Debugger_ShowInfo ( Debugger * debugger, byte * prompt, int64 signal )
{
    if ( ( ! debugger->w_Word ) || debugger->w_Word != debugger->LastShowWord )
    {
        Context * cntx = _Context_ ;
        int64 sif = 0 ;
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
            _Printf ( ( byte* ) "\nDebug Stepping Address : 0x%08lx", ( uint64 ) debugger->DebugAddress ) ;
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
    int64 cflag = 0 ;
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
            if ( word->ContainingNamespace ) _Printf ( ( byte* ) "\ncprimitive :> %s.%s <:> 0x%08x <: => evaluating ...", word->ContainingNamespace->Name, name, ( uint64 ) word->Definition ) ;
        }
        else
        {
            SetState ( debugger, DBG_CAN_STEP, true ) ;
            if ( GetState ( debugger, DBG_INTERNAL ) )
            {
                Debugger_Info ( debugger ) ;
                //_Printf ( ( byte* ) "\nInternal DebugAddress = :> 0x%08x <: ", ( uint64 ) debugger->DebugAddress ) ;
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
                    word->ContainingNamespace->Name, name, ( uint64 ) word->Definition ) ;
            }
            else if ( CompileMode )
            {
                _Printf ( ( byte* ) "\nCompileMode :> %s.%s : %s <: not a macro => compiling ...",
                    word->ContainingNamespace->Name, name, Debugger_GetStateString ( debugger ) ) ;
            }
            else
            {
                _Printf ( ( byte* ) "\nCompiled word :> %s.%s <:> 0x%08x <: => evaluating ...",
                    word->ContainingNamespace->Name, name, ( uint64 ) word->Definition ) ;
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
    //Debugger_InitDebugWordList ( debugger ) ;
}

void
LO_Debug_ExtraShow ( int64 showStackFlag, int64 verbosity, int64 wordList, byte *format, ... )
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

byte *
DelimitSourceCodeStartForLispCfrTil ( char * sc )
{
    byte * start = 0 ;
    while ( *sc )
    {
        switch ( *sc )
        {
            case '"':
            {
                sc ++ ;
                while ( *sc != '"' ) sc ++ ;
                break ;
            }
            case ':':
            {
                if ( *( sc + 1 ) == ':' )
                {
                    sc ++ ;
                    break ;
                }
                else
                {
                    start = sc ;
                    goto next ;
                }
            }
        }
        sc ++ ;
    }
next:
    while ( *sc )
    {
        switch ( *sc )
        {
            case '"':
            {
                while ( *sc ++ != '"' ) ;
                sc ++ ;
                break ;
            }
            case ';':
            {
                if ( *( sc + 1 ) == ';' )
                {
                    sc += 2 ;
                }
                *( sc + 1 ) = ' ' ;
                *( sc + 2 ) = 0 ;
                return start ;
            }
        }
        sc ++ ;
    }
    return start ;
}



