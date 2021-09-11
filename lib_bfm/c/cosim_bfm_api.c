//------------------------------------------------------------------------------
// Copyright (c) 2021 by Ando Ki.
// All rights reserved by Ando Ki.
//------------------------------------------------------------------------------
// cosim_bfm_api.c
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cosim_ipc.h"
#include "cosim_bfm_packet.h"
#include "cosim_bfm_api.h"

#if defined(DEBUG)||defined(RIGOR)
#define PRINTF(format, ...) do { printf("%s(): " format, __func__, ##__VA_ARGS__);\
                                 fflush(stdout); } while (0)
#else
#define PRINTF(...) do { } while (0)
#endif

static int   m_cid =0;
static int   m_verbose=0;

//------------------------------------------------------------------------------
int bfm_open(int cid)
{
    if ((cid<0)||(cid>MAX_NUM_CHAN)) return -1;
    m_cid = cid;
    if (chn_create_connect(cid, CHAN_HOST)<0) return -1;
    return 0;
}

//------------------------------------------------------------------------------
int bfm_close(int cid)
{
    bfm_packet_t pkt_req, pkt_rsp;
    pkt_req.cmd_type  = COSIM_CMD_TERM_REQ;
    pkt_req.cmd_size  = 0;
    pkt_req.cmd_length= 0;
    pkt_req.cmd_ack   = COSIM_CMD_ACK_ERR;
    pkt_req.attr      = 0;
    pkt_req.trans_id  = 0;
    pkt_req.addr      = 0;
    int len = (int)sizeof(pkt_req);
    if (chn_send(cid, len, (void*)&pkt_req)!=len) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    len = (int)sizeof(pkt_rsp);
    if (chn_recv(cid, len, (void*)&pkt_rsp)<0) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    if (pkt_rsp.cmd_type!=COSIM_CMD_TERM_RSP) {
         PRINTF("unexpected response for termination: %d\n", pkt_rsp.cmd_type);
         return -1;
    }
    if (pkt_rsp.cmd_ack!=COSIM_CMD_ACK_OK) {
         PRINTF("unexpected ack for termination: %d\n", pkt_rsp.cmd_ack);
         return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------
int bfm_barrier(int cid)
{
    if ((cid<0)||(cid>MAX_NUM_CHAN)) return -1;
    if (chn_barrier(cid)<0) return -1;
    return 0;
}

//------------------------------------------------------------------------------
int bfm_set_verbose(int level)
{
    m_verbose = level;
    chn_set_verbose(level);
    return 0;
}

//------------------------------------------------------------------------------
int bfm_get_verbose(void)
{
    return m_verbose;
}

//------------------------------------------------------------------------------
// data[s*L] carries little-endian byte-stream, not justified word.
//    1    2                  L
// +----+----+----+----+----+----+
// |    |    |    |    |    |    |
// +-+--+----+----+----+----+----+
//   |
//   +-- sz-bytes
int bfm_write( uint32_t addr
             , uint8_t* data
             , unsigned int sz
             , unsigned int length)
{
    // use channel ID previously opened by bfm_open(cid)
    if (addr%sz) PRINTF("un-aligned access.\n");
    if ((sz!=1)&&(sz!=2)&&(sz!=4)) PRINTF("data size un-supported.\n");
    if (length>256) PRINTF("too long burst.\n");
    return bfm_write_core( m_cid
                         , 1
                         , addr
                         , data
                         , sz
                         , length
                         , 1);
}

int bfm_write_core( int      cid        // IPC channel ID
                  , uint32_t trans_id   // transaction ID
                  , uint32_t addr       // address to write
                  , uint8_t* data       // data buffer
                  , unsigned int sz     // num of bytes for each beat
                  , unsigned int length // length of burst, i.e., num of beats
                  , unsigned int attr)  // user defined attribute
{
    #if defined(ROGOR)
    if ((sz*length)>COSIM_DATA_BNUM) PRINTF("data size exceeds.\n");
    #endif
    bfm_packet_t pkt_req, pkt_rsp;
    pkt_req.cmd_type  = COSIM_CMD_WR_REQ;
    pkt_req.cmd_size  = sz;
    pkt_req.cmd_length= length;
    pkt_req.cmd_ack   = COSIM_CMD_ACK_ERR;
    pkt_req.attr      = attr;
    pkt_req.trans_id  = trans_id;
    pkt_req.addr      = addr;
    // If 'addr' is not algned with 'sz', 'sz*length' can cause exception.
    // bnum = (addr%sz) ? (sz-(addr%sz))+sz*(length-1)
    memcpy(&pkt_req.data, data, sz*length);
    int len = (int)sizeof(pkt_req);
    if (chn_send(cid, len, (void*)&pkt_req)!=len) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    len = (int)sizeof(pkt_rsp);
    if (chn_recv(cid, len, (void*)&pkt_rsp)<0) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    if (m_verbose>1) {
        PRINTF("chn_recv:\n");
        PRINTF("\tcid        =0x%x\n", cid);
        PRINTF("\ttype       =0x%x\n", pkt_rsp.cmd_type);
        PRINTF("\tsize       =0x%x\n", pkt_rsp.cmd_size);
        PRINTF("\tlength     =0x%x\n", pkt_rsp.cmd_length);
        PRINTF("\tack        =0x%x\n", pkt_rsp.cmd_ack);
        PRINTF("\tattr       =0x%x\n", pkt_rsp.attr);
        PRINTF("\ttrans_id   =0x%x\n", pkt_rsp.trans_id);
        PRINTF("\taddr       =0x%x\n", pkt_rsp.addr);
        PRINTF("\tdata[0]    =0x%x\n", pkt_rsp.data[0]);
        PRINTF("\tpacket size=%lu\n", sizeof(pkt_rsp));
    }
    if (pkt_rsp.cmd_type==COSIM_CMD_TERM_REQ) {
        if (chn_close(cid)<0) {
            PRINTF("fail to close channel: CHN%d\n", cid);
            return -2;
        }
        return -1;
    }
    if (pkt_rsp.cmd_type!=COSIM_CMD_WR_RSP) {
         PRINTF("unexpected response for write A:0x%08x: %d\n", addr, pkt_rsp.cmd_type);
         return -1;
    }
    if (pkt_rsp.trans_id!=trans_id) {
         PRINTF("unexpected transaction-id for write A:0x%08x: %d\n", addr, pkt_rsp.cmd_type);
         return -1;
    }
    if (pkt_rsp.cmd_ack!=COSIM_CMD_ACK_OK) {
         PRINTF("unexpected ack for write A:0x%08x: %d\n", addr, pkt_rsp.cmd_ack);
         return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------
// data[s*L] carries little-endian byte-stream, not justified word.
//    1    2                  L
// +----+----+----+----+----+----+
// |    |    |    |    |    |    |
// +-+--+----+----+----+----+----+
//   |
//   +-- sz-bytes
int bfm_read ( uint32_t addr
             , uint8_t* data
             , unsigned int sz
             , unsigned int length)
{
    // use channel ID previously opened by bfm_open(cid)
    if (addr%sz) PRINTF("un-aligned access.\n");
    if ((sz!=1)&&(sz!=2)&&(sz!=4)) PRINTF("data size un-supported.\n");
    if (length>256) PRINTF("too long burst.\n");
    return bfm_read_core ( m_cid
                         , 1
                         , addr
                         , data
                         , sz
                         , length
                         , 1);
}

int bfm_read_core ( int      cid        // IPC channel ID
                  , uint32_t trans_id   // transaction ID
                  , uint32_t addr       // address to read
                  , uint8_t* data       // data buffer
                  , unsigned int sz     // num of bytes for each beat
                  , unsigned int length // length of burst, i.e., num of beats
                  , unsigned int attr)  // user defined attribute
{
    #if defined(ROGOR)
    if ((sz*length)>COSIM_DATA_BNUM) PRINTF("data size exceeds.\n");
    #endif
    bfm_packet_t pkt_req, pkt_rsp;
    pkt_req.cmd_type  = COSIM_CMD_RD_REQ;
    pkt_req.cmd_size  = sz;
    pkt_req.cmd_length= length;
    pkt_req.cmd_ack   = COSIM_CMD_ACK_ERR;
    pkt_req.attr      = attr;
    pkt_req.trans_id  = trans_id;
    pkt_req.addr      = addr;
    int len = (int)sizeof(pkt_req);
    if (chn_send(cid, len, (void*)&pkt_req)!=len) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    len = (int)sizeof(pkt_rsp);
    if (chn_recv(cid, len, (void*)&pkt_rsp)<0) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    if (m_verbose>1) {
        PRINTF("chn_recv:\n");
        PRINTF("\tcid        =0x%x\n", cid);
        PRINTF("\ttype       =0x%x\n", pkt_rsp.cmd_type);
        PRINTF("\tsize       =0x%x\n", pkt_rsp.cmd_size);
        PRINTF("\tlength     =0x%x\n", pkt_rsp.cmd_length);
        PRINTF("\tack        =0x%x\n", pkt_rsp.cmd_ack);
        PRINTF("\tattr       =0x%x\n", pkt_rsp.attr);
        PRINTF("\ttrans_id   =0x%x\n", pkt_rsp.trans_id);
        PRINTF("\taddr       =0x%x\n", pkt_rsp.addr);
        PRINTF("\tdata[0]    =0x%x\n", pkt_rsp.data[0]);
        PRINTF("\tpacket size=%lu\n", sizeof(pkt_rsp));
    }
    if (pkt_rsp.cmd_type==COSIM_CMD_TERM_REQ) {
        if (chn_close(cid)<0) {
            PRINTF("fail to close channel: CHN:%d\n", cid);
            return -2;
        }
        return -1;
    }
    if (pkt_rsp.cmd_type!=COSIM_CMD_RD_RSP) {
         PRINTF("unexpected response for read A:0x%08x: %d\n", addr, pkt_rsp.cmd_type);
         return -1;
    }
    if (pkt_rsp.trans_id!=trans_id) {
         PRINTF("unexpected transaction-id for read A:0x%08x: %d\n", addr, pkt_rsp.cmd_type);
         return -1;
    }
    if (pkt_rsp.cmd_ack!=COSIM_CMD_ACK_OK) {
         PRINTF("unexpected ack for read A:0x%08x: %d\n", addr, pkt_rsp.cmd_ack);
         return -1;
    }
    // If 'addr' is not algned with 'sz', 'sz*length' can cause exception.
    // bnum = (addr%sz) ? (sz-(addr%sz))+sz*(length-1)
    memcpy(data, (void*)&pkt_rsp.data, sz*length);
    return 0;
}

//------------------------------------------------------------------------------
// data[s] carries s-byte
int bfm_put_gp( int cid
              , uint32_t *data )
{
    bfm_packet_t pkt_req, pkt_rsp;
    pkt_req.cmd_type  = COSIM_CMD_PUT_GP_REQ;
    pkt_req.cmd_size  = 4;
    pkt_req.cmd_length= 1;
    pkt_req.cmd_ack   = COSIM_CMD_ACK_ERR;
    pkt_req.attr      = 0;
    pkt_req.trans_id  = 0;
    pkt_req.addr      = 0;
    memcpy(&pkt_req.data, (uint8_t*)data, 4);
    int len = (int)sizeof(pkt_req);
    if (chn_send(cid, len, (void*)&pkt_req)!=len) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    len = (int)sizeof(pkt_rsp);
    if (chn_recv(cid, len, (void*)&pkt_rsp)<0) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    if (pkt_rsp.cmd_type==COSIM_CMD_TERM_REQ) {
        if (chn_close(cid)<0) {
            PRINTF("fail to close channel: CHN%d\n", cid);
            return -2;
        }
        return -1;
    }
    if (pkt_rsp.cmd_type!=COSIM_CMD_PUT_GP_RSP) {
         PRINTF("unexpected response for put_gp %d\n", pkt_rsp.cmd_type);
         return -1;
    }
    if (pkt_rsp.cmd_ack!=COSIM_CMD_ACK_OK) {
         PRINTF("unexpected ack for put_gp %d\n", pkt_rsp.cmd_ack);
         return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------
// data[s] carries s-byte
int bfm_get_gp( int cid
              , uint32_t *data )
{
    bfm_packet_t pkt_req, pkt_rsp;
    pkt_req.cmd_type  = COSIM_CMD_GET_GP_REQ;
    pkt_req.cmd_size  = 4;
    pkt_req.cmd_length= 1;
    pkt_req.cmd_ack   = COSIM_CMD_ACK_ERR;
    pkt_req.attr      = 0;
    pkt_req.trans_id  = 0;
    pkt_req.addr      = 0;
    int len = (int)sizeof(pkt_req);
    if (chn_send(cid, len, (void*)&pkt_req)!=len) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    len = (int)sizeof(pkt_rsp);
    if (chn_recv(cid, len, (void*)&pkt_rsp)<0) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    if (pkt_rsp.cmd_type==COSIM_CMD_TERM_REQ) {
        if (chn_close(cid)<0) {
            PRINTF("fail to close channel: CHN%d\n", cid);
            return -2;
        }
        return -1;
    }
    if (pkt_rsp.cmd_type!=COSIM_CMD_GET_GP_RSP) {
         PRINTF("unexpected response for get_gp %d\n", pkt_rsp.cmd_type);
         return -1;
    }
    if (pkt_rsp.cmd_ack!=COSIM_CMD_ACK_OK) {
         PRINTF("unexpected ack for get_gp %d\n", pkt_rsp.cmd_ack);
         return -1;
    }
    memcpy((uint8_t*)data, (void*)&pkt_rsp.data, 4);
    return 0;
}

//------------------------------------------------------------------------------
// data[s] carries s-byte
int bfm_get_irq( int cid
               , uint32_t *irq )
{
    bfm_packet_t pkt_req, pkt_rsp;
    pkt_req.cmd_type  = COSIM_CMD_GET_IRQ_REQ;
    pkt_req.cmd_size  = 4;
    pkt_req.cmd_length= 1;
    pkt_req.cmd_ack   = COSIM_CMD_ACK_ERR;
    pkt_req.attr      = 0;
    pkt_req.trans_id  = 0;
    pkt_req.addr      = 0;
    int len = (int)sizeof(pkt_req);
    if (chn_send(cid, len, (void*)&pkt_req)!=len) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    len = (int)sizeof(pkt_rsp);
    if (chn_recv(cid, len, (void*)&pkt_rsp)<0) {
        PRINTF("%s\n", chn_error_msg(cid, 0));
        return -1;
    }
    if (pkt_rsp.cmd_type==COSIM_CMD_TERM_REQ) {
        if (chn_close(cid)<0) {
            PRINTF("fail to close channel: CHN%d\n",cid);
            return -2;
        }
        return -1;
    }
    if (pkt_rsp.cmd_type!=COSIM_CMD_GET_IRQ_RSP) {
         PRINTF("unexpected response for get_irq %d\n", pkt_rsp.cmd_type);
         return -1;
    }
    if (pkt_rsp.cmd_ack!=COSIM_CMD_ACK_OK) {
         PRINTF("unexpected ack for get_irq %d\n", pkt_rsp.cmd_ack);
         return -1;
    }
    memcpy((uint8_t*)irq, (void*)&pkt_rsp.data, 4);
    return 0;
}

#undef PRINTF
//------------------------------------------------------------------------------
// Revision history
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//------------------------------------------------------------------------------
