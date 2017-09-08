#include "../../include/cfrtil.h"

CaseNode *
_CaseNode_New ( uint64 type, block block, int64 value )
{
    CaseNode * cnode = ( CaseNode* ) Mem_Allocate ( sizeof ( CaseNode ), type ) ;
    cnode->CaseBlock = block ;
    cnode->CN_CaseValue = (byte*) value ;
    return cnode ;
}

// ( q n -- )

void
_CfrTil_Case ( uint64 allocType )
{
    block caseBlock ;
    int64 caseValue ;
    if ( CompileMode )
    {
        caseBlock = ( block ) TOS ;
        Word * literalWord = WordsBack ( 1 ) ;
        if ( ! ( literalWord->CProperty & LITERAL ) ) CfrTil_Exception ( CASE_NOT_LITERAL_ERROR, 1 ) ;
        caseValue = ( int64 ) literalWord->W_Value ;
        SetHere ( literalWord->Coding ) ;
        DSP_DropN ( 1 ) ;
        //Dsp -- ;
    }
    else
    {
        caseBlock = ( block ) Dsp [ - 1 ] ;
        caseValue = TOS ;
        DSP_DropN ( 2 ) ;
        //Dsp -= 2 ;
    }
    if ( ! _Context_->Compiler0->CurrentSwitchList )
    {
        _Context_->Compiler0->CurrentSwitchList = _dllist_New ( allocType ) ;
    }
    CaseNode * cnode = _CaseNode_New ( allocType, caseBlock, caseValue ) ;
    dllist_AddNodeToTail ( _Context_->Compiler0->CurrentSwitchList, (dlnode*) cnode ) ;
}

void
CfrTil_Case ( )
{
    _CfrTil_Case ( DICTIONARY ) ;
}

void
Switch_MapFunction ( dlnode * node, uint64 switchValue )
{
    CaseNode * cnode = ( CaseNode* ) node ;
    if ( cnode->CN_CaseValue == (byte*) switchValue ) cnode->CaseBlock ( ) ;
}

void
SwitchAccessFunction ( )
{
    //dllist * list = ( dllist * ) _Pop ( ) ;
    //cell switchValue = _Pop ( ) ;
    //dllist_Map1 ( list, Switch_MapFunction, switchValue ) ;
    dllist_Map1 ( ( dllist* ) TOS, (MapFunction1) Switch_MapFunction, Dsp [ - 1 ] ) ;
    DSP_DropN ( 2 ) ;
}

void
CfrTil_Switch ( )
{
    if ( CompileMode )
    {
        // try to build table
        // setup SwitchAccessFunction 
        // call SwitchAccessFunction 
        //_Compile_PushLiteral ( ( int64 ) _Context_->Compiler0->CurrentSwitchList ) ;
        _Do_Literal ( ( int64 ) _Context_->Compiler0->CurrentSwitchList ) ;
        Compile_Call_With32BitDisp ( ( byte* ) SwitchAccessFunction ) ;
    }
    else
    {
        //cell switchValue = TOS ;
        //_CfrTil_DropN ( 1 ) ;
        //dllist_Map1 ( _Context->Compiler0->CurrentSwitchList, Switch_MapFunction, switchValue ) ;
        dllist_Map1 ( _Context_->Compiler0->CurrentSwitchList, (MapFunction1) Switch_MapFunction, TOS ) ;
        DSP_DropN ( 1 ) ;
    }
    _Context_->Compiler0->CurrentSwitchList = 0 ; // this allows no further "case"s to be added to this "switch" list a new list will be started with the next "case"
}

