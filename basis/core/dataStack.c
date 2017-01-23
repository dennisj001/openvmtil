
#include "../../include/cfrtil.h"

// these functions are part of the C vm and can't be compiled
// ! they are should only be called in C functions !

int32
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
    //_Dup ( ) ;
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
_CfrTil_SetStackPointerFromDebuggerCpuState ( CfrTil * cfrTil )
{
    if ( GetState ( cfrTil->Debugger0, DBG_REGS_SAVED ) ) cfrTil->DataStack->StackPointer = ( int* ) cfrTil->Debugger0->cs_CpuState->Esi ;
}

void
_CfrTil_SetStackPointerFromDsp ( CfrTil * cfrTil )
{
    //d1 ( //if ( cfrTil->DataStack->StackPointer != Dsp ) 
    cfrTil->DataStack->StackPointer = Dsp ;
}

void
CfrTil_SetStackPointerFromDsp ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack && ( cfrTil->DataStack->StackPointer != Dsp ) )
    {
        if ( Is_DebugOn )
        {
            Printf ( "\n\nBefore DataStack pointer adjust ::> DataStack->StackPointer = 0x%08x <::-- Dsp = 0x%08x\n\n", cfrTil->DataStack->StackPointer, Dsp ) ;
        }
        _CfrTil_SetStackPointerFromDsp ( cfrTil ) ;
    }
}

void
_CfrTil_SetDspFromStackPointer ( CfrTil * cfrTil )
{
    //d1 ( //if ( cfrTil->DataStack->StackPointer != Dsp ) 
    if ( Is_DebugOn )
    {
        Printf ( "\n\nBefore Dsp (ESI) adjust ::> Dsp = 0x%08x <::-- DataStack->StackPointer = 0x%08x\n\n", Dsp, cfrTil->DataStack->StackPointer ) ;
        //Pause () ;
    } //) ;
    Dsp = cfrTil->DataStack->StackPointer ;
}

void
CfrTil_SetDspFromStackPointer ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack ) _CfrTil_SetDspFromStackPointer ( cfrTil ) ;
}

void
CfrTil_DataStack_InitEssential ( CfrTil * cfrTil )
{
    Stack * stk = cfrTil->DataStack ;
    _Stack_Init ( stk, _Q_->DataStackSize ) ;
    _CfrTil_SetDspFromStackPointer ( cfrTil ) ;
    cfrTil->SaveDsp = Dsp ;
}

