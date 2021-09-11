#ifndef COSIM_IPC_LIB_H
#define COSIM_IPC_LIB_H
//----------------------------------------------------------------------------
// Copyright (c) 2021 by Ando Ki.
// All rights are reserved by Ando Ki.
//----------------------------------------------------------------------------
// cosim_ipc_linux.c
//----------------------------------------------------------------------------
#define QPATH         "/tmp"
#define QPERM         0666
#define QSEED         17
#if defined(__LP64__)
#define MTYPE_H2T     1L  // Host to Target
#define MTYPE_T2H     2L  // Target to Host
#else
#define MTYPE_H2T     1  // Host to Target
#define MTYPE_T2H     2  // Target to Host
#endif

//----------------------------------------------------------------------------
struct msgbuf_t {
    long mtype; // which sent this message: 1=host-to-target, 2=target-to-host
    char mtext[PIPE_BUFSIZE];
};

//----------------------------------------------------------------------------
// CHAN_HOST    1  // Host side
// CHAN_TARGET  2  // Target side
#if defined(__LP64__)
static long qdir[MAX_NUM_CHAN] = {
            0L, 0L, 0L, 0L, 0L
          , 0L, 0L, 0L, 0L, 0L
          , 0L, 0L, 0L, 0L, 0L
          , 0L, 0L, 0L, 0L, 0L
};
#else
static long qdir[MAX_NUM_CHAN] = {
            0, 0, 0, 0, 0
          , 0, 0, 0, 0, 0
          , 0, 0, 0, 0, 0
          , 0, 0, 0, 0, 0
};
#endif

//----------------------------------------------------------------------------
char *chn_error_msg_string[] = {
/* 0 */ "nothing wrong",
/* 1 */ "fail to allocate memory for channel handler",
/* 2 */ "invalid side reference",
/* 3 */ "fail to generate message queue key for channel",
/* 4 */ "failed to connect to the message queue",
/* 5 */ "failed to create message queue and find existing queue",
/* 6 */ "failed to close message queue for channel",
/* 7 */ "length exceeds maximum buffer size in sending for channel",
/* 8 */ "non-blocking send return",
/* 9 */ "EINTR is received. retrying..",
/*10 */ "length exceeds maximum buffer size in receiving for channel",
/*11 */ " ",
/*12 */ "channel is not exist yet for barrier",
/*13 */ "unexpected first message from target for barrier",
/*14 */ "unexpected second message from target for barrier",
/*15 */ "host pid mis-match from target for barrier",
/*16 */ "unexpected first message from host for barrier",
/*17 */ "unexpected second message from host for barrier",
/*18 */ "target pid mis-match from host for barrier"
/*19 */ "reserved"
/*20 */ "error message not valid"
};
static int max_error_num = 21;

//----------------------------------------------------------------------------
// Use 'flockfile()/funlockfile()' for exclusive execution of thie function.
static int
cc_printf (const int level, const char *format, ...)
{
    va_list ap;
    int ret = 0;
    va_start (ap, format);
    if (level <= m_verbose) {
        flockfile(stderr);
        fprintf (stderr, "-ipc- ");
        ret = vfprintf (stderr, format, ap);
        fflush(stderr);
        funlockfile(stderr);
    }
    va_end (ap);
    return ret;
}

//----------------------------------------------------------------------------
int
_chn_init(void)
{
     int i;
     for (i=0; i<MAX_NUM_CHAN; i++) {
          #if defined(__LP64__)
          qdir[i] = 0L;
          #else
          qdir[i] = 0;
          #endif
     }
     return 0;
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
// [output]
// !=-1: OK
// -1: something wrong, and 'error_msg_num' will have error number
//----------------------------------------------------------------------------
static void *
_chn_create_connect (int chan_id, int side)
{
    intptr_t   chnhdl;
    int   qkey_id;
    key_t qkey   ; // long
    
    //------------------------------------------------------------------------
    if ((side!=CHAN_HOST)&&(side!=CHAN_TARGET)) {
         error_msg_num[chan_id]=2;
         return (void*)-1L;
    }
    if (side==CHAN_HOST) {
        cc_printf (1, "Creating/Connecting channel id %d for host.\n", chan_id);
    } else {
        cc_printf (1, "Creating/Connecting channel id %d for target.\n", chan_id);
    }
    side_ht[chan_id] = side;

    //------------------------------------------------------------------------
    // message queue path, queue ID
    qkey_id = getuid() + chan_id * QSEED;
    qkey    = ftok(QPATH, qkey_id);
    if (qkey==((key_t)-1)) {
        error_msg_num[chan_id]=3;
        return (void*)-1L;
    }

    cc_printf (1, "Message key is 0x%x, key path is %s, key id is 0x%x: uid=0x%x.\n",
               (unsigned int)qkey, QPATH, qkey_id, getuid());

    //------------------------------------------------------------------------
    // Tries to create or connect to.
    // int msgget(long key, int flags); // returns message descriptor
    //     long key: message buffer ID (used as name,
    //               both processes must use th same key number)
    //     int flags: access permission bits
    chnhdl = msgget (qkey, QPERM | IPC_CREAT | IPC_EXCL);
    if (chnhdl<0) {
        if (errno==EEXIST) {
            cc_printf (1, "Warning in channel! Requested message queue already exists: 0x%x\n", (unsigned int)qkey);
            cc_printf (1, "Try to connect to the existing message queue..\n");
            chnhdl = msgget (qkey, QPERM);
            if (chnhdl<0) {
                error_msg_num[chan_id]=4; return (void*)-1L;
            } else {
                cc_printf (1, "Connected to message queue with channel id %d.\n", chan_id);
            }
        } else {
            error_msg_num[chan_id]=5; return (void*)-1L;
        }
    }

    cc_printf (1, "Allocated message queue with channel handle %d.\n", chnhdl);

    qdir[chan_id] = (long)side;

    error_msg_num[chan_id]=0;
    return (void *)chnhdl;
}

//----------------------------------------------------------------------------
// Tried to connect an existing message
//
// [input]
// chan_id: channel ID
// side:    which side of message (host or target)
// [output]
// !=-1: OK
// -1: something wrong, and 'error_msg_num' will have error number
//----------------------------------------------------------------------------
static void *
_chn_connect (int chan_id, int side)
{
    intptr_t   chnhdl;
    int   qkey_id;
    key_t qkey;

    //------------------------------------------------------------------------
    if ((side!=CHAN_HOST)&&(side!=CHAN_TARGET)) {
         error_msg_num[chan_id]=2;
         return (void*)-1L;
     }
    if (side==CHAN_HOST) {
        cc_printf (1, "Creating/Connecting channel id %d for host.\n", chan_id);
    } else {
        cc_printf (1, "Creating/Connecting channel id %d for target.\n", chan_id);
    }
    side_ht[chan_id] = side;

    //------------------------------------------------------------------------
    // message queue path, queue ID
    qkey_id = getuid() + chan_id * QSEED;
    qkey    = ftok(QPATH, qkey_id);
    if (qkey==((key_t)-1)) {
        error_msg_num[chan_id]=3; return (void*)-1L;
    }

    cc_printf (1, "Message key is 0x%x, key path is %s, key id is 0x%x.\n",
               (unsigned int)qkey, QPATH, qkey_id);

    //------------------------------------------------------------------------
    // Tries to connect to.
    chnhdl = msgget (qkey, QPERM);
    if (chnhdl <0) {
         error_msg_num[chan_id]=4; return (void*)-1L;
    } else {
        cc_printf (1, "Connected to message queue with channel id %d.\n", chan_id);
    }
    
    qdir[chan_id] = (long)side;

    error_msg_num[chan_id]=0;
    return (void*)chnhdl;
}

//----------------------------------------------------------------------------
static int
_chn_close (int chan_id)
{
    intptr_t chnhdl = (intptr_t)chan_handle[chan_id];
    if (msgctl(chnhdl, IPC_RMID, (struct msqid_ds*)0) < 0) {
        error_msg_num[chan_id]=6;
        return 6;
    } else {
        cc_printf (1, "Closed message queue with channel id.\n");
        error_msg_num[chan_id]=0;
        return 0;
    }
}

//----------------------------------------------------------------------------
// [input]
// chan_id:
// buf:
// len:
// nonblock: 0 for blocking, 1 for non-blocking
// [output]
// num of bytes sent. Otherwise, -1
//
// return the num of bytes sent that will be >=0 on successful.
// return < 0 on failure.
//----------------------------------------------------------------------------
//
static int
_chn_send (int chan_id, int len, void *buf, int nonblock)
{
    struct msgbuf_t msg;
    int ret;

    intptr_t chnhdl = (intptr_t)chan_handle[chan_id];
    error_msg_num[chan_id]=0;

    if (len >= PIPE_BUFSIZE) {
        error_msg_num[chan_id]=7;
        return -1;
    }
    msg.mtype = qdir[chan_id];
    memcpy (msg.mtext, buf, len);

    //------------------------------------------------------------------------
    // int msgsnd(int mid, message m, int n, int flag); // return -1 when error
    //     mid: message descriptor
    //     m:   message
    //     n:   num of bytes in the m
    //     flags: 
    while ((ret = msgsnd (chnhdl, &msg, len, (nonblock ? IPC_NOWAIT : 0))) < 0) {
        if (nonblock) {
            if (errno==EAGAIN) {
                // for 'IPC_NOWAIT', it return immediately if no message
                // If IPC_NOWAIT is specified in msgflg and there is in sufficient space available,
                // then the call instead fails with the error EAGAIN.
                return 0;
            } else {
               error_msg_num[chan_id]=8;
               return -1;
            }
        } else {
           // If insufficient space is available in the queue,
           // then the default behavior of msgsnd() is to block until space becomes available.
           //char *errorstr;
           //cc_printf (0, "Error in channel! Blocking receipt error for channel handler %d.\n", chnhdl);
           //errorstr = strerror(errno);
           //if (errorstr) {
           //    cc_printf (0, "Error code is %d, %s.\n", errno, errorstr);
           //} else {
           //    cc_printf (0, "Error code is %d, No error string.\n", errno);
           //}
           //if (errno == EINTR) {
           //    cc_printf (0, "EINTR is received. retrying..\n");
           //} else {
           //    return -1;
           //}
            if (errno!=EINTR) {
                error_msg_num[chan_id]=9;
                return -1;
            }
        }
    }
    return len;
}

//----------------------------------------------------------------------------
// [input]
// chan_id:
// buf:
// len:      num of bytes to be received (be carefule for non-blocking)
// nonblock: 0 for blocking, 1 for non-blocking
// [output]
// num of bytes sent. Otherwise, -1
//
// return the num of bytes received that will be >=0 on successful.
// return < 0 on failure.
static int
_chn_recv (int chan_id, int len, void *buf, int nonblock)
{
    struct msgbuf_t msg;
    int ret;
    long rev_dir;

    intptr_t chnhdl = (intptr_t)chan_handle[chan_id];
    error_msg_num[chan_id]=0;

    if (len >= PIPE_BUFSIZE) {
        error_msg_num[chan_id]=10;
        return -1;
    }
    msg.mtype = qdir[chan_id];
    rev_dir = (qdir[chan_id] == MTYPE_H2T) ? MTYPE_T2H : MTYPE_H2T;

    //------------------------------------------------------------------------
    // ssize_t msgrcv(int mid, message m, int n, int type, int flag); // return -1 when error
    //     mid: message descriptor
    //     m:   message
    //     n:   num of bytes in the m
    //     type: 0 retrieve first message from buf
    //           >0 retrived first message with m.type = type
    //           <0 retrieve first message with m.type < type
    //     flags: 
    while ((ret = msgrcv ((int)chnhdl, &msg, len, rev_dir,
                       (nonblock ? IPC_NOWAIT : 0))) < 0) {
        if (nonblock) {
            if (errno==ENOMSG) {
                // for 'IPC_NOWAIT', it return immediately if no message
                // of the requested type is in the queue.
                // The system call fails with errno set to ENOMSG.
                return 0;
            } else {
               error_msg_num[chan_id]=8;
               return -1;
            }
        } else {
           //#if defined(RIGOR)
           //char *errorstr;
           //cc_printf (0, "Error in channel! Blocking receipt error for channel handler %d.\n", chnhdl);
           //errorstr = strerror(errno);
           //if (errorstr) {
           //    cc_printf (0, "Error code is %d, %s.\n", errno, errorstr);
           //} else {
           //    cc_printf (0, "Error code is %d, No error string.\n", errno);
           //}
           //if (errno == EINTR) {
           //    cc_printf (0, "EINTR is received. retrying..\n");
           //} else {
           //    return -1;
           //}
           //#endif
           if (errno!=EINTR) {
               error_msg_num[chan_id]=9;
               return -1;
           }
        }
    }
    memcpy (buf, msg.mtext, ret);

    return ret;
}

//----------------------------------------------------------------------------
// Revision history:
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//----------------------------------------------------------------------------
#endif
