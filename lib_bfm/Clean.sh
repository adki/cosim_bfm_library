#!/bin/sh

if [ -d obj ] ; then rm -rf obj; fi
if [ -f cosim_bfm_api_lib.dll  ]; then rm -f cosim_bfm_api_lib.dll;  fi
if [ -f cosim_bfm_vpi_lib.dll  ]; then rm -f cosim_bfm_vpi_lib.dll;  fi
if [ -f cosim_bfm_api_lib.lib  ]; then rm -f cosim_bfm_api_lib.lib;  fi
if [ -f cosim_bfm_vpi_lib.lib  ]; then rm -f cosim_bfm_vpi_lib.lib;  fi
if [ -f libcosim_bfm_api_lib.a ]; then rm -f libcosim_bfm_api_lib.a; fi
if [ -f libcosim_bfm_vpi_lib.a ]; then rm -f libcosim_bfm_vpi_lib.a; fi
if [ -f cosim_bfm_dpi_lib.so   ]; then rm -f cosim_bfm_dpi_lib.so;   fi
if [ -f transcript             ]; then rm -f transcript;             fi
if [ -d xsim.dir  ]; then rm -rf xsim.dir; fi
if [ -f xelab.log ]; then rm -f xelab.log; fi
if [ -f xelab.pb  ]; then rm -f xelab.pb; fi
if [ -f xsc.log   ]; then rm -f xsc.log; fi
if [ -f xsc.pb    ]; then rm -f xsc.pb; fi
