
#include "../../include/cfrtil.h"

// these functions are part of the C vm and can't be compiled
// ! they are should only be called in C functions !

uint32
_DataStack_Pop ( )
{
    return Dsp -- [ 0 ] ;
}

void
_DataStack_Push ( int32 value )
{
    *++ Dsp = value ;
}

void
_DataStack_Dup ( )
{
    Dsp [ 1 ] = TOS ;
    Dsp ++ ;
}

void
_DataStack_DropN ( int n )
{
    Dsp -= n ;
}

void
_DataStack_Drop ( )
{
    Dsp -- ;
}

int32
DataStack_Overflow ( )
{
    return ( Dsp >= _DataStack_->StackMax ) ;
}

int32
DataStack_Underflow ( )
{
    return ( Dsp < _DataStack_->InitialTosPointer ) ;
}

void
DataStack_Check ( )
{
    if ( ( Dsp < _DataStack_->InitialTosPointer ) || ( Dsp >= _DataStack_->StackMax ) )
    {
        CfrTil_Exception ( STACK_OVERFLOW, QUIT ) ;
    }
}

int32
DataStack_Depth ( )
{
    if ( _Q_ && _CfrTil_ && _DataStack_ )
    {
        _DataStackPointer_ = Dsp ;
        return Stack_Depth ( _DataStack_ ) ;
    }
    return 0 ;
}

// safe form with stack checking

int32
DataStack_Pop ( )
{
    _DataStackPointer_ = Dsp ;
    _DataStack_Pop ( ) ;
    int32 top = Stack_Pop_WithExceptionOnEmpty ( _DataStack_ ) ;
    return top ;
}

void
CpuState_SyncStackPointersFromCpuState ( Cpu * cpus )
{
    int32 *svDsp = Dsp ;
    if ( cpus->State ) Dsp = ( int* ) cpus->Esi ;
    _CfrTil_SetStackPointerFromDsp ( _CfrTil_ ) ;
}

void
Debugger_SyncStackPointersFromCpuState ( Debugger * debugger )
{
    CfrTil * cfrTil = _CfrTil_ ;
    int32 *svDsp = Dsp, pflag = false ;
    //if ( cfrTil && cfrTil->DataStack && ( ( cfrTil->DataStack->StackPointer != Dsp ) || ( debugger->cs_Cpu->State && ( debugger->cs_Cpu->Esi != Dsp ) ) ) )
    if ( debugger->cs_Cpu->State && ( debugger->cs_Cpu->Esi != Dsp ) )
    {
        if ( Is_DebugOn && ( _Q_->Verbosity > 3 ) )
        {
            pflag = true ;
            _Printf ( ( byte* ) "\n_Debugger_SetStackPointerFromDebuggerCpuState : stack pointer adjust ::> debugger->cs_CpuState->State = %d : Dsp = 0x%08x != cpu->Esi = 0x%08x",
                debugger->cs_Cpu->State, Dsp, debugger->cs_Cpu->Esi ) ;
        }
        CpuState_SyncStackPointersFromCpuState ( debugger->cs_Cpu ) ;
        if ( pflag )
        {
            _Printf ( ( byte* ) "\n_Debugger_SetStackPointerFromDebuggerCpuState : stack pointer adjusted ::> cfrTil->DataStack->StackPointer = 0x%08x : Dsp = 0x%08x : cpu->Esi = 0x%08x\n",
                cfrTil->DataStack->StackPointer, Dsp, debugger->cs_Cpu->Esi ) ;
        }
    }
}

void
_CfrTil_SetStackPointerFromDsp ( CfrTil * cfrTil )
{
    int32 *svDsp = Dsp ;
    cfrTil->DataStack->StackPointer = Dsp ;
}

void
CfrTil_SetStackPointerFromDsp ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack && ( cfrTil->DataStack->StackPointer != Dsp ) )
    {
        if ( Is_DebugOn || ( _Q_->Verbosity > 1 ) )
        {
            _Printf ( ( byte* ) "\nCfrTil_SetStackPointerFromDsp : stack pointers adjust ::> DataStack->StackPointer = 0x%08x != Dsp = 0x%08x\n", cfrTil->DataStack->StackPointer, Dsp ) ;
        }
        //if ( cfrTil->Debugger0->cs_Cpu->State ) Debugger_SyncStackPointersFromCpuState ( cfrTil->Debugger0 ) ;
        //else 
        _CfrTil_SetStackPointerFromDsp ( cfrTil ) ;
    }
}

void
CfrTil_SyncStackPointers ( )
{
    CfrTil_SetStackPointerFromDsp ( _CfrTil_ ) ;
}

void
_CfrTil_InitDspFromStackPointer ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack && ( cfrTil->DataStack->StackPointer != Dsp ) )
    {
        if ( Is_DebugOn || ( _Q_->Verbosity > 1 ) )
        {
            _Printf ( ( byte* ) "\n_CfrTil_SetDspFromStackPointer : stack pointers adjust ::> Dsp = 0x%08x != DataStack->StackPointer = 0x%08x\n", Dsp, cfrTil->DataStack->StackPointer ) ;
        }
        Dsp = cfrTil->DataStack->StackPointer ;
    }
}

void
CfrTil_InitDspFromStackPointer ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack ) _CfrTil_InitDspFromStackPointer ( cfrTil ) ;
}

void
CfrTil_DataStack_InitEssential ( CfrTil * cfrTil )
{
    Stack * stk = cfrTil->DataStack ;
    _Stack_Init ( stk, _Q_->DataStackSize ) ;
    _CfrTil_InitDspFromStackPointer ( cfrTil ) ;
    cfrTil->SaveDsp = Dsp ;
}

