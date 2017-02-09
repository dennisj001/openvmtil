#include "../../include/cfrtil.h"

void
Block_PtrCall ( byte * ptr )
{
    GCC_REGS_PUSH ; //edi ebx are used by various gcc's must be preserved when calling cfrtil code
    ( ( VoidFunction ) ptr ) ( ) ;
    GCC_REGS_POP ; //edi ebx are used by various gcc's must be preserved when calling cfrtil code
}

int32
Block_Compile_WithLogicFlag ( byte * srcAddress, int32 bindex, int32 jccFlag, int n )
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
        jccFlag2 = Compile_ReConfigureLogicInBlock ( bi, 1 ) ;
    }
    if ( ! GetState ( _CfrTil_, INLINE_ON ) ) Compile_Call ( srcAddress ) ;
    else
    {
        _Block_Copy ( srcAddress, bi->bp_Last - bi->bp_First ) ;
    }
    if ( jccFlag )
    {
        Set_SCA ( 0 ) ;
        if ( jccFlag2 )
        {
            Compile_JCC ( n ? bi->NegFlag : ! bi->NegFlag, bi->Ttt, 0 ) ;
        }
        else
        {
            Compile_GetLogicFromTOS ( bi ) ;
            Compile_JCC ( n, ZERO_TTT, 0 ) ;
        }
        _Stack_PointerToJmpOffset_Set ( Here - CELL ) ;
    }
    return 1 ;
}

int32
Block_Compile ( byte * srcAddress, int32 bindex, int32 jccFlag )
{
    return Block_Compile_WithLogicFlag ( srcAddress, bindex, jccFlag, 0 ) ;
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
_Block_Eval ( block block )
{
    block ( ) ;
}

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
    List_Init ( compiler->WordList ) ;
}

// blocks are a notation for subroutines or blocks of code compiled in order,
// nested in code, for any purpose, worded or anonymously
// we currently jmp over them to code which pushes
// a pointer to the beginning of the block on the stack
// so the next word will see it on the top of the stack
// some combinators take more than one block on the stack

BlockInfo *
_CfrTil_BeginBlock ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    BlockInfo *bi = ( BlockInfo * ) Mem_Allocate ( sizeof (BlockInfo ), SESSION ) ;
    compiler->BlockLevel ++ ;
    if ( ! CompileMode ) // first block
    {
        CfrTil_TurnOnBlockCompiler ( ) ;
    }
    bi->ActualCodeStart = Here ;
    _Compile_UninitializedJump ( ) ;
    bi->JumpOffset = Here - CELL_SIZE ; // before CfrTil_CheckCompileLocalFrame after CompileUninitializedJump
    bi->bp_First = Here ; // after the jump for inlining
    if ( _Stack_IsEmpty ( compiler->BlockStack ) )
    {
        // remember : we always jmp around the blocks to the combinator ; the combinator sees the blocks on the stack and uses them otherwise they are lost
        // the jmps are optimized out so the word->Definition is a call to the first combinator
        //CheckAddLocalFrame ( _Context->Compiler0 ) ; // // since we are supporting nested locals a lookahead here would have to check to the end of the word, so ...
        // we always add a frame and if not needed move the blocks to overwrite it
        bi->FrameStart = Here ; // before _Compile_AddLocalFrame
#if 0       
        _Compile_ESP_Save ( ) ;
        bi->AfterEspSave = Here ; // before _Compile_AddLocalFrame
#endif        
        _Compiler_AddLocalFrame ( compiler ) ; // cf EndBlock : if frame is not needed we use BI_Start else BI_FrameStart -- ?? could waste some code space ??
        Compile_InitRegisterParamenterVariables ( compiler ) ; // this function is called twice to deal with words that have locals before the first block and regular colon words
    }
    bi->Start = Here ; // after _Compiler_AddLocalFrame and Compile_InitRegisterVariables
    return bi ;
}

BlockInfo *
CfrTil_BeginBlock ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    BlockInfo *bi = _CfrTil_BeginBlock ( ) ;
    _Stack_Push ( compiler->BlockStack, ( int32 ) bi ) ; // _Context->CompileSpace->IndexStart before set frame size after turn on
    _Stack_Push ( compiler->CombinatorBlockInfoStack, ( int32 ) bi ) ; // _Context->CompileSpace->IndexStart before set frame size after turn on
    _Context_->Compiler0->LHS_Word = 0 ;
    return bi ;
}

#if 0

BlockInfo *
_CfrTil_EndBlock0 ( )
{
    BlockInfo * bi = ( BlockInfo * ) Stack_Pop_WithExceptionOnEmpty ( _Context_->Compiler0->BlockStack ) ;
    return bi ;
}
#endif

Boolean
_Compiler_IsFrameNecessary ( Compiler * compiler )
{
    return ( compiler->NumberOfLocals || compiler->NumberOfParameterVariables ) ; //|| GetState ( compiler, SAVE_ESP ) ) ;
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
#if 0            
            if ( GetState ( compiler, SAVE_ESP ) ) // SAVE_ESP is set by 'return'
            {
                _ESP_Setup ( ) ;
                bi->bp_First = bi->FrameStart ; // include _Compile_ESP_Save code
            }
            else bi->bp_First = bi->AfterEspSave ; // 3 : after ESP_Save code in frame setup code
#else
            bi->bp_First = bi->FrameStart ; // include _Compile_ESP_Save code
#endif            
        }
        else
        {
            bi->bp_First = bi->Start ;
        }
    }
    _Compile_Return ( ) ;
    _DataStack_Push ( ( int32 ) bi->bp_First ) ;
    bi->bp_Last = Here ;
    _SetOffsetForCallOrJump ( bi->JumpOffset, Here, 0 ) ;
}

byte *
_CfrTil_EndBlock2 ( BlockInfo * bi )
{
    Compiler * compiler = _Context_->Compiler0 ;
    if ( _Stack_IsEmpty ( compiler->BlockStack ) )
    {
        _CfrTil_InstallGotoCallPoints_Keyed ( bi, GI_GOTO | GI_RECURSE | GI_CALL_LABEL ) ;
        CfrTil_TurnOffBlockCompiler ( ) ;
    }
    else _Namespace_RemoveFromUsingListAndClear ( bi->LocalsNamespace ) ; //_Compiler_FreeBlockInfoLocalsNamespace ( bi, compiler ) ;
    compiler->BlockLevel -- ;
    return bi->bp_First ;
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

