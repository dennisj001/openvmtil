
#include "../../include/cfrtil.h"

// these functions are part of the C vm and can't be compiled
// ! they are should only be called in C functions !

uint64
_DataStack_Pop ( )
{
    return Dsp -- [ 0 ] ;
}

void
_DataStack_Push ( int64 value )
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
_DataStack_DropN ( int64 n )
{
    Dsp -= n ;
}

void
_DataStack_Drop ( )
{
    Dsp -- ;
}

int64
DataStack_Overflow ( )
{
    return ( Dsp >= _DataStack_->StackMax ) ;
}

int64
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

int64
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

int64
DataStack_Pop ( )
{
    _DataStackPointer_ = Dsp ;
    _DataStack_Pop ( ) ;
    int64 top = Stack_Pop_WithExceptionOnEmpty ( _DataStack_ ) ;
    return top ;
}

void
CpuState_SyncStackPointersFromCpuState ( Cpu * cpus )
{
    if ( cpus->State ) Dsp = ( int64* ) cpus->Esi ;
    _CfrTil_SetStackPointerFromDsp ( _CfrTil_ ) ;
}

void
_Debugger_SyncStackPointersFromCpuState ( Debugger * debugger )
{
    CpuState_SyncStackPointersFromCpuState ( debugger->cs_Cpu ) ;
}

void
Debugger_SyncStackPointersFromCpuState ( Debugger * debugger )
{
    int64 pflag = false ;
    if ( debugger->cs_Cpu->State && ( debugger->cs_Cpu->Esi != Dsp ) )
    {
        if ( Is_DebugModeOn && ( _Q_->Verbosity > 3 ) )
        {
            pflag = true ;
            _Printf ( ( byte* ) "\n_Debugger_SetStackPointerFromDebuggerCpuState : stack pointer adjust ::> debugger->cs_CpuState->State = %d : Dsp = 0x%08x != cpu->Esi = 0x%08x",
                debugger->cs_Cpu->State, Dsp, debugger->cs_Cpu->Esi ) ;
        }
        CpuState_SyncStackPointersFromCpuState ( debugger->cs_Cpu ) ;
        if ( pflag )
        {
            _Printf ( ( byte* ) "\n_Debugger_SetStackPointerFromDebuggerCpuState : stack pointer adjusted ::> cfrTil->DataStack->StackPointer = 0x%08x : Dsp = 0x%08x : cpu->Esi = 0x%08x\n",
                _CfrTil_->DataStack->StackPointer, Dsp, debugger->cs_Cpu->Esi ) ;
        }
    }
    else  _CfrTil_SetStackPointerFromDsp ( _CfrTil_ ) ;
}

void
_CfrTil_SetStackPointerFromDsp ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack ) cfrTil->DataStack->StackPointer = Dsp ;
}

void
CfrTil_SetStackPointerFromDsp ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack ) //&& ( cfrTil->DataStack->StackPointer != Dsp ) )
    {
        if ( ( Is_DebugModeOn || ( _Q_->Verbosity > 3 ) ) && ( cfrTil->DataStack->StackPointer != Dsp ) )
        {
            _Printf ( ( byte* ) "\nCfrTil_SetStackPointerFromDsp : stack pointers adjust ::> DataStack->StackPointer = 0x%08x != Dsp = 0x%08x\n", cfrTil->DataStack->StackPointer, Dsp ) ;
        }
        _CfrTil_SetStackPointerFromDsp ( cfrTil ) ;
    }
}

void
CfrTil_SyncStackPointersFromDsp ( )
{
    CfrTil_SetStackPointerFromDsp ( _CfrTil_ ) ;
}

#if 0
void
CfrTil_SyncStackPointersFromESI ( )
{
    
}
#endif

void
_CfrTil_InitDspFromStackPointer ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack && ( cfrTil->DataStack->StackPointer != Dsp ) )
    {
        if ( Is_DebugModeOn || ( _Q_->Verbosity > 1 ) )
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

