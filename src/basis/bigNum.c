#include "../include/cfrtil.h"

mpfr_t *
_BigNum_New ( byte * token )
{
    mpfr_t *bfr = ( mpfr_t* ) Mem_Allocate ( sizeof ( mpfr_t ), OBJECT_MEMORY ) ;
    double bf ; 
    int32 bi ;
    if ( token )
    {
        if ( sscanf ( ( char* ) token, "%lf", &bf ) ) mpfr_init_set_d ( *bfr, bf, MPFR_RNDN ) ;
        else if ( sscanf ( ( char* ) token, "%d", &bi ) ) mpfr_init_set_si ( *bfr, bi, MPFR_RNDN ) ;
        else goto done ;
        goto retrn ;
    }
    done :
    mpfr_init_set_si ( *bfr, 0, MPFR_RNDN ) ;
    retrn :
    return bfr ;
}

//"For a, A, e, E, f and F specifiers: this is the number of digits to be printed after the decimal point" 
void
BigNum_PrintfPrecision ( )
{
    mpfr_t * prec = ( mpfr_t* ) _DataStack_Pop ( ) ; // number of decimal digits
    int32 precision = mpfr_get_si ( *prec, MPFR_RNDN ) ;
    _Context_->System0->BigNumPrecision = precision ; // this precision is used by BigNum_FPrint like printf
}

//"For a, A, e, E, f and F specifiers: this is the number of digits to be printed after the decimal point" 
void
BigNum_BitPrecision ( )
{
    mpfr_t * prec = ( mpfr_t* ) _DataStack_Pop ( ) ; // number of decimal digits
    int32 precision = mpfr_get_si ( *prec, MPFR_RNDN ) ;
    //eint32 precision = _DataStack_Pop ( ) ; // number of decimal digits
    //int32 precision = mpfr_get_si ( *prec, MPFR_RNDN ) ;
    mpfr_set_default_prec ( precision ) ; // "precision is the number of bits used to represent the significand of a floating-point number"
}

// width is a parameter to mpfr_printf; it works like printf and sets minimum number of characters to print
// "Minimum number of characters to be printed. If the value to be printed is shorter than this number, 
// the result is padded with blank spaces. The value is not truncated even if the result is larger."
void
BigNum_Width ( )
{
    mpfr_t * mpfwidth = ( mpfr_t* ) _DataStack_Pop ( ) ;
    int32 width = mpfr_get_si ( *mpfwidth, MPFR_RNDN ) ; //, precision = _Context_->System0->BigNumPrecision ;
    _Context_->System0->BigNumWidth = width ; //precision > 1 ? precision : 2 ;
}

void
BigNum_FPrint ( )
{
    mpfr_t * value = ( mpfr_t* ) _DataStack_Pop ( ) ;
    if ( _Q_->Verbosity ) mpfr_printf ( "%*.*Rf", _Context_->System0->BigNumWidth, _Context_->System0->BigNumPrecision, *value ) ;
    fflush ( stdout ) ;
}

// scientific format

void
BigNum_EPrint ( )
{
    mpfr_t * value = ( mpfr_t* ) _DataStack_Pop ( ) ;
    if ( _Q_->Verbosity ) mpfr_printf ( "%*.*Re", _Context_->System0->BigNumWidth, _Context_->System0->BigNumPrecision, *value ) ;
    fflush ( stdout ) ;
}

void
BigNum_Add ( )
{
    mpfr_t *sum = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ), *op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_add ( *sum, *op1, *op2, MPFR_RNDN ) ;
    _DataStack_Push ( ( int32 ) sum ) ;
}

void
BigNum_Multiply ( )
{
    mpfr_t *prod = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ), *op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_mul ( *prod, *op1, *op2, MPFR_RNDN ) ;
    _DataStack_Push ( ( int32 ) prod ) ;
}

void
BigNum_Divide ( )
{
    mpfr_t *quotient = _BigNum_New ( 0 ) ;
    mpfr_t * denominator = ( mpfr_t* ) _DataStack_Pop ( ), *numerator = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_div ( *quotient, *numerator, *denominator, MPFR_RNDN ) ;
    _DataStack_Push ( ( int32 ) quotient ) ;
}

void
BigNum_MultiplyEqual ( )
{
    mpfr_t *prod = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ), **p_op2 = ( mpfr_t** ) _DataStack_Pop ( ) ;
    mpfr_mul ( *prod, *op1, **p_op2, MPFR_RNDN ) ;
    *p_op2 = prod ;
}

void
BigNum_DivideEqual ( ) // remainder discarded
{
    mpfr_t *quotient = _BigNum_New ( 0 ) ;
    mpfr_t * denominator = ( mpfr_t* ) _DataStack_Pop ( ), **p_numerator = ( mpfr_t** ) _DataStack_Pop ( ) ; ;
    
    mpfr_div ( *quotient, **p_numerator, *denominator, MPFR_RNDN ) ;
    *p_numerator = quotient ;
    //_DataStack_Push ( ( int32 ) quotient ) ;
}

void
BigNum_Power ( )
{
    mpfr_t *rop = _BigNum_New ( 0 ) ;
    mpfr_t * expf = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_pow ( *rop, *op1, *expf, MPFR_RNDN ) ;
    _DataStack_Push ( ( int32 ) rop ) ;
}

void
BigNum_SquareRoot ( )
{
    mpfr_t *rop = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_sqrt ( *rop, *op1, MPFR_RNDN ) ;
    _DataStack_Push ( ( int32 ) rop ) ;
}

void
BigNum_Subtract ( )
{
    mpfr_t *diff = _BigNum_New ( 0 ) ;
    mpfr_t * op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_t *op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_sub ( *diff, *op1, *op2, MPFR_RNDN ) ; // diff = op1 - op2
    _DataStack_Push ( ( int32 ) diff ) ;
}

// returns op1 - op2
int32
BigNum_Cmp ( )
{
    mpfr_t * op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_t *op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    return mpfr_cmp ( *op1, *op2 ) ;
}

void
BigNum_Equal ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigNum_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigNum_Cmp ( ) == 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigNum_Cmp ( ) == 0 ? 1 : 0 ) ;
#endif
}

// op1 < op2 => (op1 - op2 < 0 )
void
BigNum_LessThan ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigNum_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigNum_Cmp ( ) < 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( (BigNum_Cmp ( ) < 0) ? 1 : 0 ) ;
#endif
}

// op1 <= op2 => (op1 - op2 <= 0 )
void
BigNum_LessThanOrEqual ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigNum_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigNum_Cmp ( ) > 0 ? 0 : 1 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigNum_Cmp ( ) <= 0 ? 1 : 0 ) ;
#endif
}

// op1 > op2 => (op1 - op2 > 0 )
void
BigNum_GreaterThan ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigNum_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigNum_Cmp ( ) > 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigNum_Cmp ( ) > 0 ? 1 : 0 ) ;
#endif
}

// op1 >= op2 => (op1 - op2 >= 0 )
void
BigNum_GreaterThanOrEqual ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigNum_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigNum_Cmp ( ) < 0 ? 0 : 1 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( (BigNum_Cmp ( ) >= 0) ? 1 : 0 ) ;
#endif
}

// ++

void
BigNum_PlusPlus ( )
{
    mpfr_t *sum = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_GetTop ( ), *op2 = ( mpfr_t* ) _BigNum_New ( "1" ) ;
    mpfr_add ( *sum, *op1, *op2, MPFR_RNDN ) ;
    _DataStack_SetTop ( ( int32 ) sum ) ;
}

void
BigNum_PlusEqual ( )
{
    mpfr_t *sum = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ), **p_op2 = ( mpfr_t** ) _DataStack_Pop ( ) ;
    mpfr_add ( *sum, *op1, **p_op2, MPFR_RNDN ) ;
    *p_op2 = sum ;
    //_DataStack_Push ( ( cell_t ) sum ) ;
}

void
BigNum_MinusEqual ( )
{
    mpfr_t *diff = _BigNum_New ( 0 ) ;
    mpfr_t * op2 = ( mpfr_t* ) _DataStack_Pop ( ), **p_op1 = ( mpfr_t** ) _DataStack_Pop ( ) ;
    mpfr_sub ( *diff, **p_op1, *op2, MPFR_RNDN ) ; // diff = op1 - op2
    *p_op1 = diff ;
    //_DataStack_Push ( ( cell_t ) diff ) ;
}

// --

void
BigNum_MinusMinus ( )
{
    mpfr_t *sum = _BigNum_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_GetTop ( ), *op2 = ( mpfr_t* ) _BigNum_New ( "1" ) ;
    mpfr_sub ( *sum, *op1, *op2, MPFR_RNDN ) ;
    _DataStack_SetTop ( ( int32 ) sum ) ;
}

void
BigNum_LogicalEquals ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigNum_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigNum_Cmp ( ) == 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigNum_Cmp ( ) == 0 ? 1 : 0 ) ;
#endif
}

void
BigNum_LogicalDoesNotEqual ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigNum_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigNum_Cmp ( ) == 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigNum_Cmp ( ) == 0 ? 0 : 1 ) ;
#endif
}

#if 0

void
BigNum_Init ( )
{
    // assuming TOS is a uint32 
    _DataStack_SetTop ( ( int32 ) _BigNum_New ( _DataStack_GetTop ( ) ) ) ;
}

void
BigNum_DivideWithRemainder ( )
{
    mpfr_t *quotient = _BigNum_New ( 0 ) ;
    mpfr_t *remainder = _BigNum_New ( 0 ) ;
    mpfr_t * denominator = ( mpfr_t* ) _DataStack_Pop ( ), *numerator = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_cdiv_qr ( *quotient, *remainder, *numerator, *denominator ) ;
    _DataStack_Push ( ( int32 ) remainder ) ;
    _DataStack_Push ( ( int32 ) quotient ) ;
}

#endif

