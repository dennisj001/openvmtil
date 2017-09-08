
#include "../include/cfrtil.h"

#if 0
ListObject *
_LO_Read ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    Lexer * lexer = _Context_->Lexer0 ;
    ListObject *l0, *lreturn, *lnew = compiler->LispParenLevel ? LO_New ( LIST, 0 ) : 0 ;
    byte * token, *token1 ;
    if ( ! _Q_->OVT_LC ) LC_New ( 1 ) ;
    lreturn = lnew ;
    do
    {
next:
        if ( Lexer_IsTokenQualifiedID ( lexer ) ) SetState ( compiler, PARSING_QUALIFIED_ID, true ) ;
        else SetState ( compiler, PARSING_QUALIFIED_ID, false ) ;
        token = _Lexer_ReadToken ( lexer, ( byte* ) " ,;\n\r\t" ) ;
        if ( token )
        {
            if ( String_Equal ( ( char* ) token, "(" ) )
            {
                Stack_Push ( _Q_->OVT_LC->QuoteStateStack, _Q_->OVT_LC->QuoteState ) ;
                _Q_->OVT_LC->QuoteState = _Q_->OVT_LC->ItemQuoteState ;
                compiler->LispParenLevel ++ ;
                l0 = _LO_Read ( ) ;
                l0 = LO_New ( LIST_NODE, l0 ) ;
                _Q_->OVT_LC->QuoteState = Stack_Pop ( _Q_->OVT_LC->QuoteStateStack ) ;
            }
            else if ( String_Equal ( ( char* ) token, ")" ) ) break ;
            else
            {
                Word * word = _LO_FindWord ( token, 0 ) ;
                if ( word )
                {
                    if ( word->LType & T_LISP_READ_MACRO )
                    {
                        //word->Definition ( ) ; // scheme read macro preprocessor 
                        _Block_Eval (  word->Definition ) ;
                        goto next ;
                    }
                    if ( word->LType & T_LISP_TERMINATING_MACRO )
                    {
                        //word->Definition ( ) ; // scheme read macro preprocessor 
                        _Block_Eval (  word->Definition ) ;
                        token1 = ( byte* ) _DataStack_Pop ( ) ;
                        l0 = LO_New_ParseRawStringOrLiteral ( token1, 0 ) ; //don't parse a string twice; but other macros may need to be adjusted 
                    }
                    else
                    {
                        l0 = _LO_New ( T_LISP_SYMBOL | word->LType, word->CProperty, word->Lo_Value, word, 0, 0, LispAllocType ) ; // all words are symbols
                    }
                }
                else
                {
                    l0 = LO_New_ParseRawStringOrLiteral ( token, 1 ) ;
                    if ( GetState ( compiler, PARSING_QUALIFIED_ID ) ) SetState ( l0, QUALIFIED_ID, true ) ;
                }
            }
            l0->State |= ( _Q_->OVT_LC->ItemQuoteState | _Q_->OVT_LC->QuoteState ) ;
            if ( ( l0->State & UNQUOTE_SPLICE ) && ( ! ( l0->State & QUOTED ) ) ) //&& ( l0->LType & ( LIST_NODE | LIST ) ) )
            {
                l0 = LO_Eval ( LO_Copy ( l0 ) ) ;
                if ( lnew )
                {
                    LO_SpliceAtTail ( lnew, l0 ) ;
                }
                else
                {
                    lreturn = l0 ;
                    break ;
                }
            }
            else if ( lnew )
            {
                //if ( ! GetState ( compiler, LC_PREFIX_ARG_PARSING ) ) 
                LO_AddToTail ( lnew, l0 ) ;
            }
            else
            {
                lreturn = l0 ; // did i break this by the "if ( l0 )" ?
                break ;
            }
        }
        else Error ( "\n_LO_Read : Syntax error : no token?\n", QUIT ) ;
        _Q_->OVT_LC->ItemQuoteState = 0 ;
    }
    while ( compiler->LispParenLevel ) ;
    return lreturn ;
}

#endif
