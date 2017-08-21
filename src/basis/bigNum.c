#include "../include/cfrtil.h"
//#include "/usr/local/include/gmp.h"

mpz_t *
_BigInt_New ( int32 initializer )
{
    //cell allocationType ;
    //if ( GetState( _Context_->Compiler0, BLOCK_MODE ) ) allocationType = OBJECT_MEMORY ; 
    //else allocationType = SESSION ;
    mpz_t *bn = ( mpz_t* ) Mem_Allocate ( sizeof ( mpz_t ), OBJECT_MEMORY ) ;
    mpz_init_set_si ( *bn, initializer ) ;
    return bn ;
}

void
BigInt_Init ( )
{
    // assuming TOS is a uint32 
    _DataStack_SetTop ( ( int32 ) _BigInt_New ( _DataStack_GetTop ( ) ) ) ;
}

void
BigInt_Add ( )
{
    mpz_t *sum = _BigInt_New ( 0 ) ;
    mpz_t * op1 = ( mpz_t* ) _DataStack_Pop ( ), *op2 = ( mpz_t* ) _DataStack_Pop ( ) ;
    mpz_add ( *sum, *op1, *op2 ) ;
    _DataStack_Push ( ( int32 ) sum ) ;
}

void
BigInt_PlusEqual ( )
{
    mpz_t *sum = _BigInt_New ( 0 ) ;
    mpz_t * op1 = ( mpz_t* ) _DataStack_Pop ( ), **p_op2 = ( mpz_t** ) _DataStack_Pop ( ) ;
    mpz_add ( *sum, *op1, **p_op2 ) ;
    *p_op2 = sum ;
    //_DataStack_Push ( ( cell_t ) sum ) ;
}

// --

void
BigInt_MinusMinus ( )
{
    mpz_t *sum = _BigInt_New ( 0 ) ;
    mpz_t * op1 = ( mpz_t* ) _DataStack_GetTop ( ), *op2 = ( mpz_t* ) _BigInt_New ( 1 ) ;
    mpz_sub ( *sum, *op1, *op2 ) ;
    _DataStack_SetTop ( ( int32 ) sum ) ;
}

// ++

void
BigInt_PlusPlus ( )
{
    mpz_t *sum = _BigInt_New ( 0 ) ;
    mpz_t * op1 = ( mpz_t* ) _DataStack_GetTop ( ), *op2 = ( mpz_t* ) _BigInt_New ( 1 ) ;
    mpz_add ( *sum, *op1, *op2 ) ;
    _DataStack_SetTop ( ( int32 ) sum ) ;
}

void
BigInt_Multiply ( )
{
    mpz_t *prod = _BigInt_New ( 0 ) ;
    mpz_t * op1 = ( mpz_t* ) _DataStack_Pop ( ), *op2 = ( mpz_t* ) _DataStack_Pop ( ) ;
    mpz_mul ( *prod, *op1, *op2 ) ;
    //mpz_clear ( *op1 ) ; mpz_clear ( *op2 ) ;
    _DataStack_Push ( ( int32 ) prod ) ;
}

void
BigInt_Power ( )
{
    //void mpz_pow_ui (mpz_t rop, mpz_t base, unsigned long int exp)
    mpz_t *rop = _BigInt_New ( 0 ) ;
    // unsigned long int mpz_get_ui (mpz_t exp) 
    mpz_t * expz = ( mpz_t* ) _DataStack_Pop ( ) ;
    int32 exp = mpz_get_ui ( *expz ) ;
    mpz_t * base = ( mpz_t* ) _DataStack_Pop ( ) ;
    mpz_pow_ui ( *rop, *base, exp ) ;
    _DataStack_Push ( ( int32 ) rop ) ;
}

void
BigInt_MultiplyEqual ( )
{
    mpz_t *prod = _BigInt_New ( 0 ) ;
    mpz_t * op1 = ( mpz_t* ) _DataStack_Pop ( ), **p_op2 = ( mpz_t** ) _DataStack_Pop ( ) ;
    mpz_mul ( *prod, *op1, **p_op2 ) ;
    *p_op2 = prod ;
}

void
BigInt_DivideWithRemainder ( )
{
    mpz_t *quotient = _BigInt_New ( 0 ) ;
    mpz_t *remainder = _BigInt_New ( 0 ) ;
    mpz_t * denominator = ( mpz_t* ) _DataStack_Pop ( ), *numerator = ( mpz_t* ) _DataStack_Pop ( ) ;
    mpz_cdiv_qr ( *quotient, *remainder, *numerator, *denominator ) ;
    _DataStack_Push ( ( int32 ) remainder ) ;
    _DataStack_Push ( ( int32 ) quotient ) ;
}

void
BigInt_DivideEqual_RemainderDiscarded ( ) // remainder discarded
{
    mpz_t *quotient = _BigInt_New ( 0 ) ;
    mpz_t *remainder = _BigInt_New ( 0 ) ;
    mpz_t * denominator = ( mpz_t* ) _DataStack_Pop ( ), **numerator = ( mpz_t** ) _DataStack_Pop ( ) ;
    mpz_cdiv_qr ( *quotient, *remainder, **numerator, *denominator ) ;
    *numerator = quotient ;
    //_DataStack_Push ( ( cell_t ) remainder ) ;
    //_DataStack_Push ( ( cell_t ) quotient ) ;
}

void
BigInt_Subtract ( )
{
    mpz_t *diff = _BigInt_New ( 0 ) ;
    mpz_t * op2 = ( mpz_t* ) _DataStack_Pop ( ), *op1 = ( mpz_t* ) _DataStack_Pop ( ) ;
    mpz_sub ( *diff, *op1, *op2 ) ; // diff = op1 - op2
    _DataStack_Push ( ( int32 ) diff ) ;
}

void
BigInt_MinusEqual ( )
{
    mpz_t *diff = _BigInt_New ( 0 ) ;
    mpz_t * op2 = ( mpz_t* ) _DataStack_Pop ( ), **p_op1 = ( mpz_t** ) _DataStack_Pop ( ) ;
    mpz_sub ( *diff, **p_op1, *op2 ) ; // diff = op1 - op2
    *p_op1 = diff ;
    //_DataStack_Push ( ( cell_t ) diff ) ;
}

int32
BigInt_Cmp ( )
{
    mpz_t * op2 = ( mpz_t* ) _DataStack_Pop ( ), *op1 = ( mpz_t* ) _DataStack_Pop ( ) ;
    return mpz_cmp ( *op1, *op2 ) ;
}

void
BigInt_LessThanOrEqual ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigInt_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigInt_Cmp ( ) > 0 ? 0 : 1 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigInt_Cmp ( ) > 0 ? 0 : 1 ) ;
#endif
}

void
BigInt_LogicalEquals ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigInt_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigInt_Cmp ( ) == 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigInt_Cmp ( ) == 0 ? 1 : 0 ) ;
#endif
}

void
BigInt_LogicalDoesNotEqual ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigInt_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigInt_Cmp ( ) == 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigInt_Cmp ( ) == 0 ? 0 : 1 ) ;
#endif
}

void
BigInt_GreaterThan ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigInt_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigInt_Cmp ( ) > 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigInt_Cmp ( ) > 0 ? 1 : 0 ) ;
#endif
}

void
BigInt_GreaterThanOrEqual ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigInt_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigInt_Cmp ( ) < 0 ? 0 : 1 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigInt_Cmp ( ) < 0 ? 0 : 1 ) ;
#endif
}

void
BigInt_LessThan ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigInt_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigInt_Cmp ( ) < 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigInt_Cmp ( ) < 0 ? 1 : 0 ) ;
#endif
}

void
BigInt_Print ( )
{
    mpz_t * value = ( mpz_t* ) _DataStack_Pop ( ) ;
    if ( _Q_->Verbosity ) gmp_printf ( " %Zd", * value ) ;
}

mpfr_t *
_BigFloat_New ( byte * token )
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

#if 0

void
BigFloat_Init ( )
{
    // assuming TOS is a uint32 
    _DataStack_SetTop ( ( int32 ) _BigFloat_New ( _DataStack_GetTop ( ) ) ) ;
}
#endif

void
BigFloat_Precision ( )
{
    mpfr_t * prec = ( mpfr_t* ) _DataStack_Pop ( ) ; // number of decimal digits
    int32 precision = mpfr_get_si ( *prec, MPFR_RNDN ) ;
    mpfr_set_default_prec ( ( ( precision / 3 ) * 10 ) + 16 ) ; // + 16 : add 5 extra digits of precision :: "precision is the number of bits used to represent the significand of a floating-point number"
    _Context_->System0->BigNumPrecision = precision ;
}

// width is a parameter to mpfr_printf; it works like printf and sets minimum number of characters to print
void
BigFloat_Width ( )
{
    mpfr_t * mpfwidth = ( mpfr_t* ) _DataStack_Pop ( ) ;
    int32 width = mpfr_get_si ( *mpfwidth, MPFR_RNDN ) ;
    _Context_->System0->BigNumWidth = width < ( _Context_->System0->BigNumPrecision - 4 ) ? width : ( _Context_->System0->BigNumPrecision - 4 ) ;
}

void
BigFloat_FPrint ( )
{
    mpfr_t * value = ( mpfr_t* ) _DataStack_Pop ( ) ;
    if ( _Q_->Verbosity ) mpfr_printf ( " %*.*Rf\n", _Context_->System0->BigNumWidth, _Context_->System0->BigNumPrecision, *value ) ;
    //if ( _Q_->Verbosity ) mpfr_printf ( " %*.*Rf\n", *value ) ;
    //if ( _Q_->Verbosity ) mpfr_printf ( " %Rf\n", *value ) ;
    fflush ( stdout ) ;
}

// scientific format

void
BigFloat_EPrint ( )
{
    mpfr_t * value = ( mpfr_t* ) _DataStack_Pop ( ) ;
    if ( _Q_->Verbosity ) mpfr_printf ( " %*.*Re\n", _Context_->System0->BigNumWidth, _Context_->System0->BigNumPrecision, *value ) ;
    //if ( _Q_->Verbosity ) mpfr_printf ( " %Re\n", *value ) ;
    fflush ( stdout ) ;
}

void
BigFloat_Add ( )
{
    mpfr_t *sum = _BigFloat_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ), *op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_add ( *sum, *op1, *op2, MPFR_RNDN ) ;
    _DataStack_Push ( ( int32 ) sum ) ;
}

void
BigFloat_Multiply ( )
{
    mpfr_t *prod = _BigFloat_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ), *op2 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_mul ( *prod, *op1, *op2, MPFR_RNDN ) ;
    _DataStack_Push ( ( int32 ) prod ) ;
}

void
BigFloat_Divide ( )
{
    mpfr_t *quotient = _BigFloat_New ( 0 ) ;
    mpfr_t * denominator = ( mpfr_t* ) _DataStack_Pop ( ), *numerator = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_div ( *quotient, *numerator, *denominator, MPFR_RNDN ) ;
    _DataStack_Push ( ( int32 ) quotient ) ;
}

void
BigFloat_Power ( )
{
    mpfr_t *rop = _BigFloat_New ( 0 ) ;
    mpfr_t * expf = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_pow ( *rop, *op1, *expf, MPFR_RNDN ) ;
    _DataStack_Push ( ( int32 ) rop ) ;
}

void
BigFloat_SquareRoot ( )
{
    mpfr_t *rop = _BigFloat_New ( 0 ) ;
    mpfr_t * op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_sqrt ( *rop, *op1, MPFR_RNDN ) ;
    _DataStack_Push ( ( int32 ) rop ) ;
}

void
BigFloat_Subtract ( )
{
    mpfr_t *diff = _BigFloat_New ( 0 ) ;
    mpfr_t * op2 = ( mpfr_t* ) _DataStack_Pop ( ), *op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    mpfr_sub ( *diff, *op1, *op2, MPFR_RNDN ) ; // diff = op1 - op2
    _DataStack_Push ( ( int32 ) diff ) ;
}

int32
BigFloat_Cmp ( )
{
    mpfr_t * op2 = ( mpfr_t* ) _DataStack_Pop ( ), *op1 = ( mpfr_t* ) _DataStack_Pop ( ) ;
    return mpfr_cmp ( *op1, *op2 ) ;
}

void
BigFloat_Equal ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigFloat_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigFloat_Cmp ( ) == 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigFloat_Cmp ( ) == 0 ? 1 : 0 ) ;
#endif
}

void
BigFloat_LessThanOrEqual ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigFloat_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigFloat_Cmp ( ) > 0 ? 0 : 1 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigFloat_Cmp ( ) > 0 ? 0 : 1 ) ;
#endif
}

void
BigFloat_GreaterThan ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigFloat_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigFloat_Cmp ( ) > 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigFloat_Cmp ( ) > 0 ? 1 : 0 ) ;
#endif
}

void
BigFloat_GreaterThanOrEqual ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigFloat_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigFloat_Cmp ( ) < 0 ? 0 : 1 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigFloat_Cmp ( ) < 0 ? 0 : 1 ) ;
#endif
}

void
BigFloat_LessThan ( )
{
#if USE_ZF_TO_PASS_LOGIC
    BigFloat_Cmp ( ) ;
#elif USE_AL_TO_PASS_LOGIC
    return BigFloat_Cmp ( ) < 0 ? 1 : 0 ;
#elif USE_TOS_TO_PASS_BIGNUM_LOGIC
    _DataStack_Push ( BigFloat_Cmp ( ) < 0 ? 1 : 0 ) ;
#endif
}

