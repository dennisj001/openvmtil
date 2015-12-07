
#include "../includes/cfrtil.h"
//LambdaCalculus * _Q_->OVT_LC ;
// --------------------------------------------------------
// LC  : an abstract core of a prefix language related to Lambda Calculus with list objects
//       (it seems neither lisp nor scheme as they exist meet my more abstract intent so it's "Lambda Calculus")
// LO  : ListObject
// nb. : a Word is a ListObject is a Namespace is a DObject 
//     : a ListObject is a Word is a Namespace is a DObject 
// ---------------------------------------------------------

// lisp type lists can maybe be thought of as not rpn - not reverse polish notation, 
// not postfix but prefix - cambridge polish/prefix notation blocks

// sections 
// _LO_Eval
// _LO_Apply
// _LO_Read 
// _LO_Print
// _LO_SpecialFunctions
// LO_Repl
// LO_...misc : _L0_New _L0_Copy
// LC_x

//===================================================================================================================
//| LO_Eval
//===================================================================================================================
// #define EVAL(x)         (isNum(x)? x : isSym(x)? val(x) : evList(x)) // picolisp

ListObject *
_LO_Eval ( ListObject * l0, ListObject * locals, int32 applyFlag )
{
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    LambdaCalculus * lc = _Q_->OVT_LC ;
    ListObject *lfunction, *ldata, *lfirst ;
    Word * w ;
start:
    if ( l0 )
    {
        if ( LO_IsQuoted ( l0 ) ) return l0 ;
        else if ( ( l0->LType & T_LISP_SYMBOL ) )
        {
            w = _LO_FindWord ( l0, l0->Name, locals ) ;
            if ( w )
            {
                if ( w->CType & ( CPRIMITIVE | CFRTIL_WORD | LOCAL_VARIABLE | PARAMETER_VARIABLE | T_LISP_COMPILED_WORD ) )
                {
                    if ( ! lc->DontCopyFlag ) l0 = LO_CopyOne ( l0 ) ;
                    l0->Lo_Value = w->Lo_Value ;
                    l0->Lo_CfrTilWord = w ;
                    l0->CType |= w->CType ;
                    l0->LType |= w->LType ;
                }
                else if ( w->LType & T_LISP_DEFINE ) // after macro because a macro is also a define
                {
                    l0 = ( ListObject * ) w->Lo_Value ;
                }
                else
                {
                    l0 = w ;
                }
                if ( CompileMode )
                {
                    // this maybe could be improved to be a list-block ?!?
                    if ( LO_CheckBeginBlock ( ) )
                    {
                        _LO_CompileOrInterpret_One ( l0 ) ;
                    }
                }
                if ( w->CType & COMBINATOR )
                    //if ( w->CType0.Combinator ) // test use of bitfields
                {
                    Compiler_SetState ( compiler, LISP_COMBINATOR_MODE, true ) ;
                    CombinatorInfo ci ; // remember sizeof of CombinatorInfo = 4 bytes
                    ci.BlockLevel = compiler->BlockLevel ;
                    ci.ParenLevel = lc->LispParenLevel ;
                    _Stack_Push ( compiler->CombinatorInfoStack, ( int32 ) ci.CI_i32_Info ) ; // this stack idea works because we can only be in one combinator at a time
                }
            }
        }
        else if ( l0->LType & ( LIST | LIST_NODE ) )
        {
            if ( CompileMode )
            {
                LO_CheckEndBlock ( ) ;
                LO_CheckBeginBlock ( ) ;
            }
            lc->LispParenLevel ++ ;
            lfirst = _LO_First ( l0 ) ;
            if ( lfirst )
            {
                if ( lfirst->LType & T_LISP_SPECIAL )
                {
                    l0 = LO_SpecialFunction ( l0, locals ) ;
                    lc->LispParenLevel -- ;
                    goto done ;
                }
                lfunction = _LO_Eval ( lfirst, locals, applyFlag ) ;
                ldata = _LO_EvalList ( _LO_Next ( lfirst ), locals, applyFlag ) ;
                if ( applyFlag && lfunction && (
                    ( lfunction->CType & ( CPRIMITIVE | CFRTIL_WORD ) ) ||
                    ( lfunction->LType & ( T_LISP_COMPILED_WORD ) )
                    ) )
                {
                    l0 = _LO_Apply ( l0, lfunction, ldata ) ;
                }
                else if ( lfunction && ( lfunction->LType & T_LAMBDA ) && lfunction->Lo_LambdaFunctionBody )
                {
                    locals = ldata ;
                    // LambdaArgs, the formal args, are not changed by LO_Substitute (locals - lvals are just essentially renamed) and thus don't need to be copied
                    LO_Substitute ( _LO_First ( ( ListObject * ) lfunction->Lo_LambdaFunctionParameters ), _LO_First ( locals ) ) ;
                    //l0 = LO_Copy ( ( ListObject * ) lproc->Lo_CfrTilWord->Lo_LambdaBody ) ; 
                    // nb. this logic (below) is to prevent having to copy the whole LambdaBody ; saving a lot of unneeded memory use
                    if ( ! lc->CurrentFunction )
                    {
                        lc->CurrentFunction = lfunction ;
                    }
                    else if ( lc->CurrentFunction == lfunction )
                    {
                        if ( lc->Loop ++ > 1 ) lc->DontCopyFlag = 1 ;
                    }
                    else
                    {
                        lc->CurrentFunction = 0 ;
                        lc->DontCopyFlag = 0 ;
                    }
                    l0 = ( ListObject * ) lfunction->Lo_LambdaFunctionBody ;
                    goto start ;
                }
                else
                {
                    SetState ( lc, LC_COMPILE_MODE, false ) ;
                    if ( ldata )
                    {
                        // return the evaluated list
                        DLList_AddNodeToHead ( ldata->Lo_List, ( DLNode* ) lfunction ) ;
                        return ldata ;
                    }
                    else
                        //if (lfunction->LType & T_LAMBDA )
                    {
                        return lfunction ; //seems more common sense for this !?!? ...
                        //return l0 ; //... also a possibility - how should this be defined/is it standardly defined?
                    }
                }
            }
        }
    }
done:
    return l0 ;
}

ListObject *
_LO_EvalList ( ListObject * lorig, ListObject * locals, int32 applyFlag )
{
    ListObject * lnew = 0, *lnode ;
    if ( lorig )
    {
        lnew = LO_New ( LIST, 0 ) ;
        for ( lnode = lorig ; lnode ; lnode = _LO_Next ( lnode ) ) // eval each node
        {
            lnode->State |= LISP_EVALLIST_ARG ;
            LO_AddToTail ( lnew, LO_CopyOne ( _LO_Eval ( lnode, locals, applyFlag ) ) ) ; // this should be optimizable
        }
    }
    return lnew ;
}

// untested

ListObject *
_LO_MacroPreprocess ( ListObject * l0 )
{
    ListObject * l1, *first ;
    do
    {
        l1 = 0 ;
        if ( LO_IsQuoted ( l0 ) ) return l0 ;
        while ( ( int32 ) ( first = _LO_First ( l0 ) ) ? first->LType & T_LISP_MACRO : 0 )
        {
            l0 = _LO_Eval ( l0, 0, 0 ) ;
            l1 = l0 ;
        }
    }
    while ( l1 ) ;
    return l0 ;
}

//===================================================================================================================
//| LO_SpecialFunction(s) 
//===================================================================================================================

block
CompileLispBlock ( ListObject *args, ListObject * body )
{
    Compiler *compiler = _Q_->OVT_Context->Compiler0 ;
    block code ;
    byte * here = Here ;
    LO_BeginBlock ( ) ; // must have a block before local variables
    Namespace * locals = _CfrTil_Parse_LocalsAndStackVariables ( 1, 0, 1, args ) ;
    Word * word = compiler->CurrentWord ;
    word->CType = BLOCK ; // | CATEGORY_RECURSIVE ;
    word->LType |= T_LISP_COMPILED_WORD ;
    SetState ( _Q_->OVT_LC, ( LC_COMPILE_MODE | LC_BLOCK_COMPILE ), true ) ;
    _LO_Eval ( body, locals, 1 ) ;
    LO_EndBlock ( ) ;
    if ( GetState ( _Q_->OVT_LC, LC_COMPILE_MODE ) )
    {
        code = ( block ) _DataStack_Pop ( ) ;
    }
    else // nb. LISP_COMPILE_MODE : this state can change with some functions that can't be compiled yet
    {
        SetHere ( here ) ; //recover the unused code space
        code = 0 ;
        word->LType &= ~ T_LISP_COMPILED_WORD ;
        if ( _Q_->Verbosity > 1 )
        {
            AlertColors ;
            Printf ( "\nLisp can not compile this word yet : %s : -- interpreting ...\n ", _Word_Location_pbyte ( word ) ) ;
            DefaultColors ;
        }
    }
    _Word ( word, ( byte* ) code ) ; // nb. LISP_COMPILE_MODE is reset by _Word_Finish
    SetState ( word, NOT_COMPILED, false ) ;

    return code ;
}

// need to get a clear picture (diagram) of this structure and its different forms

ListObject *
_LO_MakeLambda ( ListObject * l0 )
{
    // lambdaSignature is "lambda" or "/.", etc.
    ListObject *args, *body, *word, *signature = _LO_First ( l0 ), *lnew, *last ;
    // allow args to be optionally an actual parenthesized list or just vars after the lambda
    if ( GetState ( _Q_->OVT_LC, LC_DEFINE_MODE ) ) word = _Q_->OVT_Context->Compiler0->CurrentWord ;
    else word = _Word_New ( "anonymous", WORD_CREATE, 0, DICTIONARY ) ;
    args = _LO_Next ( signature ) ;
    last = _LO_Last ( l0 ) ;
    if ( args->LType & ( LIST | LIST_NODE ) )
    {
        args = _LO_Copy ( args, LispAllocType ) ;
    }
    else
    {
        // this list could/should be just W_dll_SymbolList
        lnew = LO_New ( LIST, 0 ) ;
        do
        {
            LO_AddToTail ( lnew, _LO_CopyOne ( args, LispAllocType ) ) ;
        }
        while ( ( args = _LO_Next ( args ) ) != last ) ;
        args = lnew ;
    }
    if ( ! ( last->LType & ( LIST | LIST_NODE ) ) )
    {
        lnew = LO_New ( LIST, 0 ) ;
        LO_AddToTail ( lnew, _LO_CopyOne ( last, LispAllocType ) ) ;
        body = lnew ;
    }
    else body = _LO_Copy ( last, LispAllocType ) ;
    if ( GetState ( _Q_->OVT_LC, LC_COMPILE_MODE ) )
    {
        SetState ( _Q_->OVT_LC, LC_LAMBDA_MODE, true ) ;
        block codeBlk = CompileLispBlock ( args, body ) ;
        word->Lo_Value = ( uint32 ) codeBlk ;
    }
    if ( ! GetState ( _Q_->OVT_LC, LC_COMPILE_MODE ) ) // nb! this needs to be 'if' not 'else' or else if' because the state is sometimes changed by CompileLispBlock, eg. for function parameters
    {
        word->Lo_CfrTilWord = word ;
        word->Lo_LambdaFunctionParameters = args ;
        word->Lo_LambdaFunctionBody = body ;
        word->LType = T_LAMBDA ;
        word->CType = 0 ;
    }
    return word ;
}

// need to get a clear picture (diagram) of this structure and its different forms

ListObject *
_LO_Define0 ( byte * sname, ListObject * nameNode, ListObject * locals )
{
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    byte * b = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    ListObject *value0, *value, *l1 ;
    Word * word = nameNode->Lo_CfrTilWord ;
    word->Definition = 0 ; // reset the definition from LO_Read
    value0 = _LO_Next ( nameNode ) ;
    compiler->CurrentWord = word ;
    word->Lo_CfrTilWord = word ;
    snprintf ( ( char* ) b, BUFFER_SIZE, " ( %s%s )", sname, ( byte* ) _LO_PrintList ( nameNode, 0, 0, 0 ) ) ;
    word->SourceCode = String_New ( b, DICTIONARY ) ;
    _Namespace_DoAddWord ( _Q_->OVT_LC->LispNamespace, word ) ; // put it at the beginning of the list to be found first
    word->CType = VARIABLE ; // nb. !
    SetState ( word, NOT_COMPILED, true ) ; // a needed property to compile recursive words 
    SetState ( _Q_->OVT_LC, ( LC_DEFINE_MODE ), true ) ;
    value = _LO_Eval ( value0, locals, 0 ) ; // 0 : don't apply
    // traverse tree, move all to LispNamespace - we want to keep these as a non-temporary part of Lisp
    if ( value && ( value->LType & ( T_LAMBDA | T_LISP_SPECIAL ) ) ) // | T_LISP_MACRO ) )
    {
        value->Lo_LambdaFunctionParameters = _LO_Copy ( value->Lo_LambdaFunctionParameters, LISP ) ;
        value->Lo_LambdaFunctionBody = _LO_Copy ( value->Lo_LambdaFunctionBody, LISP ) ;
    }
    else value = _LO_Copy ( value, LISP ) ; // this object now becomes part of LISP permanent memory - not a temp
    word->Lo_Value = ( uint32 ) value ; // Lo_Value = Lo_Object
    word->LType |= ( T_LISP_DEFINE | T_LISP_SYMBOL ) ; //| value->LType) ; // Lo_Value = Lo_Object
    // the value was entered into the LISP memory, now we need a temporary carrier to print
    l1 = _LO_New ( word->LType, word->CType, ( byte* ) value, word, 0, 0, LispAllocType ) ; // all words are symbols
    CfrTil_NewLine ( ) ; // always print nl before a define to make easier reading
    l1->LType |= ( T_LISP_DEFINE | T_LISP_SYMBOL ) ;
    return l1 ;
}

ListObject *
_LO_Define ( ListObject * l0, ListObject * locals )
{
    ListObject * nameNode = _LO_Next ( l0 ) ;
    return _LO_Define0 ( "define", nameNode, locals ) ;
}

ListObject *
_LO_Compile ( ListObject * l0, ListObject * locals )
{
    Compiler_SetState ( _Q_->OVT_Context->Compiler0, RETURN_TOS, true ) ;
    SetState ( _Q_->OVT_LC, LC_COMPILE_MODE, true ) ;
    l0 = _LO_Define ( l0, locals ) ;
    l0->LType |= T_LISP_COMPILED_WORD ;
    SetState ( _Q_->OVT_LC, LC_COMPILE_MODE, false ) ;

    return l0 ;
}

ListObject *
LO_Define ( ListObject * l0, ListObject * locals )
{
    return _LO_Compile ( l0, locals ) ;
}

ListObject *
LO_MakeLambda ( ListObject * l0 )
{
    Word * word = _LO_MakeLambda ( l0 ) ;
    word->LType |= T_LAMBDA ;

    return word ;
}

ListObject *
_LO_Cons ( ListObject *first, ListObject * second, uint64 allocType )
{
    ListObject * l0 = _LO_New ( LIST, 0, 0, 0, 0, 0, allocType ) ;
    LO_AddToTail ( l0->Lo_List, first ) ;
    LO_AddToTail ( l0->Lo_List, second ) ;

    return l0 ;
}

ListObject *
LO_If ( ListObject * l0, ListObject * locals )
{
    ListObject * tf, *test, *trueList, *elseList, *value ;
    test = _LO_Next ( l0 ) ;
    trueList = _LO_Next ( test ) ;
    elseList = _LO_Next ( trueList ) ;
    tf = _LO_Eval ( test, locals, 1 ) ;
    if ( tf->Lo_Value && ( tf != nil ) ) value = _LO_Eval ( trueList, locals, 1 ) ;
    else value = _LO_Eval ( elseList, locals, 1 ) ;

    return value ;
}

// lisp cond : conditional

ListObject *
LO_Cond ( ListObject * l0, ListObject * locals )
{
    ListObject * tf, *trueNode, * elseNode = nil ;
    // 'cond' is first node ; skip it.
    l0 = _LO_First ( l0 ) ;
    tf = _LO_Next ( l0 ) ;
    while ( ( trueNode = _LO_Next ( tf ) ) )
    {
        tf = _LO_Eval ( tf, locals, 1 ) ;
        if ( ( tf != nil ) && ( tf->Lo_Value ) ) return _LO_Eval ( LO_CopyOne ( trueNode ), locals, 1 ) ;
            // nb we have to copy one here else we return the whole rest of the list 
            //and we can't remove it else it could break a LambdaBody, etc.

        else tf = elseNode = _LO_Next ( trueNode ) ;
    }
    return _LO_Eval ( elseNode, locals, 1 ) ; // last one no need to copy
}

// lisp 'list' function

ListObject *
LO_List ( ListObject * l0 )
{
    ListObject * lnew ;
    if ( l0 )
    {
        lnew = LO_New ( LIST, 0 ) ;
        l0 = _LO_First ( l0 ) ; // first is the 'list' token

        while ( l0 = _LO_Next ( l0 ) ) LO_AddToTail ( lnew, LO_Copy ( l0 ) ) ;
    }
    else lnew = 0 ;
    return lnew ;
}

ListObject *
LO_Begin ( ListObject * l0, ListObject * locals )
{
    ListObject * leval ;
    // 'begin' is first node ; skip it.
    if ( l0 )
    {
        for ( l0 = _LO_Next ( l0 ) ; l0 ; l0 = _LO_Next ( l0 ) )
        {
            leval = _LO_Eval ( l0, locals, 1 ) ;
        }
    }
    else leval = 0 ;
    return leval ;
}

ListObject *
LO_SpecialFunction ( ListObject * l0, ListObject * locals )
{
    ListObject * lfirst = _LO_First ( l0 ) ;
    switch ( lfirst->LType & ( ~ ( T_LISP_SYMBOL | T_LISP_SPECIAL | LISP_VOID_RETURN ) ) )
    {
        case T_LISP_BEGIN:
        {
            return LO_Begin ( lfirst, locals ) ;
        }
        case T_LISP_DEFINE:
        {
            return LO_Define ( lfirst, locals ) ; // same as compile
        }
        case T_LISP_MACRO:
        {
            return _LO_Macro ( lfirst, locals ) ; // same as compile
        }
        case T_LISP_CFRTIL:
        {
            return _LO_CfrTil ( lfirst ) ;
        }
        case T_LISP_COMPILE:
        {
            return _LO_Compile ( lfirst, locals ) ;
        }
        case T_LAMBDA:
        {
            return LO_MakeLambda ( l0 ) ;
        }
        case T_LISP_IF:
        {
            return LO_Cond ( lfirst, locals ) ;
        }
        case T_LISP_SET: return LO_Set ( lfirst, locals ) ;
        case T_LISP_LET: return LO_Let ( lfirst, locals ) ;
        case LISP_C_RTL_ARGS:
        {
            // ?!? LISP_C_RTL_ARGS here is hasn't been maintained ?!?
            //return _LO_CompileRun_C_Rtl_ArgList ( l0, lfirst ) ;
            return _LO_Apply_A_LtoR_ArgList_For_C_RtoL ( l0, lfirst ) ;
        }
        default:
        {

            return (( LispFunction2 ) ( lfirst->Lo_CfrTilWord->Definition ) ) ( lfirst, locals ) ;
        }
    }
}

ListObject *
_LO_Macro ( ListObject * l0, ListObject * locals )
{
    ListObject * nameNode = _LO_Next ( l0 ) ;
    l0 = _LO_Define0 ( "macro", nameNode, locals ) ; //nameNode, locals ) ;
    l0->LType |= T_LISP_MACRO ;

    return l0 ;
}

// setq

ListObject *
_LO_Set ( ListObject * lfirst, Namespace * ns )
{
    ListObject *l0, *lnext, * lsymbol, *lvalue ;
    // lfirst is the 'set' signature
    for ( l0 = lfirst ; ( lnext = _LO_Next ( l0 ) ) && ( lvalue = _LO_Next ( lnext ) ) ; l0 = lvalue )
    {
        lsymbol = lnext ; // we want to return the last symbol
        LO_Print ( _LO_Define0 ( "set", lsymbol, ns ) ) ;
    }
    return _Q_->OVT_LC->True ;
}

ListObject *
LO_Set ( ListObject * lfirst, ListObject * locals )
{
    return _LO_Set ( lfirst, locals ? locals : _Q_->OVT_LC->LispNamespace ) ;
}

ListObject *
LO_Let ( ListObject * lfirst, ListObject * locals )
{
    return _LO_Set ( lfirst, locals ) ;
}

ListObject *
_LO_CfrTil ( ListObject * lfirst )
{
    Context * cntx = _Q_->OVT_Context ;
    LambdaCalculus * lc = 0 ;
    ListObject *ldata, *locals, *word = 0 ; //, *ldata1 ;
    if ( _Q_->OVT_LC )
    {
        SetState ( _Q_->OVT_LC, LC_INTERP_MODE, true ) ;
        lc = _Q_->OVT_LC ;
        _Q_->OVT_LC = 0 ;
    }
    //_CfrTil_Namespace_NotUsing ( "Lisp" ) ; // nb. don't use Lisp words when compiling cfrTil
    for ( ldata = _LO_Next ( lfirst ) ; ldata ; ldata = _LO_Next ( ldata ) )
    {
        if ( ldata->LType & LIST_NODE )
        {
            locals = _CfrTil_Parse_LocalsAndStackVariables ( 1, 0, 1, ldata ) ;
            _Namespace_ActivateAsPrimary ( locals ) ;
        }
#if 1        
        else if ( ( ! GetState ( cntx, C_SYNTAX ) ) && String_Equal ( ldata->Name, ";" ) )
        {
            _LO_Semi ( word ) ;
        }
#endif        
        else if ( String_Equal ( ldata->Name, ":" ) )
        {
            word = _LO_Colon ( ldata ) ;
            ldata = _LO_Next ( ldata ) ; // bump ldata to account for name
        }
        else _Interpreter_InterpretAToken ( cntx->Interpreter0, ldata->Name ) ;
    }
    //Namespace_DoNamespace ( "Lisp" ) ;
    if ( lc )
    {
        _Q_->OVT_LC = lc ;
        SetState ( _Q_->OVT_LC, LC_INTERP_DONE, true ) ;
    }
    return nil ;
}

void
_LO_Semi ( Word * word )
{
    if ( word )
    {
        byte * blk = _CfrTil_EndBlock ( ) ;
        _Word ( word, blk ) ;
        SetState ( word, NOT_COMPILED, false ) ; // nb! necessary in recursive words
        Namespace_DoNamespace ( "Lisp" ) ;
    }
}

Word *
_LO_Colon ( ListObject * lfirst )
{
    Context * cntx = _Q_->OVT_Context ;
    ListObject *lcolon = lfirst, *lname, *ldata ;
    lname = _LO_Next ( lcolon ) ;
    ldata = _LO_Next ( lname ) ;
    _CfrTil_Namespace_NotUsing ( "Lisp" ) ; // nb. don't use Lisp words when compiling cfrTil
    CfrTil_RightBracket ( ) ;
    _CfrTil_InitSourceCode_WithName ( lname->Name ) ;
    Word * word = _Word_Create ( lname->Name ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, true ) ;
    CfrTil_BeginBlock ( ) ;
    return word ;
}

//
// nb. we must not use the word as the ListObject because it is on its own namespace list
// so we create a new ListObject with a pointer to the word for its values/properties

// can we just use the 'word' instead of this

//===================================================================================================================
//| _LO_Read 
//===================================================================================================================

// remember a Word is a ListObject 

ListObject *
LO_New_ParseRawStringOrLiteral ( byte * token, int32 parseFlag, int32 qidFlag )
{
    Lexer * lexer = _Q_->OVT_Context->Lexer0 ;
    if ( parseFlag ) _Lexer_Parse ( lexer, token, LispAllocType ) ; // same as Lexer_Literal except also consider a raw string as a token

    if ( Lexer_GetState ( lexer, KNOWN_OBJECT ) )
    {
        uint64 ctokenType = qidFlag ? OBJECT : lexer->TokenType | LITERAL ;
        Word * word = _DObject_New ( lexer->OriginalToken, lexer->Literal, ctokenType, ctokenType, ctokenType, ( byte* ) DataObject_Run, 0, 0, 0, 0 ) ;
        if ( ( ! qidFlag ) && ( lexer->TokenType & T_RAW_STRING ) )
        {
            // nb. we don't want to do this block with literals it slows down the eval and is wrong
            word->LType |= ( T_LISP_SYMBOL | T_RAW_STRING ) ;
            _Namespace_DoAddWord ( _Q_->OVT_LC->LispTemporariesNamespace, word ) ; // put it at the beginning of the list to be found first
            word->Lo_Value = ( int32 ) word->Lo_Name ; //( uint32 ) nil ;
        }
        word->Lo_CfrTilWord = word ;
        if ( qidFlag ) word->CType &= ~ T_LISP_SYMBOL ;
        return word ; // remember a Word is ListObject is a Namespace is a DObject is a Class is a DynamicObject.
    }
    else
    {
        Printf ( ( byte* ) "\n%s ?\n", ( char* ) token ) ;
        CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
        return 0 ;
    }
}

void
LO_Quote ( )
{
    if ( _Q_->OVT_LC ) _Q_->OVT_LC->ItemQuoteState |= QUOTED ;
    else Lexer_CheckMacroRepl ( _Q_->OVT_Context->Lexer0 ) ;
}

void
LO_QuasiQuote ( )
{
    if ( _Q_->OVT_LC ) _Q_->OVT_LC->ItemQuoteState |= QUASIQUOTED ;
    else Lexer_CheckMacroRepl ( _Q_->OVT_Context->Lexer0 ) ;
}

void
LO_UnQuoteSplicing ( )
{
    _Q_->OVT_LC->ItemQuoteState |= UNQUOTE_SPLICE ;
}

void
LO_UnQuote ( )
{
    _Q_->OVT_LC->ItemQuoteState |= UNQUOTED ;
}

ListObject *
_LO_Read ( )
{
    Context * cntx = _Q_->OVT_Context ;
    Compiler * compiler = cntx->Compiler0 ;
    Lexer * lexer = cntx->Lexer0 ;
    ListObject *l0, *lreturn, *lnew ;
    byte * token, *token1 ;
    LambdaCalculus * lc = LC_New ( 0 ) ;
    lnew = lc->LispParenLevel ? LO_New ( LIST, 0 ) : 0 ;
    lreturn = lnew ;
    if ( GetState ( _Q_->OVT_CfrTil, DEBUG_MODE ) ) Printf ( "\nEntering _LO_Read..." ) ;
    do
    {
next:
        token = _Lexer_ReadToken ( lexer, ( byte* ) " ,\n\r\t" ) ;
        if ( Lexer_IsTokenQualifiedID ( lexer ) ) SetState ( cntx, CONTEXT_PARSING_QID, true ) ;
        else SetState ( cntx, CONTEXT_PARSING_QID, false ) ;
        int32 qidFlag = GetState ( cntx, CONTEXT_PARSING_QID ) ;

        if ( token )
        {
            if ( String_Equal ( ( char* ) token, "(" ) )
            {
                Stack_Push ( _Q_->OVT_LC->QuoteStateStack, _Q_->OVT_LC->QuoteState ) ;
                _Q_->OVT_LC->QuoteState = _Q_->OVT_LC->ItemQuoteState ;
                lc->LispParenLevel ++ ;
                l0 = _LO_Read ( ) ;
                l0 = LO_New ( LIST_NODE, l0 ) ;
                _Q_->OVT_LC->QuoteState = Stack_Pop ( _Q_->OVT_LC->QuoteStateStack ) ;
            }
            else if ( String_Equal ( ( char* ) token, ")" ) ) break ;
            else
            {
                if ( qidFlag ) SetState ( cntx->Finder0, QID, true ) ;
                Word * word = _LO_FindWord ( 0, token, 0 ) ;
                if ( qidFlag ) SetState ( cntx->Finder0, QID, false ) ;
                if ( word )
                {
                    if ( word->LType & T_LISP_READ_MACRO )
                    {
                        word->Definition ( ) ; // scheme read macro preprocessor 
                        goto next ;
                    }
                    if ( word->LType & T_LISP_TERMINATING_MACRO )
                    {
                        word->Definition ( ) ; // scheme read macro preprocessor 
                        token1 = ( byte* ) _DataStack_Pop ( ) ;
                        l0 = LO_New_ParseRawStringOrLiteral ( token1, 0, 0 ) ; //don't parse a string twice; but other macros may need to be adjusted 
                    }
                    else
                    {
                        if ( word->CType & NAMESPACE_TYPE ) DataObject_Run ( word ) ;
                        l0 = _LO_New ( T_LISP_SYMBOL | word->LType, word->CType, ( byte* ) word->Lo_Value, word, 0, 0, LispAllocType ) ; // all words are symbols
                    }
                }
                else
                {
                    l0 = LO_New_ParseRawStringOrLiteral ( token, 1, qidFlag ) ;
                }
            }
            if ( qidFlag ) SetState ( l0, QID, true ) ;
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
                LO_AddToTail ( lnew, l0 ) ;
            }
            else
            {
                lreturn = l0 ; // did i break this by the "if ( l0 )" ?
                break ;
            }
        }
        else return nil ; //Error ( "\n_LO_Read : Syntax error : no token?\n", QUIT ) ;
        _Q_->OVT_LC->ItemQuoteState = 0 ;
    }
    while ( lc->LispParenLevel ) ;
    if ( GetState ( _Q_->OVT_CfrTil, DEBUG_MODE ) ) Printf ( "\nLeaving _LO_Read..." ) ;
    return lreturn ;
}

//===================================================================================================================
//| _LO_Apply 
//===================================================================================================================

ListObject *
LO_PrepareReturnObject ( )
{
    uint64 type = 0 ;
    byte * name ;
    if ( ! CompileMode )
    {
        Namespace * ns = _CfrTil_InNamespace ( ) ;
        name = ns->Name ;
        if ( String_Equal ( ( char* ) name, "BigInt" ) )
        {
            type = T_BIG_INT ;
        }
        else if ( String_Equal ( ( char* ) name, "BigFloat" ) )
        {
            type = T_BIG_FLOAT ;
        }
        return _LO_New ( LITERAL | type, LITERAL | type, ( byte* ) _DataStack_Pop ( ), 0, 0, 0, LispAllocType ) ;
    }
    else return nil ;
}

void
LO_BeginBlock ( )
{
    if ( ! _Q_->OVT_Context->Compiler0->BlockLevel ) _Q_->OVT_LC->SavedCodeSpace = CompilerMemByteArray ;
    _Compiler_SetCompilingSpace ( CompileMode ? ( byte* ) "CodeSpace" : ( byte* ) "TempObjectSpace" ) ;
    CfrTil_BeginBlock ( ) ;
}

void
LO_EndBlock ( )
{
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    if ( _Q_->OVT_LC && _Q_->OVT_LC->SavedCodeSpace )
    {
        NGR_Dsp_To_ESI ;
        CfrTil_EndBlock ( ) ;
        if ( ! GetState ( _Q_->OVT_LC, LC_COMPILE_MODE ) ) CfrTil_BlockRun ( ) ;

        if ( ! compiler->BlockLevel ) Set_CompilerSpace ( _Q_->OVT_LC->SavedCodeSpace ) ;
    }
}

void
LO_CheckEndBlock ( )
{
    LambdaCalculus * lc = _Q_->OVT_LC ;
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    if ( Compiler_GetState ( compiler, LISP_COMBINATOR_MODE ) )
    {
        int32 cii = _Stack_Top ( compiler->CombinatorInfoStack ) ;
        CombinatorInfo ci ; // remember sizeof of CombinatorInfo = 4 bytes
        ci.CI_i32_Info = cii ;
        if ( ( lc->LispParenLevel == ci.ParenLevel ) && ( compiler->BlockLevel > ci.BlockLevel ) )
        {
            LO_EndBlock ( ) ;
        }
    }
}

int32
LO_CheckBeginBlock ( )
{
    LambdaCalculus * lc = _Q_->OVT_LC ;
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    int32 cii = _Stack_Top ( compiler->CombinatorInfoStack ) ;
    CombinatorInfo ci ; // remember sizeof of CombinatorInfo = 4 bytes
    ci.CI_i32_Info = cii ;
    if ( ( Compiler_GetState ( compiler, LISP_COMBINATOR_MODE ) ) && ( lc->LispParenLevel == ci.ParenLevel ) && ( compiler->BlockLevel == ci.BlockLevel ) )
    {
        LO_BeginBlock ( ) ;

        return true ;
    }
    return false ;
}

int32
_LO_Apply_Arg ( ListObject ** pl1, int32 applyRtoL, int32 i )
{
    Context * cntx = _Q_->OVT_Context ;
    ListObject * l0, *l1, * l2 ;
    int32 svcm = CompileMode ;
    l1 = * pl1 ;
    Word * word = l1 ;
    byte * token = word->Name ; // for DEBUG macros
    DEBUG_START ;
    if ( GetState ( l1, QID ) )
    {
        // we're compiling right to left but we have a quid which implies the last token  of a quid so find the first token of the quid and then start compiling left to right
        if ( applyRtoL )
        {
            // find the first token of the quid
            for ( ; l1 ? ( l1->Name[0] != '.' ? GetState ( l1, QID ) : 1 ) : 0 ; l1 = LO_Previous ( l1 ) ) l2 = l1 ;
            l0 = l1 ;
            // start compiling left to right
            for ( l1 = l2 ; l1 ? ( l1->Name[0] != '.' ? GetState ( l1, QID ) : 1 ) : 0 ; l1 = LO_Next ( l1 ) )
            {
                i = _LO_Apply_Arg ( &l1, 0, i ) ; // 0 : don't recurse 
            }
            *pl1 = LO_Next ( l0 ) ; // when it returns it will need to do LO_Previous so LO_Next adjusts for that
            goto done ;
        }
    }
    if ( l1->LType & ( LIST | LIST_NODE ) )
    {
        Set_CompileMode ( false ) ;
        l2 = LO_Eval ( l1 ) ;
        Set_CompileMode ( svcm ) ;
        if ( ! l2 || ( l2->LType & T_NIL ) )
        {
            Compile_DspPop_EspPush ( ) ;
        }
        else
        {
            _Compile_PushEspImm ( ( int32 ) l2->Lo_Value ) ;
        }
    }
    else if ( ( l1->CType & NON_MORPHISM_TYPE ) ) //&& ( l1->Name [0] != '.' ) ) //l1->CType & NON_MORPHISM_TYPE ) //|| ( l1->Name [0] == '.' ) )
    {
        word = l1->S_CfrTilWord ;
        _Interpreter_Do_MorphismWord ( cntx->Interpreter0, word ) ;
        if ( CompileMode && ( ! ( l1->CType & ( NAMESPACE_TYPE | OBJECT_FIELD ) ) ) )
        {
            if ( word->StackPushRegisterCode ) SetHere ( word->StackPushRegisterCode ) ;
            _Compile_PushReg ( EAX ) ;
            i ++ ;
        }
    }
    else if ( l1->Name [0] == '.' )
    {
        _Interpreter_Do_MorphismWord ( cntx->Interpreter0, l1->S_CfrTilWord ) ;
    }
    else
    {
        _Compile_Esp_Push ( _DataStack_Pop ( ) ) ;
        i ++ ;
    }
done:
    DEBUG_SHOW ;
    return i ;
}
// for calling 'C' functions such as printf or other system functions
// where the arguments are pushed first from the end of the list like 'C' arguments
// this is a little confusing : the args are LO_Read left to Right for C we want them right to left except qid word which remain left to right

ListObject *
_LO_Apply_ArgList ( ListObject * l0, Word * word, int32 applyRtoL )
{
    Context * cntx = _Q_->OVT_Context ;
    ListObject *l1 ;
    ByteArray * scs ;
    Compiler * compiler = cntx->Compiler0 ;
    int32 i, svcm = CompileMode ;

    if ( GetState ( _Q_->OVT_CfrTil, DEBUG_MODE ) ) Printf ( "\nEntering _LO_Apply_ArgList..." ) ;
    if ( l0 )
    {
        if ( ! svcm )
        {
            scs = CompilerMemByteArray ;
            _Compiler_SetCompilingSpace ( ( byte* ) "SessionObjectsSpace" ) ;
            CfrTil_BeginBlock ( ) ;
        }
        if ( applyRtoL )
        {
            for ( i = 0, l1 = LO_Last ( l0 ) ; l1 ; l1 = LO_Previous ( l1 ) )
            {
                i = _LO_Apply_Arg ( &l1, applyRtoL, i ) ;
            }
        }
        else for ( i = 0, l1 = _LO_First ( l0 ) ; l1 ; l1 = LO_Next ( l1 ) ) i = _LO_Apply_Arg ( &l1, applyRtoL, i ) ;
    }
    byte * token = word->Name ; // only for DEBUG macros
    DEBUG_START ;
    {
        _Compiler_WordStack_PushWord ( compiler, word ) ; // ? l0 or word ?
        Compile_Call ( ( byte* ) word->Definition ) ;
        if ( i > 0 ) Compile_ADDI ( REG, ESP, 0, i * sizeof (int32 ), 0 ) ;
        if ( ! svcm )
        {
            CfrTil_EndBlock ( ) ;
            CfrTil_BlockRun ( ) ;
            Set_CompilerSpace ( scs ) ;
        }
        else if ( word->CType & C_RETURN )
        {
            _Word_CompileAndRecord_PushEAX ( word ) ;
        }
    }
    DEBUG_SHOW ;
    Set_CompileMode ( svcm ) ;
    if ( GetState ( _Q_->OVT_CfrTil, DEBUG_MODE ) ) Printf ( "\nLeaving _LO_Apply_ArgList..." ) ;
    return nil ;
}

ListObject *
_LO_Apply_C_LtoR_ArgList ( ListObject * l0, Word * word )
{
    _LO_Apply_ArgList ( l0, word, 0 ) ;
}

ListObject *
_LO_Apply_A_LtoR_ArgList_For_C_RtoL ( ListObject * l0, Word * word )
{
    _LO_Apply_ArgList ( l0, word, 1 ) ;
}

void
LC_Interpret_MorphismWord ( Word * word )
{
    _Interpret_MorphismWord_Default ( _Q_->OVT_Context->Interpreter0, word ) ;
}

void
LC_Interpret_AListObject ( ListObject * l0 )
{
    Word * word = l0->Lo_CfrTilWord ;
    if ( ! ( word->CType & LITERAL ) )
    {
        LC_Interpret_MorphismWord ( word ) ;
    }
    else _Q_->OVT_Context->CurrentRunWord = Lexer_Do_ObjectToken_New ( _Q_->OVT_Context->Lexer0, word->Name, 1 ) ;
}

void
LC_CompileRun_ArgList ( Word * word ) // C protocol : right to left arguments from a list pushed on the stack
{
    LambdaCalculus * lc = LC_New ( 1 ) ;
    Context * cntx = _Q_->OVT_Context ;
    Lexer * lexer = cntx->Lexer0 ;
    Compiler * compiler = cntx->Compiler0 ;
    byte *svDelimiters = lexer->TokenDelimiters ;
    ListObject * l0 ;
    byte * token = _Lexer_ReadToken ( lexer, ( byte* ) " ,;\n\r\t" ) ;
    if ( word->CType & ( C_PREFIX | C_PREFIX_RTL_ARGS ) )
    {
        if ( ( ! token ) || strcmp ( "(", ( char* ) token ) ) Error ( "Syntax error : C RTL Args : no '('", ABORT ) ; // should be '('
    }
    lc->LispParenLevel = 1 ;
    SetState ( compiler, LC_ARG_PARSING, true ) ;
    if ( word->CType & ( C_PREFIX | C_PREFIX_RTL_ARGS ) )
    {
        int32 svcm = CompileMode ;
        Set_CompileMode ( false ) ; // we must have the arguments pushed and not compiled for _LO_Apply_C_Rtl_ArgList which will compile them for a C_Rtl function
        l0 = _LO_Read ( ) ;
        Set_CompileMode ( svcm ) ; // we must have the arguments pushed and not compiled for _LO_Apply_C_Rtl_ArgList which will compile them for a C_Rtl function
        _LO_Apply_A_LtoR_ArgList_For_C_RtoL ( l0, word ) ;
        LC_Clear ( _Q_->OVT_LC ) ;
    }
    SetState ( compiler, LC_ARG_PARSING | LC_C_RTL_ARG_PARSING, false ) ;
    Lexer_SetTokenDelimiters ( lexer, svDelimiters, SESSION ) ;
}

// assumes list contains only one application 

void
_LO_CompileOrInterpret_One ( ListObject * l0 )
{
    Context * cntx = _Q_->OVT_Context ;
    // just interpret the non-nil, non-list objects
    // nil means that it doesn't need to be interpreted any more
    if ( ( l0 ) && ( ! ( l0->LType & ( LIST | LIST_NODE | T_NIL ) ) ) )
    {
        Word * word = l0->Lo_CfrTilWord ;
        byte * token = word ? word->Name : 0 ;
        DEBUG_START ;
        if ( GetState ( _Q_->OVT_CfrTil, DEBUG_MODE ) )
        {
            DebugColors ;
            Printf ( ( byte* ) "\n_LO_CompileOrInterpret_One : \n\tl0 =%s, l0->Lo_CfrTilWord = %s", _LO_PRINT ( l0 ), word ? word->Name : ( byte* ) "" ) ;
            DefaultColors ;
        }
        if ( word &&
            (
            ( ! ( l0->LType & ( LITERAL | T_LISP_SYMBOL ) ) ) ||
            ( l0->CType & ( BLOCK | CFRTIL_WORD | CPRIMITIVE ) ) ||
            ( CompileMode && ( l0->CType & ( LOCAL_VARIABLE | PARAMETER_VARIABLE ) ) ) )
            )
        {
            _Interpreter_Do_MorphismWord ( cntx->Interpreter0, word ) ;
        }
        else
        {
            _Compiler_WordStack_PushWord ( cntx->Compiler0, word ? word : l0 ) ; // ? l0 or word ?
            if ( word ) DataObject_Run ( word ) ;
            else DataObject_Run ( l0 ) ;
        }
        DEBUG_SHOW ;
    }
}

void
_LO_CompileOrInterpret ( ListObject * lfunction, ListObject * ldata )
{
    if ( GetState ( _Q_->OVT_CfrTil, DEBUG_MODE ) )
    {
        DebugColors ;
        Printf ( ( byte* ) "\n_LO_CompileOrInterpret : \n\tlfunction =%s\n\tldata =%s", _LO_PRINT ( lfunction ), _LO_PRINT ( ldata ) ) ;
        DefaultColors ;
    }
    ListObject * lfword = lfunction->Lo_CfrTilWord ;

    if ( ldata && lfword && ( lfword->CType & ( CATEGORY_OP_ORDERED | CATEGORY_OP_UNORDERED ) ) ) // ?!!? 2 arg op with multi-args : this is not a precise distinction yet : need more types ?!!? 
    {
        _LO_CompileOrInterpret_One ( ldata ) ;
        while ( ( ldata = _LO_Next ( ldata ) ) )
        {
            _LO_CompileOrInterpret_One ( ldata ) ; // two args first then op, then after each arg the operator : nb. assumes word can take unlimited args 2 at a time
            _LO_CompileOrInterpret_One ( lfword ) ;
        }
    }
    else
    {
        for ( ; ldata ; ldata = _LO_Next ( ldata ) )
        {
            if ( GetState ( _Q_->OVT_LC, LC_INTERP_DONE ) ) return ;
            _LO_CompileOrInterpret_One ( ldata ) ;
        }
        if ( lfword && ( ! ( lfword->CType & LISP_CFRTIL ) ) ) _LO_CompileOrInterpret_One ( lfword ) ; // ( ! ( lfword->CType & LISP_CFRTIL ) ) : don't do it twice (see above)
    }
}
// for calling 'C' functions such as printf or other system functions
// where the arguments are pushed first from the end of the list like 'C' arguments

ListObject *
_LO_Apply ( ListObject * l0, ListObject * lfunction, ListObject * ldata )
{
    LambdaCalculus * lc = _Q_->OVT_LC ;
    ListObject * lfdata = _LO_First ( ldata ), *vReturn ;
    if ( GetState ( _Q_->OVT_CfrTil, DEBUG_MODE ) )
    {
        DebugColors ;
        Printf ( ( byte* ) "\n_LO_Apply : \n\tl0 =%s", _LO_PRINT ( l0 ) ) ;
        DefaultColors ;
    }
    if ( lfunction->LType & LIST_FUNCTION ) return ( ( ListFunction ) lfunction->Lo_CfrTilWord->Definition ) ( l0 ) ;
    if ( lfunction->CType & CFRTIL_WORD ) // this case is hypothetical for now
    {
        _Interpreter_Do_MorphismWord ( _Q_->OVT_Context->Interpreter0, lfunction->Lo_CfrTilWord ) ;
        return nil ;
    }
    else if ( lfdata )
    {
        _LO_CompileOrInterpret ( lfunction, lfdata ) ;
        lc->LispParenLevel -- ;
        // this is necessary in "lisp" mode : eg. if user hits return but needs to be clarified, refactored, maybe renamed, etc.    
        if ( ! GetState ( _Q_->OVT_LC, LC_INTERP_DONE ) )
        {
            if ( CompileMode ) LO_CheckEndBlock ( ) ;
            vReturn = LO_PrepareReturnObject ( ) ;
        }
        else
        {
            vReturn = nil ;
        }
    }
    else
    {
        lc->LispParenLevel -- ;
        if ( CompileMode ) LO_CheckEndBlock ( ) ;
        SetState ( _Q_->OVT_LC, LC_COMPILE_MODE, false ) ;
        vReturn = lfunction ;
    }
    return vReturn ;
}

// subst : lisp 1.5
// set the value of the lambda parameters to the function call values - a beta reduction in the lambda calculus 

void
LO_Substitute ( ListObject *lambdaParameters, ListObject * funcCallValues )
{
    while ( lambdaParameters && funcCallValues )
    {
        // just preserve the name of the arg for the finder
        // so we now have the call values with the parameter names - parameter names are unchanged 
        // so when we eval/print these parameter names they will have the function calling values -- lambda calculus substitution - beta reduction
        funcCallValues->Lo_Name = lambdaParameters->Lo_Name ;
        lambdaParameters = _LO_Next ( lambdaParameters ) ;
        funcCallValues = _LO_Next ( funcCallValues ) ;
    }
}

//===================================================================================================================
//| LO_Print
//===================================================================================================================

char *
_LO_Print ( ListObject * l0, char * buffer, int lambdaFlag, int printValueFlag )
{
    char * format ;
    if ( ! buffer )
    {
        buffer = ( char* ) _Q_->OVT_CfrTil->LispPrintBuffer ;
        buffer [0] = 0 ;
    }
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) )
        {
            byte * buffer2 = Buffer_New_pbyte ( BUFFER_SIZE ) ;
            _LO_PrintList ( l0, buffer2, lambdaFlag, printValueFlag ) ;
            if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
        }
        else if ( ( l0 == nil ) || ( l0->LType == T_NIL ) )
        {
            if ( _AtCommandLine ( ) )
            {
                snprintf ( buffer, BUFFER_SIZE, " nil" ) ;
            }
        }
        else if ( l0->LType == true )
        {
            if ( _AtCommandLine ( ) )
            {
                snprintf ( buffer, BUFFER_SIZE, " T" ) ;
            }
        }
        else if ( l0->LType & T_LISP_COMPILED_WORD )
        {
            if ( ( l0->Lo_CfrTilWord ) && ( ! GetState ( _Q_->OVT_LC, LC_PRINTED_SOURCE_CODE ) ) )
            {
                snprintf ( buffer, BUFFER_SIZE, " %s", ( char* ) l0->Lo_CfrTilWord->SourceCode ) ;
                SetState ( _Q_->OVT_LC, LC_PRINTED_SOURCE_CODE, true ) ;
            }
        }
        else if ( l0->LType == T_RAW_STRING )
        {
            snprintf ( buffer, BUFFER_SIZE, " %s", ( char* ) l0->Lo_Value ) ;
        }
        else if ( l0->LType & T_LISP_SYMBOL )
        {
            if ( LO_IsQuoted ( l0 ) ) snprintf ( buffer, BUFFER_SIZE, " %s", l0->Lo_Name ) ;
            else if ( ( ! lambdaFlag ) && l0->Lo_CfrTilWord && ( l0->LType & T_LAMBDA ) && ( ! ( l0->LType & T_LISP_SPECIAL ) ) ) // lambdaFlag == lambdaFlag : don't print internal lambda param/body
            {
                byte * buffer2 = Buffer_New_pbyte ( BUFFER_SIZE ) ;
                snprintf ( buffer2, BUFFER_SIZE, " %s", l0->Lo_Name ) ;
                if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
                _LO_PrintList ( ( ListObject * ) l0->Lo_LambdaFunctionParameters, buffer2, 1, printValueFlag ) ; // 1 : lambdaFlag = 1 
                if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
                _LO_PrintList ( ( ListObject * ) l0->Lo_LambdaFunctionBody, buffer2, 1, printValueFlag ) ; // 1 : lambdaFlag = 1 
                if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
                if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
            }
            else if ( printValueFlag ) //&& GetState ( _Q_->OVT_LC, ( PRINT_VALUE ) ) )
            {
                if ( l0->Lo_Value != ( uint32 ) nil )
                {
                    if ( ( ! l0->Lo_Value ) && l0->Lo_CfrTilWord )
                    {
                        if ( _Q_->Verbosity > 2 ) snprintf ( buffer, BUFFER_SIZE, " %s = 0x%08x", l0->Lo_CfrTilWord->Lo_Name, ( int32 ) l0->Lo_CfrTilWord ) ;
                        else snprintf ( buffer, BUFFER_SIZE, " %s", l0->Lo_Name ) ;
                    }
                    else if ( l0->LType & T_RAW_STRING )
                    {
                        snprintf ( buffer, BUFFER_SIZE, " %s", ( char* ) l0->Lo_Value ) ;
                    }
                    else
                    {
                        return _LO_Print ( ( ListObject * ) l0->Lo_Value, buffer, 0, printValueFlag ) ; //lambdaFlag, printValueFlag ) ;
                    }
                }
                else
                {
                    if ( _Q_->Verbosity > 2 ) snprintf ( buffer, BUFFER_SIZE, " nil: %s", l0->Lo_Name ) ;
                    else snprintf ( buffer, BUFFER_SIZE, " %s", l0->Lo_Name ) ;
                }
            }
            else snprintf ( buffer, BUFFER_SIZE, " %s", l0->Lo_Name ) ;
        }
        else if ( l0->LType & T_STRING )
        {
            if ( l0->State & UNQUOTED ) snprintf ( buffer, BUFFER_SIZE, " %s", ( char* ) l0->Lo_String ) ;
            else snprintf ( buffer, BUFFER_SIZE, " \"%s\"", ( char* ) l0->Lo_String ) ;
        }
        else if ( l0->LType & BLOCK )
        {
            snprintf ( buffer, BUFFER_SIZE, " %s:#<BLOCK>:0x%08x", l0->Lo_Name, ( uint ) l0->Lo_UInteger ) ;
        }
        else if ( l0->LType & T_BIG_INT )
        {
            gmp_sprintf ( buffer, " %Zd\n", l0->Lo_Value ) ;
        }
        else if ( l0->LType & T_BIG_FLOAT )
        {
            gmp_sprintf ( buffer, " %*.*Ff\n", _Q_->OVT_Context->System0->BigNumWidth, _Q_->OVT_Context->System0->BigNumPrecision, l0->Lo_Value ) ;
        }
        else if ( l0->LType & T_INT )
        {
            if ( _Q_->OVT_Context->System0->NumberBase == 16 ) snprintf ( buffer, BUFFER_SIZE, " 0x%08x", ( uint ) l0->Lo_UInteger ) ;
            else
            {
                format = ( ( ( int32 ) l0->Lo_Integer ) < 0 ) ? ( char* ) " 0x%08x" : ( char* ) " %d" ;
                snprintf ( buffer, BUFFER_SIZE, format, l0->Lo_Integer ) ;
            }
        }
        else if ( l0->LType & T_LAMBDA )
        {
            snprintf ( buffer, BUFFER_SIZE, " #<LAMBDA>" ) ;
        }
        else if ( l0->LType & LITERAL )
        {
            format = ( ( ( int32 ) l0->Lo_Integer ) < 0 ) ? ( char* ) " 0x%08x" : ( char* ) " %d" ;
            if ( ( l0->Lo_Integer < 0 ) || ( _Q_->OVT_Context->System0->NumberBase == 16 ) ) snprintf ( buffer, BUFFER_SIZE, " 0x%08x", ( uint ) l0->Lo_UInteger ) ;
            else snprintf ( buffer, BUFFER_SIZE, format, l0->Lo_Integer ) ;
        }
        else if ( l0->LType & ( CPRIMITIVE | CFRTIL_WORD ) )
        {
            snprintf ( buffer, BUFFER_SIZE, " %s", l0->Lo_Name ) ;
        }
        else if ( l0->LType & ( T_HEAD | T_TAIL ) )
        {
            ; //break ;
        }
        else
        {
            if ( l0->Lo_CfrTilWord && l0->Lo_CfrTilWord->Lo_Name ) snprintf ( buffer, BUFFER_SIZE, " %s", l0->Lo_CfrTilWord->Lo_Name ) ;
        }
    }
done:

    return buffer ;
}

char *
_LO_PrintList ( ListObject * l0, char * buffer, int lambdaFlag, int printValueFlag )
{
    byte * buffer2 = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    ListObject * l1, *lnext ;
    if ( ! buffer )
    {
        buffer = ( char* ) _Q_->OVT_CfrTil->LispPrintBuffer ;
        buffer [0] = 0 ;
    }
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) ) snprintf ( buffer2, BUFFER_SIZE, " (" ) ;
        if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
        for ( l1 = _LO_First ( l0 ) ; l1 ; l1 = lnext ) //_LO_Next ( l1 ) ) 
        {
            lnext = _LO_Next ( l1 ) ; //
            if ( l1->LType & ( T_LAMBDA | T_LISP_SPECIAL ) ) lambdaFlag = 1 ;
            if ( l1->LType & ( LIST | LIST_NODE ) )
            {
                _LO_PrintList ( l1, buffer2, lambdaFlag, printValueFlag ) ;
                if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
            }
            else
            {
                _LO_Print ( l1, buffer2, lambdaFlag, printValueFlag ) ;
                if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
            }
        }
        if ( l0->LType & ( LIST | LIST_NODE ) ) snprintf ( buffer2, BUFFER_SIZE, " )" ) ;
        LO_strcat ( buffer, buffer2 ) ;
    }
    return buffer ;
}

void
LO_Print ( ListObject * l0 )
{
    DefaultColors ;
    ConserveNewlines ;
    SetState ( _Q_->OVT_LC, ( LC_PRINT_VALUE ), true ) ;
    Printf ( ( byte* ) "%s", _LO_PRINT ( l0 ) ) ;
    SetState ( _Q_->OVT_LC, LC_PRINT_VALUE, false ) ;
    AllowNewlines ;
}

//===================================================================================================================
//| LO Misc : _LO_FindWord _LO_New _LO_Copy
//===================================================================================================================

ListObject *
_LO_First ( ListObject * l0 )
{
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) ) return ( ListObject* ) DLList_First ( ( DLList * ) l0->Lo_List ) ;

        else return l0 ;
    }
    return 0 ;
}

ListObject *
_LO_Last ( ListObject * l0 )
{
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) ) return ( ListObject* ) DLList_Last ( ( DLList * ) l0->Lo_List ) ;

        else return l0 ;
    }
    return 0 ;
}

ListObject *
_LO_Next ( ListObject * l0 )
{
    return ( ListObject* ) DLNode_Next ( ( DLNode* ) l0 ) ;
}

Word *
_LO_FindWord ( Namespace * l0, byte * name, ListObject * locals )
{
    Word * word = 0 ;
    //name = l0 ? l0->Name : name ;
#if 0 // not used yet    
    if ( l0 && l0->S_SymbolList )
    {
        //if ( word ) word->LType |= T_LISP_LOCALS_ARG ;
        word = Word_FindInOneNamespace ( l0, name ) ;
    }
    if ( ! word )
#endif     
    {
        if ( locals ) word = Word_FindInOneNamespace ( locals, name ) ;
        if ( ! word )
        {
            word = Word_FindInOneNamespace ( _Q_->OVT_LC->LispTemporariesNamespace, name ) ;
            if ( ! word )
            {
                word = Word_FindInOneNamespace ( _Q_->OVT_LC->LispNamespace, name ) ; // prefer Lisp namespace
                if ( ! word )
                {
                    word = Finder_Word_FindUsing ( _Q_->OVT_Context->Finder0, name, 0 ) ;
                }
            }
        }
        return word ;
    }
}

ListObject *
//_DataObject_New ( uint64 type, byte * name, uint64 ctype, uint64 ltype, int32 index, int32 value )
_LO_New ( uint64 ltype, uint64 ctype, byte * value, Word * word, int32 addFlag, byte * name, uint64 allocType )
{
    //_DObject_New ( byte * name, uint32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, int32 allocType )
    ListObject * l0 = _DObject_New ( word ? word->Name : name, ( uint32 ) value, ctype, ltype,
        ltype & T_LISP_SYMBOL ? word ? word->RunType : 0 : 0, 0, 0, 0, addFlag ? _Q_->OVT_LC->LispTemporariesNamespace : 0, allocType | EXISTING ) ;
    if ( ltype & LIST ) _LO_ListInit ( l0, allocType ) ;
    if ( word )
    {
        l0->Lo_CfrTilWord = word ;
        word->Lo_CfrTilWord = word ;
        word->Lo_Value = ( uint32 ) value ;
    }
    return l0 ;
}

void
LO_SpliceAtTail ( ListObject * lnew, ListObject * l0 ) // ltbat : list to be added to ; ltbat : list to be added 
{
    if ( lnew )
    {
        for ( l0 = _LO_First ( l0 ) ; l0 ; l0 = _LO_Next ( l0 ) )
        {
            LO_AddToTail ( lnew, LO_CopyOne ( l0 ) ) ;
        }
    }
}

ListObject *
_LO_AllocCopyOne ( ListObject * l0, uint64 allocType )
{
    ListObject * l1 = 0 ;
    if ( l0 )
    {
        l1 = ( ListObject * ) _Object_Allocate ( sizeof ( ListObject ), allocType ) ; //Mem_Allocate ( ( sizeof (ListObject ) + ((slots-1) * CELL), AllocType ) ;
        memcpy ( l1, l0, sizeof ( ListObject ) ) ;
        // nb. since we are coping the car/cdr are the same as the original so we must clear them else when try to add to the list and remove first it will try to remove from a wrong list so ...
        l1->Lo_Car = 0 ;
        l1->Lo_Cdr = 0 ;
    }
    return l1 ;
}

void
_LO_ListInit ( ListObject * l0, uint64 allocType )
{
    l0->Lo_Head = _DLNode_New ( allocType ) ;
    l0->Lo_Tail = _DLNode_New ( allocType ) ;
    _DLList_Init ( ( DLList * ) l0 ) ;
    l0->Lo_List = ( DLList* ) l0 ;
    l0->LType |= LIST ; // a LIST_NODE needs to be initialized also to be also a LIST
}

ListObject *
_LO_ListNode_Copy ( ListObject * l0, uint64 allocType )
{
    ListObject * l1 = _LO_AllocCopyOne ( l0, allocType ) ;
    _LO_ListInit ( l1, allocType ) ;
    return l1 ;
}

ListObject *
_LO_CopyOne ( ListObject * l0, uint64 allocType )
{
    ListObject *l1 = 0 ;
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) )
        {
            l1 = _LO_Copy ( l0, allocType ) ;
            if ( l0->LType & LIST_NODE ) l1 = _LO_New ( LIST_NODE, 0, ( byte * ) l1, 0, 0, 0, allocType ) ;
        }

        else l1 = _LO_AllocCopyOne ( l0, allocType ) ;
    }
    return l1 ;
}

// copy a whole list or a single node

ListObject *
_LO_Copy ( ListObject * l0, uint64 allocType )
{
    ListObject * lnew = 0, *l1 ;
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) ) lnew = _LO_ListNode_Copy ( l0, allocType ) ;
        for ( l0 = _LO_First ( l0 ) ; l0 ; l0 = _LO_Next ( l0 ) )
        {
            l1 = _LO_CopyOne ( l0, allocType ) ;
            if ( lnew ) LO_AddToTail ( lnew, l1 ) ;

            else return l1 ;
        }
    }
    return lnew ;
}

Boolean
LO_strcat ( char * buffer, char * buffer2 )
{
    if ( strlen ( buffer2 ) + strlen ( buffer ) >= BUFFER_SIZE )
    {
        Error ( "LambdaCalculus : LO_strcat : buffer overflow.", QUIT ) ;
    }
    else strcat ( buffer, buffer2 ) ;
    buffer2 [0] = 0 ;
    return true ;
}

//===================================================================================================================
//| LO_Repl
//===================================================================================================================

void
LC_EvalPrint ( LambdaCalculus * lc, ListObject * l0 )
{
    ListObject * l1 ;
    l0 = _LO_MacroPreprocess ( l0 ) ;
    l1 = LO_Eval ( l0 ) ;
    SetState ( _Q_->OVT_LC, LC_PRINTED_SOURCE_CODE, false ) ;
    LO_PrintWithValue ( l1 ) ;
}

void
_LO_ReadEvalPrint_ListObject ( ListObject * l0, int32 parenLevel, int32 continueFlag )
{
    LambdaCalculus * lc ;
    Lexer * lexer = _Q_->OVT_Context->Lexer0 ;
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    byte *svDelimiters = lexer->TokenDelimiters ;
    compiler->BlockLevel = 0 ;
    SetState ( compiler, LISP_MODE, true ) ;

    lc = LC_New ( ( ! continueFlag ) ) ;
    lc->LispParenLevel = parenLevel ;
    lc->SaveStackPtr = SaveStackPointer ( ) ; // ?!? maybe we should do this stuff differently : literals are pushed on the stack by the interpreter

    if ( ! l0 ) l0 = _LO_Read ( ) ;
    LC_EvalPrint ( lc, l0 ) ;

    if ( lc->SaveStackPtr ) RestoreStackPointer ( lc->SaveStackPtr ) ; // ?!? maybe we should do this stuff differently
    lc->LispParenLevel = 0 ;
    //SetState ( _Q_->OVT_LC, LC_INTERP_DONE, true ) ;

    if ( ! continueFlag )
    {
        LC_Clear ( lc ) ;
        Compiler_Init ( compiler, 0 ) ; // we could be compiling a cfrTil word as in oldLisp.cft
        SetBuffersUnused ;
        AllowNewlines ;
        Lexer_SetTokenDelimiters ( lexer, svDelimiters, 0 ) ;
    }
}

void
LO_ReadEvalPrint_ListObject ( )
{
    _LO_ReadEvalPrint_ListObject ( 0, 0, 1 ) ;
}

void
_LO_ReadEvalPrint0 ( int32 parenLevel )
{
    _LO_ReadEvalPrint_ListObject ( 0, parenLevel, 0 ) ;
}

void
LO_ReadEvalPrint1 ( )
{
    CfrTil_SourceCode_Init ( ) ;
    _LO_ReadEvalPrint_ListObject ( 0, 1, 0 ) ;
    SetState ( _Q_->OVT_CfrTil, SOURCE_CODE_INITIALIZED, false ) ;
}

void
LO_ReadEvalPrint ( )
{
    CfrTil_InitSourceCode_WithCurrentInputChar ( ) ;
    Namespace_ActivateAsPrimary ( ( byte* ) "Lisp" ) ;
    _LO_ReadEvalPrint_ListObject ( 0, 0, 0 ) ;
}

void
LO_ReadEvalPrint2 ( )
{
    CfrTil_SourceCode_Init ( ) ;
    _LO_ReadEvalPrint_ListObject ( 0, 0, 1 ) ;
}

void
LO_ReadInitFile ( byte * filename )
{
    Context * cntx = _Q_->OVT_Context ;
    FILE * file = fopen ( ( char* ) filename, "r" ) ;
    if ( file )
    {
        ReadLiner * rl = cntx->ReadLiner0 ;
        rl->Filename = String_New ( filename, DICTIONARY ) ;
        if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\nincluding %s ...\n", filename ) ;
        cntx->ReadLiner0->InputFile = file ;
        ReadLine_SetRawInputFunction ( rl, ReadLine_GetNextCharFromString ) ;
        cntx->System0->IncludeFileStackNumber ++ ;
        _SetEcho ( 0 ) ;

        ReadLine_SetInputString ( rl, _File_ReadToString_ ( file ) ) ;
        fclose ( file ) ;

        //Interpret_UntilFlaggedWithInit ( cntx->Interpreter0, END_OF_STRING ) ;

        cntx->System0->IncludeFileStackNumber -- ;
        if ( ! cntx->System0->IncludeFileStackNumber ) Ovt_AutoVarOff ( ) ;
        if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\n%s included\n", filename ) ;
    }
    else Printf ( ( byte* ) "\nError : _CfrTil_IncludeFile : \"%s\" : not found!\n", filename ) ;
}

void
LO_Repl ( )
{
    LambdaCalculus * lc = LC_New ( 1 ) ;
    Compiler * compiler = _Q_->OVT_Context->Compiler0 ;
    lc->LispParenLevel = 0 ;
    compiler->BlockLevel = 0 ;
    SetState ( compiler, LISP_MODE, true ) ;
    CfrTil_DebugModeOff ( ) ;
    Namespace_DoNamespace ( "Lisp" ) ;
    SetState ( lc, LC_REPL, true ) ;
    Printf ( ( byte* ) "\ncfrTil lisp : (type ';;' to exit)\n including init file :: './namespaces/compiler/lcinit.cft'\n" ) ;
    LO_ReadInitFile ( "./namespaces/compiler/lcinit.cft" ) ;
    lc->SaveStackPtr = SaveStackPointer ( ) ; // ?!? maybe we should do this stuff differently : literals are pushed on the stack by the interpreter
    _Repl ( ( block ) LO_ReadEvalPrint_ListObject ) ;
    if ( lc->SaveStackPtr ) RestoreStackPointer ( lc->SaveStackPtr ) ; // ?!? maybe we should do this stuff differently
    SetState ( lc, LC_REPL, false ) ;
}

//===================================================================================================================
//| LC_ : lambda calculus
//===================================================================================================================

void
_LC_Init ( LambdaCalculus * lc, int32 newFlag )
{

    lc->LispNamespace = Namespace_Find ( ( byte* ) "Lisp" ) ;
    //lc->LispTemporariesNamespace = _Namespace_New ( ( byte* ) "LispTemporaries", lc->LispNamespace ) ;
    lc->LispTemporariesNamespace = Namespace_FindOrNew_SetUsing ( ( byte* ) "LispTemporaries", lc->LispNamespace, 0 ) ;
    lc->SavedCodeSpace = 0 ;
    lc->Nil = _LO_New ( T_NIL, 0, 0, 0, 0, 0, LISP_TEMP ) ;
    lc->True = _LO_New ( ( uint64 ) true, 0, 0, 0, 0, 0, LISP_TEMP ) ;
    lc->OurCfrTil = _Q_->OVT_CfrTil ;
    lc->QuoteState = 0 ;
    lc->LispParenLevel = 0 ;
    if ( newFlag ) lc->QuoteStateStack = Stack_New ( 64, LISP_TEMP ) ;
    else _Stack_Init ( lc->QuoteStateStack, 64 ) ;
    lc->State = 0 ;
}

LC_Clear ( LambdaCalculus * lc )
{
    _Namespace_Clear ( lc->LispTemporariesNamespace ) ;
    _Q_->OVT_LC = 0 ;
}

LambdaCalculus *
LC_New ( int32 initFlag )
{
    int32 newFlag = 0 ;
    // ? do we need to do this for every lisp eval ?
    // we need a persistent lisp namespace
    if ( ! _Q_->OVT_LC )
    {
        LambdaCalculus * lc = ( LambdaCalculus * ) Mem_Allocate ( sizeof (LambdaCalculus ), LISP ) ;
        newFlag = 1 ;
        _Q_->OVT_LC = lc ;
    }
    if ( initFlag || newFlag ) _LC_Init ( _Q_->OVT_LC, newFlag ) ;
    return _Q_->OVT_LC ;
}

