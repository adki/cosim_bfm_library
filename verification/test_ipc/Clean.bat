@ECHO OFF

@DEL /Q *.o
@DEL /Q *.log
IF EXIST test     DEL   /Q   test
IF EXIST test.exe DEL   /Q   test.exe
IF EXIST obj      RMDIR /Q/S obj
IF EXIST *.exe.core DEL /Q *.exe.core
