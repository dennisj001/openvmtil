
#include "../includes/cfrtil.h"

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
    *++Dsp = value ;
}

void
_DataStack_Dup ( )
{
    _Dup ( ) ;
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
    if ( _Q_ && _Q_->OVT_CfrTil && _DataStack_ )
    {
        _DataStack_->StackPointer = Dsp ;
        return Stack_Depth ( _DataStack_ ) ;
    }
    return 0 ;
}

// safe form with stack checking

int32
DataStack_Pop ( )
{
    _DataStack_->StackPointer = Dsp ;
    _DataStack_Pop ( ) ;
    int32 top = Stack_Pop_WithExceptionOnEmpty ( _DataStack_ ) ;
    return top ;
}

