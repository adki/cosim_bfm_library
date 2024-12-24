#------------------------------------------------------------------
SHELL    = /bin/sh
MAKEFILE = Makefile
#--------------------------------------------------------
ifeq ($(MAKECMDGOALS),$(findstring $(MAKECMDGOALS), "" "all" "elab" "compile" "hw" "sim" "run"))
    ifeq (, $(shell which iverilog))
       $(error iverilog not found.)
    endif
    ifndef COSIM_HOME
       $(warning COSIM_HOME environment variable not defined)
       export COSIM_HOME = ../../../../..
    endif
    PLATFORM  = $(shell uname -s | tr '[:upper:]' '[:lower:]')
    MACHINE   = $(shell uname -m)
    export COSIM_LIB      = cosim_vpi_bfm.vpi
    export DIR_COSIM_ROOT = $(COSIM_HOME)/lib/iverilog/$(PLATFORM)_$(MACHINE)
    export DIR_VPI_BFM    = $(COSIM_HOME)/include/verilog
    export VPI_BFM        = $(DIR_VPI_BFM)/cosim_bfm_axi_vpi.v
endif
#--------------------------------------------------------
ILOG	 = iverilog
ISIM	 = vvp
#--------------------------------------------------------
TOP	  = top
DIR_RTL   = ../../design/verilog
#--------------------------------------------------------
all:

compile hw:
	($(ILOG) -o $(TOP).vvp -s $(TOP)\
		-I$(DIR_RTL)\
		-I$(DIR_VPI_BFM)\
                ./sim_define.v\
		$(DIR_RTL)/top.v\
		$(DIR_RTL)/mem_axi_beh.v\
		$(DIR_VPI_BFM)/cosim_bfm_axi_vpi.v\
		|| exit -1) 2>&1 | tee compile.log

sim run: compile
	$(ISIM) -l vvp.log\
		-M$(DIR_COSIM_ROOT) -mcosim_vpi_bfm\
		$(TOP).vvp

#--------------------------------------------------------
clean:
	/bin/rm -f  $(TOP).vvp
	/bin/rm -f  compile.log
	/bin/rm -f  wave.vcd
	/bin/rm -f  vvp.log
	/bin/rm -f  result.bmp

cleanup clobber: clean

cleanupall distclean: cleanup
#--------------------------------------------------------
