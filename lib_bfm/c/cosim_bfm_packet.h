#ifndef COSIM_BFM_PACKET_H
#define COSIM_BFM_PACKET_H
//------------------------------------------------------------------------------
// Copyright (c) 2021 by Ando Ki.
// All rights reserved by Ando Ki.
//------------------------------------------------------------------------------
// cosim_bfm_packet.h
//------------------------------------------------------------------------------
// it should be the same as 'verilog/cosim_bfm_packet.vh'
//------------------------------------------------------------------------------
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
#include "cosim_bfm_defines.h"

//------------------------------------------------------------------------------
// data[...] contains 'cmd_size*cmd_length' bytes
typedef struct {
  unsigned int cmd_type;   // RD-REQ(1), WR-REQ(2), RD-RSP(5), WR-RSP(6), TERM-REQ(8)
  unsigned int cmd_size;   // num of bytes in a beat
  unsigned int cmd_length; // num of beats, i.e., burst length
  unsigned int cmd_ack;    // ERR(0), OK(1)
  unsigned int attr;       // user-specified attribute
  uint32_t     trans_id;   // transaction identification (for multiple outstanding case)
  uint32_t     addr;
  uint8_t      data[COSIM_DATA_BNUM]; // byte-stream up to 4*256 bytes
} bfm_packet_t;

#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
// Revision history:
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//------------------------------------------------------------------------------
#endif
