@ECHO OFF

IF EXIST obj          RMDIR /Q/S obj
IF EXIST ipc_lib.dll  DEL   /Q   ipc_lib.dll
IF EXIST libipc_lib.a DEL   /Q   libipc_lib.a
