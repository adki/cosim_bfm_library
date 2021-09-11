//------------------------------------------------------------------------------
// Copyright (c) 2021 by Ando Ki.
// All rights reserved by Ando Ki.
//------------------------------------------------------------------------------
// cosim_bfm_vpi.c
//------------------------------------------------------------------------------
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32)  // cygwin use this
#       include <windows.h>
#       include <io.h>
#       include <process.h>
#else
#       include <unistd.h>
#endif
#include "vpi_user.h"
#include "cosim_ipc.h"
#include "cosim_bfm_packet.h"

//------------------------------------------------------------------------------
static int m_verbose = 0;
static int m_cid_max = MAX_NUM_CHAN; // see MAX_NUM_CHAN @ ipc_lib/ipc_lib.h

//------------------------------------------------------------------------------
void cosim_control(PLI_INT32 operation) {
#if defined(ncsim)||defined(verilogXL)
     switch (operation) {
     case vpiStop  : tf_dostop();   break; // cause $stop()
     case vpiFinish: tf_dofinish(); break; // cause $finish()
     default: vpi_printf("ERROR: %d for vpi_control() is not supported yet!\n", operation);
     }
#else
     vpi_control(operation, 0);
#endif
  if (operation==vpiFinish) {
      bfm_packet_t pkt;
      pkt.cmd_type   = COSIM_CMD_TERM_REQ;
      pkt.cmd_size   = 0;
      pkt.cmd_length = 0;
      pkt.cmd_ack    = COSIM_CMD_ACK_ERR;
      pkt.attr       = 0;
      pkt.addr       = 0;
      int len = (int)sizeof(pkt);
      int cid;
      int dir;
      for (cid=0; cid<m_cid_max; cid++) {
          if (chn_handle(cid, &dir)!=(void*)(void*)-1L) {
             if (dir==CHAN_HOST) {
             if (chn_send(cid, len, (void*)&pkt)==len) {
                 chn_recv(cid, len, (void*)&pkt);
                 if (m_verbose>1) {
                     vpi_printf("cosim_ipc_close cid=%d.\n", cid);
                 }
             }
             }
          }
      }
  }
}

//------------------------------------------------------------------------------
PLI_INT32   cosim_ipc_rcv_Calltf   (PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_rcv_Compiletf(PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_rcv_Sizetf   (PLI_BYTE8 *user_data);

PLI_INT32   cosim_ipc_snd_Calltf   (PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_snd_Compiletf(PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_snd_Sizetf   (PLI_BYTE8 *user_data);

PLI_INT32   cosim_ipc_get_Calltf   (PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_get_Compiletf(PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_get_Sizetf   (PLI_BYTE8 *user_data);

PLI_INT32   cosim_ipc_put_Calltf   (PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_put_Compiletf(PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_put_Sizetf   (PLI_BYTE8 *user_data);

PLI_INT32   cosim_ipc_set_verbose_Calltf   (PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_set_verbose_Compiletf(PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_set_verbose_Sizetf   (PLI_BYTE8 *user_data);

PLI_INT32   cosim_ipc_open_Calltf   (PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_open_Compiletf(PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_open_Sizetf   (PLI_BYTE8 *user_data);

PLI_INT32   cosim_ipc_close_Calltf   (PLI_BYTE8 *user_data); // called by $cosim_ipc_close()
PLI_INT32   cosim_ipc_close_Compiletf(PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_close_Callback (s_cb_data *callback_data); // called at the event of cbEndOfSimulation
PLI_INT32   cosim_ipc_close_Sizetf   (PLI_BYTE8 *user_data);

PLI_INT32   cosim_ipc_barrier_Calltf   (PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_barrier_Compiletf(PLI_BYTE8 *user_data);
PLI_INT32   cosim_ipc_barrier_Sizetf   (PLI_BYTE8 *user_data);

//------------------------------------------------------------------------------
void cosim_ipc_register() {
    s_vpi_systf_data tf_data;
    s_cb_data        cb_data_s;
    vpiHandle        callback_handle;

    tf_data.type        = vpiSysFunc;
    tf_data.sysfunctype = vpiSizedFunc;
    tf_data.tfname      = "$cosim_ipc_rcv";
    tf_data.calltf      = cosim_ipc_rcv_Calltf;
    tf_data.compiletf   = cosim_ipc_rcv_Compiletf;
    tf_data.sizetf      = cosim_ipc_rcv_Sizetf;
    tf_data.user_data   = NULL;
    vpi_register_systf(&tf_data);

    tf_data.type        = vpiSysFunc;
    tf_data.sysfunctype = vpiSizedFunc;
    tf_data.tfname      = "$cosim_ipc_snd";
    tf_data.calltf      = cosim_ipc_snd_Calltf;
    tf_data.compiletf   = cosim_ipc_snd_Compiletf;
    tf_data.sizetf      = cosim_ipc_snd_Sizetf;
    tf_data.user_data   = NULL;
    vpi_register_systf(&tf_data);

    tf_data.type        = vpiSysFunc;
    tf_data.sysfunctype = vpiSizedFunc;
    tf_data.tfname      = "$cosim_ipc_get";
    tf_data.calltf      = cosim_ipc_get_Calltf;
    tf_data.compiletf   = cosim_ipc_get_Compiletf;
    tf_data.sizetf      = cosim_ipc_get_Sizetf;
    tf_data.user_data   = NULL;
    vpi_register_systf(&tf_data);

    tf_data.type        = vpiSysFunc;
    tf_data.sysfunctype = vpiSizedFunc;
    tf_data.tfname      = "$cosim_ipc_put";
    tf_data.calltf      = cosim_ipc_put_Calltf;
    tf_data.compiletf   = cosim_ipc_put_Compiletf;
    tf_data.sizetf      = cosim_ipc_put_Sizetf;
    tf_data.user_data   = NULL;
    vpi_register_systf(&tf_data);

    tf_data.type        = vpiSysFunc;
    tf_data.sysfunctype = vpiSizedFunc; //vpiSysFuncSized;
    tf_data.tfname      = "$cosim_ipc_set_verbose";
    tf_data.calltf      = cosim_ipc_set_verbose_Calltf;
    tf_data.compiletf   = cosim_ipc_set_verbose_Compiletf;
    tf_data.sizetf      = cosim_ipc_set_verbose_Sizetf;
    tf_data.user_data   = NULL;
    vpi_register_systf(&tf_data);

    tf_data.type        = vpiSysFunc;
    tf_data.sysfunctype = vpiSizedFunc;
    tf_data.tfname      = "$cosim_ipc_open";
    tf_data.calltf      = cosim_ipc_open_Calltf;
    tf_data.compiletf   = cosim_ipc_open_Compiletf;
    tf_data.sizetf      = cosim_ipc_open_Sizetf;
    tf_data.user_data   = NULL;
    vpi_register_systf(&tf_data);

    tf_data.type        = vpiSysFunc;
    tf_data.sysfunctype = vpiSizedFunc;
    tf_data.tfname      = "$cosim_ipc_close";
    tf_data.calltf      = cosim_ipc_close_Calltf;
    tf_data.compiletf   = cosim_ipc_close_Compiletf;
    tf_data.sizetf      = cosim_ipc_close_Sizetf;
    tf_data.user_data   = NULL;
    vpi_register_systf(&tf_data);

    tf_data.type        = vpiSysFunc;
    tf_data.sysfunctype = vpiSizedFunc;
    tf_data.tfname      = "$cosim_ipc_barrier";
    tf_data.calltf      = cosim_ipc_barrier_Calltf;
    tf_data.compiletf   = cosim_ipc_barrier_Compiletf;
    tf_data.sizetf      = cosim_ipc_barrier_Sizetf;
    tf_data.user_data   = NULL;
    vpi_register_systf(&tf_data);

    cb_data_s.reason    = cbEndOfSimulation;
    cb_data_s.cb_rtn    = cosim_ipc_close_Callback;
    cb_data_s.obj       = NULL;
    cb_data_s.time      = NULL;
    cb_data_s.value     = NULL;
    cb_data_s.user_data = NULL;
    callback_handle     = vpi_register_cb(&cb_data_s);
    //vpi_free_object(callback_handle);
}

//------------------------------------------------------------------------------
// Barrier on the chanel id
// $cosim_ipc_barrier;   ==> $cosim_ipc_barrier(0);
// $cosim_ipc_barrier(); ==> $cosim_ipc_barrier(0);
// $cosim_ipc_barrier(cid);
//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_barrier_Calltf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator;
  vpiHandle cidH;
  s_vpi_value value;
  int cid;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) {
      cid = 0;
  } else {
      cidH         = vpi_scan(arg_iterator); // chan_id
      value.format = vpiIntVal;
      vpi_get_value(cidH, &value);
      cid = (int)value.value.integer;
      vpi_free_object(arg_iterator);
  }

  if ((cid<0)||(cid>=m_cid_max)) {
       vpi_printf("ERROR: $cosim_ipc_barrier error: unknown channel id: %d\n", cid);
       cosim_control(vpiStop);
  }
  if (chn_handle(cid, 0)==(void*)-1L) {
       vpi_printf("ERROR: $cosim_ipc_barrier error: channel id not opened yet: %d\n", cid);
       cosim_control(vpiStop);
  }

  if (m_verbose>0) {
      vpi_printf("[proc:%ld] barrier a ipc of cid=%d\n",
                  (long)getpid(), cid); vpi_flush();
  }
  if (chn_barrier(cid)<0) {
       vpi_printf("ERROR: $cosim_ipc_barrier error: channel id: %d %s\n", cid, chn_error_msg(cid, 0));
       cosim_control(vpiStop);
  }
  value.format = vpiIntVal;
  value.value.integer = 0;
  vpi_put_value(systf_handle, &value, NULL, vpiNoDelay);

  if (m_verbose>0) {
      vpi_printf("[proc:%ld] barrier cid=%d\n", (long)getpid(), cid);
      vpi_flush();
  }
  return(0);
}

//------------------------------------------------------------------------------
// $cosim_ipc_barrier(channel_id);
PLI_INT32 cosim_ipc_barrier_Compiletf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator, arg_handle;
  PLI_INT32 tfarg_type;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) { /* no argument */
     return(0);
  }
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle==NULL) {
       vpi_printf("ERROR: $cosim_ipc_barrier must have only one argument.\n");
       cosim_control(vpiFinish);
  }
  tfarg_type = vpi_get(vpiType, arg_handle);
  if ((tfarg_type!=vpiIntegerVar)&&
      (tfarg_type!=vpiParameter)&&
      (tfarg_type!=vpiReg)&&
      (tfarg_type!=vpiConstant)) {
       vpi_printf("ERROR: $cosim_ipc_barrier must have integer argument, but %d.\n",
                   tfarg_type);
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  arg_handle = vpi_scan(arg_iterator);
  if (arg_handle!=NULL) {
       vpi_printf("ERROR: $cosim_ipc_barrier must have one arguments.\n");
       cosim_control(vpiFinish);
  }
  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_barrier_Sizetf(PLI_BYTE8 *user_data) {
  return(32); /* $cosim_ipc_barrier() returns 32-bit */
}

//------------------------------------------------------------------------------
// create and open IPC channel for HDL side for BFM (open IPC as CHAN_TARGET)
// $cosim_ipc_open;   ==> $cosim_ipc_open(0);
// $cosim_ipc_open(); ==> $cosim_ipc_open(0);
// $cosim_ipc_open(cid);
//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_open_Calltf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator;
  vpiHandle cidH;
  s_vpi_value value;
  int cid;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) {
      cid = 0;
  } else {
      cidH         = vpi_scan(arg_iterator); // chan_id
      value.format = vpiIntVal;
      vpi_get_value(cidH, &value);
      cid = (int)value.value.integer;
      vpi_free_object(arg_iterator);
  }

  if ((cid<0)||(cid>=m_cid_max)) {
       vpi_printf("ERROR: $cosim_ipc_open error: unknown channel id: %d\n", cid);
       cosim_control(vpiStop);
  }
  if (chn_handle(cid, 0)!=(void*)-1L) {
       vpi_printf("ERROR: $cosim_ipc_open error: channel id in use: %d\n", cid);
       cosim_control(vpiStop);
  }

  if (m_verbose>0) {
      vpi_printf("[proc:%ld] opening a ipc of cid=%d\n",
                  (long)getpid(), cid); vpi_flush();
  }
  if (chn_create_connect(cid, CHAN_TARGET)<0) {
       vpi_printf("ERROR: $cosim_ipc_open error: channel id: %d %s\n", cid, chn_error_msg(cid, 0));
       cosim_control(vpiStop);
  }
  value.format = vpiIntVal;
  value.value.integer = 0;
  vpi_put_value(systf_handle, &value, NULL, vpiNoDelay);

  if (m_verbose>0) {
      vpi_printf("[proc:%ld] connected cid=%d\n", (long)getpid(), cid);
      vpi_flush();
  }
  return(0);
}

//------------------------------------------------------------------------------
// $cosim_ipc_open(channel_id);
PLI_INT32 cosim_ipc_open_Compiletf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator, arg_handle;
  PLI_INT32 tfarg_type;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) { /* no argument */
     return(0);
  }
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle==NULL) {
       vpi_printf("ERROR: $cosim_ipc_open must have only one argument.\n");
       cosim_control(vpiFinish);
  }
  tfarg_type = vpi_get(vpiType, arg_handle);
  if ((tfarg_type!=vpiIntegerVar)&&
      (tfarg_type!=vpiParameter)&&
      (tfarg_type!=vpiReg)&&
      (tfarg_type!=vpiConstant)) {
       vpi_printf("ERROR: $cosim_ipc_open must have integer argument, but %d.\n",
                   tfarg_type);
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  arg_handle = vpi_scan(arg_iterator);
  if (arg_handle!=NULL) {
       vpi_printf("ERROR: $cosim_ipc_open must have one arguments.\n");
       cosim_control(vpiFinish);
  }
  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_open_Sizetf(PLI_BYTE8 *user_data) {
  return(32); /* $cosim_ipc_open() returns 32-bit */
}

//------------------------------------------------------------------------------
// close IPC channel
// $cosim_ipc_close; ==> $cosim_ipc_close(0);
// $cosim_ipc_close(); ==> $cosim_ipc_close(0);
// $cosim_ipc_close(cid);
//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_close_Calltf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator;
  vpiHandle cidH;
  s_vpi_value value;
  int cid;
  bfm_packet_t pkt;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) {
      cid = 0;
  } else {
    cidH         = vpi_scan(arg_iterator); // chan_id
    value.format = vpiIntVal;
    vpi_get_value(cidH, &value);
    cid = (int)value.value.integer;
    vpi_free_object(arg_iterator);
  }

  if ((cid>=m_cid_max)||(chn_handle(cid, 0)==(void*)-1L)) {
       vpi_printf("ERROR: $cosim_ipc_close error: unknown channel id: %d\n", cid);
       cosim_control(vpiStop);
  }

  pkt.cmd_type   = COSIM_CMD_TERM_REQ;
  pkt.cmd_size   = 0;
  pkt.cmd_length = 0;
  pkt.cmd_ack    = COSIM_CMD_ACK_ERR;
  pkt.attr       = 0;
  pkt.addr       = 0;

  int len = (int)sizeof(pkt);
  if (chn_send(cid, len, (void*)&pkt)!=len) {
      s_vpi_time time;
      time.type = vpiScaledRealTime;
      vpi_get_time(NULL, &time);
      vpi_printf("ERROR: something wrong while put packet: %s\n", chn_error_msg(cid, 0));
      cosim_control(vpiStop);
  }

  if (chn_close(cid)) {
       vpi_printf("ERROR: $cosim_ipc_close error: %s\n", chn_error_msg(cid, 0));
       cosim_control(vpiStop);
  }

  if (m_verbose>0) {
      vpi_printf("cosim_ipc_closing cid=%d.\n", cid);
  }

  value.format = vpiIntVal;
  value.value.integer = 0;
  vpi_put_value(systf_handle, &value, NULL, vpiNoDelay);

  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_close_Compiletf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator, arg_handle;
  PLI_INT32 tfarg_type;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) { /* no argument */
     return(0);
  }
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle==NULL) {
       vpi_printf("ERROR: $cosim_ipc_close must have only one argument if any.\n");
       cosim_control(vpiFinish);
  }
  tfarg_type = vpi_get(vpiType, arg_handle);
  #if !defined(iverilog)
  if (tfarg_type==vpiOperation) {
     if (vpiNullOp!=vpi_get(vpiOpType,arg_handle)) {
        // deal with empty ()
        vpi_printf("ERROR: $cosim_ipc_close must have only one argument if any.\n");
        cosim_control(vpiFinish);
     }
  } else {
     if ((tfarg_type!=vpiIntegerVar)&&
         (tfarg_type!=vpiParameter)&&
         (tfarg_type!=vpiReg)&&
         (tfarg_type!=vpiConstant)) {
          vpi_printf("ERROR: $cosim_ipc_close must have integer argument, but %d.\n",
                      tfarg_type);
          vpi_free_object(arg_iterator);
          cosim_control(vpiFinish);
     }
  }
  #endif
  arg_handle = vpi_scan(arg_iterator);
  if (arg_handle!=NULL) {
       vpi_printf("ERROR: $cosim_ipc_close must have one arguments.\n");
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  return(0);
}

//------------------------------------------------------------------------------
// It will be called at the end of simulation by default.
// So, it cannot have argument.
PLI_INT32 cosim_ipc_close_Callback(s_cb_data *callback_data) {
  int cid;
  int len;
  int dir; // direction

  bfm_packet_t pkt;
  pkt.cmd_type   = COSIM_CMD_TERM_REQ;
  pkt.cmd_size   = 0;
  pkt.cmd_length = 0;
  pkt.cmd_ack    = COSIM_CMD_ACK_ERR;
  pkt.attr       = 0;
  pkt.addr       = 0;
  len = (int)sizeof(pkt);
  for (cid=0; cid<m_cid_max; cid++) {
       if (chn_handle(cid, &dir)!=(void*)(void*)-1L) {
          if (dir==CHAN_HOST) {
          if (chn_send(cid, len, (void*)&pkt)==len) {
              chn_recv(cid, len, (void*)&pkt);
              if (m_verbose>1) {
                  vpi_printf("cosim_ipc_close cid=%d.\n", cid);
              }
          }
          }
       }
       if (chn_handle(cid, &dir)!=(void*)-1L) {
           chn_close(cid);
           if (m_verbose>0) {
               vpi_printf("cosim_ipc_close cid=%d.\n", cid);
           }
       }
  }
  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_close_Sizetf(PLI_BYTE8 *user_data) {
  return(32); /* $cosim_ipc_close() returns 32-bit */
}

//------------------------------------------------------------------------------
// It tries to get data.
// It return num of bytes received.
// $cosim_ipc_rcv(cid       // channel id
//               ,bnum      // num of bytes received
//               ,pkt_data  // array
//               );
//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_rcv_Calltf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator;
  vpiHandle cidH, bnuH, datH;
  s_vpi_value value;
  int    n, m;
  int    cid;
  uint32_t    bnum;
  uint8_t pkt[1024];

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  cidH         = vpi_scan(arg_iterator);
  bnuH         = vpi_scan(arg_iterator);
  datH         = vpi_scan(arg_iterator);
  vpi_free_object(arg_iterator);

  value.format = vpiIntVal;
  vpi_get_value(cidH, &value);
  cid = (int)value.value.integer;

  bnum = chn_recv_nb(cid, 1024, (void*)pkt);
  if (bnum>0) {
      n = vpi_get(vpiSize, datH);
      if (n<bnum) {
         vpi_printf("ERROR: $cosim_ipc_rcv data element.\n");
         cosim_control(vpiFinish);
         return(0);
      }

      n = bnum;
      for (m=0; m<n; m++) {
           vpiHandle eleH = vpi_handle_by_index(datH, m);
           if (eleH==NULL) {
               vpi_printf("ERROR: $cosim_ipc_rcv data element: %d\n", m);
               cosim_control(vpiFinish);
               return(0);
           }
           value.format = vpiIntVal;
           value.value.integer = (PLI_UINT32)(pkt[m]);
           vpi_put_value(eleH, &value, NULL, vpiNoDelay);
      }
  }

  value.format        = vpiIntVal;
  value.value.integer = (PLI_UINT32)bnum;
  vpi_put_value(bnuH, &value, NULL, vpiNoDelay);

  value.format = vpiIntVal;
  value.value.integer = (PLI_UINT32)bnum;
  vpi_put_value(systf_handle, &value, NULL, vpiNoDelay);

  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_rcv_Compiletf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator, arg_handle;
  PLI_INT32 tfarg_type;
  int n;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) { /* no argument */
     vpi_printf("ERROR: $cosim_ipc_rcv must have 3 arguments.\n");
     cosim_control(vpiFinish);
     return(0);
  }
  for (n=0; n<2; n++) { // cid, bnum
      arg_handle   = vpi_scan(arg_iterator);
      if (arg_handle==NULL) {
           vpi_printf("ERROR: $cosim_ipc_rcv must have 3 arguments.\n");
           cosim_control(vpiFinish);
      }
      tfarg_type = vpi_get(vpiType, arg_handle);
      if ((tfarg_type!=vpiIntegerVar)&&
          (tfarg_type!=vpiParameter)&&
          (tfarg_type!=vpiReg)&&
          (tfarg_type!=vpiConstant)) {
           vpi_printf("ERROR: $cosim_ipc_rcv must have integer argument of %d-th, but %d.\n",
                       (n+1), tfarg_type);
           vpi_free_object(arg_iterator);
           cosim_control(vpiFinish);
      }
  }
  // data[]
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle==NULL) {
       vpi_printf("ERROR: $cosim_ipc_rcv must have 3 arguments.\n");
       cosim_control(vpiFinish);
  }
  if (!vpi_get(vpiArray, arg_handle)) { // check if it is array
       vpi_printf("ERROR: $cosim_ipc_rcv data must be array\n");
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  // check for extra
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle!=NULL) {
       vpi_printf("ERROR: $cosim_ipc_rcv must have 3 arguments.\n");
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_rcv_Sizetf(PLI_BYTE8 *user_data) {
  return(32); /* $cosim_ipc_rcv() returns 32-bit */
}

//------------------------------------------------------------------------------
// It returns how many bytes are sent.
//
// $cosim_ipc_snd(cid      // channel id
//               ,bnum     // num of bytes in 'pkt_data[]'
//               ,pkt_data // data buffer
//               );
//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_snd_Calltf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator;
  vpiHandle cidH, bnuH, datH;
  s_vpi_value value;
  int    cid;
  int    n;
  uint32_t bnum;
  uint8_t pkt[1024];

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  cidH         = vpi_scan(arg_iterator);
  bnuH         = vpi_scan(arg_iterator);
  datH         = vpi_scan(arg_iterator);
  vpi_free_object(arg_iterator);

  value.format = vpiIntVal;
  vpi_get_value(cidH,  &value); cid  = (int)value.value.integer;
  vpi_get_value(bnuH,  &value); bnum = (unsigned int)value.value.integer;

  n = vpi_get(vpiSize, datH);
  if (n<bnum) { // num of bytes to move
     vpi_printf("ERROR: $cosim_ipc_snd data element.\n");
     cosim_control(vpiFinish);
     return(0);
  }
  for (n=0; n<bnum; n++) {
       vpiHandle eleH = vpi_handle_by_index(datH, n);
       if (eleH==NULL) {
           vpi_printf("ERROR: $cosim_ipc_snd data element: %d\n", n);
           cosim_control(vpiFinish);
           return(0);
       }
       value.format = vpiIntVal;
       vpi_get_value(eleH, &value);
       pkt[n] = (value.value.integer&0xFF);
  }

  if (chn_send(cid, bnum, (void*)pkt)!=bnum) {
      s_vpi_time time;
      time.type = vpiScaledRealTime;
      vpi_get_time(NULL, &time);
      vpi_printf("ERROR: something wrong while snd packet: %s\n", chn_error_msg(cid, 0));
      cosim_control(vpiStop);
      value.format = vpiIntVal;
      value.value.integer = -1;
      vpi_put_value(systf_handle, &value, NULL, vpiNoDelay);
  } else {
    value.format = vpiIntVal;
    value.value.integer = (PLI_UINT32)bnum;
    vpi_put_value(systf_handle, &value, NULL, vpiNoDelay);
  }

  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_snd_Compiletf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator, arg_handle;
  PLI_INT32 tfarg_type;
  int n;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) { /* no argument */
     vpi_printf("ERROR: $cosim_ipc_snd must have 3 arguments.\n");
     cosim_control(vpiFinish);
     return(0);
  }
  for (n=0; n<2; n++) { // cid, bnum
      arg_handle   = vpi_scan(arg_iterator);
      if (arg_handle==NULL) {
           vpi_printf("ERROR: $cosim_ipc_snd must have 3 arguments.\n");
           cosim_control(vpiFinish);
      }
      tfarg_type = vpi_get(vpiType, arg_handle);
      if ((tfarg_type!=vpiIntegerVar)&&
          (tfarg_type!=vpiParameter)&&
          (tfarg_type!=vpiReg)&&
          (tfarg_type!=vpiConstant)) {
           vpi_printf("ERROR: $cosim_ipc_put must have integer argument for %d-th, but %d.\n",
                       (n+1), tfarg_type);
           vpi_free_object(arg_iterator);
           cosim_control(vpiFinish);
      }
  }
  // data[]
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle==NULL) {
       vpi_printf("ERROR: $cosim_ipc_snd must have 3 arguments.\n");
       cosim_control(vpiFinish);
  }
  if (!vpi_get(vpiArray, arg_handle)) { // check if it is array
       vpi_printf("ERROR: $cosim_ipc_snd data must be array\n");
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  // check for extra
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle!=NULL) {
       vpi_printf("ERROR: $cosim_ipc_snd must have 8 arguments.\n");
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_snd_Sizetf(PLI_BYTE8 *user_data) {
  return(32); /* $cosim_ipc_snd() returns 32-bit */
}

//------------------------------------------------------------------------------
// $cosim_ipc_get(cid
//               ,pkt_type 
//               ,pkt_size  // 1, 2, 4
//               ,pkt_length  // burst length
//               ,pkt_ack 
//               ,pkt_attr
//               ,pkt_trans_id
//               ,pkt_addr 
//               ,pkt_data  // array
//               );
//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_get_Calltf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator;
  vpiHandle cidH, typH, sizH, lenH, ackH, attH, tidH, addH, datH;
  s_vpi_value value;
  int    n, m;
  int    cid;
  bfm_packet_t pkt;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  cidH         = vpi_scan(arg_iterator);
  typH         = vpi_scan(arg_iterator);
  sizH         = vpi_scan(arg_iterator);
  lenH         = vpi_scan(arg_iterator);
  ackH         = vpi_scan(arg_iterator);
  attH         = vpi_scan(arg_iterator);
  tidH         = vpi_scan(arg_iterator);
  addH         = vpi_scan(arg_iterator);
  datH         = vpi_scan(arg_iterator);
  vpi_free_object(arg_iterator);

  value.format = vpiIntVal;
  vpi_get_value(cidH, &value);
  cid = (int)value.value.integer;

  int len = (int)sizeof(pkt);
  if (chn_recv(cid, len, (void*)&pkt)<0) {
      s_vpi_time time;
      time.type = vpiScaledRealTime;
      vpi_get_time(NULL, &time);
      vpi_printf("ERROR: something wrong while get packet: %s\n", chn_error_msg(cid, 0));
      cosim_control(vpiStop);
  }
  if (m_verbose>1) {
      vpi_printf("cosim_ipc_get_Calltf: after chn_recv:\n");
      vpi_printf("\tcid        =0x%x\n", cid);
      vpi_printf("\ttype       =0x%x\n", pkt.cmd_type);
      vpi_printf("\tsize       =0x%x\n", pkt.cmd_size);
      vpi_printf("\tlength     =0x%x\n", pkt.cmd_length);
      vpi_printf("\tack        =0x%x\n", pkt.cmd_ack);
      vpi_printf("\tattr       =0x%x\n", pkt.attr);
      vpi_printf("\ttrans_id   =0x%x\n", pkt.trans_id);
      vpi_printf("\taddr       =0x%x\n", pkt.addr);
      vpi_printf("\tdata[0]    =0x%x\n", pkt.data[0]);
      vpi_printf("\tpacket size=%ld\n", sizeof(pkt));
  }

  value.format        = vpiIntVal;

  value.value.integer = (PLI_UINT32)pkt.cmd_type;
  vpi_put_value(typH, &value, NULL, vpiNoDelay);

  value.value.integer = (PLI_UINT32)pkt.cmd_size;
  vpi_put_value(sizH, &value, NULL, vpiNoDelay);

  value.value.integer = (PLI_UINT32)pkt.cmd_length;
  vpi_put_value(lenH, &value, NULL, vpiNoDelay);

  value.value.integer = (PLI_UINT32)pkt.cmd_ack;
  vpi_put_value(ackH, &value, NULL, vpiNoDelay);

  value.value.integer = (PLI_UINT32)pkt.attr;
  vpi_put_value(attH, &value, NULL, vpiNoDelay);

  value.value.integer = (PLI_UINT32)pkt.trans_id;
  vpi_put_value(tidH, &value, NULL, vpiNoDelay);

  value.value.integer = (PLI_UINT32)pkt.addr;
  vpi_put_value(addH, &value, NULL, vpiNoDelay);

  n = vpi_get(vpiSize, datH);
  if (n<(pkt.cmd_size*pkt.cmd_length)) {
     vpi_printf("ERROR: $cosim_ipc_get data element.\n");
     cosim_control(vpiFinish);
     return(0);
  }
  n = pkt.cmd_size*pkt.cmd_length; // num of bytes to move
  for (m=0; m<n; m++) {
       vpiHandle eleH = vpi_handle_by_index(datH, m);
       if (eleH==NULL) {
           vpi_printf("ERROR: $cosim_ipc_get data element: %d\n", m);
           cosim_control(vpiFinish);
           return(0);
       }
       value.format = vpiIntVal;
       value.value.integer = (PLI_UINT32)(pkt.data[m]);
       vpi_put_value(eleH, &value, NULL, vpiNoDelay);
  }

  value.format = vpiIntVal;
  value.value.integer = 0;
  vpi_put_value(systf_handle, &value, NULL, vpiNoDelay);

  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_get_Compiletf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator, arg_handle;
  PLI_INT32 tfarg_type;
  int n;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) { /* no argument */
     vpi_printf("ERROR: $cosim_ipc_get must have 9 arguments.\n");
     cosim_control(vpiFinish);
     return(0);
  }
  for (n=0; n<8; n++) { // cid, cmd_type, cmd_size, cmd_length, cmd_ack, attr, trans_id, addr
      arg_handle   = vpi_scan(arg_iterator);
      if (arg_handle==NULL) {
           vpi_printf("ERROR: $cosim_ipc_get must have 9 arguments.\n");
           cosim_control(vpiFinish);
      }
      tfarg_type = vpi_get(vpiType, arg_handle);
      if ((tfarg_type!=vpiIntegerVar)&&
          (tfarg_type!=vpiParameter)&&
          (tfarg_type!=vpiReg)&&
          (tfarg_type!=vpiConstant)) {
           vpi_printf("ERROR: $cosim_ipc_get must have integer argument of %d-th, but %d.\n",
                       (n+1), tfarg_type);
           vpi_free_object(arg_iterator);
           cosim_control(vpiFinish);
      }
  }
  // data[]
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle==NULL) {
       vpi_printf("ERROR: $cosim_ipc_get must have 9 arguments.\n");
       cosim_control(vpiFinish);
  }
  #if defined(iverilog)
  tfarg_type = vpi_get(vpiType, arg_handle);
  if ((tfarg_type!=vpiArray)&&(tfarg_type!=vpiMemory)) {
       vpi_printf("ERROR: $cosim_ipc_get data must be array, but %d.\n", tfarg_type);
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  #else
  if (!vpi_get(vpiArray, arg_handle)) { // check if it is array
       vpi_printf("ERROR: $cosim_ipc_get data must be array\n");
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  #endif
  // check for extra
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle!=NULL) {
       vpi_printf("ERROR: $cosim_ipc_get must have 9 arguments.\n");
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_get_Sizetf(PLI_BYTE8 *user_data) {
  return(32); /* $cosim_ipc_get() returns 32-bit */
}

//------------------------------------------------------------------------------
// $cosim_ipc_put(cid
//               ,pkt_type
//               ,pkt_size
//               ,pkt_leng // burst length
//               ,pkt_ack
//               ,pkt_attr
//               ,pkt_trans_id
//               ,pkt_addr
//               ,pkt_data
//               );
//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_put_Calltf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator;
  vpiHandle cidH, typH, sizH, lenH, ackH, attH, tidH, addH, datH;
  s_vpi_value value;
  int    n, m;
  int    cid;
  bfm_packet_t pkt;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  cidH         = vpi_scan(arg_iterator);
  typH         = vpi_scan(arg_iterator);
  sizH         = vpi_scan(arg_iterator);
  lenH         = vpi_scan(arg_iterator);
  ackH         = vpi_scan(arg_iterator);
  attH         = vpi_scan(arg_iterator);
  tidH         = vpi_scan(arg_iterator);
  addH         = vpi_scan(arg_iterator);
  datH         = vpi_scan(arg_iterator);
  vpi_free_object(arg_iterator);

  value.format = vpiIntVal;
  vpi_get_value(cidH,  &value); cid = (int)value.value.integer;
  vpi_get_value(typH,  &value); pkt.cmd_type   = (unsigned int)value.value.integer;
  vpi_get_value(sizH,  &value); pkt.cmd_size   = (unsigned int)value.value.integer;
  vpi_get_value(lenH,  &value); pkt.cmd_length = (unsigned int)value.value.integer;
  vpi_get_value(ackH,  &value); pkt.cmd_ack    = (unsigned int)value.value.integer;
  vpi_get_value(attH,  &value); pkt.attr       = (unsigned int)value.value.integer;
  vpi_get_value(tidH,  &value); pkt.trans_id   = (unsigned int)value.value.integer;
  vpi_get_value(addH,  &value); pkt.addr       = (unsigned int)value.value.integer;

  n = vpi_get(vpiSize, datH);
  if (n<(pkt.cmd_size*pkt.cmd_length)) {
     vpi_printf("ERROR: $cosim_ipc_put data element.\n");
     cosim_control(vpiFinish);
     return(0);
  }
  n = pkt.cmd_size*pkt.cmd_length; // num of bytes to move
  for (m=0; m<n; m++) {
       vpiHandle eleH = vpi_handle_by_index(datH, m);
       if (eleH==NULL) {
           vpi_printf("ERROR: $cosim_ipc_get data element: %d\n", m);
           cosim_control(vpiFinish);
           return(0);
       }
       value.format = vpiIntVal;
       vpi_get_value(eleH, &value);
       pkt.data[m] = (value.value.integer&0xFF);
  }

  if (m_verbose>1) {
    vpi_printf("cosim_ipc_put_Calltf: before chn_send:\n");
    vpi_printf("\tcid        =0x%x\n", cid);
    vpi_printf("\tcmd.type   =0x%x\n", pkt.cmd_type);
    vpi_printf("\tcmd.size   =0x%x\n", pkt.cmd_size);
    vpi_printf("\tcmd.length =0x%x\n", pkt.cmd_length);
    vpi_printf("\tcmd.ack    =0x%x\n", pkt.cmd_ack);
    vpi_printf("\tattr       =0x%x\n", pkt.attr);
    vpi_printf("\ttrans_id   =0x%x\n", pkt.trans_id);
    vpi_printf("\taddr       =0x%x\n", pkt.addr);
    vpi_printf("\tdata[0]    =0x%x\n", pkt.data[0]);
    vpi_printf("\tpacket size=%ld\n", sizeof(pkt));
  }

  int len = (int)sizeof(pkt);
  if (chn_send(cid, len, (void*)&pkt)!=len) {
      s_vpi_time time;
      time.type = vpiScaledRealTime;
      vpi_get_time(NULL, &time);
      vpi_printf("ERROR: something wrong while put packet: %s\n", chn_error_msg(cid, 0));
      cosim_control(vpiStop);
  }

  value.format = vpiIntVal;
  value.value.integer = 0;
  vpi_put_value(systf_handle, &value, NULL, vpiNoDelay);

  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_put_Compiletf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator, arg_handle;
  PLI_INT32 tfarg_type;
  int n;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) { /* no argument */
     vpi_printf("ERROR: $cosim_ipc_put must have 9 arguments.\n");
     cosim_control(vpiFinish);
     return(0);
  }
  for (n=0; n<8; n++) { // cid, cmd_type, cmd_size, cmd_length, cmd_ack, attr, trans_id, addr
      arg_handle   = vpi_scan(arg_iterator);
      if (arg_handle==NULL) {
           vpi_printf("ERROR: $cosim_ipc_put must have 9 arguments.\n");
           cosim_control(vpiFinish);
      }
      tfarg_type = vpi_get(vpiType, arg_handle);
      if ((tfarg_type!=vpiIntegerVar)&&
          (tfarg_type!=vpiParameter)&&
          (tfarg_type!=vpiReg)&&
          (tfarg_type!=vpiConstant)) {
           vpi_printf("ERROR: $cosim_ipc_put must have integer argument for %d-th, but %d.\n",
                       (n+1), tfarg_type);
           vpi_free_object(arg_iterator);
           cosim_control(vpiFinish);
      }
  }
  // data[]
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle==NULL) {
       vpi_printf("ERROR: $cosim_ipc_put must have 9 arguments.\n");
       cosim_control(vpiFinish);
  }
  #if defined(iverilog)
  tfarg_type = vpi_get(vpiType, arg_handle);
  if ((tfarg_type!=vpiArray)&&(tfarg_type!=vpiMemory)) {
       vpi_printf("ERROR: $cosim_ipc_put data must be array, but %d.\n", tfarg_type);
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  #else
  if (!vpi_get(vpiArray, arg_handle)) { // check if it is array
       vpi_printf("ERROR: $cosim_ipc_put data must be array\n");
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  #endif
  // check for extra
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle!=NULL) {
       vpi_printf("ERROR: $cosim_ipc_put must have 9 arguments.\n");
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_put_Sizetf(PLI_BYTE8 *user_data) {
  return(32); /* $cosim_ipc_put() returns 32-bit */
}

//------------------------------------------------------------------------------
// $cosim_ipc_set_verbose; ==> $cosim_ipc_set_verbose(0);
// $cosim_ipc_set_verbose(); ==> $cosim_ipc_set_verbose(0);
// $cosim_ipc_set_verbose(n);
//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_set_verbose_Calltf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_itr, arg1, arg2;
  s_vpi_value value;
  int level;
  PLI_INT32 result;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_itr      = vpi_iterate(vpiArgument, systf_handle);
  if (arg_itr==NULL) {
      level = 0;
  } else {
     arg1 = vpi_scan(arg_itr);
     #if !defined(iverilog)
     if (vpiOperation==vpi_get(vpiType,arg1)) {
         level = 0;
     } else {
         value.format = vpiIntVal;
         vpi_get_value(arg1, &value);
         level = value.value.integer;
     }
     #else
         value.format = vpiIntVal;
         vpi_get_value(arg1, &value);
         level = value.value.integer;
     #endif
     vpi_free_object(arg_itr);
  }
  chn_set_verbose(level);
  m_verbose = level; 
  value.format = vpiIntVal;
  value.value.integer = 0;
  vpi_put_value(systf_handle, &value, NULL, vpiNoDelay);
  return(0);
}

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_set_verbose_Compiletf(PLI_BYTE8 *user_data) {
  vpiHandle systf_handle, arg_iterator, arg_handle;
  PLI_INT32 tfarg_type;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) { /* no argument */
     return(0);
  }
  arg_handle   = vpi_scan(arg_iterator);
  if (arg_handle==NULL) {
       vpi_printf("ERROR: $cosim_ipc_set_verbose must have only one argument.\n");
       cosim_control(vpiFinish);
  }
  tfarg_type = vpi_get(vpiType, arg_handle);
  #if !defined(iverilog)
  if (tfarg_type==vpiOperation) {
     if (vpiNullOp!=vpi_get(vpiOpType,arg_handle)) {
        // deal with empty ()
        vpi_printf("ERROR: $cosim_ipc_set_verbose must have only one argument if any.\n");
        cosim_control(vpiFinish);
     }
  } else {
     if ((tfarg_type!=vpiIntegerVar)&&
         (tfarg_type!=vpiParameter)&&
         (tfarg_type!=vpiReg)&&
         (tfarg_type!=vpiConstant)) {
          vpi_printf("ERROR: $cosim_ipc_set_verbose must have integer argument, but %d.\n",
                      tfarg_type);
          vpi_free_object(arg_iterator);
          cosim_control(vpiFinish);
     }
  }
  #endif
  arg_handle = vpi_scan(arg_iterator);
  if (arg_handle!=NULL) {
       vpi_printf("ERROR: $cosim_ipc_set_verbose must have one arguments.\n");
       vpi_free_object(arg_iterator);
       cosim_control(vpiFinish);
  }
  return(0);
}
//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_set_verbose_Sizetf(PLI_BYTE8 *user_data) {
  return(32); /* $cosim_ipc_set_verbose() returns 32-bit */
}

//------------------------------------------------------------------------------
void (*vlog_startup_routines[])() = {
      cosim_ipc_register,
      0
};

//------------------------------------------------------------------------------
PLI_INT32 cosim_ipc_arg_num_check(int num, char* name) {
  vpiHandle systf_handle, arg_iterator, arg_handle;
  PLI_INT32 tfarg_type;
  int i;

  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  arg_iterator = vpi_iterate(vpiArgument, systf_handle);
  if (arg_iterator==NULL) { /* no argument */
     vpi_printf("ERROR: %s must have %d arguments.\n", name, num);
     return(1);
  }
  for (i = 0; i<num; i++) {
    arg_handle = vpi_scan(arg_iterator);
    if (arg_handle==NULL) { /* no argument */
       vpi_printf("ERROR: %s must have %d arguments.(%d)\n", name, num, i+1);
       return(1);
    }
    tfarg_type = vpi_get(vpiType, arg_handle);
    if ((tfarg_type!=vpiIntegerVar)&&(tfarg_type!=vpiParameter)&&(tfarg_type!=vpiConstant)) {
       vpi_printf("ERROR: %s must have integer argument, but %d.(%d)\n",
                   name, tfarg_type, i+1);
       vpi_free_object(arg_iterator);
       return(1);
    }
  }
  arg_handle = vpi_scan(arg_iterator);
  if (arg_handle!=NULL) {
       vpi_printf("ERROR: %s must have %d arguments.\n", name, num);
       vpi_free_object(arg_iterator);
       return(1);
  }
  vpi_free_object(arg_iterator);
  return(0);
}
//------------------------------------------------------------------------------
// Revision history:
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//------------------------------------------------------------------------------
