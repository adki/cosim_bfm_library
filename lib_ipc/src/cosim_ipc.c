//----------------------------------------------------------------------------
// Copyright (c) 2021 by Ando Ki.
// All rights are reserved by Ando Ki.
//----------------------------------------------------------------------------
// cosim_ipc.c
//----------------------------------------------------------------------------
#if defined(WIN32) // cygwin uses this
#	include <windows.h>
#	include <stdio.h>
#	include <io.h>
#	include <assert.h>
#	include <signal.h>
#	include <inttypes.h>
#else
#	include <stdio.h>
#	include <stdlib.h>
#	include <stdarg.h>
#	include <signal.h>
#	include <assert.h>
#	include <unistd.h>
#	include <string.h>
#	include <errno.h>
#	include <inttypes.h>
#	include <sys/types.h>
#	include <sys/ipc.h>
#	include <sys/msg.h>
#endif
#include "cosim_ipc.h"

//----------------------------------------------------------------------------
#if defined(DEBUG)||defined(RIGOR)
#define PRINTF(format, ...) do { printf("%s(): " format, __func__, ##__VA_ARGS__);\
                                 fflush(stdout); } while (0)
#else
#define PRINTF(...) do { } while (0)
#endif

//----------------------------------------------------------------------------
#define PIPE_BUFSIZE  (1024*4)

//----------------------------------------------------------------------------
static void *chan_handle[MAX_NUM_CHAN] ={
            (void*)-1L, (void*)-1L, (void*)-1L, (void*)-1L, (void*)-1L
          , (void*)-1L, (void*)-1L, (void*)-1L, (void*)-1L, (void*)-1L
          , (void*)-1L, (void*)-1L, (void*)-1L, (void*)-1L, (void*)-1L
          , (void*)-1L, (void*)-1L, (void*)-1L, (void*)-1L, (void*)-1L
};

//----------------------------------------------------------------------------
// it identifies channel type:
// - CHAN_HOST   (1) : it has responsibility to send channel termination message
// - CHAN_TARGET (2)
static int  side_ht[MAX_NUM_CHAN] = {
            0, 0, 0, 0, 0
          , 0, 0, 0, 0, 0
          , 0, 0, 0, 0, 0
          , 0, 0, 0, 0, 0
};

//----------------------------------------------------------------------------
static int error_msg_num[MAX_NUM_CHAN] = {
            0, 0, 0, 0, 0
          , 0, 0, 0, 0, 0
          , 0, 0, 0, 0, 0
          , 0, 0, 0, 0, 0
};

//----------------------------------------------------------------------------
static int  m_init_done = 0;
static int  m_verbose = 0; // vervose level

//----------------------------------------------------------------------------
#if defined(WIN32)
    #include "cosim_ipc_win.c"
#else
    #include "cosim_ipc_linux.c"
#endif

//----------------------------------------------------------------------------
// It is called after exit().
void chn_exit(void)
{
     chn_close(-1);
}

//----------------------------------------------------------------------------
int
chn_init(void)
{
    int i;
    m_init_done = 0;
  //m_verbose = 0; // set by calling chn_set_verbose (int level)
    for (i=0; i<MAX_NUM_CHAN; i++) {
         chan_handle[i] = (void*)-1L;
         side_ht[i] = 0;
         error_msg_num[i] = 0;
    }
    int ret = _chn_init();
    if (ret) { return ret; }
    atexit(chn_exit);
    m_init_done = 1;
    return 0;
}

//----------------------------------------------------------------------------
// return numer that identifies error message.
int
chn_error_num(int chan_id)
{
    return ((error_msg_num!=NULL)&&(chan_id<MAX_NUM_CHAN)) ? error_msg_num[chan_id]
                                                           : 20;
}
//----------------------------------------------------------------------------
// return string pointer of reason of error.
// when 'enum' is 0, use the latest 'error_msg_num'.
char*
chn_error_msg(int chan_id, int err)
{
      if ((error_msg_num==(int*)NULL)||
          (chan_id>=MAX_NUM_CHAN)) return chn_error_msg_string[20];
      if (err>=max_error_num) return NULL;
      if (err==0) return chn_error_msg_string[error_msg_num[chan_id]];
      else return chn_error_msg_string[err];
}

//----------------------------------------------------------------------------
void
chn_set_verbose (int level)
{
    m_verbose = level;
}

int
chn_get_verbose (void)
{
    return m_verbose;
}

//----------------------------------------------------------------------------
// Return channel handle if any.
// Otherwise NULL.
void *
chn_handle(int chan_id, int *type)
{
    if (chan_id>=MAX_NUM_CHAN) return (void*)-1L;
    if (chan_handle[chan_id]==(void*)-1L) return (void*)-1L;
    if (type) *type = side_ht[chan_id];
    return chan_handle[chan_id];
}

//----------------------------------------------------------------------------
// Channel Creation/Connect
//
// The first arriving process create a message queue.
// The later arriving process met "EEXIST" error and then
// tried to connect.
//
// [input]
// chan_id: channel ID
// side:    which side of message (host or target)
// [return]
// 0: OK
// -1: something wrong, and 'error_msg_num' will have error number
//----------------------------------------------------------------------------
int
chn_create_connect (int chan_id, int side)
{
    int ret = 0;
    void *hdl;
    if (m_init_done==0) {
        ret = chn_init();
    }
    if (ret) return ret;
    if (chan_id>=MAX_NUM_CHAN) return -1;
    if ((hdl=_chn_create_connect(chan_id, side))==(void*)-1L) return -1;
    chan_handle[chan_id] = hdl;
    return 0;
}

//----------------------------------------------------------------------------
// Tried to connect an existing message
//
// [input]
// chan_id: channel ID
// side:    which side of message (host or target)
// [return]
// 0: OK
// -1: something wrong, and 'error_msg_num' will have error number
//----------------------------------------------------------------------------
int
chn_connect (int chan_id, int side)
{
    int ret = 0;
    void *hdl;
    if (m_init_done==0) {
        ret = chn_init();
    }
    if (ret) return ret;
    if ((chan_id>=MAX_NUM_CHAN)||(chan_handle[chan_id]==(void*)-1L)) return -1;
    if ((hdl=_chn_connect (chan_id, side))==(void*)-1L) return -1;
    return 0;
}

//----------------------------------------------------------------------------
int
chn_close (int chan_id)
{
    if (chan_id>=0) {
        if (chan_id>=MAX_NUM_CHAN) return -1;
        if (chan_handle[chan_id]==(void*)-1L) return -1;
        _chn_close(chan_id);
    } else {
        int i;
        for (i=0; i<MAX_NUM_CHAN; i++) {
             if (chan_handle[i]!=(void*)-1L) _chn_close(i);
        }
    }
    return 0;
}

//----------------------------------------------------------------------------
// return 'len' on successful.
// return < 0 on failure.
// use 'chn_error_num()' to get error message identifier.
int
chn_send (int chan_id, int len, void *buf)
{
    if (chan_id>=MAX_NUM_CHAN) return -1;
    if (chan_handle[chan_id]==(void*)-1L) return -1;
    return _chn_send (chan_id, len, buf, 0);
}

//----------------------------------------------------------------------------
// return the num of bytes sent that will be >=0 on successful.
// return < 0 on failure.
// use 'chn_error_num()' to get error message identifier.
int
chn_send_nb (int chan_id, int len, void *buf)
{
    if (chan_id>=MAX_NUM_CHAN) return -1;
    if (chan_handle[chan_id]==(void*)-1L) return -1;
    return _chn_send (chan_id, len, buf, 1);
}

//----------------------------------------------------------------------------
// it blocked until any bytes received.
// return the num of bytes received that will be >0 on successful.
// return < 0 on failure.
// use 'chn_error_num()' to get error message identifier.
int
chn_recv (int chan_id, int len, void *buf)
{
    if (chan_id>=MAX_NUM_CHAN) return -1;
    if (chan_handle[chan_id]==(void*)-1L) return -1;
    return _chn_recv (chan_id, len, (char*)buf, 0);
}

//----------------------------------------------------------------------------
// return the num of bytes received that will be >=0 on successful.
// return < 0 on failure.
// use 'chn_error_num()' to get error message identifier.
int
chn_recv_nb (int chan_id, int len, void *buf)
{
    if (chan_id>=MAX_NUM_CHAN) return -1;
    if (chan_handle[chan_id]==(void*)-1L) return -1;
    return _chn_recv (chan_id, len, (char*)buf, 1);
}

//----------------------------------------------------------------------------
// [input]
// [return]
// 0: OK
// -1: return error message number on failure
//
//        | HOST                      | TARGET
// step 1:  sends its pid to target   | wait message from host
// step 2:  waits message from target | sends its pid to target
//
// step 3:  sends target pid to target| wait message from host
// step 4:  waits message from target | sends host pid to target
int
chn_barrier(int chan_id)
{
    if (chan_id>=MAX_NUM_CHAN) return -1;
    if (chan_handle[chan_id]==(void*)-1L) return -1;
    char bmsgTx[64];
    char bmsgRx[64];
    pid_t  pid;
    static pid_t  pid_host   = (pid_t)-1;
    static pid_t  pid_target = (pid_t)-1;
    if((side_ht[chan_id]!=CHAN_HOST)&&(side_ht[chan_id]!=CHAN_TARGET)) {
        error_msg_num[chan_id] = 12;
        return error_msg_num[chan_id];
    }
    if (side_ht[chan_id]==CHAN_HOST) {
        //--------------------------------------------------------------------
        // host sends its pid to target
        pid_host = getpid();
        sprintf(&bmsgTx[0], "HOST  ");
        sprintf(&bmsgTx[8], "%08x", pid_host);
        if (chn_send(chan_id, 24, bmsgTx)!=24) {
            error_msg_num[chan_id] = 13;
            return error_msg_num[chan_id];
        }
        //--------------------------------------------------------------------
        // wait message from target
        if (chn_recv(chan_id, 24, bmsgRx)<0) {
            error_msg_num[chan_id] = 13;
            return error_msg_num[chan_id];
        }
        if (strncmp(bmsgRx, "TARGET", 6)) {
            error_msg_num[chan_id] = 13;
            return error_msg_num[chan_id];
        }
        pid_target = (int)strtoul(&bmsgRx[8], NULL, 16);
        //--------------------------------------------------------------------
        cc_printf(1, "HOST: my_pid=0x%08x target_pid=0x%08x\n", pid_host, pid_target);
        //--------------------------------------------------------------------
        #if defined(RIGOR)
            // host sends target_pid to target
            sprintf(&bmsgTx[0], "HOST  ");
            sprintf(&bmsgTx[8], "%08x", pid_target);
            if (chn_send(chan_id, 24, bmsgTx)!=24) {
                error_msg_num[chan_id] = 14;
                return error_msg_num[chan_id];
            }
            //--------------------------------------------------------------------
            // wait message from target
            if (chn_recv(chan_id, 24, bmsgRx)<0) {
                error_msg_num[chan_id] = 14;
                return error_msg_num[chan_id];
            }
            if (strncmp(bmsgRx, "TARGET", 6)) {
                error_msg_num[chan_id] = 14;
                return error_msg_num[chan_id];
            }
            //--------------------------------------------------------------------
            // check host pid that target has
            pid = (int)strtoul(&bmsgRx[8], NULL, 24);
            if (pid_host!=pid) {
                error_msg_num[chan_id] = 15;
                return error_msg_num[chan_id];
            }
        #endif
        cc_printf(1,"barrier done: channel %d for host.\n", chan_id);
    } else {
        //--------------------------------------------------------------------
        // wait message from host
        if (chn_recv(chan_id, 24, bmsgRx)<0) {
            error_msg_num[chan_id] = 16;
            return error_msg_num[chan_id];
        }
        if (strncmp(bmsgRx, "HOST  ", 6)) {
            error_msg_num[chan_id] = 16;
            return error_msg_num[chan_id];
        }
        pid_host = (pid_t)strtoul(&bmsgRx[8], NULL, 16);
        //--------------------------------------------------------------------
        // target sends its pid to host
        pid_target = getpid();
        sprintf(&bmsgTx[0], "TARGET");
        sprintf(&bmsgTx[8], "%08x", pid_target);
        if (chn_send(chan_id, 24, bmsgTx)!=24) {
            error_msg_num[chan_id] = 17;
            return error_msg_num[chan_id];
        }
        //--------------------------------------------------------------------
        cc_printf(1, "TARGET: my_pid=0x%08x host_pid=0x%08x\n", pid_target, pid_host);
        //--------------------------------------------------------------------
        #if defined(RIGOR)
            // target sends host_pid to host
            sprintf(&bmsgTx[0], "TARGET");
            sprintf(&bmsgTx[8], "%08x", pid_host);
            if (chn_send(chan_id, 24, bmsgTx)!=24) {
                error_msg_num[chan_id] = 17;
                return error_msg_num[chan_id];
            }
            //--------------------------------------------------------------------
            // wait message from host
            if (chn_recv(chan_id, 24, bmsgRx)<0) {
                error_msg_num[chan_id] = 17;
                return error_msg_num[chan_id];
            }
            if (strncmp(bmsgRx, "HOST  ", 6)) {
                error_msg_num[chan_id] = 17;
                return error_msg_num[chan_id];
            }
            //--------------------------------------------------------------------
            pid = (int)strtoul(&bmsgRx[8], NULL, 16);
            if (pid_target!=pid) {
                error_msg_num[chan_id] = 18;
                return error_msg_num[chan_id];
            }
        #endif
        cc_printf(1,"barrier done: channel %d for target.\n", chan_id);
    }
    return 0;
}

#undef PRINTF
//----------------------------------------------------------------------------
// Revision history:
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//----------------------------------------------------------------------------
