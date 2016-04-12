
#include "../../includes/cfrtil.h"

void
_Compiler_SetCompilingSpace ( byte * name )
{
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    Set_CompilerSpace ( nba->ba_CurrentByteArray ) ;
}

void
Compiler_ShowWordStack ( byte * prefix )
{
    if ( IsDebugOn ) NoticeColors ;
    Printf ( (byte*) "%s", prefix ) ;
    _Stack_Show_N_Word_Names ( CompilerWordStack, (uint32) 256, (byte*) "WordStack", IsDebugOn ) ;
    if ( IsDebugOn ) DefaultColors ;
}

Word *
Compiler_PreviousNonDebugWord ( int startIndex )
{
    Word * word ;
    int32 i ;
    for ( i = startIndex ; ( word = ( Word* ) Compiler_WordStack ( i ) ) && i > - 3 ; i -- )
    {
        if ( ( Symbol* ) word && ( ! ( word->CType & DEBUG_WORD ) ) ) break ;
    }
    return word ;
}

void
_Compiler_FreeBlockInfoLocalsNamespace ( BlockInfo * bi, Compiler * compiler )
{
    Namespace * ns = bi->LocalsNamespace ;
    if ( ns )
    {
        //ns = (Namespace*) Stack_Pop ( compiler->LocalNamespaces ) ;
        _Namespace_RemoveFromUsingListAndClear ( ns ) ;
    }
}

void
_Compiler_FreeLocalsNamespace ( Compiler * compiler )
{
    Namespace * ns = ( Namespace* ) Stack_Pop ( compiler->LocalNamespaces ) ;
    if ( ns ) _Namespace_RemoveFromUsingListAndClear ( ns ) ;
}

void
_Compiler_WordStack_PushWord ( Compiler * compiler, Word * word )
{
    if ( ! ( word->CType & ( DEBUG_WORD ) ) ) Stack_Push ( compiler->WordStack, ( int32 ) word ) ;
}

void
_Compiler_FreeAllLocalsNamespaces ( Compiler * compiler )
{
#if 0    
    Namespace * ns ;
    while ( ( ns = ( Namespace* ) Stack_Pop_WithZeroOnEmpty ( compiler->LocalNamespaces ) ) )
    {
        _Namespace_RemoveFromUsingListAndClear ( ns ) ;
    }
#else
    while ( Stack_Depth ( compiler->LocalNamespaces ) )
    {
        _Compiler_FreeLocalsNamespace ( compiler ) ;
    }
#endif    
}

void
CompileOptimizer_Init ( Compiler * compiler )
{
    CompileOptimizer * optimizer = compiler->Optimizer ;
    memset ( optimizer, 0, sizeof (CompileOptimizer ) ) ;
    optimizer->O_zero = _Compiler_WordStack ( compiler, 0 ) ;
    optimizer->O_one = _Compiler_WordStack ( compiler, - 1 ) ;
    optimizer->O_two = _Compiler_WordStack ( compiler, - 2 ) ;
    optimizer->O_three = _Compiler_WordStack ( compiler, - 3 ) ;
    optimizer->O_four = _Compiler_WordStack ( compiler, - 4 ) ;
    optimizer->O_five = _Compiler_WordStack ( compiler, - 5 ) ;
}

CompileOptimizer *
CompileOptimizer_New ( Compiler * compiler, uint32 type )
{
    compiler->Optimizer = ( CompileOptimizer * ) Mem_Allocate ( sizeof (CompileOptimizer ), type ) ;
    CompileOptimizer_Init ( compiler ) ;
}

void
CompileOptimizer_Delete ( CompileOptimizer * optimizer )
{
    Mem_FreeItem ( &_Q_->PermanentMemList, ( byte* ) optimizer ) ;
}

void
CfrTil_InitBlockSystem ( Compiler * compiler )
{
    Stack_Init ( compiler->BlockStack ) ;
    Stack_Init ( compiler->CombinatorBlockInfoStack ) ;
}

void
Compiler_Init ( Compiler * compiler, uint64 state )
{
    _DLList_Init ( compiler->GotoList ) ;
    Stack_Init ( compiler->WordStack ) ;
    CfrTil_InitBlockSystem ( compiler ) ;
    compiler->ContinuePoint = 0 ;
    compiler->BreakPoint = 0 ;
    compiler->InitHere = Here ;
    compiler->State = state ;
    compiler->ParenLevel = 0 ;
    compiler->BlockLevel = 0 ;
    compiler->ArrayEnds = 0 ;
    CompileOptimizer_Init ( compiler ) ;
    compiler->NumberOfLocals = 0 ;
    compiler->NumberOfParameterVariables = 0 ;
    compiler->NumberOfRegisterVariables = 0 ;
    compiler->LocalsFrameSize = 0 ;
    //compiler->FunctionTypesArray = 0 ;
    compiler->AccumulatedOffsetPointer = 0 ;
    compiler->ReturnVariableWord = 0 ;
    Stack_Init ( compiler->PointerToOffset ) ;
    Stack_Init ( compiler->CombinatorInfoStack ) ;
    _Compiler_FreeAllLocalsNamespaces ( compiler ) ;
    Stack_Init ( compiler->LocalNamespaces ) ;
    Stack_Init ( compiler->InfixOperatorStack ) ;
    _Compiler_SetCompilingSpace ( ( byte* ) "CodeSpace" ) ;
    OVT_MemListFree_TempObjects ( ) ;
    //_DLList_Init ( _Q_->OVT_CfrTil->TokenList ) ;
    SetBuffersUnused ;
}

Compiler *
Compiler_New ( uint32 type )
{
    Compiler * compiler = ( Compiler * ) Mem_Allocate ( sizeof (Compiler ), type ) ;
    compiler->BlockStack = Stack_New ( 64, type ) ;
    compiler->WordStack = Stack_New ( 1 * K, type ) ;
    compiler->CombinatorBlockInfoStack = Stack_New ( 64, type ) ;
    compiler->GotoList = _DLList_New ( type ) ;
    compiler->LocalNamespaces = Stack_New ( 32, type ) ;
    compiler->NamespacesStack = Stack_New ( 32, type ) ;
    compiler->PointerToOffset = Stack_New ( 32, type ) ;
    compiler->CombinatorInfoStack = Stack_New ( 64, type ) ;
    compiler->InfixOperatorStack = Stack_New ( 32, type ) ;
    CompileOptimizer_New ( compiler, type ) ;
    Compiler_Init ( compiler, 0 ) ;
    return compiler ;
}

void
CfrTil_CalculateAndSetPreviousJmpOffset ( byte * jmpToAddress )
{
    // we can now not compile blocks (cf. _Compile_Block_WithLogicFlag ) if their logic is not called so depth check is necessary
    if ( _Stack_Depth ( _Q_->OVT_Context->Compiler0->PointerToOffset ) ) _SetOffsetForCallOrJump ( ( byte* ) Stack_Pop ( _Q_->OVT_Context->Compiler0->PointerToOffset ), jmpToAddress, 0 ) ;
}

void
CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( )
{
    CfrTil_CalculateAndSetPreviousJmpOffset ( Here ) ;
}

void
_Stack_PointerToJmpOffset_Set ( byte * address )
{
    Stack_Push ( _Q_->OVT_Context->Compiler0->PointerToOffset, ( int32 ) address ) ;
}

void
Stack_PointerToJmpOffset_Set ( )
{
    _Stack_PointerToJmpOffset_Set ( Here - CELL_SIZE ) ;
}

void
_Compiler_CompileAndRecord_PushEAX ( Compiler * compiler )
{
    _Word_CompileAndRecord_PushEAX ( Compiler_WordStack ( 0 ) ) ;
}


