#-------------------------------------------------------------------------------
# Copyright (c) 2021 by Ando Ki.
# All rights are reserved by Ando Ki.
#-------------------------------------------------------------------------------
SHELL    = /bin/sh
MAKEFILE = Makefile
#-------------------------------------------------------------------------------
ifeq ($(MAKECMDGOALS),$(findstring $(MAKECMDGOALS), "" "all" "hw" "run" "cosim"))
    ifndef COSIM_HOME
       $(warning COSIM_HOME environment variable not defined.)
       export COSIM_HOME = $(abspath ../../..)
    endif
    ifneq ($(findstring "include/python",$(PYTHONPATH)),"include/python")
       $(warning PYTHONPATH updated.)
       ifdef PYTHONPATH
          export PYTHONPATH := $(COSIM_HOME)/include/python:$(PYTHONPATH)
       else
          export PYTHONPATH := $(COSIM_HOME)/include/python
       endif
    endif
    export SIMULATOR ?= xsim
   #export SIMULATOR ?= iverilog
    export DIR_SIM   := ../hw/sim/$(SIMULATOR)
endif

#-------------------------------------------------------------------------------
TARGET     = test.py
PYTHON     = python3
SIMULATOR ?= xsim

#-------------------------------------------------------------------------------
.PHONY: all
all:

.PHONY: hw
hw:
	make -C $(DIR_SIM) compile

DBUS = $(shell dbus-launch)
TERM=GNOME
#TERM=XTERM

.PHONY: run
run:
	$(PYTHON) $(TARGET) --simulator=$(SIMULATOR) --cid=0 --verbose=0

.PHONY: cosim
cosim:
        ifeq ("$(SIMULATOR)","xsim")
		@if [ ! -x "`which xelab 2>/dev/null`" ]; then\
			echo "xelab not found."; exit 1;\
		fi
        else
		@if [ ! -x "`which iverilog 2>/dev/null`" ]; then\
			echo "iverilog not found."; exit 1;\
		fi
        endif
	export $(DBUS) > /dev/null 2>&1
        ifeq ("$(TERM)","XTERM")
		xterm -T "client" -fg 'black' -bg 'white' -fa 'Monospace' -fs 10\
			-e /bin/bash -l -c\
			"cd $(DIR_SIM); make sim; bash"&
		$(PYTHON) $(TARGET) --simulator=$(SIMULATOR) --cid=0 --verbose=0
        else
		gnome-terminal --title="client" --window -- bash -c\
			"cd $(DIR_SIM); make sim; bash"
		$(PYTHON) $(TARGET) --simulator=$(SIMULATOR) --cid=0 --verbose=0
        endif

run_dpi dpi_run run_xsim xsim_run:
	@if [ ! -x "`which xelab 2>/dev/null`" ]; then\
		echo "xelab not found."; exit 1;\
	fi
	if [ ! -f $(TARGET) ]; then make; fi
	gnome-terminal --window -- bash -c "make -C $(DIR_XSIM); bash"
	make run SIMULATOR=xsim

run_vpi vpi_run run_iverilog iverilog_run:
	@if [ ! -x "`which iverilog 2>/dev/null`" ]; then\
		echo "iverilog not found."; exit 1;\
	fi
	if [ ! -f $(TARGET) ]; then make; fi
	gnome-terminal --window -- bash -c "make -C $(DIR_IVERILOG); bash"
	make run SIMULATOR=iverilog

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

.PHONY: cleanupall distclean
cleanupall distclean: cleanup

#----------------------------------------------------------------------------
# Revision history
#
# 2021.07.01: Started by Ando Ki (andoki@gmail.com)
#----------------------------------------------------------------------------
