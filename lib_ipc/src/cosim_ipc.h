#ifndef COSIM_IPC_H
#define COSIM_IPC_H
//----------------------------------------------------------------------------
// Copyright (c) 2021 by Ando Ki.
// All rights are reserved by Ando Ki.
//----------------------------------------------------------------------------
// cosim_ipc.h
//----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

extern int   chn_init          (void);
extern int   chn_create_connect(int chan_id, int host_or_target);
extern int   chn_connect       (int chan_id, int host_or_target);
extern int   chn_close         (int chan_id);
extern int   chn_send          (int chan_id, int bnum, void* buf);
extern int   chn_send_nb       (int chan_id, int bnum, void* buf);
extern int   chn_recv          (int chan_id, int bnum, void* buf);
extern int   chn_recv_nb       (int chan_id, int bnum, void* buf);
extern void* chn_handle        (int chan_id, int *type);
extern int   chn_barrier       (int chan_id);
extern void  chn_set_verbose   (int level);
extern int   chn_get_verbose   (void);
extern int   chn_error_num     (int chan_id);
extern char* chn_error_msg     (int chan_id, int errn);

#define  MAX_NUM_CHAN    20L // Maximum number of channels to be used

#define CHAN_HOST        1L  // Host side
#define CHAN_TARGET      2L  // Target side

#if defined(WIN32)
#define SIGNAL_INTERRUPT    SIGINT   // 2  (Ctrl+C)
#define SIGNAL_TERMINATION  SIGTERM  // 15 (polite version of SIGKILL)
#else
#define SIGNAL_INTERRUPT    SIGINT   // 2 (Ctrl+C)
#define SIGNAL_TERMINATION  SIGQUIT  // 3 (Ctrl+D)
#endif

#ifdef __cplusplus
}
#endif
//----------------------------------------------------------------------------
// Revision history:
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//----------------------------------------------------------------------------
#endif
