#include "../include/cfrtil.h"

mpfr_t *
_BigNum_New ( byte * token )
{
    mpfr_t *bfr = ( mpfr_t* ) Mem_Allocate ( sizeof ( mpfr_t ), OBJECT_MEMORY ) ;
    double bf ; 
    int64 bi ;
    if ( token )
    {
        if ( sscanf ( ( char* ) token, "%lf", &bf ) ) mpfr_init_set_d ( *bfr, bf, MPFR_RNDN ) ;
        else if ( sscanf ( ( char* ) token, "%ld", &bi ) ) mpfr_init_set_si ( *bfr, bi, MPFR_RNDN ) ;
        else goto done ;
        goto retrn ;
    }
    done :
    mpfr_init_set_si ( *bfr, 0, MPFR_RNDN ) ;
    retrn :
    return bfr ;
}

mpfr_t bn_precision ;
mpfr_t bn_width ;

//"For a, A, e, E, f and F specifiers: this is the number of digits to be printed after the decimal point" 
mpfr_t *
BigNum_GetPrintfPrecision_BigNum ( )
{
    _DataStack_Push ( _Context_->System0->BigNum_Printf_Precision ) ; // this precision is used by BigNum_FPrint like printf
}

// width is a parameter to mpfr_printf; it works like printf and sets minimum number of characters to print
// "Minimum number of characters to be printed. If the value to be printed is shorter than this number, 
// the result is padded with blank spaces. The value is not truncated even if the result is larger."
void
BigNum_GetPrintfWidth ()
{
    _DataStack_Push ( (int64) _Context_->System0->BigNum_Printf_Width ) ;
}

// set from BigNum 
void
BigNum_Set_PrintfPrecision ( )
{
    mpfr_t * prec = ( mpfr_t* ) _DataStack_Pop ( ) ; // number of decimal digits
    int64 precision = mpfr_get_si ( *prec, MPFR_RNDN ) ;
    _Context_->System0->BigNum_Printf_Precision = precision ; // this precision is used by BigNum_FPrint like printf
}

// set from BigNum 
void
BigNum_Set_PrintfWidth ( )
{
    mpfr_t * mpfwidth = ( mpfr_t* ) _DataStack_Pop ( ) ;
    int64 width = mpfr_get_si ( *mpfwidth, MPFR_RNDN ) ; 
    _Context_->System0->BigNum_Printf_Width = width ; 
}

// internal mpfr bit precision
void
BigNum_GetAndPrint_BitPrecision ( )
{
    //mpfr_prec_t == long
    mpfr_prec_t precision = mpfr_get_default_prec ()  ; // number of decimal digits
    _Printf ( (byte*) "\nBigNum Internal Bit Precision = %ld", precision ) ;
}

// set from BigNum 
void
BigNum_SetDefaultBitPrecision ( )
{
    mpfr_t * prec = ( mpfr_t* ) _DataStack_Pop ( ) ; // number of decimal digits
    long precision = mpfr_get_si ( *prec, MPFR_RNDN ) ;
    mpfr_set_default_prec ( precision ) ; // "precision is the number of bits used to represent the significand of a floating-point number"
}

void
BigNum_StateShow ()
{
    BigNum_GetAndPrint_BitPrecision ( ) ;
    _Printf ( (byte *) "\nBigNum :: Width = %d : Preciosn = %d", _Context_->System0->BigNum_Printf_Width, _Context_->System0->BigNum_Printf_Precision ) ;
}

void
BigNum_FPrint ( )
{
    mpfr_t * value = ( mpfr_t* ) _DataStack_Pop ( ) ;
    if ( _Q_->Verbosity ) mpfr_printf ( "%*.*Rf", _Context_->System0->BigNum_Printf_Width, _Context_->System0->BigNum_Printf_Precision, *value ) ;
    fflush ( stdout ) ;
}

// scientific format

void
BigNum_EPrint ( )
{
    mpfr_t * value = ( mpfr_t* ) _DataStack_Pop ( ) ;
    if ( _Q_->Verbosity ) mpfr_printf ( "%*.*Re", _Context_->System0->BigNum_Printf_Width, _Context_->System0->BigNum_Printf_Precision, *value ) ;
    fflush ( stdout ) ;
}

void
BigNum_PopTwoOpsPushResult ( mpf2andOutFunc func )
{
    mpfr_t *result = _BigNum_New ( 0 ) ;
    mpfr_t *op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_t * op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    func ( *result, *op2, *op1, MPFR_RNDN ) ; 
    _DataStack_Push ( ( int64 ) result ) ;
}

void
BigNum_Add ( )
{
    BigNum_PopTwoOpsPushResult ( mpfr_add ) ;
}
void
BigNum_Multiply ( )
{
    BigNum_PopTwoOpsPushResult ( mpfr_mul ) ;
}

void
BigNum_Divide ( )
{
    BigNum_PopTwoOpsPushResult ( mpfr_div ) ;
}

void
BigNum_Subtract ( )
{
    BigNum_PopTwoOpsPushResult ( mpfr_sub ) ;
}

void
_BigNum_OpEqualTemplate ( mpf2andOutFunc func )
{
    mpfr_t *result = _BigNum_New ( 0 ) ;
    mpfr_t * op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_t **p_op1 = ( mpfr_t** ) _DataStack_Pop ( ) ;
    func ( *result, **p_op1, *op2, MPFR_RNDN ) ; 
    *p_op1 = result ;
}

void
BigNum_PlusEqual ( )
#if 0
{
    mpfr_t *sum = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ), **p_op2 = ( mpfr_t** ) _DataStack_Pop ( ) ;
    mpfr_add ( *sum, *op1, **p_op2, MPFR_RNDN ) ;
    *p_op2 = sum ;
}
#else
{
    _BigNum_OpEqualTemplate ( mpfr_add ) ;
}
#endif

void
BigNum_MinusEqual ( )
#if 0
{
    mpfr_t *diff = _BigNum_New ( 0 ) ;
    mpfr_t * op2 = ( mpfr_t* ) _DataStack_Pop ( ), **p_op1 = ( mpfr_t** ) _DataStack_Pop ( ) ;
    mpfr_sub ( *diff, **p_op1, *op2, MPFR_RNDN ) ; 
    *p_op1 = diff ;
#else
{
    _BigNum_OpEqualTemplate ( mpfr_sub ) ;
}
#endif
    
void
BigNum_MultiplyEqual ( )
#if 0
{
    mpfr_t *prod = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ), **p_op2 = ( mpfr_t** ) _DataStack_Pop ( ) ;
    mpfr_mul ( *prod, *op1, **p_op2, MPFR_RNDN ) ;
    *p_op2 = prod ;
}
#else
{
    _BigNum_OpEqualTemplate ( mpfr_mul ) ;
}
#endif

void
BigNum_DivideEqual ( ) // remainder discarded
#if 0
{
    mpfr_t *quotient = _BigNum_New ( 0 ) ;
    mpfr_t * denominator = ( mpfr_t* ) _DataStack_Pop ( ), **p_numerator = ( mpfr_t** ) _DataStack_Pop ( ) ; ;
    mpfr_div ( *quotient, **p_numerator, *denominator, MPFR_RNDN ) ;
    *p_numerator = quotient ;
    //_DataStack_Push ( ( int64 ) quotient ) ;
}
#else
{
    _BigNum_OpEqualTemplate ( mpfr_div ) ;
}
#endif

// ++

void
BigNum_PlusPlus ( )
{
    mpfr_t *sum = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_GetTop ( ), *op2 = ( mpfr_t* ) _BigNum_New ( "1" ) ;
    mpfr_add ( *sum, *op1, *op2, MPFR_RNDN ) ;
    _DataStack_SetTop ( ( int64 ) sum ) ;
}

void
BigNum_MinusMinus ( )
{
    mpfr_t *sum = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_GetTop ( ), *op2 = ( mpfr_t* ) _BigNum_New ( "1" ) ;
    mpfr_sub ( *sum, *op1, *op2, MPFR_RNDN ) ;
    _DataStack_SetTop ( ( int64 ) sum ) ;
}

void
BigNum_SquareRoot ( )
{
    mpfr_t *rop = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_sqrt ( *rop, *op1, MPFR_RNDN ) ;
    _DataStack_Push ( ( int64 ) rop ) ;
}

void
BigNum_Power ( )
{
    mpfr_t *rop = _BigNum_New ( 0 ) ;
    mpfr_t * expf = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_pow ( *rop, *op1, *expf, MPFR_RNDN ) ;
    _DataStack_Push ( ( int64 ) rop ) ;
}

// returns op1 - op2
int64
BigNum_Cmp ( )
{
    mpfr_t * op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_t *op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    return mpfr_cmp ( *op1, *op2 ) ;
}

void
BigNum_Equal ( )
{
    _DataStack_Push ( BigNum_Cmp ( ) == 0 ? 1 : 0 ) ;
}

// op1 < op2 => (op1 - op2 < 0 )
void
BigNum_LessThan ( )
{
    _DataStack_Push ( (BigNum_Cmp ( ) < 0) ? 1 : 0 ) ;
}

// op1 <= op2 => (op1 - op2 <= 0 )
void
BigNum_LessThanOrEqual ( )
{
    _DataStack_Push ( BigNum_Cmp ( ) <= 0 ? 1 : 0 ) ;
}

// op1 > op2 => (op1 - op2 > 0 )
void
BigNum_GreaterThan ( )
{
    _DataStack_Push ( BigNum_Cmp ( ) > 0 ? 1 : 0 ) ;
}

// op1 >= op2 => (op1 - op2 >= 0 )
void
BigNum_GreaterThanOrEqual ( )
{
    _DataStack_Push ( (BigNum_Cmp ( ) >= 0) ? 1 : 0 ) ;
}

void
BigNum_LogicalEquals ( )
{
    _DataStack_Push ( BigNum_Cmp ( ) == 0 ? 1 : 0 ) ;
}

void
BigNum_LogicalDoesNotEqual ( )
{
    _DataStack_Push ( BigNum_Cmp ( ) == 0 ? 0 : 1 ) ;
}

#if 0

void
BigNum_Init ( )
{
    // assuming TOS is a uint64 
    _DataStack_SetTop ( ( int64 ) _BigNum_New ( _DataStack_GetTop ( ) ) ) ;
}

void
BigNum_DivideWithRemainder ( )
{
    mpfr_t *quotient = _BigNum_New ( 0 ) ;
    mpfr_t *remainder = _BigNum_New ( 0 ) ;
    mpfr_t * denominator = ( mpfr_t* ) _DataStack_Pop ( ), *numerator = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_cdiv_qr ( *quotient, *remainder, *numerator, *denominator ) ;
    _DataStack_Push ( ( int64 ) remainder ) ;
    _DataStack_Push ( ( int64 ) quotient ) ;
}

void
BigNum_Add ( )
{
    mpfr_t *sum = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ), *op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_add ( *sum, *op1, *op2, MPFR_RNDN ) ;
    _DataStack_Push ( ( int64 ) sum ) ;
}

void
BigNum_Multiply ( )
{
    mpfr_t *prod = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ), *op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_mul ( *prod, *op1, *op2, MPFR_RNDN ) ;
    _DataStack_Push ( ( int64 ) prod ) ;
}

void
BigNum_Divide ( )
{
    mpfr_t *quotient = _BigNum_New ( 0 ) ;
    mpfr_t * denominator = ( mpfr_t* ) _DataStack_Pop ( ), *numerator = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_div ( *quotient, *numerator, *denominator, MPFR_RNDN ) ;
    _DataStack_Push ( ( int64 ) quotient ) ;
}

void
BigNum_Subtract ( )
{
    mpfr_t *diff = _BigNum_New ( 0 ) ;
    mpfr_t * op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_t *op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_sub ( *diff, *op1, *op2, MPFR_RNDN ) ; // diff = op1 - op2
    _DataStack_Push ( ( int64 ) diff ) ;
}
void
BigNum_GetPrintfPrecision_Pointer ( )
{
    _DataStack_Push ( (int64) &_Context_->System0->BigNumPrecision ) ;
}
void
BigNum_GetPrintfWidth_Pointer ( )
{
     _DataStack_Push ( (int64) &_Context_->System0->BigNumWidth ) ;
}

#endif


