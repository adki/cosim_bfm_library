#----------------------------------------------------------------------------
# Copyright (c) 2021 by Ando Ki.
# All rights are reserved by Ando Ki.
#----------------------------------------------------------------------------
# Makefile
#------------------------------------------------------------------------
#include ../../Makefile.in
PLATFORM        = $(shell uname -s | tr '[:upper:]' '[:lower:]')
#------------------------------------------------------------------------
OBJECTDIR = obj
DUMMY    := $(shell [ -d $(OBJECTDIR) ] || mkdir $(OBJECTDIR) )
#------------------------------------------------------------------------
IPC_DIR = ../../lib_ipc
ifeq ($(PLATFORM),cygwin)
IPC_LIB = cosim_ipc.dll
else ifeq ($(PLATFORM),mingw)
IPC_LIB = cosim_ipc.dll
else
IPC_LIB = libcosim_ipc.a
endif
#------------------------------------------------------------------------
TARGET	= test
SRCS	= main.c
OBJS	= $(SRCS:.c=.o)
#------------------------------------------------------------------------
vpath %.h	src:$(IPC_DIR)/src
vpath %.c	src:$(IPC_DIR)/src
#------------------------------------------------------------------------
GXX             = gcc

#------------------------------------------------------------------------
ifeq ($(PLATFORM),cygwin)
	C_USER_DEFS = -DWIN32 -mno-cygwin
	C_USER_FLAGS =
else ifeq ($(PLATFORM),mingw)
	C_USER_DEFS = -DWIN32
	C_USER_FLAGS =
else ifeq ($(PLATFORM),linux)
	C_USER_DEFS =
	C_USER_FLAGS = -fPIC
else
	C_USER_DEFS =
	C_USER_FLAGS =
endif
C_CFLAGS = -g -O0 -Werror $(C_USER_DEFS) $(C_USER_FLAGS)
C_LFLAGS = -O0 $(C_USER_DEFS) $(C_USER_FLAGS)

CFLAGS      = $(C_CFLAGS) -Isrc -I$(IPC_DIR)/src
LDFLAGS     = $(C_LFLAGS)
LDLIBS      =

#------------------------------------------------------------------------
$(OBJECTDIR)/%.o: %.c
	$(GXX) -c $(CFLAGS) $< -o $@ 2>&1 | tee -a compile.log
#------------------------------------------------------------------------
.PHONY: all
all: pre $(TARGET)

$(TARGET): $(IPC_LIB) $(addprefix $(OBJECTDIR)/,$(OBJS))
	$(GXX) $(LDFLAGS) -o $@ $(addprefix $(OBJECTDIR)/,$(OBJS)) $(IPC_DIR)/$(IPC_LIB)

$(IPC_LIB):
	make -C $(IPC_DIR);

pre:
	if [ -f compile.log ]; then /bin/rm -f compile.log; fi
#------------------------------------------------------------------------
.PHONY: run
run.tx run_tx tx: $(TARGET)
	./$(TARGET) -tx

run.rx run_rx rx: $(TARGET)
	./$(TARGET) -rx

run run.txrx run_txrx: $(TARGET)
    ifeq ($(PLATFORM),cygwin)
	#-xterm -hold -e "./$(TARGET) -rx" &
	-xterm -e "./$(TARGET) -v 2 -rx" &
	./$(TARGET) -v 2 -tx
    else ifeq ($(PLATFORM),mingw)
	start $(TARGET) -rx
	./$(TARGET) -tx
    else ifeq ($(PLATFORM),linux)
	-xterm -e "./$(TARGET) -v 2 -rx" &
	./$(TARGET) -v 2 -tx
    else ifeq ($(PLATFORM),linux64)
	-xterm -e "./$(TARGET) -v 2 -rx" &
	./$(TARGET) -v 2 -tx
    endif

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
