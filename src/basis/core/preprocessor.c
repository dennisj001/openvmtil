
#include "../../include/cfrtil.h"

/* preprocessor BNF :
 *  ppBlock      =:=     #if (elifBlock)* (elseBlock)? #endif
 *  elifBlock    =:=     #elif (ppBlock)*
 *  elseBlock    =:=     #else (ppBlock)*
 */
// "#if" stack pop is 'true' interpret until "#else" and this does nothing ; if stack pop 'false' skip to "#else" token skip those tokens and continue interpreting

int64
GetOuterBlockStatus ( )
{
    int64 i, llen = List_Length ( _Context_->Interpreter0->PreprocessorStackList ) ;
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

int64
_GetCondStatus ( )
{
    Context * cntx = _Context_ ;
    int64 status ;
    int64 svcm = GetState ( cntx->Compiler0, COMPILE_MODE ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, false ) ;
    _Interpret_ToEndOfLine ( cntx->Interpreter0 ) ;
    SetState ( cntx->Compiler0, COMPILE_MODE, svcm ) ;
    status = _DataStack_Pop ( ) ;
    if ( status > 0 ) status = 1 ;
    else return 0 ; //return status ;
}

int64
GetIfStatus ( )
{
    Ppibs obstatus, cstatus, top ;
    cstatus.int32_Ppibs = 0 ;
    int64 cond = _GetCondStatus ( ) ;
    top.int32_Ppibs = List_Top ( _Context_->Interpreter0->PreprocessorStackList ) ;
    if ( cstatus.ElifStatus = top.ElifStatus )
    {
        if ( top.DoIfStatus )
        {
            cstatus.DoIfStatus = ! cond ;
            cstatus.IfBlockStatus = cond ;
        }
        else cstatus.IfBlockStatus = 0 ;
    }
    else
    {
        cstatus.IfBlockStatus = cond ;
        obstatus.int32_Ppibs = GetOuterBlockStatus ( ) ;
        int64 llen = List_Length ( _Context_->Interpreter0->PreprocessorStackList ) ;
        cstatus.IfBlockStatus = cstatus.IfBlockStatus && (llen ? top.IfBlockStatus : 1 ) && obstatus.IfBlockStatus ; //( llen ? obstatus.DoIfStatus : 1 ) ;
    }
    List_Push ( _Context_->Interpreter0->PreprocessorStackList, cstatus.int32_Ppibs, COMPILER_TEMP ) ;
    return cstatus.IfBlockStatus ;
}

int64
GetElxxStatus ( int64 cond, int64 type )
{
    Ppibs status, obstatus, top ;
    status.int32_Ppibs = 0, obstatus.int32_Ppibs = 0 ;
    top.int32_Ppibs = List_Top ( _Context_->Interpreter0->PreprocessorStackList ) ;
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

int64
GetElifStatus ( )
{
    int64 cond = _GetCondStatus ( ) ;
    return GetElxxStatus ( cond, PP_ELIF ) ;
}

int64
GetElseStatus ( )
{
    return GetElxxStatus ( 1, PP_ELSE ) ; // 
}

int64
GetEndifStatus ( )
{
    Ppibs status ;
    status.int32_Ppibs = GetOuterBlockStatus ( ) ;
    List_Pop ( _Context_->Interpreter0->PreprocessorStackList ) ;
    return status.IfBlockStatus ;
}

void
SkipPreprocessorCode ( )
{
    Context * cntx = _Context_ ;
    Lexer * lexer = cntx->Lexer0 ;
    byte * token ; 
    Lexer_SourceCodeOff ( lexer ) ;
    do
    {
        int64 inChar = ReadLine_PeekNextChar ( cntx->ReadLiner0 ) ;
        if ( ( inChar == - 1 ) || ( inChar == eof ) )
        {
            SetState ( lexer, LEXER_END_OF_LINE, true ) ;
            goto done ;
        }
        token = Lexer_ReadToken ( lexer ) ;
        if ( token )
        {
            if ( String_Equal ( token, "//" ) ) { CfrTil_CommentToEndOfLine ( ) ; Lexer_SourceCodeOff ( lexer ) ; }
            else if ( String_Equal ( token, "/*" ) ) { CfrTil_ParenthesisComment ( ) ; Lexer_SourceCodeOff ( lexer ) ; }
            else if ( String_Equal ( token, "#" ) )
            {
                byte * token1 = Lexer_ReadToken ( lexer ) ;
                if ( token1 )
                {
                    if ( String_Equal ( token1, "if" ) )
                    {
                        if ( GetIfStatus ( ) ) goto done ; // PP_INTERP
                    }
                    else if ( String_Equal ( token1, "else" ) )
                    {
                        if ( GetElseStatus ( ) ) goto done ;
                    }
                    else if ( String_Equal ( token1, "elif" ) )
                    {
                        if ( GetElifStatus ( ) ) goto done ;
                    }
                    else if ( String_Equal ( token1, "endif" ) )
                    {
                        if ( GetEndifStatus ( ) ) goto done ;
                    }
                    //else syntax error
                }
                else goto done ;
            }
        }
    }
    while ( token ) ;
    done :
    Lexer_SourceCodeOn ( lexer ) ;
}

