
#include "../includes/cfrtil.h"

void
CfrTil_Plus ( ) // +
{
    if ( CompileMode )
    {
        Compile_X_Group1 ( _Q_->OVT_Context->Compiler0, ADD, ZERO_CC, NZ ) ;
    }
    else
    {
        Dsp [ - 1 ] = Dsp [ 0 ] + Dsp-- [ - 1 ] ;
    }
}

// if rvalue leave on stack else drop after inc/dec
void
_CfrTil_Do_IncDec ( int32 incrementFlag )
{
    Context * cntx = _Q_->OVT_Context ;
    Compiler * compiler = cntx->Compiler0 ;
    int32 sd = Stack_Depth ( CompilerWordStack ) ;
    Word *one = ( Word* ) Compiler_WordStack ( compiler, - 1 ) ; // the operand
    if ( CompileMode )
    {
        Compile_X_Group5 ( compiler, incrementFlag ? INC : DEC ) ; //, RVALUE ) ;
    }
    else
    {
        if ( incrementFlag )
        {
            if ( ( sd > 1 ) && one->CType & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | VARIABLE ) )
            {
                *( ( int32* ) ( TOS ) ) += 1 ;
                _Drop ( ) ;
            }
            else Dsp [0] ++ ;
        }
        else
        {
            if ( ( sd > 1 ) && one->CType & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | VARIABLE ) )
            {
                *( ( int32* ) ( TOS ) ) -= 1 ;
                _Drop ( ) ;
            }
            else Dsp [0] -- ;
        }
    }
}

void
CfrTil_IncDec ( int32 incrementFlag ) // +
{
    Context * cntx = _Q_->OVT_Context ;
    Compiler * compiler = cntx->Compiler0 ;
    Word * currentWord = _Q_->OVT_Context->CurrentRunWord ;
    byte * nextToken = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0 ) ;
    Word * nextWord = Finder_Word_FindUsing ( cntx->Interpreter0->Finder, nextToken, 0 ) ;
    Word *one = ( Word* ) Compiler_WordStack ( compiler, - 1 ) ; // the operand
    int32 sd = Stack_Depth ( CompilerWordStack ) ;
    if ( nextWord && ( nextWord->CType & ( CATEGORY_OP_ORDERED | CATEGORY_OP_UNORDERED | CATEGORY_OP_DIVIDE | CATEGORY_OP_EQUAL ) ) ) // postfix
    {
        _Stack_DropN ( CompilerWordStack, 1 ) ; // the operator; let higher level see the variable
        Interpreter_InterpretNextToken ( cntx->Interpreter0 ) ;
        if ( sd > 1 )
        {
            _Interpreter_Do_MorphismWord ( cntx->Interpreter0, one ) ; // don't lex the peeked nextWord let it be lexed after this so it remains 
            _Interpreter_Do_MorphismWord ( cntx->Interpreter0, currentWord ) ; // don't lex the peeked nextWord let it be lexed after this so it remains 
            return ;
        }
    }
    else if ( ( sd > 1 ) && ( one->CType & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | VARIABLE ) ) ) ; //return ;
    else if ( nextWord && ( nextWord->CType & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | VARIABLE ) ) ) // in case of prefix plus_plus/minus_minus  ?!? case of solitary postfix with no semicolon
    {
        _Stack_DropN ( CompilerWordStack, 1 ) ; // the operator
        _Interpreter_Do_MorphismWord ( cntx->Interpreter0, nextWord ) ; // don't lex the peeked nextWord let it be lexed after this so it remains 
        Compiler_PushCheckAndCopyDuplicates ( compiler, currentWord, compiler->WordStack ) ; // the operator
    }
    _CfrTil_Do_IncDec ( incrementFlag ) ;
}

void
CfrTil_PlusPlus ( ) // +
{
    CfrTil_IncDec ( 1 ) ;
}

void
CfrTil_MinusMinus ( ) // --
{
    CfrTil_IncDec ( 0 ) ;
}
#if 0

void
CfrTil_PlusPlusLValue ( ) // +
{
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    if ( CompileMode )
    {
        Compile_X_Group5 ( compiler, INC, LVALUE ) ;
    }
    else
    {
        Word *one = ( Word* ) Compiler_WordStack ( compiler, - 1 ) ;
        // if ( Namespace_Using ( "C" ) )  -- ToDo needs to be implemented
        // take variable like C as an rvalue 
        if ( one->CType & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | VARIABLE ) ) *( ( int32* ) ( TOS ) ) += 1 ;

        else Dsp [0] ++ ;
    }
}

void
CfrTil_MinusMinusRValue ( ) // --
{
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    if ( CompileMode )
    {
        Compile_X_Group5 ( compiler, DEC, RVALUE ) ;
    }
    else
    {
        Word *one = ( Word* ) Compiler_WordStack ( compiler, - 1 ) ;
        if ( one->CType & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | VARIABLE ) ) *( ( int32* ) ( TOS ) ) -= 1 ;

        else Dsp [0] -- ;
    }
}
#endif

void
CfrTil_PlusEqual ( ) // +=
{
    if ( CompileMode )
    {
        Compile_Group1_X_OpEqual ( _Q_->OVT_Context->Compiler0, ADD ) ;
    }
    else
    {

        int32 *x, n ;
        n = _DataStack_Pop ( ) ;
        x = ( int32* ) _DataStack_Pop ( ) ;
        *x = * x + n ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () + _DataStack_GetTop ( Dsp ) ) ;
    }
}

void
CfrTil_MinusEqual ( ) // -=
{
    if ( CompileMode )
    {
        Compile_Group1_X_OpEqual ( _Q_->OVT_Context->Compiler0, SUB ) ;
    }
    else
    {

        int32 *x, n ;
        n = _DataStack_Pop ( ) ;
        x = ( int32* ) _DataStack_Pop ( ) ;
        *x = * x - n ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () + _DataStack_GetTop ( Dsp ) ) ;
    }
}

void
CfrTil_MultiplyEqual ( ) // *=
{
    if ( CompileMode )
    {
        Compile_MultiplyEqual ( _Q_->OVT_Context->Compiler0 ) ;
    }
    else
    {

        int32 *x, n ;
        n = _DataStack_Pop ( ) ;
        x = ( int32* ) _DataStack_Pop ( ) ;
        *x = * x * n ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () + _DataStack_GetTop ( Dsp ) ) ;
    }
}

void
CfrTil_DivideEqual ( ) // +=
{
    if ( CompileMode )
    {
        Compile_DivideEqual ( _Q_->OVT_Context->Compiler0 ) ;
    }
    else
    {

        int32 *x, n ;
        n = _DataStack_Pop ( ) ;
        x = ( int32* ) _DataStack_Pop ( ) ;
        *x = * x / n ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () + _DataStack_GetTop ( Dsp ) ) ;
    }
}

// ( b top | b - top ) dpans

void
CfrTil_Minus ( )
{
    if ( CompileMode )
    {
        Compile_X_Group1 ( _Q_->OVT_Context->Compiler0, SUB, ZERO_CC, NZ ) ;
    }

    else
#if 0        
    {

        int32 top, b ;
        top = _DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int32 ) ( b - top ) ) ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () - _DataStack_GetTop ( Dsp ) ;
    }
#elif 0    
#elif 1
        Dsp [ - 1 ] = Dsp [ - 1 ] - Dsp-- [ 0 ] ;
#endif        
}

void
CfrTil_Multiply ( ) // *
{
    if ( CompileMode )
    {
        Compile_IMultiply ( _Q_->OVT_Context->Compiler0 ) ;
    }
    else
    {
#if 0       
        _DataStack_SetTop ( _DataStack_Pop ( ) * _DataStack_GetTop ( ) ) ;
#elif 0        
        Interpreter * interp = _Q_->OVT_Context->Interpreter0 ;
        if ( interp->CurrentPrefixWord )
        {
            int i, prod = 0, *ssp ;
            for ( ssp = interp->PrefixWord_SaveSP + 1 ; ssp <= Dsp ; ssp ++ )
            {
                if ( prod ) prod *= * ssp ;
                else prod = * ssp ;
            }
            Dsp = interp->PrefixWord_SaveSP + 1 ;
            *Dsp = prod ;
        }

        else _DataStack_SetTop ( _DataStack_Pop ( ) * _DataStack_GetTop ( ) ) ;
#elif 1
        Dsp [ - 1 ] = Dsp [ 0 ] * Dsp-- [ - 1 ] ;
#endif       
    }
}

// ( b top | b < top ) dpans

void
CfrTil_Divide ( ) // *
{
    if ( CompileMode )
    {
        Compile_Divide ( _Q_->OVT_Context->Compiler0 ) ;
    }
    else
    {

        int32 a, b ;
        a = _DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int32 ) ( b / a ) ) ;
    }
}

void
CfrTil_Mod ( ) // *
{
    if ( CompileMode )
    {
        Compile_Mod ( _Q_->OVT_Context->Compiler0 ) ;
    }
    else
    {

        int32 a, b ;
        a = _DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int32 ) ( b % a ) ) ;
    }
}

// #pragma GCC optimize ( "-O2" ) 

int32
_CFib ( int n )
{
    if ( n < 2 ) return n ;
    else return ( _CFib ( n - 1 ) + _CFib ( n - 2 ) ) ;
}

void
CFib ( )
{

    TOS = ( _CFib ( TOS ) ) ;
}

void
CFactorial ( )
{
    int32 n = TOS ;
    if ( n > 1 )
    {
        TOS = TOS - 1 ;
        CFactorial ( ) ;
        TOS *= n ;
    }

    else TOS = 1 ;
}

int32
_CFactorial ( int32 n )
{
    if ( n > 1 ) return ( n * _CFactorial ( n - 1 ) ) ;

    else return 1 ;
}

void
CFactorial2 ( )
{

    TOS = ( _CFactorial ( TOS ) ) ;
}

void
CFactorial3 ( void )
{
    int32 rec1 = 1, n = TOS ;
    while ( n > 1 )
    {

        rec1 *= n -- ;
    }
    TOS = rec1 ;
}

#if 0
#if 0
"Ar3" class : { int32 ar [3][3][2] } ;
: a7 ( n m o Ar3 a )
a.ar[n @ 1 + ][m @][o @] a.ar[n @ 2 + ][m @][o @] @ =
// nl "a6.a : " ps location a.ar[1][2][1] 72 dump
nl "a7.a : " ps location a @ 72 dump
// a.ar[n @ 2 +][m @][o @] @ dup p 89 s _assert 
a.ar[n @ 1 + ][m @][o @] @ dup p 89 s _assert
;
#endif

void
a7 ( int n, int m, int o, A3 * a )
{

    a->ar[n + 1][m][o] = a->ar[n + 2][m][o] ;
    __CfrTil_Dump ( ( int32 ) a, sizeof (*a ), 8 ) ;
}

void
call_a7 ( )
{
    A3 ar ;
    memset ( &ar, 0, sizeof (A3 ) ) ;
    ar.ar[2][2][1] = 89 ;

    int32 a = _DataStack_Pop ( ) ;
    int32 b = _DataStack_Pop ( ) ;
    int32 c = _DataStack_Pop ( ) ;

    a7 ( a, b, c, &ar ) ;
    memset ( &ar, 0, sizeof (A3 ) ) ;
    ar.ar[2][2][1] = 89 ;
    a7 ( a, b, c, &ar ) ;
}
#endif
#pragma GCC reset_options