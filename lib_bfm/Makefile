DIRS    = $(subst /,, $(dir $(wildcard */Makefile)))
  
all:

clean cleanup clobber cleanupall disclean:
	make -f Makefile.xsim $@
	make -f Makefile.iverilog $@

.PHONY: all clean cleanup clobber cleanupall disclean

