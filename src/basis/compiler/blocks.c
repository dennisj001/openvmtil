#include "../../include/cfrtil32.h"

void
Byte_PtrCall ( byte * ptr )
{
    ( ( block ) ptr ) ( ) ;
}

void
_Block_Eval ( block block )
{
    Byte_PtrCall ( ( byte * ) block ) ;
}

void
_Block_Copy ( byte * srcAddress, int32 bsize )
{
    byte * saveHere = Here, * saveAddress = srcAddress ;
    ud_t * ud = Debugger_UdisInit ( _Debugger_ ) ;
    int32 isize, left ;

    for ( left = bsize ; left > 0 ; srcAddress += isize )
    {
        isize = _Udis_GetInstructionSize ( ud, srcAddress ) ;
        left -= isize ;
        _CfrTil_AdjustSourceCodeAddress ( srcAddress, Here ) ;
        if ( * srcAddress == _RET )
        {
            if ( left && ( ( * srcAddress + 1 ) != NOOP ) ) //  noop from our logic overwrite
            {
                // ?? unable at present to compile inline with more than one return in the block
                SetHere ( saveHere ) ;
                Compile_Call ( saveAddress ) ;
            }
            break ; // don't include RET
        }
        else if ( * srcAddress == CALLI32 )
        {
            int32 offset = * ( int32* ) ( srcAddress + 1 ) ; // 1 : 1 byte opCode
            if ( ! offset )
            {
                CfrTil_SetupRecursiveCall ( ) ;
                continue ;
            }
            else
            {
                byte * jcAddress = JumpCallInstructionAddress ( srcAddress ) ;
                Word * word = Word_GetFromCodeAddress ( jcAddress ) ;
                if ( word )
                {
                    //_CfrTil_AdjustSourceCodeAddress ( jcAddress, Here ) ;
                    _Word_Compile ( word ) ;
                    continue ;
                }
                //else (drop to) _CompileN ( srcAddress, isize )
            }
        }
        else if ( * srcAddress == JMPI32 )
        {
            int32 offset = * ( int32* ) ( srcAddress + 1 ) ; // 1 : 1 byte opCode
            if ( offset == 0 ) // signature of a goto point
            {
                _CfrTil_MoveGotoPoint ( ( int32 ) srcAddress + 1, 0, ( int32 ) Here + 1 ) ;
                _CompileN ( srcAddress, isize ) ; // memcpy ( dstAddress, address, size ) ;
                continue ;
            }
            //else (drop to) _CompileN ( srcAddress, isize )
        }
        _CompileN ( srcAddress, isize ) ; // memcpy ( dstAddress, address, size ) ;
    }
}

// nb : only blocks with one ret insn can be successfully compiled inline

void
Block_Copy ( byte * dst, byte * src, int32 qsize )
{
    if ( dst > src )
    {
        Error_Abort ( ( byte* ) "\nBlock_Copy :: Error : dst > src.\n" ) ;
        //return ; // ?? what is going on here ??
    }
    SetHere ( dst ) ;
    _Block_Copy ( src, qsize ) ;
}

int32
Block_CopyCompile_WithLogicFlag ( byte * srcAddress, int32 bindex, int32 jccFlag, int negFlag )
{
    Compiler * compiler = _Context_->Compiler0 ;
    int32 jccFlag2 ;
    BlockInfo *bi = ( BlockInfo * ) _Stack_Pick ( compiler->CombinatorBlockInfoStack, bindex ) ; // -1 : remember - stack is zero based ; stack[0] is top
    if ( jccFlag )
    {
        if ( ! ( _Q_->OVT_LC && GetState ( _Q_->OVT_LC, ( LC_COMPILE_MODE ) ) ) )
        {
            if ( bi->LiteralWord )//&& bi->LiteralWord->StackPushRegisterCode ) // leave value in EAX, don't push it
            {
                if ( bi->LiteralWord->W_Value != 0 )
                {
                    return 1 ; // nothing need to be compiled 
                }
                // else somehow don't use this block at all ie. eliminate the dead code and don't just ...
                return 0 ; // TODO : don't use the block/combinator
            }
        }
        jccFlag2 = Compile_CheckReConfigureLogicInBlock ( bi, 1 ) ;
    }
    if ( ! GetState ( _CfrTil_, INLINE_ON ) ) Compile_Call ( srcAddress ) ;
    else
    {
        _Block_Copy ( srcAddress, bi->bp_Last - bi->bp_First ) ;
    }
    if ( jccFlag )
    {
        Word * svcrw = _Context_->CurrentlyRunningWord ;
        _Context_->CurrentlyRunningWord = _Context_->SC_CurrentCombinator ;
        if ( jccFlag2 )
        {
            Compile_JCC ( negFlag ? bi->NegFlag : ! bi->NegFlag, bi->Ttt, 0 ) ;
        }
        else
        {
            Compile_GetLogicFromTOS ( bi ) ;
            Compile_JCC ( negFlag, ZERO_TTT, 0 ) ;
        }
        _Context_->CurrentlyRunningWord = svcrw ;
        _Stack_PointerToJmpOffset_Set ( Here - CELL ) ;
    }
    return 1 ;
}

int32
Block_CopyCompile ( byte * srcAddress, int32 bindex, int32 jccFlag )
{
    return Block_CopyCompile_WithLogicFlag ( srcAddress, bindex, jccFlag, 0 ) ;
}

// 'tttn' is a notation from the intel manuals

void
BlockInfo_Set_tttn ( BlockInfo * bi, int32 ttt, int32 n, int32 overWriteSize )
{
    bi->LogicCode = Here ; // used by combinators
    bi->LogicCodeWord = _Context_->CurrentlyRunningWord ;
    bi->Ttt = ttt ;
    bi->NegFlag = n ;
    bi->OverWriteSize = overWriteSize ;
}

// a 'block' is merely a notation borrowed from C
// for a pointer to an anonymous subroutine 'call'

void
CfrTil_TurnOffBlockCompiler ( )
{
    SetState ( _Context_->Compiler0, COMPILE_MODE, false ) ;
    _Compiler_FreeAllLocalsNamespaces ( _Context_->Compiler0 ) ;
    _CfrTil_RemoveNamespaceFromUsingListAndClear ( ( byte* ) "__labels__" ) ;
    SetState ( _Context_->Compiler0, VARIABLE_FRAME, false ) ;
}

void
CfrTil_TurnOnBlockCompiler ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    SetState ( compiler, COMPILE_MODE, true ) ;
    Compiler_WordList_RecycleInit ( compiler ) ;
}

// blocks are a notation for subroutines or blocks of code compiled in order,
// nested in code, for any purpose, worded or anonymously
// we currently jmp over them to code which pushes
// a pointer to the beginning of the block on the stack
// so the next word will see it on the top of the stack
// some combinators take more than one block on the stack

BlockInfo *
_CfrTil_BeginBlock0 ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    BlockInfo *bi = ( BlockInfo * ) Mem_Allocate ( sizeof (BlockInfo ), COMPILER_TEMP ) ;
    compiler->BlockLevel ++ ;
    if ( ! CompileMode ) // first block
    {
        CfrTil_TurnOnBlockCompiler ( ) ;
    }
    bi->ActualCodeStart = Here ;
    _Compile_UninitializedJump ( ) ;
    bi->JumpOffset = Here - CELL_SIZE ; // before CfrTil_CheckCompileLocalFrame after CompileUninitializedJump
    bi->bp_First = Here ; // after the jump for inlining
    return bi ;
}

BlockInfo *
_CfrTil_BeginBlock1 ( BlockInfo * bi )
{
    Compiler * compiler = _Context_->Compiler0 ;
    if ( _Stack_IsEmpty ( compiler->BlockStack ) )
#if 1
    {
        // remember : we always jmp around the blocks to the combinator ; the combinator sees the blocks on the stack and uses them otherwise they are lost
        // the jmps are optimized out so the word->Definition is a call to the first combinator
        //CheckAddLocalFrame ( _Context->Compiler0 ) ; // // since we are supporting nested locals a lookahead here would have to check to the end of the word, so ...
        // we always add a frame and if not needed move the blocks to overwrite it
        bi->FrameStart = Here ; // before _Compile_AddLocalFrame
        _Compiler_AddLocalFrame ( compiler ) ; // cf EndBlock : if frame is not needed we use BI_Start else BI_FrameStart -- ?? could waste some code space ??
        Compile_InitRegisterParamenterVariables ( compiler ) ; // this function is called twice to deal with words that have locals before the first block and regular colon words
    }
#else        
        {
            bi->FrameStart = Here ; // before _Compile_AddLocalFrame
            // this is a little strange but for now i want this capacity somehow ?? 
            if ( ! ( GetState ( _Context_, C_SYNTAX ) || _Q_->OVT_LC ) )
            {
                for ( ; ; ) //int32 i = 0 ; i < 2 ; i ++ )
                {
                    byte * token = Lexer_PeekNextNonDebugTokenWord ( _Lexer_ ) ;
                    if ( String_Equal ( token, "<dbg>" ) )
                    {
                        SetHere ( bi->FrameStart ) ; // before _Compile_AddLocalFrame
                        token = Lexer_ReadToken ( _Lexer_ ) ;
                        Interpreter_InterpretAToken ( _Interpreter_, token, - 1 ) ;
                        break ;
                    }
                    else if ( ( String_Equal ( token, "(" ) ) ) //|| ( String_Equal ( token, "#" ) ) || ( String_Equal ( token, "//" ) )  || ( String_Equal ( token, "/*" ) ) )
                    {
                        token = Lexer_ReadToken ( _Lexer_ ) ;
                        Interpreter_InterpretAToken ( _Interpreter_, token, - 1 ) ;
                        continue ;
                    }
                    else break ;
                }
            }
            _Compiler_AddLocalFrame ( compiler ) ; // cf EndBlock : if frame is not needed we use BI_Start else BI_FrameStart -- ?? could waste some code space ??
            Compile_InitRegisterParamenterVariables ( compiler ) ; // this function is called twice to deal with words that have locals before the first block and regular colon words
        }
#endif    
    bi->Start = Here ; // after _Compiler_AddLocalFrame and Compile_InitRegisterVariables
    //SC_Global_On ;
    return bi ;
}

BlockInfo *
_CfrTil_BeginBlock2 ( BlockInfo * bi )
{
    Compiler * compiler = _Context_->Compiler0 ;
    _Stack_Push ( compiler->BlockStack, ( int32 ) bi ) ; // _Context->CompileSpace->IndexStart before set frame size after turn on
    _Stack_Push ( compiler->CombinatorBlockInfoStack, ( int32 ) bi ) ; // _Context->CompileSpace->IndexStart before set frame size after turn on
    compiler->LHS_Word = 0 ;
}

void
CfrTil_BeginBlock ( )
{
    BlockInfo * bi = _CfrTil_BeginBlock0 ( ) ;
    _CfrTil_BeginBlock1 ( bi ) ;
    _CfrTil_BeginBlock2 ( bi ) ;
}

Boolean
_Compiler_IsFrameNecessary ( Compiler * compiler )
{
    return ( compiler->NumberOfLocals || compiler->NumberOfArgs ) ; //|| GetState ( compiler, SAVE_ESP ) ) ;
}

void
_CfrTil_EndBlock1 ( BlockInfo * bi )
{
    Compiler * compiler = _Context_->Compiler0 ;
    if ( _Stack_IsEmpty ( compiler->BlockStack ) )
    {
        _CfrTil_InstallGotoCallPoints_Keyed ( bi, GI_RETURN ) ;
        if ( compiler->NumberOfRegisterVariables ) //&& ( compiler->NumberOfParameterVariables == 1 ) && GetState ( compiler, ( RETURN_TOS | RETURN_EAX ) ) )
        {
            bi->bp_First = bi->Start ;
            if ( GetState ( compiler, RETURN_EAX ) )
            {
                Compile_Move_EAX_To_TOS ( DSP ) ;
            }
        }
        else if ( _Compiler_IsFrameNecessary ( compiler ) && ( ! GetState ( compiler, DONT_REMOVE_STACK_VARIABLES ) ) )
        {
            _Compiler_RemoveLocalFrame ( compiler ) ;
            bi->bp_First = bi->FrameStart ; // include _Compile_ESP_Save code
        }
        else
        {
            bi->bp_First = bi->Start ;
        }
    }
    _Compile_Return ( ) ;
    _DataStack_Push ( ( int32 ) bi->bp_First ) ;
    bi->bp_Last = Here ;
    _SetOffsetForCallOrJump ( bi->JumpOffset, Here ) ;
}

byte *
_CfrTil_EndBlock2 ( BlockInfo * bi )
{
    Compiler * compiler = _Context_->Compiler0 ;
    compiler->BlockLevel -- ;
    byte * bi_bp_First = bi->bp_First ;
    if ( _Stack_IsEmpty ( compiler->BlockStack ) )
    {
        _CfrTil_InstallGotoCallPoints_Keyed ( bi, GI_GOTO | GI_RECURSE | GI_CALL_LABEL ) ;
        CfrTil_TurnOffBlockCompiler ( ) ;
        Compiler_Init ( compiler, 0 ) ;
    }
    else
    {
        _Namespace_RemoveFromUsingListAndClear ( bi->LocalsNamespace ) ; //_Compiler_FreeBlockInfoLocalsNamespace ( bi, compiler ) ;
    }
    return bi_bp_First ;
}

byte *
_CfrTil_EndBlock ( )
{
    BlockInfo * bi = ( BlockInfo * ) Stack_Pop_WithExceptionOnEmpty ( _Context_->Compiler0->BlockStack ) ;
    _CfrTil_EndBlock1 ( bi ) ;
    byte * blockStart = _CfrTil_EndBlock2 ( bi ) ;
    return blockStart ;
}

void
CfrTil_EndBlock ( )
{
    _CfrTil_EndBlock ( ) ;
}

