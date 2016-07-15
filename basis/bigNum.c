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
    _DataStack_SetTop ( (int32) _BigInt_New ( _DataStack_GetTop ( ) ) ) ;
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
    mpz_t * expz = (mpz_t*) _DataStack_Pop ( ) ;
    int32 exp = mpz_get_ui ( *expz ) ;
    mpz_t * base = ( mpz_t* ) _DataStack_Pop ( );
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
    if ( _Q_->Verbosity ) gmp_printf ( " %Zd\n", * value ) ;
}

mpf_t *
_BigFloat_New ( int32 initializer )
{
    mpf_t *bn = ( mpf_t* ) Mem_Allocate ( sizeof ( mpf_t ), OBJECT_MEMORY ) ;
    mpf_init_set_si ( *bn, ( int32 ) initializer ) ;
    return bn ;
}

void
BigFloat_Init ( )
{
    // assuming TOS is a uint32 
    _DataStack_SetTop ( (int32) _BigFloat_New ( _DataStack_GetTop ( ) ) ) ;
}

void
BigFloat_Precision ( )
{
    mpf_t * prec = ( mpf_t* ) _DataStack_Pop ( ) ;
    int32 precision = mpf_get_si ( *prec ) ;
    mpf_set_default_prec ( ( mp_bitcnt_t ) precision ) ;
    _Context_->System0->BigNumPrecision = precision ;
}

void
BigFloat_Width ( )
{
    mpf_t * _width = ( mpf_t* ) _DataStack_Pop ( ) ;
    int32 width = mpf_get_si ( *_width ) ;
    //mpf_set_default_prec ( (mp_bitcnt_t) precision ) ;
    _Context_->System0->BigNumWidth = width ;
}

void
BigFloat_FPrint ( )
{
    mpf_t * value = ( mpf_t* ) _DataStack_Pop ( ) ;
    if ( _Q_->Verbosity ) gmp_printf ( " %*.*Ff\n", _Context_->System0->BigNumWidth, _Context_->System0->BigNumPrecision, *value ) ;
}

void
BigFloat_EPrint ( )
{
    mpf_t * value = ( mpf_t* ) _DataStack_Pop ( ) ;
    if ( _Q_->Verbosity ) gmp_printf ( " %*.*Fe\n", _Context_->System0->BigNumWidth, _Context_->System0->BigNumPrecision, *value ) ;
}
void
BigFloat_Add ( )
{
    mpf_t *sum = _BigFloat_New ( 0 ) ;
    mpf_t * op1 = ( mpf_t* ) _DataStack_Pop ( ), *op2 = ( mpf_t* ) _DataStack_Pop ( ) ;
    mpf_add ( *sum, *op1, *op2 ) ;
    _DataStack_Push ( ( int32 ) sum ) ;
}

void
BigFloat_Multiply ( )
{
    mpf_t *prod = _BigFloat_New ( 0 ) ;
    mpf_t * op1 = ( mpf_t* ) _DataStack_Pop ( ), *op2 = ( mpf_t* ) _DataStack_Pop ( ) ;
    mpf_mul ( *prod, *op1, *op2 ) ;
    _DataStack_Push ( ( int32 ) prod ) ;
}

void
BigFloat_Divide ( )
{
    mpf_t *quotient = _BigFloat_New ( 0 ) ;
    mpf_t * denominator = ( mpf_t* ) _DataStack_Pop ( ), *numerator = ( mpf_t* ) _DataStack_Pop ( ) ;
    mpf_div ( *quotient, *numerator, *denominator ) ;
    _DataStack_Push ( ( int32 ) quotient ) ;
}

void
BigFloat_Power ( )
{
    // void mpf_pow_ui (mpf_t rop, mpf_t op1, unsigned long int op2)
    // unsigned long mpf_get_ui (mpf_t op)
    mpf_t *rop = _BigFloat_New ( 0 ) ;
    mpf_t * expf = ( mpf_t* ) _DataStack_Pop ( ) ;  
    int32 exp = mpf_get_ui ( *expf ) ;
    mpf_t * op1 =  ( mpf_t* ) _DataStack_Pop ( ) ;  
    mpf_pow_ui ( *rop, *op1, exp ) ;
    _DataStack_Push ( ( int32 ) rop ) ;
}

void
BigFloat_Subtract ( )
{
    mpf_t *diff = _BigFloat_New ( 0 ) ;
    mpf_t * op2 = ( mpf_t* ) _DataStack_Pop ( ), *op1 = ( mpf_t* ) _DataStack_Pop ( ) ;
    mpf_sub ( *diff, *op1, *op2 ) ; // diff = op1 - op2
    _DataStack_Push ( ( int32 ) diff ) ;
}

int32
BigFloat_Cmp ( )
{
    mpf_t * op2 = ( mpf_t* ) _DataStack_Pop ( ), *op1 = ( mpf_t* ) _DataStack_Pop ( ) ;
    return mpf_cmp ( *op1, *op2 ) ;
    //_Compiler_Setup_tttn ( _Context->Compiler0, LE, N ) ;
    //return value ;
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

