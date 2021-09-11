#ifndef COSIM_BFM_API_H
#define COSIM_BFM_API_H
//------------------------------------------------------------------------------
// Copyright (c) 2021 by Ando Ki.
// All rights reserved by Ando Ki.
//------------------------------------------------------------------------------
// cosim_bfm_api.h
//------------------------------------------------------------------------------
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// for BFM master API
extern int bfm_open   (int cid); // IPC channel ID
extern int bfm_close  (int cid); // IPC channel ID
extern int bfm_barrier(int cid); // IPC channel ID
extern int bfm_set_verbose(int level);
extern int bfm_get_verbose();
//------------------------------------------------------------------------------
extern int bfm_write( uint32_t     addr
                    , uint8_t     *data
                    , unsigned int sz
                    , unsigned int length);
extern int bfm_read ( uint32_t     addr
                    , uint8_t     *data
                    , unsigned int sz
                    , unsigned int length);

extern int bfm_write_core( int          cid       // IPC channel ID
                         , uint32_t     trans_id  // transaction ID
                         , uint32_t     addr      // address to write
                         , uint8_t     *data      // data buffer
                         , unsigned int sz       // num of bytes for each beat
                         , unsigned int length   // length of burst, i.e., num of beats
                         , unsigned int attr);   // user defined attribute
extern int bfm_read_core ( int          cid      // IPC channel ID
                         , uint32_t     trans_id // transaction ID
                         , uint32_t     addr     // address to read
                         , uint8_t     *data     // data buffer
                         , unsigned int sz       // num of bytes for each beat
                         , unsigned int length   // length of burst, i.e., num of beats
                         , unsigned int attr);   // user defined attribute

//------------------------------------------------------------------------------
// special API for BFM master
extern int bfm_put_gp ( int cid, uint32_t *data );
extern int bfm_get_gp ( int cid, uint32_t *data );
extern int bfm_get_irq( int cid, uint32_t *irq  );

#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------------
// Revision history
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//------------------------------------------------------------------------------
#endif
