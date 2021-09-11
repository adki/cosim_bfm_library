#-------------------------------------------------------------------------------
SHELL           = /bin/sh
MAKEFILE        = Makefile
#-------------------------------------------------------------------------------
ifeq ($(MAKECMDGOALS),$(findstring $(MAKECMDGOALS), "" "all" "elab" "sim"))
    ifndef XILINX_VIVADO
       $(error XILINX_VIVADO environment variable not defined)
    else
        export VIVADO         = $(XILINX_VIVADO)/bin/vivado
        export VIVADO_VERSION = $(shell vivado -version | sed -n 1p | cut -d" " -f 2 | cut -c 2-7)
        export VIVADO_VER     = vivado.$(VIVADO_VERSION)
    endif
    ifndef COSIM_HOME
       $(warning COSIM_HOME environment variable not defined.)
       export COSIM_HOME = $(abspath ../../../../..)
    endif
    PLATFORM  = $(shell uname -s | tr '[:upper:]' '[:lower:]')
    MACHINE   = $(shell uname -m)
    export COSIM_LIB      = cosim_dpi_bfm.so
    export DIR_COSIM_ROOT = $(COSIM_HOME)/lib/xsim/$(PLATFORM)_$(MACHINE)
    export DIR_DPI_BFM    = $(COSIM_HOME)/include/verilog
    export DPI_BFM        = $(DIR_DPI_BFM)/cosim_bfm_axi_dpi.sv
endif

#-------------------------------------------------------------------------------
export GUI        ?= 0
export OPTIONS    ?= 

#-------------------------------------------------------------------------------
all: elab sim

elab compile:
	xelab -prj xsim.prj -sv_root $(DIR_COSIM_ROOT) -sv_lib $(COSIM_LIB) -debug typical top -s top

sim run:
	@if [ "$(GUI)" = "0" ]; then\
		xsim top -t xsim_run.tcl $(OPTIONS);\
	else\
		xsim top -gui -t xsim_run.tcl $(OPTIONS);\
	fi

clean:
	/bin/rm -f  top.wdb
	/bin/rm -f  wave.vcd
	/bin/rm -f  webtalk_*.backup.jou
	/bin/rm -f  webtalk_*.backup.log
	/bin/rm -f  webtalk.jou
	/bin/rm -f  webtalk.log
	/bin/rm -f  xelab.log
	/bin/rm -f  xelab.pb
	/bin/rm -fr .Xil/
	/bin/rm -f  xsim_*.backup.jou
	/bin/rm -f  xsim_*.backup.log
	/bin/rm -fr xsim.dir/
	/bin/rm -f  xsim.jou
	/bin/rm -f  xsim.log
	/bin/rm -f  xvlog.log
	/bin/rm -f  xvlog.pb

cleanup clobber: clean

cleanupall distclean: cleanup
