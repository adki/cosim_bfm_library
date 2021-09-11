#!/bin/sh

if [ -d obj ] ; then rm -rf obj; fi
if [ -f ipc_lib.dll  ]; then rm -f ipc_lib.dll ; fi
if [ -f libipc_lib.a ]; then rm -f libipc_lib.a; fi
