#!/bin/csh -f

if ( -d obj ) \rm -rf obj
if ( -f cosim_bfm_api_lib.dll  ) rm -f cosim_bfm_api_lib.dll
if ( -f cosim_bfm_vpi_lib.dll  ) rm -f cosim_bfm_vpi_lib.dll
if ( -f cosim_bfm_api_lib.lib  ) rm -f cosim_bfm_api_lib.lib
if ( -f cosim_bfm_vpi_lib.lib  ) rm -f cosim_bfm_vpi_lib.lib
if ( -f libcosim_bfm_api_lib.a ) rm -f libcosim_bfm_api_lib.a
if ( -f libcosim_bfm_vpi_lib.a ) rm -f libcosim_bfm_vpi_lib.a
if ( -f cosim_bfm_dpi_lib.so   ) rm -f cosim_bfm_dpi_lib.so
if ( -f transcript             ) rm -f transcript
if ( -d xsim.dir  ) rm -rf xsim.dir
if ( -f xelab.log ) rm -f xelab.log
if ( -f xelab.pb  ) rm -f xelab.pb
if ( -f xsc.log ) rm -f xsc.log
if ( -f xsc.pb  ) rm -f xsc.pb
