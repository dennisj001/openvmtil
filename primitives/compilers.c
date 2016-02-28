#include "../includes/cfrtil.h"

void
CfrTil_Here ( )
{
    _DataStack_Push ( ( int32 ) Here ) ;
}

void
CompileCall ( )
{
    Compile_Call ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CompileACfrTilWord ( )
{
    _CompileWord ( ( Word* ) _DataStack_Pop ( ) ) ;
}

void
CompileInt64 ( )
{
    union
    {
        int q0 [2 ] ; 
        long int q ;
    } li ;
    li.q0[1] = _DataStack_Pop ( ) ;
    li.q0[0] = _DataStack_Pop ( ) ; // little endian - low order bits should be pushed first
    _Compile_Int64 ( li.q ) ;

}

void
CompileInt32 ( )
{
    int l = _DataStack_Pop ( ) ;
    _Compile_Int32 ( l ) ;

}

void
CompileInt16 ( )
{
    int32 w = _DataStack_Pop ( ) ;
    _Compile_Int16 ( ( short ) w ) ;

}

void
CompileByte ( )
{
    int32 b = _DataStack_Pop ( ) ;
    _Compile_Int8 ( b ) ;
}

void
CompileN ( )
{
    int32 size = _DataStack_Pop ( ) ;
    byte * data = ( byte* ) _DataStack_Pop ( ) ;
    _CompileN ( data, size ) ;
}

GotoInfo *
_GotoInfo_Allocate ( )
{
    GotoInfo * gi = ( GotoInfo * ) Mem_Allocate ( sizeof ( GotoInfo ), SESSION ) ;
    return gi ;
}

void
GotoInfo_Delete ( DLNode * node )
{
    GotoInfo * gi = ( GotoInfo* ) node ;
    DLNode_Remove ( ( DLNode * ) gi ) ;
}

GotoInfo *
_CfrTil_CompileCallGotoPoint ( uint64 type )
{
    GotoInfo * gotoInfo = ( GotoInfo * ) _GotoInfo_Allocate ( ) ;
    if ( type == GI_RECURSE ) _Compile_UninitializedCall ( ) ;
    else _Compile_UninitializedJump ( ) ;
    gotoInfo->pb_JmpOffsetPointer = Here - 4 ; // after the call opcode
    gotoInfo->GI_CType = type ;
    DLList_AddNodeToTail ( _Q_->OVT_Context->Compiler0->GotoList, ( DLNode* ) gotoInfo ) ;
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
CfrTil_Label ( )
{
    _CfrTil_Label ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_Return ( ) // runtime
{
    //_Compile_ESP_Restore ( ) ;
    //Compiler_SetState ( _Q_->OVT_Context->Compiler0, SAVE_ESP, true ) ;
    _CfrTil_CompileCallGotoPoint ( GI_RETURN ) ;
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

void
CfrTil_Tail ( )
{
    Printf ( ( byte* ) "\nTailCall not implemented yet. Fix me!\n" ) ;
    return ;
    _CfrTil_CompileCallGotoPoint ( GI_TAIL_CALL ) ;
}

#if 1
void
CfrTil_Literal ( )
{
    _DataObject_New ( LITERAL, 0, 0, LITERAL, 0, 0, ( uint32 ) _DataStack_Pop ( ), 0 ) ;
}

void
CfrTil_Constant ( )
{
    int32 value = _DataStack_Pop ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataObject_New ( CONSTANT, 0, name, LITERAL | CONSTANT, 0, 0, value, 0 ) ;
}
#else
void
CfrTil_Literal ( )
{
    Word * word = _DataObject_New ( LITERAL, 0, 0, LITERAL, 0, 0, ( uint32 ) _DataStack_Pop ( ), 0 ) ;
        _DataObject_Run ( word ) ;
}

void
CfrTil_Constant ( )
{
    int32 value = _DataStack_Pop ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    Word * word = _DataObject_New ( CONSTANT, 0, name, LITERAL | CONSTANT, 0, 0, value, 0 ) ;
        _DataObject_Run ( word ) ;
}
#endif

void
CfrTil_Variable ( )
{
    _DataObject_New ( VARIABLE, 0, ( byte* ) _DataStack_Pop ( ), VARIABLE, 0, 0, 0, 0 ) ;
}

// "{|" - exit the Compiler start interpreting
// named after the forth word '[' 
// meaning is reversed from forth which doesn't have blocks

void
CfrTil_LeftBracket ( )
{
    Compiler_SetState ( _Q_->OVT_Context->Compiler0, COMPILE_MODE, false ) ;
}

#if 0
void
CfrTil_CompileModeOn ( )
{
    Compiler_SetState ( _Q_->OVT_Context->Compiler0, COMPILE_MODE, true ) ;
}
#endif

// "|}" - enter the Compiler
// named after the forth word ']'

void
CfrTil_RightBracket ( )
{
    Compiler_SetState ( _Q_->OVT_Context->Compiler0, COMPILE_MODE, true ) ;
}

void
CfrTil_CompileMode ( )
{
    _DataStack_Push ( GetState ( _Q_->OVT_Context->Compiler0, COMPILE_MODE ) ) ;
}
