
#include "../../include/cfrtil.h"

void
_System_TimerInit ( System * system, int64 i )
{
    clock_gettime ( CLOCK_REALTIME, &system->Timers [ i ] ) ;
    //clock_gettime ( CLOCK_MONOTONIC, &system->Timers [ i ] ) ;
}

void
_System_Time ( System * system, uint64 timer, char * format, byte * toString )
{
    if ( timer < 8 )
    {
        __time_t seconds, seconds2 ;
        int64 nseconds, nseconds2 ;
        seconds = system->Timers [ timer ].tv_sec ;
        nseconds = system->Timers [ timer ].tv_nsec ;
        _System_TimerInit ( system, timer ) ;
        seconds2 = system->Timers [ timer ].tv_sec ;
        nseconds2 = system->Timers [ timer ].tv_nsec ;
        //clock_settime ( CLOCK_MONOTONIC, &system->Timers [ timer ] ) ;
        //clock_settime ( CLOCK_REALTIME, &system->Timers [ timer ] ) ;
        if ( nseconds > nseconds2 )
        {
            seconds2 -- ;
            nseconds2 += 1000000000 ;
        }
        sprintf ( ( char* ) toString, format, seconds2 - seconds, nseconds2 - nseconds ) ;
    }
    else Throw ( ( byte* ) "Error: timer index must be less than 8", QUIT ) ;
}

void
System_Time ( System * system, uint64 timer, char * string, int64 tflag )
{
    byte buffer [ 64 ] ;
    _System_Time ( system, timer, ( char* ) "%ld.%09ld", buffer ) ;
    if ( tflag && ( _Q_->Verbosity ) ) _Printf ( ( byte* ) "\n%s [ %d ] : elapsed time = %s seconds", string, timer, buffer ) ;
    //if ( tflag ) _Printf ( ( byte* ) "\n%s [ %d ] : elapsed time = %s seconds", string, timer, buffer ) ;
}

void
System_InitTime ( System * system )
{
    int64 i ;
    for ( i = 0 ; i < 8 ; i ++ )
    {
        _System_TimerInit ( system, i ) ;
    }
}

void
System_RunInit ( System * system )
{
    //ConserveNewlines  ;
    if ( _Q_->Signal > QUIT )
    {
        CfrTil_DataStack_Init () ;
    }
    else
    {
        CfrTil_InitDspFromStackPointer ( _CfrTil_ ) ; // for preserving stack on "quit"
        if ( DataStack_Underflow ( ) || ( DataStack_Overflow ( ) ) )
        {
            CfrTil_PrintDataStack ( ) ;
        }
    }
}

void
_System_Copy ( System * system, System * system0, uint64 type )
{
    memcpy ( system, system0, sizeof (System ) ) ;
}

System *
System_Copy ( System * system0, uint64 type )
{
    System * system = ( System * ) Mem_Allocate ( sizeof ( System ), type ) ;
    _System_Copy ( system, system0, type ) ;
    return system ;
}

#if 0
void
System_Delete ( System * system )
{
    Mem_FreeItem ( _Q_->PermanentMemList, ( byte* ) system ) ;
}
#endif

// BigNumWidth is a parameter to mpfr_printf; it works like printf and sets minimum number of characters to print
void
_System_Init ( System * system )
{
    system->NumberBase = 10 ;
    int64 width = 20, precision = 20 ; 
    //system->BigNumPrecision = precision ; // digits here
    mpfr_set_default_prec ( ( ( precision / 3 ) * 10 ) + 16 ) ; // bits here :: + 16 : add 5 extra digits of precision :: "precision is the number of bits used to represent the significand of a floating-point number"
    //system->BigNumWidth = width < system->BigNumPrecision - 4 ? width : system->BigNumPrecision ; // digits here
    system->IncludeFileStackNumber = 0 ;
}

void
System_Init ( System * system )
{
    system->State = 0 ;
    SetState ( system, ADD_READLINE_TO_HISTORY | ALWAYS_ABORT_ON_EXCEPION, true ) ;
    _System_Init ( system ) ;
}

System *
System_New ( uint64 type )
{
    System * system = ( System * ) Mem_Allocate ( sizeof (System ), type ) ;
    System_Init ( system ) ;
    System_InitTime ( system ) ;
    return system ;
}

// ( address number -- )

