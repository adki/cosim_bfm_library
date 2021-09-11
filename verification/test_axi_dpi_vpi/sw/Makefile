#-------------------------------------------------------------------------------
# Copyright (c) 2021 by Ando Ki.
# All rights are reserved by Ando Ki.
#-------------------------------------------------------------------------------
SHELL    = /bin/sh
MAKEFILE = Makefile
#-------------------------------------------------------------------------------
ifeq ($(MAKECMDGOALS),$(findstring $(MAKECMDGOALS), "" "all"))
    ifndef COSIM_HOME
       $(warning COSIM_HOME environment variable not defined.)
       export COSIM_HOME = $(abspath ../../..)
    endif
    PLATFORM  = $(shell uname -s | tr '[:upper:]' '[:lower:]')
    MACHINE   = $(shell uname -m)
    export DIR_COSIM_ROOT = $(COSIM_HOME)
    export DIR_COSIM_INC  = $(DIR_COSIM_ROOT)/include
    export SIMULATOR ?= xsim
    export DIR_COSIM_LIB  = $(DIR_COSIM_ROOT)/lib/$(SIMULATOR)/$(PLATFORM)_$(MACHINE)
endif
#-------------------------------------------------------------------------------
OBJECTDIR = obj
DUMMY    := $(shell [ -d $(OBJECTDIR) ] || mkdir $(OBJECTDIR) )
#-------------------------------------------------------------------------------
LIB_BFM = cosim_bfm
#-------------------------------------------------------------------------------
TARGET = test
SRCS   = main.c
OBJS   = $(SRCS:.c=.o)
#-------------------------------------------------------------------------------
vpath %.h	src
vpath %.c	src
#-------------------------------------------------------------------------------
CC             = gcc
CC_VERSION = $(shell gcc -dumpversion)
#-------------------------------------------------------------------------------
PLATFORM  = $(shell uname -s | tr '[:upper:]' '[:lower:]')
MACH      = $(shell uname -m)
ifeq ($(PLATFORM),cygwin)
	C_USER_DEFS = -DWIN32 -mno-cygwin
	C_USER_FLAGS =
else ifeq ($(PLATFORM),mingw)
	C_USER_DEFS = -DWIN32
	C_USER_FLAGS =
else ifeq ($(PLATFORM),linux)
	C_USER_DEFS =
        ifeq ($(MACH),x86_64)
	    C_USER_FLAGS = -m64 -fPIC
        else
	    C_USER_FLAGS = -m32 -fPIC
        endif
else
	C_USER_DEFS =
	C_USER_FLAGS =
endif
C_CFLAGS = -g -O0 -Werror $(C_USER_DEFS) $(C_USER_FLAGS)
C_LFLAGS = -O0 $(C_USER_DEFS) $(C_USER_FLAGS)

CFLAGS   = $(C_CFLAGS) -Isrc -I$(DIR_COSIM_INC)
LDFLAGS  = $(C_LFLAGS)
LDLIBS   = -Wl,-Bstatic -L$(DIR_COSIM_LIB) -l$(LIB_BFM) -Wl,-Bdynamic

#------------------------------------------------------------------------
$(OBJECTDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 2>&1 | tee -a compile.log
#------------------------------------------------------------------------
.PHONY: all
all: pre $(TARGET)

$(TARGET): $(addprefix $(OBJECTDIR)/,$(OBJS))
	$(CC) -o $(TARGET) $(LDFLAGS) $(addprefix $(OBJECTDIR)/,$(OBJS))\
		$(LDLIBS)

pre:
	if [ -f compile.log ]; then /bin/rm -f compile.log; fi

run:
	./$(TARGET) --cid=0 --verbose=0

run_dpi:
	@if [ ! -x "`which xelab 2>/dev/null`" ]; then\
		echo "xelab not found."; exit 1;\
	fi
	if [ ! -f $(TARGET) ]; then make; fi
	gnome-terminal --window -- bash -c "make -C ../hw/sim/xsim; bash"
	make run

run_vpi:
	@if [ ! -x "`which iverilog 2>/dev/null`" ]; then\
		echo "iverilog not found."; exit 1;\
	fi
	if [ ! -f $(TARGET) ]; then make; fi
	gnome-terminal --window -- bash -c "make -C ../hw/sim/iverilog; bash"
	make run

#------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf $(OBJECTDIR)
	rm -f  *.log
	rm -f  *.o
	rm -f *stackdump
	rm -f *.exe.core
	rm -f compile.log
	rm -f lock_file*.txt

.PHONY: cleanup clobber
cleanup clobber: clean
	rm -f $(TARGET) $(TARGET).exe

.PHONY: cleanupall distclean
cleanupall distclean: cleanup

#----------------------------------------------------------------------------
# Revision history
#
# 2021.07.01: Started by Ando Ki (andoki@gmail.com)
#----------------------------------------------------------------------------
