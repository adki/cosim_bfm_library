#!/bin/bash

/bin/rm -f  top.wdb
/bin/rm -f  wave.vcd
/bin/rm -f  webtalk_*.backup.jou
/bin/rm -f  webtalk_*.backup.log
/bin/rm -f  webtalk.jou
/bin/rm -f  webtalk.log
/bin/rm -f  xelab.log
/bin/rm -f  xelab.pb
/bin/rm -fr .Xil/
/bin/rm -f  xsim_*.backup.jou
/bin/rm -f  xsim_*.backup.log
/bin/rm -fr xsim.dir/
/bin/rm -f  xsim.jou
/bin/rm -f  xsim.log
/bin/rm -f  xvlog.log
/bin/rm -f  xvlog.pb

/bin/rm -f sdpram_w32s32k.mif
#for F in 1 2 3 4; do
#    /bin/rm -f sdpram_w8s8k_$${F}of4.mif;\
#done
