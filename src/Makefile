
SOURCES = basis/compiler/machineCode.c basis/compiler/_compile.c basis/compiler/memory.c\
	basis/compiler/combinators.c basis/compiler/math.c basis/compiler/cpu.c \
	basis/compiler/stack.c basis/compiler/sequence.c basis/compiler/logic.c basis/core/dataObjectRun.c\
	basis/core/conditionals.c basis/compiler/blocks.c basis/core/compile.c basis/core/_system.c\
	basis/compiler/optimize.c basis/compiler/bits.c basis/compiler/udis.c basis/compiler/arrays.c \
	basis/core/io.c basis/core/symbol.c basis/repl.c basis/core/syntax.c basis/core/dataObjectNew.c\
        basis/cfrtil.c basis/core/parse.c basis/memSpace.c basis/init.c basis/system.c basis/core/charSet.c\
	basis/core/dllist.c basis/core/interpret.c basis/core/lexer.c basis/core/cstack.c basis/core/classes.c basis/debugOutput.c\
	basis/core/namespace.c basis/history.c basis/core/readline.c basis/core/dataStack.c basis/context.c\
	basis/core/word.c basis/core/readTable.c basis/bigNum.c basis/core/_readline.c basis/core/array.c\
	basis/core/compiler.c basis/core/dllnodes.c basis/core/finder.c basis/core/interpreters.c\
	basis/tabCompletion.c basis/colors.c basis/sourceCode.c basis/debugStepping.c\
	basis/core/string.c basis/openVmTil.c basis/core/dobject.c basis/core/memory.c basis/core/preprocessor.c\
	basis/property.c basis/lists.c basis/debugDisassembly.c\
	basis/linux.c basis/exception.c basis/types.c basis/compiler/locals.c\
	basis/debugger.c basis/interpreter.c basis/_debug.c basis/lambdaCalculus.c \
	primitives/strings.c primitives/bits.c primitives/maths.c primitives/logics.c primitives/openvmtils.c\
	primitives/ios.c primitives/parsers.c primitives/interpreters.c primitives/namespaces.c primitives/systems.c\
	primitives/compilers.c primitives/words.c  primitives/file.c primitives/stacks.c\
	primitives/debuggers.c primitives/memory.c primitives/primitives.c primitives/contexts.c\
	primitives/disassembler.c primitives/syntaxes.c primitives/cmaths.c primitives/dataObjectNews.c

INCLUDES = include/machineCode.h include/defines.h include/types.h \
	include/cfrtil.h include/macros.h include/bitfields.h \
	include/machineCodeMacros.h include/stacks.h #include/gc.h

OBJECTS = $(SOURCES:%.c=%.o) 
CC = gcc #-5
OUT = cfrtil-gdb

default : debug

debug : cfrtil-gdb #cfrtils
run : cfrtil
all: cfrtil cfrtil-gdb #cfrtils

#CFLAGS_CORE = -m32 -march=core2 -finline-functions #-I/usr/include/x86_64-linux-gnu -I/usr/src/linux -I/usr/src/linux-headers-4.2.0-42-generic/arch/ia64/include -I/usr/include/x86_64-linux-gnu# -O3 -fomit-frame-pointer
CFLAGS_CORE = -m32 -finline-functions #-I/usr/include/x86_64-linux-gnu -I/usr/src/linux -I/usr/src/linux-headers-4.2.0-42-generic/arch/ia64/include -I/usr/include/x86_64-linux-gnu# -O3 -fomit-frame-pointer
CFLAGS = $(CFLAGS_CORE) -Wall 
LIBS = -L./lib -ludis86 -lgmp -lrt -lc -ldl -lm 
#LIBS = -ludis86 -lgmp -lrt -lc -ldl -lm #-lffi -lgc

oclean : 
	-rm basis/*.o primitives/*.o basis/compiler/*.o basis/core/*.o 
	
_clean : 
	make oclean
	-rm cfrtil cfrtil-gdb ../bin/cfrtil*

clean : 
	make _clean
	touch include/defines.h
	make include/prototypes.h

include/prototypes.h : $(INCLUDES)
	cp include/_proto.h include/prototypes.h
	cproto -o proto.h $(SOURCES)
	mv proto.h include/prototypes.h
	make _clean

cfrtil : CFLAGS = $(CFLAGS_CORE)
cfrtil : include/prototypes.h $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o cfrtil $(LIBS)
	strip cfrtil
	cp cfrtil ../bin/
	
cfrtils : CFLAGS = $(CFLAGS_CORE)
cfrtils : include/prototypes.h $(OBJECTS)
	$(CC) -static $(CFLAGS) $(OBJECTS) -o cfrtils $(LIBS)
	strip cfrtils
	cp cfrtils ../bin/
	
cfrtil-gdb : CFLAGS = $(CFLAGS_CORE) -ggdb 
cfrtil-gdb : include/prototypes.h $(OBJECTS) 
	$(CC) $(CFLAGS) $(OBJECTS) -o cfrtil-gdb $(LIBS)
	strip -o cfrtil cfrtil-gdb
	cp cfrtil ../bin

cfrtilo : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(OUT) $(LIBS)
	strip $(OUT)

_cfrtil_O1 : CFLAGS = $(CFLAGS_CORE) -O1
_cfrtil_O1 : OUT = cfrtilo1
_cfrtil_O1 : cfrtilo

_cfrtil_O2 : CFLAGS = $(CFLAGS_CORE) -O2
_cfrtil_O2 : OUT = cfrtilo2
_cfrtil_O2 : cfrtilo

_cfrtil_O3 : CFLAGS = $(CFLAGS_CORE) -O3
_cfrtil_O3 : OUT = cfrtilo3
_cfrtil_O3 : cfrtilo

_cfrtilo :  include/prototypes.h $(OBJECTS)
	$(CC) $(CFLAGS) basis/compiler/*.o basis/*.o primitives/*.o -o $(OUT) $(LIBS)

primitives/cmaths.o : primitives/cmaths.c
	$(CC) $(CFLAGS) -O3 -c primitives/cmaths.c -o primitives/cmaths.o
	
#primitives/maths.o : primitives/maths.c
#	$(CC) $(CFLAGS) -O3 -c primitives/maths.c -o primitives/maths.o
	
#primitives/systems.o : primitives/systems.c
#	$(CC) $(CFLAGS) -O3 -c primitives/systems.c -o primitives/systems.o
	
#basis/core/readline.o : basis/core/readline.o
#	$(CC) $(CFLAGS) -O3 -c basis/core/readline.c -o basis/core/readline.o
	
primitives/sl5.o : primitives/sl5.c
	$(CC) $(CFLAGS_CORE) -ggdb -w -fpermissive -c primitives/sl5.c -o primitives/sl5.o
	
primitives/boot-eval.o : primitives/boot-eval.c
	$(CC) $(CFLAGS_CORE) -ggdb -w -fpermissive -c primitives/boot-eval.c -o primitives/boot-eval.o
	
primitives/eval.o : primitives/eval.c
	$(CC) $(CFLAGS_CORE) -ggdb -w -fpermissive -c primitives/eval.c -o primitives/eval.o
	
primitives/eval1.o : primitives/eval1.c
	$(CC) $(CFLAGS_CORE) -ggdb -w -fpermissive -c primitives/eval1.c -o primitives/eval1.o
	
primitives/eval2.o : primitives/eval2.c
	$(CC) $(CFLAGS_CORE) -ggdb -w -fpermissive -c primitives/eval2.c -o primitives/eval2.o
	
primitives/wcs.o : primitives/wcs.c
	$(CC) $(CFLAGS_CORE) -w -fpermissive -c primitives/wcs.c -o primitives/wcs.o
	
primitives/libgc.o : primitives/libgc.c
	$(CC) $(CFLAGS_CORE) -w -fpermissive -c primitives/libgc.c -o primitives/libgc.o
	
primitives/gc.o : primitives/gc.c
	$(CC) $(CFLAGS) -w -fpermissive -c primitives/gc.c -o primitives/gc.o
	
tags :
	ctags -R --c++-types=+px --excmd=pattern --exclude=Makefile --exclude=. /home/dennisj/projects/workspace/cfrtil

proto:
	touch include/defines.h
	make include/prototypes.h

optimize1 : oclean _cfrtil_O1

optimize2 : oclean _cfrtil_O2

optimize3 : oclean _cfrtil_O3

optimize :
	make optimize1
	make optimize2
	make optimize3
	make clean
	make
	-rm bin/cfrtilo*
	mv cfrtil cfrtil* bin

editorClean :
	rm *.*~ basis/*.*~ basis/compiler/*.*~ primitives/*.*~ include/*.*~

realClean : _clean editorClean
	rm cfrtil cfrtil-gdb

udis :
	wget http://prdownloads.sourceforge.net/udis86/udis86-1.7.2.tar.gz
	tar -xvf udis86-1.7.2.tar.gz 
	cd udis86-1.7.2 && \
	./configure CFLAGS=-m32 --enable-shared && \
	make && \
	sudo make install && \
	sudo ldconfig
	
gmp : 
	wget https://gmplib.org/download/gmp/gmp-6.0.0a.tar.bz2
	tar -xjvf gmp-6.0.0a.tar.bz2
	cd gmp-6.0.0 && \
	./configure CFLAGS=-m32 ABI=32 --enable-shared && \
	make && \
	sudo make install && \
	sudo ldconfig
	
cproto :
	wget https://launchpad.net/ubuntu/+archive/primary/+files/cproto_4.7l.orig.tar.gz
	tar -xvf cproto_4.7l.orig.tar.gz
	cd cproto-4.7l && \
	./configure CFLAGS=-m32 && \
	make && \
	sudo make install
	
# suffix for zipfiles
suffix = *.[cfht^~]*
sfx = *.[a-np-wz]*
zip : 
	-mv nbproject ../backup
	zip  cfrtil.zip */$(sfx) */*/$(sfx) *.htm* Makefile m r script* netbeansReset *.cft* retroImage $(sfx)			    
	-mv ../backup/nbproject .

tar.xz :	
	tar -c --xz --exclude=nbproject --exclude=.git --exclude=*.png --exclude=cfrtil-gdb  --exclude=*.o -f cfrtil.tar.xz * .init.cft

_xz : 
	-rm core
	-rm -rf ~/backup/openvmtil/
	-cp -r ~/openvmtil/ ~/backup/openvmtil/
	-mv cfrtil cfrtil* ../bin/

xz : 
	-make default
	make _xz

_all : realClean install
	make xz

_install :
	-sudo rm -rf /usr/local/lib/cfrTil
	-sudo mkdir /usr/local/lib/cfrTil
	-cp ../.init.cft namespaces
	-sudo cp ../.init.cft /usr/local/lib/cfrTil
	-sudo cp -r namespaces /usr/local/lib/cfrTil
	-sudo cp lib/lib*.* /usr/local/lib
	-sudo cp bin/cfrtil /usr/local/bin/cfrtil
#	-sudo chown dennisj /usr/local/bin/cfrtil
	-sudo ldconfig

_install_1 :
	-cp ./.init.cft namespaces
	-sudo rm -rf /usr/local/lib/cfrTil/
	-sudo cp -r ../ /usr/local/lib/cfrTil/
	-sudo cp -r ../etc /usr/local/
#	-sudo rm -r /usr/local/lib/cfrTil/nbproject
	sudo cp ../bin/* /usr/local/bin
	-sudo ldconfig

install0 :
	make _clean
	make 
	make _install
	ls -l ./cfrtil
	
install :
	make
	make _install
	ls -l bin/cfrtil
	
install_1 :
	make
	make _install_1
	ls -l /usr/local/bin/cfrtil
	
run :
	cfrtil

runLocal :
	./cfrtil

#	as -adhls=vm.list -o stack.o vm.s
# DO NOT DELETE
