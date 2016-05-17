#ifndef __CFRTIL_H__
#define __CFRTIL_H__

#define LINUX 1

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>

#if LINUX
#include <termios.h>
//#include <ncurses.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include <errno.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>
#include <assert.h>
#include <asm/unistd.h>
//#include <gc/gc.h>

//#include "/usr/local/include/udis86.h"
#include "udis86.h"
#include "defines.h"
#include "bitfields.h"
#include "types.h"
#include "macros.h"
//#include "/usr/local/include/gmp.h"
#include "gmp.h"

#if MARU || MARU_2_4 || MARU_NILE
typedef void (*GC_finaliser_t)(void *ptr, void *data);
#endif

#if LISP_IO
#define MAXLEN (2 * M)
#if SL5
extern byte * sl_inputBuffer;
extern int sl_inputBufferIndex;
#endif
extern int Maru_RawReadFlag;
#endif

extern OpenVmTil * _Q_;
extern CPrimitive CPrimitives [];
extern MachineCodePrimitive MachineCodePrimitives [];

#include "codegen_x86.h"
#include "machineCode.h"
#include "machineCodeMacros.h"
#include "stacks.h"
#include "prototypes.h"
#endif
