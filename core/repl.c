#include "../includes/cfrtil.h"

void
_Repl ( block repl )
{
    ReadLiner * rl = _Q_->OVT_Context->ReadLiner0 ;

    byte * snp = rl->NormalPrompt, *sap = rl->AltPrompt ;
    rl->NormalPrompt = ( byte* ) "<= " ;
    rl->AltPrompt = ( byte* ) "=> " ;
    while ( 1 )
    {
        if ( ! setjmp ( _Q_->OVT_Context->JmpBuf0 ) )
        {
            Printf ( ( byte* ) "<= " ) ;
            ReadLine_GetLine ( rl ) ;
            if ( strstr ( ( char* ) rl->InputLine, ";;" ) ) break ;
            repl ( ) ;
            Printf ( ( byte* ) "\n" ) ;
            continue ;
        }
        else
        {
            AlertColors ;
            Printf ( ( byte* ) "\nError" ) ;
            DefaultColors ;
            continue ;
        } ;
    }
    rl->NormalPrompt = snp ;
    rl->AltPrompt = sap ;
    AllowNewlines ;
}

