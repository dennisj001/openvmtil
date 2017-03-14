#include "../include/cfrtil.h"

void
_Repl ( block repl )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;

    byte * snp = rl->NormalPrompt, *sap = rl->AltPrompt ;
    rl->NormalPrompt = ( byte* ) "<= " ;
    rl->AltPrompt = ( byte* ) "=> " ;
    //SetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, true ) ;
    SetState ( _Context_->System0, ADD_READLINE_TO_HISTORY, true ) ;
    start:
    while ( ! setjmp ( _Context_->JmpBuf0 ) )
    {
        while ( 1 )
        {
            _Printf ( ( byte* ) "<= " ) ;
            //LC_SaveStack ( ) ; // ?!? maybe we should do this stuff differently : literals are pushed on the stack by the interpreter
            ReadLine_GetLine ( rl ) ;
            if ( strstr ( ( char* ) rl->InputLine, ".." ) || strstr ( ( char* ) rl->InputLine, "bye" ) || strstr ( ( char* ) rl->InputLine, "exit" ) ) goto done ;
            repl ( ) ;
            _Printf ( ( byte* ) "\n" ) ;
            //LC_RestoreStack ( ) ; // ?!? maybe we should do this stuff differently : literals are pushed on the stack by the interpreter
       }
    }
    {
        AlertColors ;
        _Printf ( ( byte* ) "\n_Repl Error ... continuing" ) ;
        DefaultColors ;
        goto start ;
    }
done:
    rl->NormalPrompt = snp ;
    rl->AltPrompt = sap ;
    //AllowNewlines ;
}

