@ECHO OFF

RMDIR /Q /S  obj
DEL   /Q  *.log
DEL   /Q  *.o
DEL   /Q  *stackdump
DEL   /Q  *.exe.core
DEL   /Q  compile.log
DEL   /Q  lock_file*.txt
