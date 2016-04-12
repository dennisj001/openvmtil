
#include "../../includes/cfrtil.h"

void
_CfrTil_SetStackPointerFromDsp ( CfrTil * cfrTil )
{
    d0 ( if ( cfrTil->DataStack->StackPointer != Dsp ) 
    {
        Printf ( "\n\nDataStack pointer adjust. DataStack->StackPointer = 0x%08x :: Dsp = 0x%08x\n\n", cfrTil->DataStack->StackPointer, Dsp ) ;
        Pause () ;
    } ) ;
    cfrTil->DataStack->StackPointer = Dsp ;
}

void
CfrTil_SyncStackPointerFromDsp ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack ) _CfrTil_SetStackPointerFromDsp ( cfrTil ) ;
}

void
_CfrTil_SetDspFromStackPointer ( CfrTil * cfrTil )
{
    Dsp = cfrTil->DataStack->StackPointer ;
}

void
CfrTil_SetDspFromStackPointer ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack ) _CfrTil_SetDspFromStackPointer ( cfrTil ) ;
}

void
CfrTil_DataStack_InitEssential ( CfrTil * cfrTil )
{
    Stack * stk = cfrTil->DataStack ;
    _Stack_Init ( stk, _Q_->DataStackSize ) ;
    _CfrTil_SetDspFromStackPointer ( cfrTil ) ;
    cfrTil->SaveDsp = Dsp ;
}

