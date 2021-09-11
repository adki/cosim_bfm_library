#----------------------------------------------------------------------------
# Copyright (c) 2021 by Ando Ki.
# All rights are reserved by Ando Ki.
#----------------------------------------------------------------------------
# Makefile
#------------------------------------------------------------------------
#include ../Makefile.in
PLATFORM = $(shell uname -s | tr '[:upper:]' '[:lower:]')
#------------------------------------------------------------------------
.PHONY: all clean cleanup clobber cleanupall distclean
all clean cleanup clobber cleanupall distclean:
    ifeq ($(PLATFORM),cygwin)
		make -f Makefile.cygwin $@
    else ifeq ($(PLATFORM),cygwin64)
		make -f Makefile.cygwin $@
    else ifeq ($(PLATFORM),mingw)
		make -f Makefile.mingw $@
    else ifeq ($(PLATFORM),mingw64)
		make -f Makefile.mingw $@
    else ifeq ($(PLATFORM),linux)
		make -f Makefile.linux $@
    else ifeq ($(PLATFORM),linux64)
		make -f Makefile.linux $@
    else
         $(error un-supported platform $(PLATFORM))
    endif

#----------------------------------------------------------------------------
# Revision history
#
# 2021.07.01: Started by Ando Ki (andoki@gmail.com)
#----------------------------------------------------------------------------
