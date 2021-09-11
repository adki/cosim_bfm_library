SHELL    = /bin/sh
MAKEFILE = Makefile

ifeq ($(MAKECMDGOALS),$(findstring $(MAKECMDGOALS), "" "all" "run" "run_dpi" "run_vpi"))
    ifndef COSIM_HOME
       $(info COSIM_HOME environment variable not defined.)
       export COSIM_HOME = $(abspath ../..)
    endif
    PLATFORM  = $(shell uname -s | tr '[:upper:]' '[:lower:]')
    MACHINE   = $(shell uname -m)
    export DIR_COSIM_ROOT = $(COSIM_HOME)
    export DIR_COSIM_INC  = $(DIR_COSIM_ROOT)/include
    export SIMULATOR ?= xsim
    export DIR_COSIM_LIB  = $(DIR_COSIM_ROOT)/lib/$(SIMULATOR)/$(PLATFORM)_$(MACHINE)
    export DIR_XSIM := ../../verification/test_axi_dpi_vpi/hw/sim/xsim
    export DIR_IVERILOG := ../../verification/test_axi_dpi_vpi/hw/sim/iverilog
    ifneq ($(findstring "include/python",$(PYTHONPATH)),"include/python")
       $(warning PYTHONPATH updated.)
       ifdef PYTHONPATH
          PYTHONPATH := $(COSIM_HOME)/include/python:$(PYTHONPATH)
       else
          PYTHONPATH := $(COSIM_HOME)/include/python
       endif
    endif
endif

TARGET     = cosim_bfm.py
PYTHON     = python3
SIMULATOR ?= xsim

all:
	gnome-terminal --window -- bash -c "cd $(DIR_XSIM); make; bash"

run:
	@if [ ! -z "`(ipcs -q | grep "^0x" | cut -d' ' -f 2) 2>/dev/null`" ]; then\
		echo "IPC message queue is still working. Use \"$$ ipcs -q\" and \"$$ ipcrm -q mid.\"";\
	fi
	$(PYTHON) $(TARGET) --simulator=$(SIMULATOR) --cid=0 --verbose=0

run_dpi dpi_run run_xsim xsim_run:
	@if [ -z "`which xelab 2>/dev/null`" ]; then\
		echo "xelab not found."; exit 1;\
	fi
	gnome-terminal --window -- bash -c "cd $(DIR_XSIM); make; bash"
	make run SIMULATOR=xsim

run_vpi vpi_run run_iverilog iverilog_run:
	@if [ -z "`which iverilog 2>/dev/null`" ]; then\
		0echo "iverilog not found."; exit 1;\
	fi
	gnome-terminal --window -- bash -c "cd $(DIR_IVERILOG); make; bash"
	make run SIMULATOR=iverilog

clean:
	/bin/rm -f  __init__.pyc
	/bin/rm -f  cosim_bfm.pyc
	/bin/rm -rf __pycache__

cleanup clobber: clean

cleanupall distclean: cleanup
