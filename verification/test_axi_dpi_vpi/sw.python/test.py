#!/usr/bin/env python3

import sys
import cosim_bfm as cosim

def mem_test( addr=0x0
            , size=4
            , length=1
            , rigor=False
            , verbose=False):

    Wdata = bytearray([0x00] * size * length)
    Rdata = bytearray([0x00] * size * length)
    for i in range(size*length): Wdata[i] = i+1;

    cosim.bfm_write(0x0, Wdata, size, length, rigor=1)
    cosim.bfm_read (0x0, Rdata, size, length, rigor=1)

    error = 0
    for i in range(size*length):
        if Wdata[i]!=Rdata[i]: error = error + 1;

    if error>0:
        print(f"mis-match {error} out of {size*length} bytes")
    else:
        print(f"OK {size*length} bytes")

if __name__=='__main__':
    import getopt
    level = 0
    simulator = 'xsim'
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hg:C:S:", ['help'
                                                            ,'verbose='
                                                            ,'cid='
                                                            ,'simulator='])
    except getopt.GetoptError:
        sys.exit(2)
    else:
        for opt, arg in opts:
            if opt in ("-h", "--help"):
                print(f"{sys.argv[0]} [options]")
                print(f" -h/--help: help")
                print(f" -g/--verbose=num: set verbose level")
                print(f" -C/--cid=num: set channel id")
                print(f" -S/--simulator='xsim'|'iverilog': set simulator")
                sys.exit()
            elif opt in ("-C","--cid"):
                _cid = int(arg)
            elif opt in ("-S","--simulator"):
                simulator = str(arg)
            elif opt in ("-g","--verbose"):
                level = int(arg)
            else:
                print("Unknown options: "+str(opt))
                sys.exit(1)

    cosim.LoadCosimLib(simulator) # mandatory
    cosim.bfm_set_verbose(level) # optional
    cosim.bfm_open(_cid) # mandatory
    cosim.bfm_barrier(_cid) # mandatory

    mem_test(0x0, 4, 10, rigor=0, verbose=level)
    mem_test(0x0, 2, 10, rigor=0, verbose=level)
    mem_test(0x0, 1, 10, rigor=0, verbose=level)

    cosim.bfm_close(_cid)
