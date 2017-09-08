
#include "../../include/cfrtil.h"

/*
struct _Stack
{
        cell_t				*StackPointer ;
        cell_t				*StackMin ;
        cell_t				*StackMax ;
        cell_t				StackData 			[  ] ;
}; 
typedef struct _Stack 			Stack ;

 */
void
Stack_Print_AValue_WordName ( Stack * stack, int64 i, byte * stackName, byte * buffer )
{
    uint64 * stackPointer = stack->StackPointer ;
    Word * word = ( Word * ) ( stackPointer [ - i ] ) ;
    if ( word )
    {
        byte wname [ 128 ] ;
        //_String_ConvertStringToBackSlash ( wname, word->Name ) ;
        sprintf ( ( char* ) buffer, "< %s.%s >", word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : "<literal>", c_dd ( _String_ConvertStringToBackSlash ( wname, word->Name ) ) ) ;
        _Printf ( ( byte* ) "\n\t\t    %s   [ %3ld ] < " UINT_FRMT_0x08 " > = " UINT_FRMT_0x08 "\t\t%s", stackName, i, ( uint64 ) & stackPointer [ - i ], stackPointer [ - i ], word ? ( char* ) buffer : "" ) ;
    }
}

void
Stack_Print_AValue ( uint64 * stackPointer, int64 i, byte * stackName, byte * buffer )
{
    Word * word ;
    byte * string = 0 ;
    word = Word_GetFromCodeAddress ( ( byte* ) ( stackPointer [ i ] ) ) ;
    if ( word )
    {
        if ( NON_MORPHISM_TYPE ( word ) ) sprintf ( ( char* ) buffer, "< word : %s.%s : value = 0x%08lx >", word->ContainingNamespace->Name, c_dd ( word->Name ), ( uint64 ) word->S_Value ) ;
        else sprintf ( ( char* ) buffer, "< word : %s.%s : definition = 0x%08lx >", word->ContainingNamespace->Name, c_dd ( word->Name ), ( uint64 ) word->Definition ) ;
    }
    else string = String_CheckForAtAdddress ( ( byte* ) ( ( byte* ) ( stackPointer[i] ) ) ) ;
    _Printf ( ( byte* ) "\n\t\t    %s   [ %3ld ] < " UINT_FRMT_0x08 " > = " UINT_FRMT_0x08 "\t\t%s",
        stackName, i, ( uint64 ) & stackPointer [ i ], stackPointer [ i ], word ? buffer : string ? string : ( byte* ) "" ) ;
}

void
_Stack_PrintHeader ( Stack * stack, byte * name )
{
    int64 size = Stack_Depth ( stack ) ;
    uint64 * sp = stack->StackPointer ; // 0 based stack
    //byte * location = c_dd (Context_IsInFile ( _Context_ ) ? Context_Location ( ) : (byte*) "a command line") ;
    byte * location = c_dd ( Context_Location ( ) ) ;
    _Printf ( ( byte* ) "\nStack at : %s :\n%s depth =%4d : %s = Top = " UINT_FRMT_0x08 ", InitialTos = " UINT_FRMT_0x08 ","
        " Max = " UINT_FRMT_0x08 ", Min = " UINT_FRMT_0x08 ", Size = " UINT_FRMT_0x08, location,
        name, size, stack == _DataStack_ ? "Dsp (ESI)" : "", ( int64 ) sp, ( int64 ) stack->InitialTosPointer, ( int64 ) stack->StackMax, ( int64 ) stack->StackMin, stack->StackMax - stack->StackMin + 1 ) ;
}

void
_Stack_PrintValues ( byte * name, uint64 * stackPointer, int64 depth )
{
    int64 i ; //, stackDepth = _Stack_Depth ( stack ), * stackPointer = stack->StackPointer ; // 0 based stack
    byte * buffer = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    if ( depth >= 0 )
    {
        for ( i = 0 ; depth -- > 0 ; i -- )
        {
            Stack_Print_AValue ( stackPointer, i, name, buffer ) ;
        }
    }
    else //if ( stackDepth < 0 )
    {
        CfrTil_Exception ( STACK_UNDERFLOW, QUIT ) ;
    }
    //_Printf ( ( byte* ) "\n" ) ;
}

void
Stack_PrintValues ( byte * name, Stack *stack, int64 depth )
{
    _Stack_PrintValues ( name, stack->StackPointer, depth ) ;
}

#if 0 // save

void
_Stack_Show_Word_Name_AtN ( Stack * stack, int64 i, byte * stackName, byte * buffer )
{
    Stack_Print_AValue_WordName ( stack, i, stackName, buffer ) ;
}

void
_Stack_Show_N_Word_Names ( Stack * stack, uint64 n, byte * stackName, int64 dbgFlag )
{
    uint64 i ;
    int64 depth = Stack_Depth ( stack ) ;
    byte * buffer = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    if ( dbgFlag ) NoticeColors ;
    _Stack_PrintHeader ( stack, stackName ) ;
    for ( i = 0 ; ( n > i ) && ( i < depth ) ; i ++ )
    {
        if ( Stack_N ( stack, - i ) ) _Stack_Show_Word_Name_AtN ( stack, i, stackName, buffer ) ;
        else break ;
    }
    //_Stack_Print ( stack, stackName ) ;
    if ( dbgFlag ) DefaultColors ;
}
#endif

void
_Stack_Print ( Stack * stack, byte * name )
{
    _Stack_PrintHeader ( stack, name ) ;
    Stack_PrintValues ( name, stack, Stack_Depth ( stack ) ) ;
    CfrTil_NewLine ( ) ;
}

int64
_Stack_Overflow ( Stack * stack )
{
    return ( stack->StackPointer >= stack->StackMax ) ;
}

int64
_Stack_IsEmpty ( Stack * stack )
{
    return ( stack->StackPointer < stack->StackMin ) ;
}

void
_Stack_Push ( Stack * stack, int64 value )
{
    stack->StackPointer ++ ;
    *( stack->StackPointer ) = value ;
}

void
_Stack_Dup ( Stack * stack )
{
    _Stack_Push ( stack, *( stack->StackPointer ) ) ;
}

int64
__Stack_Pop ( Stack * stack )
{
    return *( stack->StackPointer -- ) ;
}

int64
_Stack_Pop_ExceptionFlag ( Stack * stack, int64 exceptionOnEmptyFlag )
{
    if ( _Stack_IsEmpty ( stack ) )
    {
        if ( exceptionOnEmptyFlag ) CfrTil_Exception ( STACK_UNDERFLOW, QUIT ) ;
        else return 0 ;
    }
    return __Stack_Pop ( stack ) ;
}

int64
_Stack_Pop ( Stack * stack )
{
    if ( _Stack_IsEmpty ( stack ) ) CfrTil_Exception ( STACK_UNDERFLOW, QUIT ) ;
    return __Stack_Pop ( stack ) ;
}

int64
_Stack_PopOrTop ( Stack * stack )
{
    int64 sd = Stack_Depth ( stack ) ;
    if ( sd <= 0 ) CfrTil_Exception ( STACK_UNDERFLOW, QUIT ) ;
    else if ( sd == 1 ) return _Stack_Top ( stack ) ;
    else return __Stack_Pop ( stack ) ;
}

int64
_Stack_DropN ( Stack * stack, int64 n )
{
    return * ( stack->StackPointer -= n ) ;
}

int64
_Stack_Top ( Stack * stack )
{
    return *stack->StackPointer ;
}

int64
_Stack_Pick ( Stack * stack, int64 offset )
{
    return * ( stack->StackPointer - offset ) ;
}

int64
_Stack_PickFromBottom ( Stack * stack, int64 offset )
{
    return * ( stack->StackMin + offset ) ;
}

int64
_Stack_Bottom ( Stack * stack )
{
    return * ( stack->StackMin ) ;
}

void
_Stack_SetBottom ( Stack * stack, int64 value )
{
    *stack->StackMin = value ;
}

void
_Stack_SetTop ( Stack * stack, int64 value )
{
    *stack->StackPointer = value ;
}

int64
_Stack_NOS ( Stack * stack )
{
    return *( stack->StackPointer - 1 ) ;
}

void
_Stack_Drop ( Stack * stack )
{
    stack->StackPointer -- ;
}

void
Stack_Push ( Stack * stack, int64 value )
{
#if STACK_CHECK_ERROR
    if ( _Stack_Overflow ( stack ) )
    {
        AlertColors ;
        _Printf ( ( byte* ) "\nStackDepth = %d\n", Stack_Depth ( stack ) ) ;
        CfrTil_Exception ( STACK_OVERFLOW, QUIT ) ;
    }
    _Stack_Push ( stack, value ) ;
#else
    _Stack_Push ( stack, value ) ;
#endif
}

int64
Stack_Pop_WithExceptionOnEmpty ( Stack * stack )
{
#if STACK_CHECK_ERROR
    if ( _Stack_IsEmpty ( stack ) ) CfrTil_Exception ( STACK_UNDERFLOW, QUIT ) ;
    return _Stack_Pop ( stack ) ;
#else
    _Stack_Pop ( stack ) ;
#endif
}

int64
Stack_Pop_WithZeroOnEmpty ( Stack * stack )
{
    if ( _Stack_IsEmpty ( stack ) ) return 0 ;
    return _Stack_Pop ( stack ) ;
}

void
Stack_Dup ( Stack * stack )
{
#if STACK_CHECK_ERROR
    if ( _Stack_Overflow ( stack ) ) CfrTil_Exception ( STACK_OVERFLOW, QUIT ) ;
    _Stack_Dup ( stack ) ;
#else
    _Stack_Dup ( stack ) ;
#endif
}

int64
_Stack_IntegrityCheck ( Stack * stack )
{
    // first a simple integrity check of the stack info struct
    _CfrTil_SetStackPointerFromDsp ( _CfrTil_ ) ;
    if ( ( stack->StackMin == & stack->StackData [ 0 ] ) &&
        ( stack->StackMax == & stack->StackData [ stack->StackSize - 1 ] ) && // -1 : zero based array
        ( stack->InitialTosPointer == & stack->StackData [ - 1 ] ) )
    {
        return true ;
    }
    return false ;
}

int64
_Stack_Depth ( Stack * stack )
{
    int64 depth = stack->StackPointer - stack->InitialTosPointer ; //+ 1 ; // + 1 :: zero based array - include the zero in depth 
    if ( depth <= stack->StackSize ) return depth ;
    return ( 0 ) ;
}

int64
Stack_Depth ( Stack * stack )
{
    // first a simple integrity check of the stack info struct
    if ( _Stack_IntegrityCheck ( stack ) )
    {
        return _Stack_Depth ( stack ) ;
    }
    return ( 0 ) ;
}

void
Stack_SetStackMax ( Stack * stack, int64 value )
{
    stack->StackData [ stack->StackSize - 1 ] = value ;
}

// Stack_Clear => Stack_Init

void
_Stack_Init ( Stack * stack, int64 slots )
{
    memset ( stack, 0, sizeof ( Stack ) + ( slots * sizeof (int64 ) ) ) ;
    stack->StackSize = slots ; // re-init size after memset cleared it
    stack->StackMin = & stack->StackData [ 0 ] ; // 
    stack->StackMax = & stack->StackData [ stack->StackSize - 1 ] ;

    stack->InitialTosPointer = & stack->StackData [ - 1 ] ; // first push goes to stack->StackData [ 0 ]
    stack->StackPointer = stack->InitialTosPointer ;
}

void
Stack_Delete ( Stack * stack )
{
    Mem_FreeItem ( &_Q_->PermanentMemList, ( byte* ) stack ) ;
}

void
Stack_Init ( Stack * stack )
{
    _Stack_Init ( stack, stack->StackSize ) ;
}

Stack *
Stack_New ( int64 slots, uint64 type )
{
    Stack * stack = ( Stack* ) Mem_Allocate ( sizeof ( Stack ) + ( slots * sizeof (int64 ) ), type ) ;
    _Stack_Init ( stack, slots ) ;
    return stack ;
}

Stack *
Stack_Copy ( Stack * stack, uint64 type )
{
    Stack * nstack = Stack_New ( stack->StackSize, type ) ;
    memcpy ( nstack->StackData, stack->StackData, stack->StackSize * sizeof (int64 ) ) ;

    // ?? -> preserve relative stack pointer
    int64 depth = Stack_Depth ( stack ) ;
    //depth = stack->StackPointer - stack->InitialTosPointer ;
    nstack->StackPointer = nstack->InitialTosPointer + depth ;

    return nstack ;
}

void
_PrintNStackWindow ( uint64 * reg, byte * name, byte * regName, int64 size )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    byte * buffer = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    int64 saveSize = size ;
    if ( reg )
    {
        _Printf ( ( byte* ) "\n%s   :%3i  : %s = " UINT_FRMT_0x08 " : Top = " UINT_FRMT_0x08 "", name, size, regName, ( uint64 ) reg, ( uint64 ) reg ) ;
        // print return stack in reverse of usual order first
        while ( size -- > 1 )
        {
            Stack_Print_AValue ( reg, size, name, buffer ) ;
        }
        _Stack_PrintValues ( ( byte* ) name, reg, saveSize ) ;
    }
}

void
_CfrTil_PrintNReturnStack ( int64 size )
{
    Debugger * debugger = _Debugger_ ;
    if ( GetState ( debugger, DBG_STEPPING ) && debugger->ReturnStackCopyPointer )
    {
        _PrintNStackWindow ( ( uint64* ) debugger->ReturnStackCopyPointer, ( byte * ) "ReturnStackCopy", ( byte * ) "RSCP", size ) ;
    }
#if 0    
    else if ( _CfrTil_->cs_Cpu->Esp )
    {
        _PrintNStackWindow ( ( uint64* ) _CfrTil_->cs_Cpu->Esp, ( byte * ) "CpuState->Esp", ( byte * ) "CpuState->Esp", size ) ;
    }
#endif
    else if ( debugger->cs_Cpu->Esp ) //debugger->DebugESP )
    {
        //_PrintNStackWindow ( ( uint64* ) debugger->DebugESP, ( byte * ) "Return Stack", ( byte * ) "DebugEsp", size ) ;
        _PrintNStackWindow ( ( uint64* ) debugger->cs_Cpu->Esp, ( byte * ) "Return Stack", ( byte * ) "Esp (ESP)", size ) ;
        _Stack_PrintValues ( ( byte* ) "DebugStack ", debugger->DebugStack->StackPointer, Stack_Depth ( debugger->DebugStack ) ) ;
    }
    else
    {
        _CfrTil_WordName_Run ( ( byte* ) "getESP" ) ;
        uint64 * esp = ( uint64 * ) _DataStack_Pop ( ) ;
        _PrintNStackWindow ( esp, ( byte* ) "Return Stack", ( byte* ) "Esp (ESP)", size ) ;
    }
}

void
_CfrTil_PrintNDataStack ( int64 size )
{
    _PrintNStackWindow ( Dsp, ( byte* ) "Data Stack", ( byte* ) "Dsp (DSP)", size ) ;
}

