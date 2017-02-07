
#include "../../include/cfrtil.h"

/* preprocessor BNF :
 *  ppBlock      =:=     #if (elifBlock)* (elseBlock)? #endif
 *  elifBlock    =:=     #elif (ppBlock)*
 *  elseBlock    =:=     #else (ppBlock)*
 */
// "#if" stack pop is 'true' interpret until "#else" and this does nothing ; if stack pop 'false' skip to "#else" token skip those tokens and continue interpreting

int32
GetOuterBlockStatus ( )
{
    int32 i, llen = List_Length ( _Context_->Interpreter0->PreprocessorStackList ) ;
    Ppibs status, obstatus ;
    if ( llen > 1 ) status.int32_Ppibs = List_GetN ( _Context_->Interpreter0->PreprocessorStackList, 1 ) ;
    else return 1 ; // no outer block -> we should be interpreting there
    if ( status.IfBlockStatus && ( llen > 2 ) ) // a non existing list element will have 0 status
    {
        for ( i = 2 ; i < llen ; i ++ )
        {
            obstatus.int32_Ppibs = List_GetN ( _Context_->Interpreter0->PreprocessorStackList, i ) ;
            status.IfBlockStatus = status.IfBlockStatus && obstatus.IfBlockStatus ;
            status.ElifStatus = obstatus.ElifStatus ;
        }
    }
    return status.int32_Ppibs ;
}

int32
_GetCondStatus ( )
{
    Context * cntx = _Context_ ;
    int32 status ;
    int32 svcm = GetState ( cntx->Compiler0, COMPILE_MODE ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, false ) ;
    _Interpret_ToEndOfLine ( cntx->Interpreter0 ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, svcm ) ;
    status = _DataStack_Pop ( ) ;
    if ( status > 0 ) status = 1 ;
    else return 0 ; //return status ;
}

int32
GetIfStatus ( )
{
    Ppibs obstatus, cstatus, top ;
    cstatus.int32_Ppibs = 0 ;
    top.int32_Ppibs = List_Top ( _Context_->Interpreter0->PreprocessorStackList ) ;
    Namespace_SetAsNotUsing ( ( byte* ) "PreProcessor" ) ;
    int32 cond = _GetCondStatus ( ) ;
    if ( top.ElifStatus )
    {
        cstatus.ElifStatus = 1 ;
        if ( top.DoIfStatus )
        {
            cstatus.DoIfStatus = ! cond ;
            cstatus.IfBlockStatus = cond ;
        }
        else cstatus.IfBlockStatus = 0 ;
    }
    else
    {
        cstatus.int32_Ppibs = cond ;
        obstatus.int32_Ppibs = GetOuterBlockStatus ( ) ;
        cstatus.IfBlockStatus = cstatus.IfBlockStatus && obstatus.IfBlockStatus ;
    }
    List_Push ( _Context_->Interpreter0->PreprocessorStackList, cstatus.int32_Ppibs ) ;
    return cstatus.IfBlockStatus ;
}

int32
_GetElxxStatus ( int32 cond, int32 type )
{
    Ppibs status, obstatus, top ;
    status.int32_Ppibs = 0, obstatus.int32_Ppibs = 0 ;
    top.int32_Ppibs = List_Top ( _Context_->Interpreter0->PreprocessorStackList ) ;
    //Namespace_SetAsNotUsing ( ( byte* ) "PreProcessor" ) ;
    if ( ! top.IfBlockStatus )
    {
        obstatus.IfBlockStatus = GetOuterBlockStatus ( ) ;
        if ( type == PP_ELIF ) status.IfBlockStatus = cond && obstatus.IfBlockStatus ;
        else status.IfBlockStatus = top.ElifStatus ? top.DoIfStatus && obstatus.IfBlockStatus : obstatus.IfBlockStatus ;
    }
    else status.IfBlockStatus = 0 ; // if top is true the 'elxx' block should not be interpreted
    if ( top.ElifStatus )
    {
        status.ElifStatus = 1 ;
    }
    else if ( type == PP_ELIF )
    {
        if ( cond ) status.ElifStatus = 1 ;
        status.DoIfStatus = 1 ;
    }
    List_SetTop ( _Context_->Interpreter0->PreprocessorStackList, status.int32_Ppibs ) ;
    return status.IfBlockStatus ;
}

int32
GetElifStatus ( )
{
    int32 cond = _GetCondStatus ( ) ;
    return _GetElxxStatus ( cond, PP_ELIF ) ;
}

int32
GetElseStatus ( )
{
    return _GetElxxStatus ( 1, PP_ELSE ) ; // 
}

int32
GetEndifStatus ( )
{
    Ppibs status, top ;
    status.int32_Ppibs = GetOuterBlockStatus ( ) ;
    List_Pop ( _Context_->Interpreter0->PreprocessorStackList ) ;
    return status.IfBlockStatus ;
}

void
SkipPreprocessorCode ( )
{
    Context * cntx = _Context_ ;
    byte * token ; //= ( byte* ) 1 ;
    do
    {
        int inChar = ReadLine_PeekNextChar ( cntx->ReadLiner0 ) ;
        if ( ( inChar == - 1 ) || ( inChar == eof ) )
        {
            SetState ( cntx->Lexer0, LEXER_END_OF_LINE, true ) ;
            return ;
        }
        token = Lexer_ReadToken ( cntx->Lexer0 ) ;
        if ( token )
        {
            if ( String_Equal ( token, "//" ) ) CfrTil_CommentToEndOfLine ( ) ;
            else if ( String_Equal ( token, "/*" ) ) CfrTil_ParenthesisComment ( ) ;
            else if ( String_Equal ( token, "#" ) )
            {
                byte * token1 = Lexer_ReadToken ( cntx->Lexer0 ) ;
                if ( token1 )
                {
                    //Finder_SetNamedQualifyingNamespace ( _Context_->Finder0, ( byte* ) "PreProcessor" ) ;
                    if ( String_Equal ( token1, "if" ) )
                    {
                        if ( GetIfStatus ( ) ) return ; // PP_INTERP
                    }
                    else if ( String_Equal ( token1, "else" ) )
                    {
                        if ( GetElseStatus ( ) ) return ;
                    }
                    else if ( String_Equal ( token1, "elif" ) )
                    {
                        if ( GetElifStatus ( ) ) return ;
                    }
                    else if ( String_Equal ( token1, "endif" ) )
                    {
                        if ( GetEndifStatus ( ) ) return ;
                    }
                    //else syntax error
                }
                else return ;
            }
        }
    }
    while ( token ) ;
}

