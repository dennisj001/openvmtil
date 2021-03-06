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
#include <asm/unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#if 0
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
//#include "/usr/include/x86_64-linux-gnu/asm/unistd.h"
//#include <arch/ia64/asm/thread_info.h>
#endif
#endif
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include <errno.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>
#include <assert.h>
//#include <gc/gc.h>

//#include "/usr/local/include/udis86.h"
#include "udis86.h"
#include "gmp.h"
#include <mpfr.h>
#include "defines.h"
#include "bitfields.h"
#include "types.h"
#include "macros.h"
//#include "/usr/local/include/gmp.h"

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
extern int32 *_Dsp_ ;
extern CPrimitive CPrimitives [];
extern MachineCodePrimitive MachineCodePrimitives [];
extern uint64 mmap_TotalMemAllocated, mmap_TotalMemFreed ;
typedef int32 ( *mpf2andOutFunc) (mpfr_ptr, mpfr_srcptr, mpfr_srcptr, mpfr_rnd_t) ;

//#include "codegen_x86.h" // i want to make sure i have this - not using much now but probably later on
#include "machineCode.h"
#include "machineCodeMacros.h"
#include "stacks.h"
#include "prototypes.h"
#endif
