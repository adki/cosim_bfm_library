#!/bin/csh -f

if ( -d obj ) then
	\rm -rf obj
endif
/bin/rm -f *.o
/bin/rm -f test test.exe
/bin/rm -f *.log
/bin/rm -f *.exe.core
