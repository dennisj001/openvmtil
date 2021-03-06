
#include "../include/cfrtil32.h"

void
CfrTil_Plus ( ) // +
{
    if ( CompileMode )
    {
        Compile_X_Group1 ( _Context_->Compiler0, ADD, ZERO_TTT, NZ ) ;
    }
    else
    {
        Dsp [ - 1 ] = Dsp [ 0 ] + Dsp [ - 1 ] ;
        Dsp -- ;
    }
}

// if rvalue leave on stack else drop after inc/dec

void
CfrTil_IncDec ( int32 op ) // +
{
    Context * cntx = _Context_ ;
    Compiler * compiler = cntx->Compiler0 ;
    int32 sd = List_Depth ( compiler->WordList ) ;
    Word *one = ( Word* ) Compiler_WordList ( 1 ), *two = Compiler_WordList ( 2 ), *three = Compiler_WordList ( 3 ) ; // the operand
    byte * nextToken = Lexer_PeekNextNonDebugTokenWord ( cntx->Lexer0, 1 ) ;
    if ( ! GetState ( compiler, LC_CFRTIL ) )
    {
        Word * currentWord = _Context_->CurrentlyRunningWord ;
        Word * nextWord = Finder_Word_FindUsing ( cntx->Interpreter0->Finder0, nextToken, 0 ) ;
        SetState ( _Debugger_, DEBUG_SHTL_OFF, true ) ;
        if ( nextWord && ( nextWord->CProperty & ( CATEGORY_OP_ORDERED | CATEGORY_OP_UNORDERED | CATEGORY_OP_DIVIDE | CATEGORY_OP_EQUAL ) ) ) // postfix
        {
            List_DropN ( compiler->WordList, 1 ) ; // the operator; let higher level see the variable
            Interpreter_InterpretNextToken ( cntx->Interpreter0 ) ;
            if ( GetState ( compiler, C_INFIX_EQUAL ) && GetState ( _CfrTil_, OPTIMIZE_ON ) && CompileMode )
            {
                dllist * postfixList = List_New ( ) ;
                List_Push_1Value_Node ( postfixList, currentWord, COMPILER_TEMP ) ;
                List_Push_1Value_Node ( postfixList, one, COMPILER_TEMP ) ;
                List_Push_1Value_Node ( compiler->PostfixLists, postfixList, COMPILER_TEMP ) ;
                return ;
            }
            else
            {
                if ( sd > 1 )
                {
                    _Interpreter_DoWord ( cntx->Interpreter0, one, - 1 ) ;
                    _Interpreter_DoWord ( cntx->Interpreter0, currentWord, - 1 ) ;
                    return ;
                }
            }
        }
        if ( one->CProperty & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | NAMESPACE_VARIABLE ) )
        {
            if ( ( ! ( two->CProperty & ( KEYWORD ) ) ) && GetState ( _Context_, C_SYNTAX ) )
            {
                if ( ! GetState ( compiler, INFIX_LIST_INTERPRET ) )
                {
                    dllist * postfixList = List_New ( ) ;
                    List_Push_1Value_Node ( postfixList, currentWord, COMPILER_TEMP ) ;
                    List_Push_1Value_Node ( postfixList, one, COMPILER_TEMP ) ;
                    List_Push_1Value_Node ( compiler->PostfixLists, postfixList, COMPILER_TEMP ) ;
                    List_DropN ( compiler->WordList, 1 ) ; // the operator; let higher level see the variable for optimization
                    _Interpreter_DoWord ( cntx->Interpreter0, one, - 1 ) ;
                    return ;
                }
            }
        }
        else if ( nextWord && ( nextWord->CProperty & ( PARAMETER_VARIABLE | LOCAL_VARIABLE | NAMESPACE_VARIABLE ) ) ) // prefix
        {
            List_DropN ( compiler->WordList, 1 ) ; // the operator; let higher level see the variable
            _Interpreter_DoWord ( cntx->Interpreter0, nextWord, - 1 ) ;
            _Compiler_CopyDuplicatesAndPush ( compiler, currentWord ) ; // the operator
        }
        else
        {
            if ( GetState ( compiler, C_INFIX_EQUAL ) )
            {
                int32 i ;
                Word * word ;
                dllist * postfixList = List_New ( ) ;
                List_Push_1Value_Node ( postfixList, currentWord, COMPILER_TEMP ) ; // remember : this will be lifo
                for ( i = 1 ; word = Compiler_WordList ( i ), ( word->CProperty & ( CATEGORY_OP_ORDERED | CATEGORY_OP_UNORDERED | CATEGORY_OP_DIVIDE | CATEGORY_OP_EQUAL ) ) ; i ++ ) ;
                List_Push_1Value_Node ( postfixList, Compiler_WordList ( i ), COMPILER_TEMP ) ;
                List_Push_1Value_Node ( compiler->PostfixLists, postfixList, COMPILER_TEMP ) ;
                List_DropN ( compiler->WordList, 1 ) ; // the operator; let higher level see the variable for optimization
                return ;
            }
        }
    }
    _CfrTil_Do_IncDec ( op ) ;
    SetState ( _Debugger_, DEBUG_SHTL_OFF, false ) ;
}

void
CfrTil_PlusPlus ( ) // +
{
    CfrTil_IncDec ( INC ) ;
}

void
CfrTil_MinusMinus ( ) // --
{
    CfrTil_IncDec ( DEC ) ;
}

void
CfrTil_PlusEqual ( ) // +=
{
    if ( CompileMode )
    {
        Compile_Group1_X_OpEqual ( _Context_->Compiler0, ADD ) ;
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
        Compile_Group1_X_OpEqual ( _Context_->Compiler0, SUB ) ;
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
        Compile_MultiplyEqual ( _Context_->Compiler0 ) ;
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
        Compile_DivideEqual ( _Context_->Compiler0 ) ;
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
        Compile_X_Group1 ( _Context_->Compiler0, SUB, ZERO_TTT, NZ ) ;
    }

    else
    {
        Dsp [ - 1 ] = Dsp [ - 1 ] - Dsp [ 0 ] ;
        Dsp -- ;
    }
}

void
CfrTil_Multiply ( ) // *
{
    if ( CompileMode )
    {
        Compile_IMultiply ( _Context_->Compiler0 ) ;
    }
    else
    {
        Dsp [ - 1 ] = Dsp [ 0 ] * Dsp [ - 1 ] ;
        Dsp -- ;
    }
}

// ( b top | b < top ) dpans

void
CfrTil_Divide ( ) // *
{
    if ( CompileMode )
    {
        Compile_Divide ( _Context_->Compiler0 ) ;
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
        Compile_Mod ( _Context_->Compiler0 ) ;
    }
    else
    {

        int32 a, b ;
        a = _DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int32 ) ( b % a ) ) ;
    }
}

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
CfrTil_Power ( ) // **
{
    int32 pow = Dsp [ 0 ], base = Dsp [ - 1 ], n ;
    for ( n = base ; -- pow ; )
    {
        n *= base ;
    }
    Dsp [ - 1 ] = n ;
    Dsp -- ;
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