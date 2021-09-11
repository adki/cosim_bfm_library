#ifndef COSIM_BFM_DEFINES_H
#define COSIM_BFM_DEFINES_H
//------------------------------------------------------------------------------
// Copyright (c) 2021 by Ando Ki.
// All rights reserved by Ando Ki.
//------------------------------------------------------------------------------
// cosim_bfm_packet.h
//------------------------------------------------------------------------------
// 'c/cosim_bfm_defines.h' and 'verilog/cosim_bfm_defines.vh'
//------------------------------------------------------------------------------
// for 'cmd_type' field
#define COSIM_CMD_NULL        0x00  // skip
#define COSIM_CMD_RD_REQ      0x01
#define COSIM_CMD_WR_REQ      0x02
#define COSIM_CMD_RD_RSP      0x05
#define COSIM_CMD_WR_RSP      0x06
#define COSIM_CMD_TERM_REQ    0x08
#define COSIM_CMD_TERM_RSP    0x09
#define COSIM_CMD_GET_GP_REQ  0x11
#define COSIM_CMD_PUT_GP_REQ  0x12
#define COSIM_CMD_GET_GP_RSP  0x15
#define COSIM_CMD_PUT_GP_RSP  0x16
#define COSIM_CMD_GET_IRQ_REQ 0x21
#define COSIM_CMD_GET_IRQ_RSP 0x22

//------------------------------------------------------------------------------
// for 'cmd_ack' field
#define COSIM_CMD_ACK_ERR     0x0
#define COSIM_CMD_ACK_OK      0x1

//------------------------------------------------------------------------------
// for 'data[]' field
#define COSIM_DATA_BNUM       1024// 256-beat * 4-byte

//------------------------------------------------------------------------------
// Revision history:
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//------------------------------------------------------------------------------
#endif
