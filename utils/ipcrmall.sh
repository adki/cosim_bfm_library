#!/bin/sh -f

IDS=`ipcs -q | grep "^0x" | cut -d' ' -f 2`
for id in ${IDS}; do
    ipcrm -q ${id}
done
