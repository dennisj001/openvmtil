
#include "../include/cfrtil32.h"



#if 0

void
_Compiler_PropertyCheck1 ( Compiler * compiler, uint32 type32 )
{
    //return ;
    Word * word = ( Word* ) _Stack_Pop ( compiler->TypeStack ) ;
    if ( ! ( word->S_Category & type32 ) ) Error_Abort ( ( byte* ) "CProperty Error", 1 ) ;
}

void
_Compiler_PropertyCheck2 ( Compiler * compiler, uint64 type0, uint64 type1 )
{
    //return ;
    Word * word = ( Word* ) _Stack_Pop ( compiler->TypeStack ) ;
    if ( ! ( word->S_Category & type0 ) ) Error_Abort ( ( byte* ) "CProperty Error", 1 ) ;
    if ( ! ( word->S_Category & type1 ) ) Error_Abort ( ( byte* ) "CProperty Error", 1 ) ;
}
int32
_PropertyCheck1 ( uint64 zero )
{
    //if ( GetState( _Context_->Compiler0, BLOCK_MODE ) )
    {
        if ( ( Compiler_WordStack ( 0 )->S_Category & zero ) ) return true ;
        else Error_Abort ( "", ABORT ) ;
        return false ;
    }
    //else return true ;
}

int32
_PropertyCheck2 ( uint64 zero, uint64 one )
{
    //if ( GetState( _Context_->Compiler0, BLOCK_MODE ) )
    {
        if ( ( Compiler_WordStack ( 0 )->S_Category & zero ) && ( Compiler_WordStack ( 1 )->S_Category & one ) ) return true ;
        else Error_Abort ( "\nTypeError", ABORT ) ;
        return false ;
    }
    //else return true ;
}
#endif

