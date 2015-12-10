SRCPATH=.
prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
libdir=${exec_prefix}/lib
includedir=${prefix}/include
ARCH=ARM
SYS=LINUX
CC=arm-linux-androideabi-gcc
CFLAGS=-Wshadow -O3 -fno-fast-math  -Wall -I. -I$(SRCPATH) -std=gnu99 -fPIC -fomit-frame-pointer -fno-tree-vectorize
DEPMM=-MM -g0
DEPMT=-MT
LD=arm-linux-androideabi-gcc -o 
LDFLAGS= -lm -ldl
LIBX264=libx264.a
AR=arm-linux-androideabi-ar rc 
RANLIB=arm-linux-androideabi-ranlib
STRIP=arm-linux-androideabi-strip
AS=
ASFLAGS= -DHAVE_ALIGNED_STACK=1 -DPIC -DHIGH_BIT_DEPTH=0 -DBIT_DEPTH=8
RC=
RCFLAGS=
EXE=
HAVE_GETOPT_LONG=1
DEVNULL=/dev/null
PROF_GEN_CC=-fprofile-generate
PROF_GEN_LD=-fprofile-generate
PROF_USE_CC=-fprofile-use
PROF_USE_LD=-fprofile-use
HAVE_OPENCL=yes
default: cli
install: install-cli
SOSUFFIX=so
SONAME=libx264.so
SOFLAGS=-shared -Wl,-soname,$(SONAME)  -Wl,-Bsymbolic
default: lib-shared
install: install-lib-shared
LDFLAGSCLI = -ldl 
CLI_LIBX264 = $(LIBX264)
