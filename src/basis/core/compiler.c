
#include "../../include/cfrtil.h"

void
Compiler_IncrementCurrentAccumulatedOffset ( Compiler * compiler, int64 increment )
{
    if ( compiler->AccumulatedOffsetPointer )
    {
        ( *( int64* ) ( compiler->AccumulatedOffsetPointer ) ) += ( increment ) ;
    }
    if ( compiler->AccumulatedOptimizeOffsetPointer )
    {
        ( *( int64* ) ( compiler->AccumulatedOptimizeOffsetPointer ) ) += ( increment ) ;
    }
}

void
Compiler_SetCurrentAccumulatedOffsetValue ( Compiler * compiler, int64 value )
{
    if ( compiler->AccumulatedOffsetPointer )
    {
        ( *( int64* ) ( compiler->AccumulatedOffsetPointer ) ) = ( value ) ;
    }
    if ( compiler->AccumulatedOptimizeOffsetPointer )
    {
        ( *( int64* ) ( compiler->AccumulatedOptimizeOffsetPointer ) ) = ( value ) ;
    }
}

NamedByteArray *
_Compiler_SetCompilingSpace ( byte * name )
{
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    Set_CompilerSpace ( nba->ba_CurrentByteArray ) ;
    return nba ;
}

byte *
_Compiler_GetCodeSpaceHere ( )
{
    NamedByteArray *nba = _OVT_Find_NBA ( "CodeSpace" ) ;
    byte * here = _ByteArray_Here ( nba->ba_CurrentByteArray ) ;
    return here ;
}

void
Compiler_SetCompilingSpace ( byte * name )
{
    _Compiler_SetCompilingSpace ( name ) ;
}

void
_Compiler_SetCompilingSpace_MakeSureOfRoom ( byte * name, int64 room )
{
    NamedByteArray * nba = _Compiler_SetCompilingSpace ( name ) ; // same problem as namespace ; this can be called in the middle of compiling another word 
    ByteArray * ba = _ByteArray_AppendSpace_MakeSure ( nba->ba_CurrentByteArray, room ) ;
    if ( ! ba ) Error_Abort ( ( byte* ) "\nCompiler_SetCompilingSpace_MakeSureOfRoom : no ba?!\n" ) ;
}

void
Compiler_SetCompilingSpace_MakeSureOfRoom ( byte * name )
{
    _Compiler_SetCompilingSpace_MakeSureOfRoom ( name, 4 * K ) ;
}

void
Compiler_Show_WordList ( byte * prefix )
{
    if ( Is_DebugModeOn ) NoticeColors ;
    _Printf ( ( byte* ) "\n%s\nCompiler0->WordList : ", prefix ) ;
    dllist * list = _Context_->Compiler0->WordList ;
    _List_Show_N_Word_Names ( list, List_Depth ( list ), 0, 1 ) ; //( uint64 ) 256, ( byte* ) "WordList", Is_DebugOn ) ;
    if ( Is_DebugModeOn ) DefaultColors ;
}

Word *
Compiler_PreviousNonDebugWord ( int64 startIndex )
{
    Word * word ;
    int64 i ;
    for ( i = startIndex ; ( word = ( Word* ) Compiler_WordList ( i ) ) && i > - 3 ; i -- )
    {
        if ( ( Symbol* ) word && ( ! ( word->CProperty & DEBUG_WORD ) ) ) break ;
    }
    return word ;
}

void
_Compiler_FreeAllLocalsNamespaces ( Compiler * compiler )
{
    _Namespace_FreeNamespacesStack ( compiler->LocalsNamespacesStack ) ;
}

Word *
Compiler_WordList ( int64 n )
{
    return ( Word * ) _dllist_Get_N_Node_M_Slot ( _Context_->Compiler0->WordList, n, 0 ) ;
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
CompileOptInfo_New ( Compiler * compiler, uint64 type )
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
Compiler_WordList_RecycleInit ( Compiler * compiler )
{
    if ( ! IsSourceCodeOn )
    {
        //dllist_Map ( compiler->WordList, ( MapFunction0 ) CheckRecycleWord ) ;
        DLList_RecycleWordList ( compiler->WordList ) ;
        List_Init ( compiler->WordList ) ;
        _CfrTil_->DebugWordList = 0 ;
        _CfrTil_->DebugWordListWord = 0 ;
    }
}

void
Compiler_Init ( Compiler * compiler, uint64 state )
{
    compiler->State = state ;
    _dllist_Init ( compiler->GotoList ) ;
    //if ( _ReadLiner_ ) Compiler_Show_WordList ( (byte*) _ReadLiner_->InputLineString ) ;
    Compiler_WordList_RecycleInit ( compiler ) ;
    CfrTil_InitBlockSystem ( compiler ) ;
    compiler->ContinuePoint = 0 ;
    compiler->BreakPoint = 0 ;
    compiler->InitHere = Here ;
    compiler->ParenLevel = 0 ;
    compiler->BlockLevel = 0 ;
    compiler->ArrayEnds = 0 ;
    //CompileOptInfo_Init ( compiler ) ;
    compiler->NumberOfLocals = 0 ;
    compiler->NumberOfArgs = 0 ;
    compiler->NumberOfRegisterVariables = 0 ;
    compiler->LocalsFrameSize = 0 ;
    //compiler->FunctionTypesArray = 0 ;
    compiler->AccumulatedOffsetPointer = 0 ;
    compiler->ReturnVariableWord = 0 ;
    Stack_Init ( compiler->PointerToOffset ) ;
    Stack_Init ( compiler->CombinatorInfoStack ) ;
    _Compiler_FreeAllLocalsNamespaces ( compiler ) ;
    Stack_Init ( compiler->LocalsNamespacesStack ) ;
    Stack_Init ( compiler->InfixOperatorStack ) ;
    //Compiler_SetCompilingSpace ( ( byte* ) "CodeSpace" ) ;
    //Compiler_SetCompilingSpace_MakeSureOfRoom ( ( byte* ) "CodeSpace" ) ; // 2 * K : should be enough at least for now ??
    //OVT_MemListFree_TempObjects ( ) ;
    //compiler->RegOrder [4] = { EBX, EDX, ECX, EAX } ;
    SetBuffersUnused ( 1 ) ;
    SetState ( compiler, VARIABLE_FRAME, false ) ;
}

Compiler *
Compiler_New ( uint64 type )
{
    Compiler * compiler = ( Compiler * ) Mem_Allocate ( sizeof (Compiler ), type ) ;
    compiler->BlockStack = Stack_New ( 64, type ) ;
    compiler->WordList = _dllist_New ( type ) ;
    compiler->PostfixLists = _dllist_New ( type ) ;
    compiler->CombinatorBlockInfoStack = Stack_New ( 64, type ) ;
    compiler->GotoList = _dllist_New ( type ) ;
    compiler->LocalsNamespacesStack = Stack_New ( 32, type ) ;
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
    if ( _Stack_Depth ( _Context_->Compiler0->PointerToOffset ) )
        _SetOffsetForCallOrJump ( ( byte* ) Stack_Pop ( _Context_->Compiler0->PointerToOffset ), jmpToAddress ) ;
}

void
CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( )
{
    CfrTil_CalculateAndSetPreviousJmpOffset ( Here ) ;
}

void
_Stack_PointerToJmpOffset_Set ( byte * address )
{
    Stack_Push ( _Context_->Compiler0->PointerToOffset, ( int64 ) address ) ;
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


