
#include "../include/cfrtil32.h"

void
CfrTil_Power_03 ( ) // **
{
    int32 pow = Dsp [ 0 ], base = Dsp [ -1 ], n ;
    for ( n = base ; -- pow ; )
    {
        n *= base ;
    }
    Dsp [ -1 ] = n ;
    Dsp -- ;
}

int32
_CFib_O3 ( int n )
{
    if ( n < 2 ) return n ;
    else return ( _CFib_O3 ( n - 1 ) + _CFib_O3 ( n - 2 ) ) ; 
}

void
CFib_O3 ( )
{
    TOS = ( _CFib_O3 ( TOS ) ) ;
}

int32
_CFib2_O3 ( int n )
{
    int32 fn, fn1, fn2 ;
    for ( fn = 0, fn1 = 0, fn2 = 1 ; n ; n-- ) 
    {   
        fn1 = fn2 ; 
        fn2 = fn ;
        fn = fn1 + fn2 ; 
    }
    return fn ;
}

void
CFib2_O3 ( )
{
    TOS = ( _CFib2_O3 ( TOS ) ) ;
}

void
CFactorial_O3 ( )
{
    int32 n = TOS ;
    if ( n > 1 )
    {
        TOS = TOS - 1 ;
        CFactorial_O3 ( ) ;
        TOS *= n ;
    }
    else TOS = 1 ;
}

int32
_CFactorial_O3 ( int32 n )
{
    if ( n > 1 ) return ( n * _CFactorial_O3 ( n - 1 ) ) ;
    else return 1 ;
}

void
CFactorial2_O3 ( )
{
    TOS = ( _CFactorial_O3 ( TOS ) ) ;
}

void
CFactorial3_O3 ( void )
{
    int32 rec1 = 1, n = TOS ;
    while ( n > 1 )
    {

        rec1 *= n -- ;
    }
    TOS = rec1 ;
}

