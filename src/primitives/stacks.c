#include "../include/cfrtil.h"

void
CfrTil_Dsp ( )
{
    _DataStack_Push ( ( int64 ) Dsp ) ;
}

#if 0 //use macros

void
Drop ( )
{
    Dsp -- ;
}

void
DropN ( int64 n )
{
    Dsp -= n ;
}
#endif

void
CfrTil_Drop ( )
{
    if ( CompileMode )
    {
        //Word * one = Compiler_WordStack ( - 1 ) ;
        Word * one = Compiler_WordList ( 1 ) ;
        if ( GetState ( _CfrTil_, OPTIMIZE_ON ) && one && ( one->StackPushRegisterCode ) ) SetHere ( one->StackPushRegisterCode ) ;
        else _Compile_Stack_Drop ( DSP ) ;
    }
    else
    {

        DSP_Drop ( ) ;
    }
}

void
CfrTil_DropN ( )
{
    if ( CompileMode ) _Compile_Stack_DropN ( DSP, _DataStack_Pop ( ) ) ;
    else _DataStack_DropN ( TOS + 1 ) ;
}

void
_CfrTil_Push ( int64 value )
{
    if ( CompileMode )
    {
        _Compile_Stack_Push ( DSP, value ) ;
    }
    else
    {
        _DataStack_Push ( value ) ;
    }
}

void
CfrTil_Dup ( )
{
    if ( CompileMode )
    {
        _Compile_Stack_Dup ( DSP ) ;
    }
    else
    {

        _DataStack_Dup ( ) ;
    }
}

// result is as if one did n dups in a row 

void
CfrTil_NDup ( )
{
    int64 n = TOS ;
    int64 value = * -- Dsp ; // -1 : n now occupies 1 to be also used slot
    while ( n -- )
    {

        * ++ Dsp = value ;
    }
}

// pick is from stack below top index
// 0 pick is Dsp [ 0] - TOS 
// 1 pick is Dsp [-1]
// ..., etc.

void
CfrTil_Pick ( ) // pick
{
    if ( CompileMode )
    {
        _Compile_Stack_Pick ( DSP ) ;
    }
    else
    {
        //* Dsp = ( * ( Dsp - * ( Dsp ) - 1 ) ) ;
        //int64 top = Dsp [0] ;
        Dsp [0] = Dsp [ - (Dsp [0] + 1) ] ;
    }
}

void
CfrTil_Swap ( )
{
    if ( CompileMode )
    {
        _Compile_Stack_Swap ( DSP ) ;
    }
    else
    {
        int64 a = TOS ;
        TOS = Dsp [ - 1 ] ;
        Dsp [ - 1 ] = a ;
    }
}

void
CfrTil_PrintNReturnStack ( )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    int64 size = _DataStack_Pop ( ) ;
    _CfrTil_PrintNReturnStack ( size ) ;
}

void
CfrTil_PrintNDataStack ( )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    int64 size = _DataStack_Pop ( ) ;
    _CfrTil_PrintNDataStack ( size ) ;
}

void
CfrTil_PrintReturnStack ( )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    _CfrTil_PrintNReturnStack ( 8 ) ;
}

void
CfrTil_PrintNDataStack_8 ( )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    _CfrTil_PrintNDataStack ( 8 ) ;
}

void
_CfrTil_PrintDataStack ( )
{
    _Stack_Print ( _DataStack_, ( byte* ) "DataStack" ) ;
}

void
CfrTil_PrintDataStack ( )
{
    //CfrTil_SetStackPointerFromDsp ( _CfrTil_ ) ;
    CfrTil_SyncStackPointersFromDsp ( ) ;
    _CfrTil_PrintDataStack ( ) ;
}

void
CfrTil_CheckInitDataStack ( )
{
    CfrTil_SetStackPointerFromDsp ( _CfrTil_ ) ;
    if ( Stack_Depth ( _DataStack_ ) < 0 )
    {
        _Stack_PrintHeader ( _DataStack_, ( byte* ) "DataStack" ) ;
        _Printf ( ( byte* ) c_ad ( "\n\nError : %s : %s : Stack Underflow!" ), _Context_->CurrentlyRunningWord ? _Context_->CurrentlyRunningWord->Name : ( byte * ) "", _Context_Location ( _Context_ ) ) ;
        _Printf ( ( byte* ) c_dd ( "\nReseting DataStack.\n" ) ) ;
        _CfrTil_DataStack_Init ( _CfrTil_ ) ;
        _Stack_PrintHeader ( _DataStack_, ( byte* ) "DataStack" ) ;
    }
    //_Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_DataStack_Size ( )
{
    _DataStack_Push ( DataStack_Depth ( ) ) ;
}

