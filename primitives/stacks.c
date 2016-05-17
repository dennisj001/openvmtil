#include "../includes/cfrtil.h"

void
CfrTil_Dsp ( )
{
    _DataStack_Push ( ( int32 ) Dsp ) ;
}

#if 0 //use macros

void
Drop ( )
{
    Dsp -- ;
}

void
DropN ( int n )
{
    Dsp -= n ;
}
#endif

void
CfrTil_Drop ( )
{
    if ( CompileMode )
    {
        Word * one = Compiler_WordStack ( -1 ) ;
        if ( GetState ( _Q_->OVT_CfrTil, OPTIMIZE_ON ) && one->StackPushRegisterCode ) SetHere ( one->StackPushRegisterCode ) ;
        else _Compile_Stack_Drop ( DSP ) ;
    }
    else
    {

        _Drop ( ) ;
    }
}

void
CfrTil_DropN ( )
{
    if ( CompileMode ) _Compile_Stack_DropN ( DSP, _DataStack_Pop ( ) ) ;

    else _DataStack_DropN ( TOS + 1 ) ;
}

void
_CfrTil_Push ( int32 value )
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
    int32 n = TOS ;
    int32 value = * -- Dsp ; // -1 : n now occupies 1 to be also used slot
    while ( n -- )
    {

        * ++ Dsp = value ;
    }
}

// pick is from stack below top index

void
CfrTil_Pick ( ) // pick
{
    if ( CompileMode )
    {
        _Compile_Stack_Pick ( DSP ) ;
    }
    else
    {

        * Dsp = ( * ( Dsp - * ( Dsp ) - 1 ) ) ;
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
        int32 a = TOS ;
        TOS = Dsp [ - 1 ] ;
        Dsp [ - 1 ] = a ;
    }
}

void
CfrTil_PrintNReturnStack ( )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    int32 size = _DataStack_Pop ( ) ;
    _CfrTil_PrintNReturnStack ( size ) ;
}

void
CfrTil_PrintReturnStack ( )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    _CfrTil_PrintNReturnStack ( 8 ) ;
}

void
CfrTil_PrintDataStack ( )
{
    CfrTil_SyncStackPointerFromDsp ( _Q_->OVT_CfrTil ) ;
    _Stack_Print ( _DataStack_, ( byte* ) "DataStack" ) ;
    Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_CheckInitDataStack ( )
{
    CfrTil_SyncStackPointerFromDsp ( _Q_->OVT_CfrTil ) ;
    if ( Stack_Depth ( _DataStack_ ) < 0 )
    {
        _Stack_PrintHeader ( _DataStack_, ( byte* ) "DataStack" ) ;
        Printf ( ( byte* ) c_ad ( "\n\nError : %s : %s : Stack Underflow!" ), _Context_->CurrentRunWord ? _Context_->CurrentRunWord->Name : ( byte * ) "", _Context_Location ( _Context_ ) ) ;
        Printf ( ( byte* ) c_dd ( "\nReseting DataStack.\n" ) ) ;
        _CfrTil_DataStack_Init ( _Q_->OVT_CfrTil ) ;
        _Stack_PrintHeader ( _DataStack_, ( byte* ) "DataStack" ) ;
    }
    Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_DataStack_Size ( )
{
    _DataStack_Push ( DataStack_Depth ( ) ) ;
}

