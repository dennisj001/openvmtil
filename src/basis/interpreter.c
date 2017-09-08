
#include "../include/cfrtil.h"

void
Interpreter_Init ( Interpreter * interp )
{
    //if ( _Debugger_ ) SetState ( _Debugger_, DBG_AUTO_MODE, false ) ;
    //_Q_->OVT_Interpreter = _Context_->Interpreter0 = interp ;
    _Q_->OVT_Interpreter = interp ;
    interp->State = 0 ;
}

Interpreter *
Interpreter_New ( uint64 type )
{
    Interpreter * interp = ( Interpreter * ) Mem_Allocate ( sizeof (Interpreter ), type ) ;

    interp->Lexer0 = Lexer_New ( type ) ;
    interp->ReadLiner0 = interp->Lexer0->ReadLiner0 ;
    interp->Lexer0->OurInterpreter = interp ;
    interp->Finder0 = Finder_New ( type ) ;
    interp->Compiler0 = Compiler_New ( type ) ;
    interp->PreprocessorStackList = _dllist_New ( type ) ;
    Interpreter_Init ( interp ) ;
    return interp ;
}

void
_Interpreter_Copy ( Interpreter * interp, Interpreter * interp0 )
{
    memcpy ( interp, interp0, sizeof (Interpreter ) ) ;
}

Interpreter *
Interpreter_Copy ( Interpreter * interp0, uint64 type )
{
    Interpreter * interp = ( Interpreter * ) Mem_Allocate ( sizeof (Interpreter ), type ) ;
    _Interpreter_Copy ( interp, interp0 ) ;
    Interpreter_Init ( interp ) ;
    return interp ;
}

int64
Interpreter_IsDone ( Interpreter * interp, int64 flags )
{
    return GetState ( interp, flags | INTERPRETER_DONE ) ;
}

