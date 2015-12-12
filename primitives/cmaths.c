
#include "../includes/cfrtil.h"

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

