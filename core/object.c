
#include "../includes/cfrtil.h"

void
Class_Object_Init ( byte * object, Word * word, Namespace * ns )
{
    if ( object )// size : not for "this" or anything else without a size
    {
        Stack * stack = _Q_->OVT_Context->Compiler0->NamespacesStack ;
        Stack_Init ( stack ) ; // !! ?? put this in Compiler ?? !!
        // init needs to be done by the most super class first successively down to the current class 
        do
        {
            Word * initWord ;
            if ( ( initWord = Word_FindInOneNamespace ( ns, ( byte* ) "init" ) ) )
            {
                _Stack_Push ( stack, ( int32 ) initWord ) ;
            }
            ns = ns->ContainingNamespace ;
        }
        while ( ns ) ;
        int32 i ;
        for ( i = Stack_Depth ( stack ) ; i > 0 ; i -- )
        {
            _Push ( ( int32 ) word->W_Object ) ;
            Word * initWord = ( Word* ) _Stack_Pop ( stack ) ;
            _Word_Eval ( initWord ) ; //_Word_Run ( initWord ) ;
        }
        while ( ns ) ;
    }
}




