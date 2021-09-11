#!/bin/bash

if [ -f __init__.pyc ]; then /bin/rm -f  __init__.pyc; fi
if [ -f cosim_bfm.pyc ]; then /bin/rm -f  cosim_bfm.pyc; fi

for F in *; do
    if [[ -d "${F}" && ! -L "${F}" ]]; then
    if [ -f ${F}/Clean.sh ]; then
       ( cd ${F}; ./Clean.sh )
    fi
    fi
done
