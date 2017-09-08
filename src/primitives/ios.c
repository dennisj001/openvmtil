
#include "../include/cfrtil.h"

void
Fflush ( )
{
    fflush ( stdout ) ;
}

void
CfrTil_Kbhit ( void )
{
    _DataStack_Push ( ( int64 ) kbhit ( ) ) ;
}

void
_CfrTil_PrintString ( byte * string ) //  '."'
{
    _Printf ( ( byte* ) string ) ;
}

void
CfrTil_PrintString ( )
{
    _CfrTil_PrintString ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_NewLine ( )
{
    //ConserveNewlines ;
    //int64 sstate = GetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE ) ;
    //AllowNewlines ;
    _Printf ( ( byte* ) "\n" ) ;
    //_Q_->psi_PrintStateInfo->State = sstate ;
}

void
CfrTil_CarriageReturn ( )
{
    _Printf ( ( byte* ) "\r" ) ;
}

void
CfrTil_SPACE ( ) // '.'
{
    _CfrTil_PrintString ( ( byte* ) " " ) ;
}

void
CfrTil_TAB ( ) // '.'
{
    _CfrTil_PrintString ( ( byte* ) "\t" ) ;
}

void
_Print_Binary ( byte* buffer, int64 n )
{
    int64 i, size = 42 ; // 8 - bits/byte ; 4 - spacing
    byte * ptr = & buffer [ size - 1 ] ;
    buffer [ size ] = 0 ;
    for ( i = 0 ; i < size ; i ++ ) buffer [ i ] = ' ' ;
    for ( i = 0 ; i < 32 ; ptr -- )
    {
        if ( n & ( 1 << i ) )
        {
            *ptr = '1' ;
        }
        else
        {
            *ptr = '0' ;
        }
        i ++ ;
        if ( ! ( i % 4 ) ) ptr -- ;
        if ( ! ( i % 8 ) ) ptr -- ;
        //if ( ! ( i % 16 ) ) ptr --;
    }
}

void
Print_Binary ( int64 n, int64 min, int64 max )
{
    if ( n )
    {
        int64 chars, modulo, rem, adj, size = 42 ;
        byte * endOfBuffer, *ptr, buffer [ size ] ; // 8 - bits/byte ; 4 - spacing
        _Print_Binary ( buffer, n ) ;
        endOfBuffer = & buffer [ size ] ; // 1 : dont count final null
        for ( ptr = buffer ; ( * ptr == '0' ) || ( * ptr == ' ' ) ; ptr ++ ) ;
        chars = endOfBuffer - ptr ;
        if ( chars < 5 ) modulo = 4 ;
        else if ( chars < 10 ) modulo = 9 ;
        else if ( chars < 21 ) modulo = 20 ;
        else modulo = 42 ;
        rem = chars % modulo ;
        if ( rem )
        {
            adj = modulo - rem ;
            ptr -= adj ;
        }
        _Printf ( ptr ) ;
    }
    else _Printf ( (byte*) "%d", n ) ;
}

void
PrintfInt ( int64 n )
{
    byte * buffer = Buffer_Data ( _CfrTil_->ScratchB1 ) ;
    if ( _Context_->System0->NumberBase == 10 ) sprintf ( ( char* ) buffer, INT_FRMT, n ) ;
    else if ( _Context_->System0->NumberBase == 2 )
    {
        Print_Binary ( n, 4, 46 ) ;
        return ;
    }
    else /* if ( _Context->System0->NumberBase == 16 ) */ sprintf ( ( char* ) buffer, UINT_FRMT_0x09, n ) ; // hex
    // ?? any and all other number bases ??
    _Printf ( ( byte* ) buffer ) ;
}

void
CfrTil_PrintInt ( )
{
    PrintfInt ( _DataStack_Pop ( ) ) ;
}

void
CfrTil_Emit ( )
{
    int64 c = _DataStack_Pop ( ) ;
    if ( ( c >= 0 ) && ( c < 256 ) ) _Printf ( ( byte* ) "%c", c ) ;
    else Emit ( c ) ; //_Printf ( ( byte* ) "%c", ( ( CString ) c )[0] ) ;
}

void
CfrTil_Key ( )
{
#if 0    
    ReadLine_Key ( _Context_->ReadLiner0 ) ;
    _DataStack_Push ( _Context_->ReadLiner0->InputKeyedCharacter ) ;
#else
    _DataStack_Push ( Key ( ) ) ;

#endif    
}

void
CfrTil_LogOn ( )
{
    _CfrTil_->LogFlag = true ;
    if ( ! _CfrTil_->LogFILE ) _CfrTil_->LogFILE = fopen ( ( char* ) "cfrtil.log", "w" ) ;

}

void
CfrTil_LogAppend ( )
{
    byte * logFilename = ( byte* ) _DataStack_Pop ( ) ;
    _CfrTil_->LogFILE = fopen ( ( char* ) logFilename, "a" ) ;
    CfrTil_LogOn ( ) ;
}

void
CfrTil_LogWrite ( )
{
    byte * logFilename = ( byte* ) _DataStack_Pop ( ) ;
    _CfrTil_->LogFILE = fopen ( ( char* ) logFilename, "w" ) ;
    CfrTil_LogOn ( ) ;
}

void
CfrTil_LogOff ( )
{
    CfrTil * cfrtil = _CfrTil_ ;
    if ( cfrtil )
    {
        fflush ( cfrtil->LogFILE ) ;
        if ( cfrtil->LogFILE ) fclose ( cfrtil->LogFILE ) ; // ? not needed  ?
        cfrtil->LogFlag = false ;
        cfrtil->LogFILE = 0 ;
    }
}

