#include "../include/cfrtil.h"

void
CfrTil_Here ( )
{
    _DataStack_Push ( ( int64 ) Here ) ;
}

void
CompileCall ( )
{
    Compile_Call_With32BitDisp ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CompileACfrTilWord ( )
{
    _Word_Compile ( ( Word* ) _DataStack_Pop ( ) ) ;
}

void
CompileInt64 ( )
{

    union
    {
        int64 q0 [2 ] ;
        int64 q ;
    } li ;
    li.q0[1] = _DataStack_Pop ( ) ;
    li.q0[0] = _DataStack_Pop ( ) ; // little endian - low order bits should be pushed first
    _Compile_Int64 ( li.q ) ;

}

void
CompileInt32 ( )
{
    int64 l = _DataStack_Pop ( ) ;
    _Compile_Int32 ( l ) ;

}

void
CompileInt16 ( )
{
    int64 w = _DataStack_Pop ( ) ;
    _Compile_Int16 ( ( short ) w ) ;

}

void
CompileByte ( )
{
    int64 b = _DataStack_Pop ( ) ;
    _Compile_Int8 ( b ) ;
}

void
CompileN ( )
{
    int64 size = _DataStack_Pop ( ) ;
    byte * data = ( byte* ) _DataStack_Pop ( ) ;
    _CompileN ( data, size ) ;
}

GotoInfo *
_GotoInfo_Allocate ( )
{
    GotoInfo * gi = ( GotoInfo * ) Mem_Allocate ( sizeof ( GotoInfo ), COMPILER_TEMP ) ;
    return gi ;
}

void
GotoInfo_Delete ( dlnode * node )
{
    GotoInfo * gi = ( GotoInfo* ) node ;
    dlnode_Remove ( ( dlnode * ) gi ) ;
}

GotoInfo *
_CfrTil_CompileCallGotoPoint ( uint64 type )
{
    GotoInfo * gotoInfo = ( GotoInfo * ) _GotoInfo_Allocate ( ) ;
    if ( type == GI_RECURSE ) _Compile_UninitializedCall ( ) ;
    else _Compile_UninitializedJump ( ) ;
    gotoInfo->pb_JmpOffsetPointer = Here - 4 ; // after the call opcode
    gotoInfo->GI_CProperty = type ;
    dllist_AddNodeToHead ( _Context_->Compiler0->GotoList, ( dlnode* ) gotoInfo ) ;
    return gotoInfo ;
}

GotoInfo *
GotoInfo_New ( byte * lname )
{
    GotoInfo * gotoInfo = _CfrTil_CompileCallGotoPoint ( GI_GOTO ) ;
    gotoInfo->pb_LabelName = lname ;
    return gotoInfo ;
}

void
_CfrTil_Goto ( byte * lname )
{
    GotoInfo_New ( lname ) ;
}

void
CfrTil_Goto ( ) // runtime
{
    _CfrTil_Goto ( ( byte * ) _DataStack_Pop ( ) ) ; // runtime
}

void
CfrTil_Goto_Prefix ( ) // runtime
{
    byte * gotoToken = Lexer_ReadToken ( _Context_->Lexer0 ) ;
    _CfrTil_Goto ( gotoToken ) ; // runtime
}

void
CfrTil_Label ( )
{
    _CfrTil_Label ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_Label_Prefix ( )
{
    byte * labelToken = Lexer_ReadToken ( _Context_->Lexer0 ) ;
    _CfrTil_Label ( labelToken ) ;
}

void
CfrTil_Return ( )
{
    if ( ! _Readline_Is_AtEndOfBlock ( _Context_->ReadLiner0 ) )
    {
        //SetState ( _Context_->Compiler0, SAVE_ESP, true ) ;
        _CfrTil_CompileCallGotoPoint ( GI_RETURN ) ;
        //byte * token = Lexer_PeekNextNonDebugTokenWord ( _Lexer_, 0 ) ;
        //Word * word = Finder_Word_FindUsing ( _Finder_, token, 0 ) ;
        //_Compiler_->ReturnVariableWord = word ;
    }
    d0 ( else if ( Is_DebugModeOn )
    {
        _OVT_Pause ( "\nCheck \'return\'\n" ) ;
    }
    ) ;
}

void
CfrTil_Continue ( )
{
    _CfrTil_CompileCallGotoPoint ( GI_CONTINUE ) ;
}

void
CfrTil_Break ( )
{
    _CfrTil_CompileCallGotoPoint ( GI_BREAK ) ;
}

void
CfrTil_SetupRecursiveCall ( )
{
    _CfrTil_CompileCallGotoPoint ( GI_RECURSE ) ;
}

#if 0

void
CfrTil_Tail ( )
{
    _Printf ( ( byte* ) "\nTailCall not implemented yet. Fix me!\n" ) ;
    return ;
    _CfrTil_CompileCallGotoPoint ( GI_TAIL_CALL ) ;
}
#endif

void
CfrTil_Literal ( )
{
    Word * word = _DataObject_New ( LITERAL, 0, 0, LITERAL, 0, 0, ( uint64 ) _DataStack_Pop ( ), 0 ) ;
    _Interpreter_DoWord ( _Context_->Interpreter0, word, - 1 ) ;
}

void
CfrTil_Constant ( )
{
    int64 value = _DataStack_Pop ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataObject_New ( CONSTANT, 0, name, LITERAL | CONSTANT, 0, 0, value, 0 ) ;
}

void
CfrTil_Variable ( )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataObject_New ( NAMESPACE_VARIABLE, 0, name, NAMESPACE_VARIABLE, 0, 0, 0, 0 ) ;
}

// "{|" - exit the Compiler start interpreting
// named after the forth word '[' 
// meaning is reversed from forth which doesn't have blocks

void
CfrTil_LeftBracket ( )
{
    SetState ( _Context_->Compiler0, COMPILE_MODE, false ) ;
}

#if 0

void
CfrTil_CompileModeOn ( )
{
    SetState ( _Context_->Compiler0, COMPILE_MODE, true ) ;
}
#endif

// "|}" - enter the Compiler
// named following the forth word ']'

void
CfrTil_RightBracket ( )
{
    SetState ( _Context_->Compiler0, COMPILE_MODE, true ) ;
}

void
CfrTil_CompileMode ( )
{
    _DataStack_Push ( GetState ( _Context_->Compiler0, COMPILE_MODE ) ) ;
}

