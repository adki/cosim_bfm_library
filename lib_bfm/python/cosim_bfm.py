#!/usr/bin/env python3
"""
This file contains common part of Python interface of of Co-Simulation BFM.
"""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

#-------------------------------------------------------------------------------
__author__     = "Ando Ki"
__copyright__  = "Copyright 2021, Ando Ki"
__credits__    = ["none", "some"]
__license__    = "FUTURE DESIGN SYSTEMS SOFTWARE END-USER LICENSE AGREEMENT"
__version__    = "0"
__revision__   = "0"
__maintainer__ = "Ando Ki"
__email__      = "andoki@gmail.com"
__status__     = "Development"
__date__       = "2021.08.10"
__description__= "Python interface of Co-Simulation BFM"

#-------------------------------------------------------------------------------
import os
import sys
import platform
import traceback
import inspect
import ctypes
import ctypes.util

#===============================================================================
# utility functions
# print GetFunctionName()+'('+str(GetFunctionParametersAndValues())+')'
def GetLineno():
    """
    Returns the current line number in the program.
    :return: integer of the line number.
    """
    return inspect.currentframe().f_back.f_lineno

def GetFunctionName():
    """
    Returns the current function name in the program.
    :return: string of the function name.
    """
    return traceback.extract_stack(None, 2)[0][2]

def GetFunctionParametersAndValues():
    """
    Returns the dictionary of function arguments in the program.
    :return: disctionary of function arguments.
    """
    frame = inspect.currentframe().f_back
    args, _, _, values = inspect.getargvalues(frame)
    return ([(i, values[i]) for i in args])

def CosimError(*args, **kwargs):
    print(traceback.extract_stack(None, 2)[0][2], "Error: ".join(map(str,args)), **kwargs)

def CosimWarn(*args, **kwargs):
    print(traceback.extract_stack(None, 2)[0][2], "Warning: ".join(map(str,args)), **kwargs)

def CosimInfo(*args, **kwargs):
    print(traceback.extract_stack(None, 2)[0][2], "Info: ".join(map(str,args)), **kwargs)

def CosimPrint(*args, **kwargs):
    print(traceback.extract_stack(None, 2)[0][2], " ".join(map(str,args)), **kwargs)

#-------------------------------------------------------------------------------
# utility functions: function signature or function wrapper
def WrapFunction(lib, funcname, restype, argtypes):
    """
    Simplify wrapping ctypes functions
    :param lib: library (object returned from ctypes.CDLL()
    :param funcname: string of function name
    :param restype: type of return value
    :param argtypes: a list of types of the function arguments
    :return: Python object holding function, restype and argtypes.
    """
    func = lib.__getattr__(funcname)
    func.restype = restype
    func.argtypes = argtypes
    return func

#===============================================================================
#verbose = True
#verboseprint = print if verbose else lambda *a, **k: None

#-------------------------------------------------------------------------------
# need debug for this 'rigor' and 'verbose'
rigor = False
verbose = False

def set_rigor( ri ): global rigor; rigor = ri
def get_rigor(): global rigor; return rigor

def set_verbose ( ve ): global verbose; verbose = ve
def get_verbose (): global verbose; return verbose

#===============================================================================
# let check 'COSIM_HOME' environment variable
if 'COSIM_HOME' not in os.environ:
   print (GetFunctionName(), "Warning: the environment variable COSIM_HOME not defined.", flush=True)
   COSIM_HOME='../..'
else:
   COSIM_HOME=os.environ["COSIM_HOME"]

_libcosim = None; #path to libcosim_bfm.so
_cid      = 0; #default channel id.
_cosim    = None

#-------------------------------------------------------------------------------
def LoadCosimLib( simulator
                , rigor=False
                , verbose=False):
    """
    Load C shared library.
    """
    global _libcosim
    global _cosim
    system=platform.system().lower()
    machine=platform.machine()
    sys_mach=system+'_'+machine
    _libcosim = os.path.abspath( os.path.join(COSIM_HOME, "lib", simulator, sys_mach, "libcosim_bfm.so"))
    if not os.path.isfile(_libcosim):
       print (GetFunctionName(), _libcosim+' is not a file', flush=True)
       traceback.print_exc(file=sys.stdout)
       sys.exit(1)
    else:
       # '__debug__' This constant is true if Python was not started with an -O option
       if __debug__: print (GetFunctionName(), _libcosim+" found.", flush=True)

    try:
        _cosim = ctypes.CDLL(_libcosim, ctypes.RTLD_GLOBAL)
    except:
        traceback.print_exc(file=sys.stdout)
        sys.exit(1)

#===============================================================================
def bfm_open   ( cid=0
               , rigor=False
               , verbose=False ):
    """
    Open an IPC channel
    :param cid: integer, IPC channel ID
    :return: 0 on success, otherwise return negative number
    """
    global _cosim
    global _cid
    _bfm_open = WrapFunction( _cosim
                            , 'bfm_open'
                            , ctypes.c_int
                            ,[ctypes.c_int])
    ret =  _bfm_open( cid )
    if ret: _cid = cid
    return ret

def bfm_close  ( cid=0
               , rigor=False
               , verbose=False ):
    """
    Close the IPC channel.
    :param cid: integer, IPC channel ID
    :return: 0 on success, otherwise return negative number.
    """
    global _cosim
    _bfm_close = WrapFunction( _cosim
                             , 'bfm_close'
                             , ctypes.c_int
                             ,[ctypes.c_int])
    return _bfm_close( cid )

def bfm_barrier( cid=0
               , rigor=False
               , verbose=False ):
    """
    Barrier
    :param cid: integer, IPC channel ID
    :return: 0 on success, otherwise return negative number.
    """
    _bfm_barrier = WrapFunction( _cosim
                               , 'bfm_barrier'
                               , ctypes.c_int
                               ,[ctypes.c_int])
    return _bfm_barrier( cid )

def bfm_set_verbose( level=0
                   , rigor=False
                   , verbose=False ):
    """
    Set verbose level.
    :param cid: integer, IPC channel ID
    :return: 0 on success, otherwise return negative number.
    """
    global _cosim
    _bfm_set_verbose = WrapFunction( _cosim
                                   , 'bfm_set_verbose'
                                   , ctypes.c_int
                                   ,[ctypes.c_int])
    return _bfm_set_verbose( level )

def bfm_get_verbose( rigor=False
                   , verbose=False ):
    """
    Return verbose level.
    :param cid: integer, IPC channel ID
    :return verbose level
    """
    global _cosim
    _bfm_get_verbose = WrapFunction( _cosim
                                   , 'bfm_get_verbose'
                                   , ctypes.c_int
                                   , None )
    return _bfm_get_verbose()

#-------------------------------------------------------------------------------
# int bfm_write( uint32_t     addr
#              , uint8_t     *data
#              , unsigned int sz
#              , unsigned int length);
def bfm_write( addr
             , data
             , sz=4
             , length=1
             , rigor=False
             , verbose=False ):
    """
    Generate write transaction
    :param addr: starting address to write
    :param data: array (list) holding 8-bit data, which is byte-stream in little-endian
    :param size: number of bytes of each data items, can be 1, 2, 4.
    :param length: number of burst length, should be <=len(data)
    :return: 0 on success
    """
    global _cosim
    if rigor>0:
        if type(data) is not bytearray:
            CosimError(f"{GetFunctionName()} data type error", flush=True)
            return -1
        if sz!=1 and sz!=2 and sz!=4:
            CosimError(f"{GetFunctionName()} size error", flush=True)
            return -1
        if (addr%sz)!=0:
            CosimError(f"{GetFunctionName()} mis-aligned access", flush=True)
            return -1
        if length!=len(data)/sz or length>256:
            CosimError(f"{GetFunctionName()} length error", flush=True)
            return -1
    _bfm_write = WrapFunction( _cosim
                             , 'bfm_write'
                             , ctypes.c_int
                             ,[ctypes.c_uint32
                             , ctypes.POINTER(ctypes.c_ubyte)
                             , ctypes.c_uint
                             , ctypes.c_uint ])
    pdata = (ctypes.c_ubyte * len(data))(*data)
    return _bfm_write(addr, pdata, sz, length)

#-------------------------------------------------------------------------------
# int bfm_read ( uint32_t     addr
#              , uint8_t     *data
#              , unsigned int sz
#              , unsigned int length);
def bfm_read( addr
            , data
            , sz=4
            , length=1
            , rigor=False
            , verbose=False ):
    """
    Generate read transaction
    :param addr: starting address to write
    :param data: array (list) holding 8-bit data, which is byte-stream in little-endian
    :param size: number of bytes of each data items, can be 1, 2, 4.
    :param length: number of burst length, should be <=len(data)
    :return: void
    """
    global _cosim
    if rigor>0:
        if type(data) is not bytearray:
            CosimError(f"{GetFunctionName()} data type error", flush=True)
            return -1
        if sz!=1 and sz!=2 and sz!=4:
            CosimError(f"{GetFunctionName()} size error", flush=True)
            return -1
        if (addr%sz)!=0:
            CosimError(f"{GetFunctionName()} mis-aligned access", flush=True)
            return -1
        if length!=len(data)/sz or length>256:
            CosimError(f"{GetFunctionName()} length error", flush=True)
            return -1
    _bfm_read = WrapFunction( _cosim
                             , 'bfm_read'
                             , ctypes.c_int
                             ,[ctypes.c_uint32
                             , ctypes.POINTER(ctypes.c_ubyte)
                             , ctypes.c_uint
                             , ctypes.c_uint ])
    pdata = (ctypes.c_ubyte * len(data))(*data)
    ret = _bfm_read(addr, pdata, sz, length)
    if ret==0:
        for i in range(sz*length): data[i] = pdata[i]
    return ret

#-------------------------------------------------------------------------------
# int bfm_write_core( int          cid      // IPC channel ID
#                   , uint32_t     trans_id // transaction ID
#                   , uint32_t     addr     // address to write
#                   , uint8_t     *data     // data buffer
#                   , unsigned int sz       // num of bytes for each beat
#                   , unsigned int length   // length of burst, i.e., num of beats
#                   , unsigned int attr);   // user defined attribute
def bfm_write_core( cid
                  , trans_id
                  , addr
                  , data
                  , sz=4
                  , length=1
                  , rigor=False
                  , verbose=False ):
    """
    Generate write transaction
    :param cid: channel id
    :param addr: starting address to write
    :param data: array (list) holding 8-bit data, which is byte-stream in little-endian
    :param size: number of bytes of each data items, can be 1, 2, 4.
    :param length: number of burst length, should be <=len(data)
    :return: 0 on success
    """
    global _cosim
    if rigor>0:
        if type(data) is not bytearray:
            CosimError(f"{GetFunctionName()} data type error", flush=True)
            return -1
        if sz!=1 and sz!=2 and sz!=4:
            CosimError(f"{GetFunctionName()} size error", flush=True)
            return -1
        if (addr%sz)!=0:
            CosimError(f"{GetFunctionName()} mis-aligned access", flush=True)
            return -1
        if length!=len(data)/sz or length>256:
            CosimError(f"{GetFunctionName()} length error", flush=True)
            return -1
    _bfm_write_core = WrapFunction( _cosim
                                  , 'bfm_write_core'
                                  , ctypes.c_int
                                  ,[ctypes.c_int
                                  , ctypes.c_uint32
                                  , ctypes.c_uint32
                                  , ctypes.POINTER(ctypes.c_ubyte)
                                  , ctypes.c_uint
                                  , ctypes.c_uint ])
    pdata = (ctypes.c_ubyte * len(data))(*data)
    return _bfm_write_core(cid, trans_id, addr, pdata, sz, length)

#-------------------------------------------------------------------------------
# int bfm_read_core ( int          cid      // IPC channel ID
#                   , uint32_t     trans_id // transaction ID
#                   , uint32_t     addr     // address to read
#                   , uint8_t     *data     // data buffer
#                   , unsigned int sz       // num of bytes for each beat
#                   , unsigned int length   // length of burst, i.e., num of beats
#                   , unsigned int attr);   // user defined attribute
def bfm_read_core( cid
                 , trans_id
                 , addr
                 , data
                 , sz=4
                 , length=1
                 , rigor=False
                 , verbose=False ):
    """
    Generate read transaction
    :param cid: channel id
    :param addr: starting address to write
    :param data: array (list) holding 8-bit data, which is byte-stream in little-endian
    :param size: number of bytes of each data items, can be 1, 2, 4.
    :param length: number of burst length, should be <=len(data)
    :return: void
    """
    global _cosim
    if rigor>0:
        if type(data) is not bytearray:
            CosimError(f"{GetFunctionName()} data type error", flush=True)
            return -1
        if sz!=1 and sz!=2 and sz!=4:
            CosimError(f"{GetFunctionName()} size error", flush=True)
            return -1
        if (addr%sz)!=0:
            CosimError(f"{GetFunctionName()} mis-aligned access", flush=True)
            return -1
        if length!=len(data)/sz or length>256:
            CosimError(f"{GetFunctionName()} length error", flush=True)
            return -1
    _bfm_read_core = WrapFunction( _cosim
                                 , 'bfm_read_core'
                                 , ctypes.c_int
                                 ,[ctypes.c_int
                                 , ctypes.c_uint32
                                 , ctypes.c_uint32
                                 , ctypes.POINTER(ctypes.c_ubyte)
                                 , ctypes.c_uint
                                 , ctypes.c_uint ])
    pdata = (ctypes.c_ubyte * len(data))(*data)
    ret = _bfm_read_core(cid, trans_id, addr, pdata, sz, length)
    if ret==0:
        for i in range(sz*length): data[i] = pdata[i]
    return ret

#===============================================================================
def mem_test( addr=0x0
            , size=4
            , length=1
            , rigor=False
            , verbose=False):

    Wdata = bytearray([0x00] * size * length)
    Rdata = bytearray([0x00] * size * length)
    for i in range(size*length): Wdata[i] = i+1;

    bfm_write(0x0, Wdata, size, length, rigor=1)
    bfm_read (0x0, Rdata, size, length, rigor=1)

    error = 0
    for i in range(size*length):
        if Wdata[i]!=Rdata[i]: error = error + 1;

    if error>0:
        print(f"{GetFunctionName()} mis-match {error} out of {size*length} bytes")
    else:
        print(f"{GetFunctionName()} OK {size*length} bytes")

#-------------------------------------------------------------------------------
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

    LoadCosimLib(simulator) # mandatory
    bfm_set_verbose(level) # optional
    bfm_open(_cid) # mandatory
    bfm_barrier(_cid) # mandatory

    print("start mem_test");

    mem_test(0x0, 4, 10, rigor=0, verbose=level)
    mem_test(0x0, 2, 10, rigor=0, verbose=level)
    mem_test(0x0, 1, 10, rigor=0, verbose=level)

    bfm_close(_cid)

#===============================================================================
# Revision history:
#
# 2021.08.10: Started        by Ando Ki     (andoki@gmail.com)
#===============================================================================
