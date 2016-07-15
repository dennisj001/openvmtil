#include "../include/cfrtil.h"

void
_Repl ( block repl )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;

    byte * snp = rl->NormalPrompt, *sap = rl->AltPrompt ;
    rl->NormalPrompt = ( byte* ) "<= " ;
    rl->AltPrompt = ( byte* ) "=> " ;
    SetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, true ) ;
    start :
    while ( ! setjmp ( _Context_->JmpBuf0 ) )
    {
        while ( 1 )
        {
            Printf ( ( byte* ) "<= " ) ;
            ReadLine_GetLine ( rl ) ;
            if ( strstr ( ( char* ) rl->InputLine, ".." ) || strstr ( (char*) rl->InputLine, "bye") || strstr ( (char*) rl->InputLine, "exit" )) goto done ;
            repl ( ) ;
            Printf ( ( byte* ) "\n" ) ;
        }
    }
    {
        AlertColors ;
        Printf ( ( byte* ) "\n_Repl Error ... continuing" ) ;
        DefaultColors ;
        goto start ;
    }
    done:
    rl->NormalPrompt = snp ;
    rl->AltPrompt = sap ;
    AllowNewlines ;
}

