#!/bin/sh

if [ -d obj ] ; then rm -rf obj; fi
/bin/rm -f *.o
/bin/rm -f test test.exe
/bin/rm -f *.log
/bin/rm -f *.exe.core
