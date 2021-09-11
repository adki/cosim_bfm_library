@ECHO OFF

IF EXIST obj                    RMDIR /Q/S obj
IF EXIST cosim_bfm_api_lib.dll  DEL   /Q   cosim_bfm_api_lib.dll
IF EXIST cosim_bfm_vpi_lib.dll  DEL   /Q   cosim_bfm_vpi_lib.dll
IF EXIST cosim_bfm_api_lib.lib  DEL   /Q   cosim_bfm_api_lib.lib
IF EXIST cosim_bfm_vpi_lib.lib  DEL   /Q   cosim_bfm_vpi_lib.lib
IF EXIST libcosim_bfm_api_lib.a DEL   /Q   libcosim_bfm_api_lib.a
IF EXIST libcosim_bfm_vpi_lib.a DEL   /Q   libcosim_bfm_vpi_lib.a
if EXIST cosim_bfm_dpi_lib.so   DEL   /Q   cosim_bfm_dpi_lib.so
IF EXIST transcript             DEL   /Q   transcript
