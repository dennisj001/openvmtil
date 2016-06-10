
#include "../../includes/cfrtil.h"

void
_Compiler_SetCompilingSpace ( byte * name )
{
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    Set_CompilerSpace ( nba->ba_CurrentByteArray ) ;
}

#if 1 // save

void
Compiler_Show_WordList ( byte * prefix )
{
    if ( Is_DebugOn ) NoticeColors ;
    Printf ( ( byte* ) "%s\nWordList : ", prefix ) ;
    dllist * list = _Context_->Compiler0->WordList ;
    _List_Show_N_Word_Names ( list, List_Depth ( list ), 0, 1 ) ;//( uint32 ) 256, ( byte* ) "WordList", Is_DebugOn ) ;
    if ( Is_DebugOn ) DefaultColors ;
}
#endif

Word *
Compiler_PreviousNonDebugWord ( int startIndex )
{
    Word * word ;
    int32 i ;
    for ( i = startIndex ; ( word = ( Word* ) Compiler_WordList ( i ) ) && i > - 3 ; i -- )
    {
        if ( ( Symbol* ) word && ( ! ( word->CProperty & DEBUG_WORD ) ) ) break ;
    }
    return word ;
}

void
_Compiler_FreeLocalsNamespace ( Compiler * compiler )
{
    Namespace * ns = ( Namespace* ) Stack_Pop ( compiler->LocalNamespaces ) ;
    if ( ns ) _Namespace_RemoveFromUsingListAndClear ( ns ) ;
}

void
_Compiler_WordList_PushWord ( Compiler * compiler, Word * word )
{
    if ( ! ( word->CProperty & ( DEBUG_WORD ) ) ) List_Push ( compiler->WordList, ( int32 ) word ) ;
}

void
_Compiler_FreeAllLocalsNamespaces ( Compiler * compiler )
{
    int32 n ;
    for ( n = Stack_Depth ( compiler->LocalNamespaces ) ; n ; n -- )
    {
        _Compiler_FreeLocalsNamespace ( compiler ) ;
    }
}

Word *
Compiler_WordList ( int32 n )
{
    return ( Word * ) _dllist_GetNValue ( _Context_->Compiler0->WordList, n ) ;
}

void
_CompileOptInfo_Init ( Compiler * compiler )
{
    CompileOptimizeInfo * optInfo = compiler->optInfo ;
    memset ( optInfo, 0, sizeof (CompileOptimizeInfo ) ) ;
}

void
CompileOptInfo_Init ( Compiler * compiler )
{
    CompileOptimizeInfo * optInfo = compiler->optInfo ;
    _CompileOptInfo_Init ( compiler ) ;
    optInfo->O_zero = Compiler_WordList ( 0 ) ;
    optInfo->O_one = Compiler_WordList ( 1 ) ;
    optInfo->O_two = Compiler_WordList ( 2 ) ;
    optInfo->O_three = Compiler_WordList ( 3 ) ;
    optInfo->O_four = Compiler_WordList ( 4 ) ;
    optInfo->O_five = Compiler_WordList ( 5 ) ;
    optInfo->O_six = Compiler_WordList ( 6 ) ;
}

CompileOptimizeInfo *
CompileOptInfo_New ( Compiler * compiler, uint32 type )
{
    compiler->optInfo = ( CompileOptimizeInfo * ) Mem_Allocate ( sizeof (CompileOptimizeInfo ), type ) ;
    //CompileoptInfo_Init ( compiler ) ;
}

void
CompileoptInfo_Delete ( CompileOptimizeInfo * optInfo )
{
    Mem_FreeItem ( &_Q_->PermanentMemList, ( byte* ) optInfo ) ;
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
    compiler->State = state ;
    _dllist_Init ( compiler->GotoList ) ;
    List_Init ( compiler->WordList ) ;
    List_Init ( compiler->PostfixLists ) ;
    CfrTil_InitBlockSystem ( compiler ) ;
    compiler->ContinuePoint = 0 ;
    compiler->BreakPoint = 0 ;
    compiler->InitHere = Here ;
    compiler->ParenLevel = 0 ;
    compiler->BlockLevel = 0 ;
    compiler->ArrayEnds = 0 ;
    //CompileOptInfo_Init ( compiler ) ;
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
    //compiler->RegOrder [4] = { EBX, EDX, ECX, EAX } ;
    //SetBuffersUnused ;
}

Compiler *
Compiler_New ( uint32 type )
{
    Compiler * compiler = ( Compiler * ) Mem_Allocate ( sizeof (Compiler ), type ) ;
    compiler->BlockStack = Stack_New ( 64, type ) ;
    compiler->WordList = _dllist_New ( type ) ;
    compiler->PostfixLists = _dllist_New ( type ) ;
    compiler->CombinatorBlockInfoStack = Stack_New ( 64, type ) ;
    compiler->GotoList = _dllist_New ( type ) ;
    compiler->LocalNamespaces = Stack_New ( 32, type ) ;
    compiler->NamespacesStack = Stack_New ( 32, type ) ;
    compiler->PointerToOffset = Stack_New ( 32, type ) ;
    compiler->CombinatorInfoStack = Stack_New ( 64, type ) ;
    compiler->InfixOperatorStack = Stack_New ( 32, type ) ;
    CompileOptInfo_New ( compiler, type ) ;
    compiler->RegOrder [ 0 ] = EBX ;
    compiler->RegOrder [ 1 ] = EDX ;
    compiler->RegOrder [ 2 ] = ECX ;
    compiler->RegOrder [ 3 ] = EAX ;
    Compiler_Init ( compiler, 0 ) ;
    return compiler ;
}

void
CfrTil_CalculateAndSetPreviousJmpOffset ( byte * jmpToAddress )
{
    // we can now not compile blocks (cf. _Compile_Block_WithLogicFlag ) if their logic is not called so depth check is necessary
    if ( _Stack_Depth ( _Context_->Compiler0->PointerToOffset ) ) _SetOffsetForCallOrJump ( ( byte* ) Stack_Pop ( _Context_->Compiler0->PointerToOffset ), jmpToAddress, 0 ) ;
}

void
CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( )
{
    CfrTil_CalculateAndSetPreviousJmpOffset ( Here ) ;
}

void
_Stack_PointerToJmpOffset_Set ( byte * address )
{
    Stack_Push ( _Context_->Compiler0->PointerToOffset, ( int32 ) address ) ;
}

void
Stack_PointerToJmpOffset_Set ( )
{
    _Stack_PointerToJmpOffset_Set ( Here - CELL_SIZE ) ;
}

void
_Compiler_CompileAndRecord_PushEAX ( Compiler * compiler )
{
    //_Word_CompileAndRecord_PushEAX ( Compiler_WordStack ( 0 ) ) ;
    _Word_CompileAndRecord_PushReg ( Compiler_WordList ( 0 ), EAX ) ;
}


