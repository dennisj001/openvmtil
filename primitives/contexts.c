
#include "../includes/cfrtil.h"

void
CfrTil_Contex_New_RunWord ( )
{
    Word * word = ( Word * ) _DataStack_Pop ( ) ;
    _CfrTil_Contex_NewRun_Void ( _Q_->OVT_CfrTil, word, SESSION ) ;
}
