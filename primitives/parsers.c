
#include "../includes/cfrtil.h"

void
_CfrTil_Tick ( )
{
    _Tick ( _Q_->OVT_CfrTil->Context0 ) ;
}

void
CfrTil_Token ( )
{
    _DataStack_Push ( ( int32 ) Lexer_ReadToken ( _Q_->OVT_Context->Lexer0 ) ) ;
}

void
CfrTil_Tick ( )
{
    CfrTil_Token ( ) ;
    _CfrTil_Tick ( ) ;
}

void
CfrTil_CPreProcessor ( )
{
    Finder_SetNamedQualifyingNamespace ( _Q_->OVT_Context->Finder0, "PreProcessor" ) ;
    CfrTil_InterpretNextToken ( ) ;
}

#if 0
void
CfrTil_DoubleQuote ( )
{
    CfrTil_SourceCode_Init ( ) ; // ?? an adhoc to work with ":" ??
    CfrTil_Token ( ) ;
}
#endif
