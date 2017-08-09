#include "../include/cfrtil.h"

#if 0
// example from : http://www.kernel.org/doc/man-pages/online/pages/man3/dlsym.3.html

Load the math library, and _print the cosine of 2.0 :

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int
main ( int argc, char **argv )
{
    void *handle ;
    double (*cosine )( double ) ;
    char *error ;

    handle = dlopen ( "libm.so", RTLD_LAZY ) ;
    if ( ! handle )
    {
        fprintf ( stderr, "%s\n", dlerror ( ) ) ;
        exit ( EXIT_FAILURE ) ;
    }

    dlerror ( ) ; /* Clear any existing error */

    /* Writing: cosine = (double (*)(double)) dlsym(handle, "cos");
       would seem more natural, but the C99 standard leaves
       casting from "void *" to a function pointer undefined.
       The assignment used below is the POSIX.1-2003 (Technical
       Corrigendum 1) workaround; see the Rationale for the
       POSIX specification of dlsym(). */

    *( void ** ) ( &cosine ) = dlsym ( handle, "cos" ) ;

    if ( ( error = dlerror ( ) ) != NULL )
    {
        fprintf ( stderr, "%s\n", error ) ;
        exit ( EXIT_FAILURE ) ;
    }

    printf ( "%f\n", ( *cosine )( 2.0 ) ) ;
    dlclose ( handle ) ;
    exit ( EXIT_SUCCESS ) ;
}
/*
       If this program were in a file named "foo.c", you would build the program with
       the following command:

           gcc -rdynamic -o foo foo.c -ldl

       Libraries exporting _init() and _fini() will want to be compiled as follows,
       using bar.c as the example name:

           gcc -shared -nostartfiles -o bar bar.c
 */

#endif

// lib : full library path

#define RTLD_DEFAULT ((void *) 0)

void *
_dlsym ( byte * sym, byte * lib )
{
    void *fp, * hLibrary = dlopen ( ( char* ) lib, RTLD_GLOBAL | RTLD_LAZY ) ;
    if ( hLibrary )
    {
        //fp = ( void* ) dlsym ( hLibrary, ( char* ) sym ) ;
        fp = ( void* ) dlsym ( RTLD_DEFAULT, ( char* ) sym ) ; // either work after dlopen
    }
    if ( ( ! hLibrary ) || ( ! fp ) )
    {
        //_Printf ( ( byte* ) c_ad ( "\ndlsym : dlerror = %s\n" ), dlerror () ) ;
        return 0 ;
    }
    return fp ;
}

void *
_Dlsym ( byte * sym, byte * lib )
{
    void *functionPointer = _dlsym ( sym, lib ) ;
    if ( ( ! functionPointer ) )
    {
        char buffer [256], *sharedLib = ( char* ) lib ;
        int32 ll ;
        for ( ll = Strlen ( sharedLib ) ; sharedLib [ ll ] != '/' ; ll -- ) ;
        strcpy ( buffer, "./lib" ) ;
        strcat ( buffer, &sharedLib [ll] ) ;
        functionPointer = _dlsym ( sym, ( byte* ) buffer ) ;
        if ( ! functionPointer )
        {
            _Printf ( ( byte* ) c_ad ( "\ndlsym : dlerror = %s\n" ), dlerror ( ) ) ;
            return 0 ;
        }
    }
    return functionPointer ;
}

void
Dlsym ( byte * sym, byte * lib )
{
    block b = ( block ) _Dlsym ( sym, lib ) ;
    Word * word = _DataObject_New ( CFRTIL_WORD, 0, sym, DLSYM_WORD | C_PREFIX | C_RETURN | C_PREFIX_RTL_ARGS, 0, 0, ( int32 ) b, 0 ) ;
    word->WProperty |= WT_C_PREFIX_RTL_ARGS ;
}

// lib sym | addr
#if 0
void
_CfrTil_Dlsym ( )
{
    byte * sym = ( byte* ) _DataStack_Pop ( ) ;
    byte * lib = ( byte* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int ) _Dlsym ( sym, lib ) ) ;
}

void
CfrTil_DlsymWord ( )
{
    byte * lib = ( byte* ) _DataStack_Pop ( ) ;
    byte * sym = ( byte* ) _DataStack_Pop ( ) ;
    Dlsym ( sym, lib ) ;
}
#endif
// takes semi - ";" - after the definition

void
CfrTil_Dlsym ( )
{
    byte * sym = Lexer_ReadToken ( _Context_->Lexer0 ) ;
    byte * lib = _Lexer_LexNextToken_WithDelimiters ( _Context_->Lexer0, 0, 1, LEXER_ALLOW_DOT ) ;
    byte * semi = Lexer_ReadToken ( _Context_->Lexer0 ) ; // drop the semi
    Dlsym ( sym, lib ) ;
}

// callNumber | errno

void
CfrTil_system0 ( )
{
    _Compile_Stack_PopToReg ( DSP, EAX ) ;
    _Compile_INT80 ( ) ;
    _Compile_Stack_PushReg ( DSP, EAX ) ;
}

void
CfrTil_system1 ( )
{
    _Compile_Stack_PopToReg ( DSP, EAX ) ;
    _Compile_Stack_PopToReg ( DSP, EBX ) ;
    _Compile_INT80 ( ) ;
    _Compile_Stack_PushReg ( DSP, EAX ) ;
}

void
CfrTil_system2 ( )
{
    _Compile_Stack_PopToReg ( DSP, EAX ) ;
    _Compile_Stack_PopToReg ( DSP, EBX ) ;
    _Compile_Stack_PopToReg ( DSP, ECX ) ;
    _Compile_INT80 ( ) ;
    _Compile_Stack_PushReg ( DSP, EAX ) ;
}

void
CfrTil_system3 ( )
{
    _Compile_Stack_PopToReg ( DSP, EAX ) ;
    _Compile_Stack_PopToReg ( DSP, EBX ) ;
    _Compile_Stack_PopToReg ( DSP, ECX ) ;
    _Compile_Stack_PopToReg ( DSP, EDX ) ;
    _Compile_INT80 ( ) ;
    _Compile_Stack_PushReg ( DSP, EAX ) ;
}

#if 0

void *
dlOpen_Dlsym ( char * lib, char * sym )
{
    void * hLibrary, *fp ;
    char * error, buffer [1024] ;

    sprintf ( buffer, "./%s.so", lib ) ;
    hLibrary = dlopen ( buffer, RTLD_GLOBAL | RTLD_LAZY ) ;
    if ( ! hLibrary )
    {
        sprintf ( buffer, "/usr/lib32/%s.so", lib ) ;
        hLibrary = dlopen ( buffer, RTLD_GLOBAL | RTLD_LAZY ) ;
    }
    if ( ! hLibrary )
    {
        sprintf ( buffer, "/usr/local/lib/%s.so", lib ) ;
        hLibrary = dlopen ( buffer, RTLD_GLOBAL | RTLD_LAZY ) ;
    }
    if ( ! hLibrary )
    {
        sprintf ( buffer, "/usr/lib/%s.so", lib ) ;
        hLibrary = dlopen ( buffer, RTLD_GLOBAL | RTLD_LAZY ) ;
    }
    if ( ! hLibrary )
    {
        _Printf ( ( byte* ) "\nCannot open %s - cannot import library\n", buffer ) ;
        return 0 ;
    }
    fp = ( void* ) dlsym ( RTLD_DEFAULT /*hLibrary*/, ( char* ) sym ) ;
    //if ( ( error = dlerror ( ) ) != NULL )
    if ( ( ! fp ) || ( ( error = dlerror ( ) ) != NULL ) )
    {
        _Printf ( ( byte* ) "dlOpen_Dlsym : dlerror: %s\n", error ) ;
        return 0 ;
    }

    //void * hLibrary = dlopen ( lib, RTLD_DEFAULT |RTLD_GLOBAL | RTLD_LAZY ) ;
    void * fp = _Dlsym ( lib, sym ) ;

    return fp ;
}
#endif

void
_CfrTil_WordAccounting_Print ( byte * functionName )
{
    _Printf ( (byte*) "\n%s :: DObjectCreateCount = %d : WordCreateCount = %d : WordsAdded = %d : FindWordCount = %d : FindWordMaxCount = %d", 
        functionName, _CfrTil_->DObjectCreateCount, _CfrTil_->WordCreateCount, _CfrTil_->WordsAdded, _CfrTil_->FindWordCount, _CfrTil_->FindWordMaxCount ) ;
}

void
CfrTil_WordAccounting ( byte * functionName )
{
    if ( _CfrTil_->FindWordCount > _CfrTil_->FindWordMaxCount ) _CfrTil_->FindWordMaxCount = _CfrTil_->FindWordCount ;
    if ( _Q_->Verbosity > 3 ) _CfrTil_WordAccounting_Print ( functionName ) ;
}

byte *
_CfrTil_GetSystemState_String0 ( byte * buf )
{
    strcpy ( ( char* ) buf, "\noptimization is " ) ;
    if ( GetState ( _CfrTil_, OPTIMIZE_ON ) ) strcat ( ( char* ) buf, "on, " ) ;
    else strcat ( ( char* ) buf, "off, " ) ;
    strcat ( ( char* ) buf, "inlining is " ) ;
    if ( GetState ( _CfrTil_, INLINE_ON ) ) strcat ( ( char* ) buf, "on, " ) ;
    else strcat ( ( char* ) buf, "off, " ) ;
    strcat ( ( char* ) buf, "infixMode is " ) ;
    if ( GetState ( _Context_, INFIX_MODE ) ) strcat ( ( char* ) buf, "on, " ) ;
    else strcat ( ( char* ) buf, "off, " ) ;
    strcat ( ( char* ) buf, "prefixMode is " ) ;
    if ( GetState ( _Context_, PREFIX_MODE ) ) strcat ( ( char* ) buf, "on, " ) ;
    else strcat ( ( char* ) buf, "off, " ) ;
    strcat ( ( char* ) buf, "c_syntax is " ) ;
    if ( GetState ( _Context_, C_SYNTAX ) ) strcat ( ( char* ) buf, "on, " ) ;
    else strcat ( ( char* ) buf, "off, " ) ;
#if 0    
    strcat ( ( char* ) buf, "LHS is " ) ;
    if ( GetState ( _Context_, C_LHS ) ) strcat ( ( char* ) buf, "on, " ) ;
    else strcat ( ( char* ) buf, "off, " ) ;
    strcat ( ( char* ) buf, "RHS is " ) ;
    if ( GetState ( _Context_, C_RHS ) ) strcat ( ( char* ) buf, "on. " ) ;
    else strcat ( ( char* ) buf, "off. " ) ;
#endif    
    return buf ;
}

byte *
_CfrTil_GetSystemState_String1 ( byte *buf )
{
    strcat ( ( char* ) buf, "\nReadLine echo is " ) ;
    if ( GetState ( _CfrTil_, READLINE_ECHO_ON ) ) strcat ( ( char* ) buf, "on. " ) ;
    else strcat ( ( char* ) buf, "off. " ) ;
    strcpy ( ( char* ) buf, "\nDebug is " ) ;
    if ( GetState ( _CfrTil_, DEBUG_MODE ) ) strcat ( ( char* ) buf, "on. " ) ;
    else strcat ( ( char* ) buf, "off. " ) ;
    sprintf ( ( char* ) &buf[Strlen ( ( char* ) buf )], "Verbosity = %d. ", _Q_->Verbosity ) ;
    sprintf ( ( char* ) &buf[Strlen ( ( char* ) buf )], "Console = %d.", _Q_->Console ) ;
    return buf ;
}

void
_CfrTil_SystemState_Print ( int32 pflag )
{
    byte * buf = Buffer_Data ( _CfrTil_->ScratchB1 ) ;
    buf = _CfrTil_GetSystemState_String0 ( buf ) ;
    _Printf ( ( byte* ) buf ) ;
    buf = _CfrTil_GetSystemState_String1 ( buf ) ;
    _Printf ( ( byte* ) buf ) ;
    if ( pflag ) OpenVmTil_Print_DataSizeofInfo ( pflag ) ;
    _CfrTil_WordAccounting_Print ( (byte*) "_CfrTil_SystemState_Print" ) ;
}

void
__CfrTil_Dump ( int32 address, int32 number, int32 dumpMod )
{
    if ( address && number )
    {
        byte * nformat ;
        int32 i, n ;
        if ( _Context_->System0->NumberBase == 16 ) nformat = ( byte* ) "\nDump : Address = " UINT_FRMT_0x08 " : Number = " UINT_FRMT " : (little endian)" ;
        else nformat = ( byte* ) "\nDump : Address = " UINT_FRMT_0x08 " : Number = " INT_FRMT " - (little endian)" ;
        _Printf ( nformat, ( int32 ) address, number ) ;
        for ( i = 0 ; i < number ; )
        {
            _Printf ( ( byte* ) "\n" UINT_FRMT_0x08 " : ", address + i ) ;
            if ( ! ( i % dumpMod ) )
            {
                for ( n = 0 ; n < dumpMod ; n += CELL_SIZE )
                {
                    _Printf ( ( byte* ) UINT_FRMT_08 " ", *( int32* ) ( address + i + n ) ) ;
                }
                _Printf ( ( byte* ) " " ) ;
                for ( n = 0 ; n < dumpMod ; n += CELL_SIZE )
                {
                    CfrTil_NByteDump ( ( byte* ) ( address + i + n ), CELL_SIZE ) ;
                }
                for ( n = 0 ; n < dumpMod ; n += CELL_SIZE )
                {
                    CfrTil_CharacterDump ( ( byte* ) ( address + i + n ), CELL_SIZE ) ;
                }
                i += dumpMod ;
            }

            else i ++ ;
        }
    }
}

void
_CfrTil_Source ( Word *word, int32 addToHistoryFlag )
{
    if ( word )
    {
        byte * name = c_dd ( word->Name ) ;
        uint64 category = word->CProperty ;
        if ( word->ContainingNamespace ) _Printf ( ( byte* ) "\n%s.", word->ContainingNamespace->Name ) ;
        if ( category & OBJECT )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "object" ) ;
        }
        else if ( category & NAMESPACE )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "namespace" ) ;
        }
        else if ( category & TEXT_MACRO )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "macro" ) ;
        }
        else if ( category & LOCAL_VARIABLE )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "local variable" ) ;
        }
        else if ( category & PARAMETER_VARIABLE )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "stack variable" ) ;
        }
        else if ( category & NAMESPACE_VARIABLE )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "variable" ) ;
        }
        else if ( category & CONSTANT )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "constant" ) ;
        }
        else if ( category & ALIAS )
        {
            Word * aword = Word_GetFromCodeAddress ( ( byte* ) ( block ) word->Definition ) ;
            if ( aword ) _Printf ( ( byte* ) "%s alias for %s", name, ( char* ) c_dd ( aword->Name ) ) ;
        }
        else if ( category & CPRIMITIVE )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "C compiled primitive" ) ;
        }
        else if ( word->LProperty & T_LISP_COMPILED_WORD )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "lambdaCalculus compiled word" ) ;
        }
        else if ( category & CFRTIL_WORD )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "cfrTil compiled word" ) ;
        }
        else if ( word->LProperty & T_LISP_DEFINE )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "lambdaCalculus defined word" ) ;
        }
        else if ( category & BLOCK )
        {
            _Printf ( ( byte* ) "%s <:> %s", name, "cfrTil compiled code block" ) ;
        }
        // else CfrTil_Exception ( 0, QUIT ) ;
        if ( category & INLINE ) _Printf ( ( byte* ) ", %s", "inline" ) ;
        if ( category & IMMEDIATE ) _Printf ( ( byte* ) ", %s", "immediate" ) ;
        if ( word->WProperty & WT_PREFIX ) _Printf ( ( byte* ) ", %s", "prefix" ) ;
        if ( category & C_PREFIX ) _Printf ( ( byte* ) ", %s", "c_prefix" ) ;
        if ( category & C_RETURN ) _Printf ( ( byte* ) ", %s", "c_return" ) ;
        if ( category & INFIXABLE ) _Printf ( ( byte* ) ", %s", "infixable" ) ;
        if ( word->S_WordData )
        {
            __Word_ShowSourceCode ( word ) ; // source code has newlines for multiline history
            if ( addToHistoryFlag ) _OpenVmTil_AddStringToHistoryList ( word->SourceCode ) ;
            if ( word->S_WordData->Filename ) _Printf ( ( byte* ) "\nSource code file location of %s : \"%s\" : %d.%d :: called at : %s", name, word->S_WordData->Filename, word->S_WordData->LineNumber, word->W_CursorPosition, Context_IsInFile ( _Context_ ) ? Context_Location ( ) : (byte*) "" ) ;
            if ( ( word->LProperty & T_LISP_DEFINE ) && ( ! ( word->LProperty & T_LISP_COMPILED_WORD ) ) ) _Printf ( ( byte* ) "\nLambda Calculus word : interpreted not compiled" ) ; // do nothing here
            else if ( ! ( category & CPRIMITIVE ) ) _Printf ( ( byte* ) "\nCompiled with : %s%s%s%s", GetState ( word, COMPILED_OPTIMIZED ) ? "optimizeOn" : "optimizeOff", GetState ( word, COMPILED_INLINE ) ? ", inlineOn" : ", inlineOff",
                GetState ( word, W_C_SYNTAX ) ? ", c_syntaxOn" : "", GetState ( word, W_INFIX_MODE ) ? ", infixOn" : "" ) ;
            if ( GetState ( word, W_SOURCE_CODE_MODE ) ) _Printf ( ( byte* ) "%s", ", sourceCodeOn" ) ;
            if ( word->Definition && word->S_CodeSize ) _Printf ( ( byte* ) " -- starting at address : 0x%x -- code size = %d bytes", word->Definition, word->S_CodeSize ) ;
            //else _Printf ( ( byte* ) " -- starting at address : 0x%x", word->Definition ) ;
        }
        //_Printf ( ( byte* ) "\n" ) ;
    }
}

void
_CfrTil_Dump ( int32 dumpMod )
{
    int32 number = _DataStack_Pop ( ) ;
    int32 address = _DataStack_Pop ( ) ;
    __CfrTil_Dump ( address, number, dumpMod ) ;
}

