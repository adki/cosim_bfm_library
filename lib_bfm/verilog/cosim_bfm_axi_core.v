//------------------------------------------------------------------------------
//  Copyright (c) 2021 by Ando Ki.
//  All rights are reserved by Ando Ki.
//------------------------------------------------------------------------------
// cosim_bfm_axi_core.v
//------------------------------------------------------------------------------
//  [MACROS]
//    AMBA_AXI4       - AMBA AXI4
//    AMBA_AXI_CACHE  -
//    AMBA_AXI_PROT   -
//------------------------------------------------------------------------------
// `ifdef VCS
//   VCS specific
// `elsif INCA
//   NCSIM specific
// `elsif MODEL_TECH
//   MODELSIM specific
// `elsif XILINX_ISIM
//   ISE simulator specific (ISim)
// `elsif XILINX_SIMULATOR
//   Vivado simulator specific (XSim)
// `endif
//------------------------------------------------------------------------------
`include "cosim_bfm_defines.vh"
`timescale 1ns/1ns
`ifdef COSIM_DPI
import "DPI-C" cosim_ipc_open   =function int cosim_ipc_open   (input int cid);
import "DPI-C" cosim_ipc_close  =function int cosim_ipc_close  (input int cid);
import "DPI-C" cosim_ipc_barrier=function int cosim_ipc_barrier(input int cid);
import "DPI-C" cosim_ipc_get    =function int cosim_ipc_get(
                   input  int       cid  // IPC channel identification
                 , output int       pkt_cmd  // see cosim_bfm_defines.vh
                 , output int       pkt_size  // 1, 2, 4
                 , output int       pkt_length  // burst length
                 , output int       pkt_ack 
                 , output int       pkt_attr
                 , output int       pkt_trans_id
                 , output int       pkt_addr 
                 , output bit [7:0] pkt_data[]  // open-array
                 );
import "DPI-C" cosim_ipc_put  =function int cosim_ipc_put(
                   input  int       cid
                 , input  int       pkt_cmd
                 , input  int       pkt_size // 1, 2, 4
                 , input  int       pkt_length // burst length
                 , input  int       pkt_ack
                 , input  int       pkt_attr
                 , input  int       pkt_trans_id
                 , input  int       pkt_addr
                 , input  bit [7:0] pkt_data[] // open-array
                );
import "DPI-C" cosim_ipc_rcv  =function int cosim_ipc_rcv(
                   input  int       cid
                 , output int       bnum
                 , output bit [7:0] pkt_data[]  // open-array
                 );
import "DPI-C" cosim_ipc_snd  =function int cosim_ipc_snd(
                   input  int       cid
                 , input  int       bnum
                 , input  bit [7:0] pkt_data[] // open-array
                );
import "DPI-C" cosim_ipc_set_verbose=function int cosim_ipc_set_verbose(
                   input  int       level
                );
`elsif COSIM_VPI
`else
        'COSIM_DPI' or 'COSIM_VPI' should be defined.
`endif

module cosim_bfm_axi
     #(parameter AXI_WIDTH_ID =4                // Transaction ID width in bits
               , AXI_WIDTH_AD =32               // address width
               , AXI_WIDTH_DA =32               // data width
               , AXI_WIDTH_DS =(AXI_WIDTH_DA/8)     // data strobe width
               , AXI_WIDTH_DSB=$clog2(AXI_WIDTH_DS) // data strobe width
               , COSIM_CHAN_ID  =0
               , COSIM_VERBOSE  =0)
(
       input  wire                     ARESETn
     , input  wire                     ACLK
     //-------------------------------------------------------------------------
     , output reg  [AXI_WIDTH_ID-1:0]  M_AWID
     , output reg  [AXI_WIDTH_AD-1:0]  M_AWADDR
     `ifdef AMBA_AXI4
     , output reg  [ 7:0]              M_AWLEN
     , output reg                      M_AWLOCK
     `else
     , output reg  [ 3:0]              M_AWLEN
     , output reg  [ 1:0]              M_AWLOCK
     `endif
     , output reg  [ 2:0]              M_AWSIZE
     , output reg  [ 1:0]              M_AWBURST
     `ifdef AMBA_AXI_CACHE
     , output reg  [ 3:0]              M_AWCACHE
     `endif
     `ifdef AMBA_AXI_PROT
     , output reg  [ 2:0]              M_AWPROT
     `endif
     , output reg                      M_AWVALID
     , input  wire                     M_AWREADY
     `ifdef AMBA_QOS
     , output reg  [ 3:0]              M_AWQOS
     , output reg  [ 3:0]              M_AWREGION
     `endif
     //-------------------------------------------------------------------------
     `ifndef AMBA_AXI4
     , output reg  [AXI_WIDTH_ID-1:0]  M_WID
     `endif
     , output reg  [AXI_WIDTH_DA-1:0]  M_WDATA
     , output reg  [AXI_WIDTH_DS-1:0]  M_WSTRB
     , output reg                      M_WLAST
     , output reg                      M_WVALID
     , input  wire                     M_WREADY
     //-------------------------------------------------------------------------
     , input  wire [AXI_WIDTH_ID-1:0]  M_BID
     , input  wire [ 1:0]              M_BRESP
     , input  wire                     M_BVALID
     , output reg                      M_BREADY
     //-------------------------------------------------------------------------
     , output reg  [AXI_WIDTH_ID-1:0]  M_ARID
     , output reg  [AXI_WIDTH_AD-1:0]  M_ARADDR
     `ifdef AMBA_AXI4
     , output reg  [ 7:0]              M_ARLEN
     , output reg                      M_ARLOCK
     `else
     , output reg  [ 3:0]              M_ARLEN
     , output reg  [ 1:0]              M_ARLOCK
     `endif
     , output reg  [ 2:0]              M_ARSIZE
     , output reg  [ 1:0]              M_ARBURST
     `ifdef AMBA_AXI_CACHE
     , output reg  [ 3:0]              M_ARCACHE
     `endif
     `ifdef AMBA_AXI_PROT
     , output reg  [ 2:0]              M_ARPROT
     `endif
     , output reg                      M_ARVALID
     , input  wire                     M_ARREADY
     `ifdef AMBA_QOS
     , output reg  [ 3:0]              M_ARQOS
     , output reg  [ 3:0]              M_ARREGION
     `endif
     //-------------------------------------------------------------------------
     , input  wire [AXI_WIDTH_ID-1:0]  M_RID
     , input  wire [AXI_WIDTH_DA-1:0]  M_RDATA
     , input  wire [ 1:0]              M_RRESP
     , input  wire                     M_RLAST
     , input  wire                     M_RVALID
     , output reg                      M_RREADY
     //-------------------------------------------------------------------------
     , input  wire                     IRQ
     //-------------------------------------------------------------------------
     , input  wire [31:0]              GPIN
     , output reg  [31:0]              GPOUT
);
     //-------------------------------------------------------------------------
     `include "cosim_bfm_axi_tasks.v"
     //-------------------------------------------------------------------------
     `ifdef COSIM_DPI
     int     result;
     int     cid, cmd, size, leng, ack, attr, tid, addr;
     `elsif COSIM_VPI
     integer result;
     integer cid, cmd, size, leng, ack, attr, tid, addr;
     `endif
     //-------------------------------------------------------------------------
     initial begin
           `ifdef COSIM_DPI
           result = cosim_ipc_set_verbose(COSIM_VERBOSE);
           result = cosim_ipc_open(COSIM_CHAN_ID);
           result = cosim_ipc_barrier(COSIM_CHAN_ID);
           `elsif COSIM_VPI
           result = $cosim_ipc_set_verbose(COSIM_VERBOSE);
           result = $cosim_ipc_open(COSIM_CHAN_ID);
           result = $cosim_ipc_barrier(COSIM_CHAN_ID);
           `endif
           cid           = COSIM_CHAN_ID;
           M_AWID        = 0;
           M_AWADDR      = ~0;
           M_AWLEN       = 0;
           M_AWLOCK      = 0;
           M_AWSIZE      = 0;
           M_AWBURST     = 0;
           `ifdef AMBA_AXI_CACHE
           M_AWCACHE     = 0;
           `endif
           `ifdef AMBA_AXI_PROT
           M_AWPROT      = 0;
           `endif
           M_AWVALID     = 0;
           `ifdef AMBA_QOS
           M_AWQOS       = 0;
           M_AWREGION    = 0;
           `endif
           `ifndef AMBA_AXI4
           M_WID         = 0;
           `endif
           M_WDATA       = ~0;
           M_WSTRB       = 0;
           M_WLAST       = 0;
           M_WVALID      = 0;
           M_BREADY      = 0;
           M_ARID        = 0;
           M_ARADDR      = ~0;
           M_ARLEN       = 0;
           M_ARLOCK      = 0;
           M_ARSIZE      = 0;
           M_ARBURST     = 0;
           `ifdef AMBA_AXI_CACHE
           M_ARCACHE     = 0;
           `endif
           `ifdef AMBA_AXI_PROT
           M_ARPROT      = 0;
           `endif
           M_ARVALID     = 0;
           `ifdef AMBA_QOS
           M_ARQOS       = 0;
           M_ARREGION    = 0;
           `endif
           M_RREADY      = 0; 
           GPOUT         =32'h0;
     end
     //-------------------------------------------------------------------------
     always @ (posedge ACLK or negedge ARESETn) begin
     if (ARESETn==1'b0) begin
           cid           = COSIM_CHAN_ID;
           M_AWID        = 0;
           M_AWADDR      = ~0;
           M_AWLEN       = 0;
           M_AWLOCK      = 0;
           M_AWSIZE      = 0;
           M_AWBURST     = 0;
           `ifdef AMBA_AXI_CACHE
           M_AWCACHE     = 0;
           `endif
           `ifdef AMBA_AXI_PROT
           M_AWPROT      = 0;
           `endif
           M_AWVALID     = 0;
           `ifdef AMBA_QOS
           M_AWQOS       = 0;
           M_AWREGION    = 0;
           `endif
           `ifndef AMBA_AXI4
           M_WID         = 0;
           `endif
           M_WDATA       = ~0;
           M_WSTRB       = 0;
           M_WLAST       = 0;
           M_WVALID      = 0;
           M_BREADY      = 0;
           M_ARID        = 0;
           M_ARADDR      = ~0;
           M_ARLEN       = 0;
           M_ARLOCK      = 0;
           M_ARSIZE      = 0;
           M_ARBURST     = 0;
           `ifdef AMBA_AXI_CACHE
           M_ARCACHE     = 0;
           `endif
           `ifdef AMBA_AXI_PROT
           M_ARPROT      = 0;
           `endif
           M_ARVALID     = 0;
           `ifdef AMBA_QOS
           M_ARQOS       = 0;
           M_ARREGION    = 0;
           `endif
           M_RREADY      = 0; 
           GPOUT       =32'h0;
     end else begin
      `ifdef COSIM_DPI
      result = cosim_ipc_get
      `elsif COSIM_VPI
      result = $cosim_ipc_get
      `endif
               (cid, cmd, size, leng, ack, attr, tid, addr, axi_dataWB);
      case (cmd)
      `COSIM_CMD_RD_REQ: begin
             if (COSIM_VERBOSE>1) begin
                $display("%0t %m COSIM_CMD_RD_REQ A=0x%08X S:%02d L:%03d",
                         $time, addr, size, leng);
             end
             axi_read_task( tid
                          , addr
                          , size
                          , leng
                          , attr[1:0]); // burst type: 1=incremental
             `ifdef COSIM_DPI
             result = cosim_ipc_put
             `elsif COSIM_VPI
             result = $cosim_ipc_put
             `endif
                                    ( cid
                                    , `COSIM_CMD_RD_RSP
                                    , size
                                    , leng
                                    , `COSIM_CMD_ACK_OK
                                    , 0
                                    , tid
                                    , addr
                                    , axi_dataRB);
             end // COSIM_CMD_RD_REQ
      `COSIM_CMD_WR_REQ: begin
             if (COSIM_VERBOSE>1) begin
                $display("%0t %m COSIM_CMD_WR_REQ A=0x%08H S:%02d L:%03d",
                         $time, addr, size, leng);
             end
             axi_write_task( tid
                           , addr
                           , size
                           , leng
                           , attr[1:0]); // burst type: 1=incremental
             `ifdef COSIM_DPI
             result = cosim_ipc_put
             `elsif COSIM_VPI
             result = $cosim_ipc_put
             `endif
                                    ( cid
                                    , `COSIM_CMD_WR_RSP
                                    , size
                                    , leng
                                    , `COSIM_CMD_ACK_OK
                                    , 0
                                    , tid
                                    , addr 
                                    , axi_dataWB);
             end // COSIM_CMD_WR_REQ
      `COSIM_CMD_TERM_REQ: begin
             if (COSIM_VERBOSE>1) begin
                $display("%0t %m COSIM_CMD_TERM_REQ A=0x%08X S:%02d L:%03d",
                         $time, addr, size, leng);
             end
             `ifdef COSIM_DPI
             result = cosim_ipc_put
             `elsif COSIM_VPI
             result = $cosim_ipc_put
             `endif
                                    ( cid
                                    , `COSIM_CMD_TERM_RSP
                                    , 0
                                    , 0
                                    , `COSIM_CMD_ACK_OK
                                    , 0
                                    , tid
                                    , addr
                                    , axi_dataRB);
             repeat(5) @ (posedge ACLK);
             `ifdef COSIM_DPI
             //cosim_ipc_close(cid); // C part will close it
             `elsif COSIM_VPI
             //$cosim_ipc_close(cid); // C part will close it
             `endif
             $finish(2);
             end // COSIM_CMD_TERM_REQ
      `COSIM_CMD_GET_GP_REQ : begin
             if (COSIM_VERBOSE>1) begin
                $display("%0t %m COSIM_CMD_GET_GP_REQ", $time);
             end
             axi_dataRB[0] = GPIN[ 7: 0];
             axi_dataRB[1] = GPIN[15: 8];
             axi_dataRB[2] = GPIN[23:16];
             axi_dataRB[3] = GPIN[31:24];
             `ifdef COSIM_DPI
             result = cosim_ipc_put
             `elsif COSIM_VPI
             result = $cosim_ipc_put
             `endif
                                    ( cid
                                    , `COSIM_CMD_GET_GP_RSP
                                    , 4
                                    , 1
                                    , `COSIM_CMD_ACK_OK
                                    , 0
                                    , tid
                                    , 0
                                    , axi_dataRB);
             end // COSIM_CMD_GET_GP_REQ
      `COSIM_CMD_PUT_GP_REQ : begin
             if (COSIM_VERBOSE>1) begin
                $display("%0t %m COSIM_CMD_PUT_GP_REQ", $time);
             end
             GPOUT = {axi_dataWB[3],axi_dataWB[2],axi_dataWB[1],axi_dataWB[0]};
             `ifdef COSIM_DPI
             result = cosim_ipc_put
             `elsif COSIM_VPI
             result = $cosim_ipc_put
             `endif
                                    ( cid
                                    , `COSIM_CMD_PUT_GP_RSP
                                    , 4
                                    , 1
                                    , `COSIM_CMD_ACK_OK
                                    , 0
                                    , tid
                                    , 0
                                    , axi_dataWB);
             end // COSIM_CMD_PUT_GP_REQ
      `COSIM_CMD_GET_IRQ_REQ: begin
             if (COSIM_VERBOSE>1) begin
                $display("%0t %m COSIM_CMD_GET_IRQ_REQ", $time);
             end
             axi_dataRB[0] = {7'h0,IRQ};
             axi_dataRB[1] = 8'h0;
             axi_dataRB[2] = 8'h0;
             axi_dataRB[3] = 8'h0;
             `ifdef COSIM_DPI
             result = cosim_ipc_put
             `elsif COSIM_VPI
             result = $cosim_ipc_put
             `endif
                                    ( cid
                                    , `COSIM_CMD_GET_IRQ_RSP
                                    , 4
                                    , 1
                                    , `COSIM_CMD_ACK_OK
                                    , 0
                                    , tid
                                    , 0
                                    , axi_dataRB);
             end // COSIM_CMD_PUT_GP_REQ
      `COSIM_CMD_RD_RSP,
      `COSIM_CMD_WR_RSP,
      `COSIM_CMD_GET_GP_RSP,
      `COSIM_CMD_PUT_GP_RSP,
      `COSIM_CMD_GET_IRQ_RSP,
      `COSIM_CMD_TERM_RSP: begin
             if (COSIM_VERBOSE>1) begin
                $display("%0t %m ERROR un-expected cmd: %d\n", $time, cmd);
             end
             `ifdef COSIM_DPI
             result = cosim_ipc_put
             `elsif COSIM_VPI
             result = $cosim_ipc_put
             `endif
                                    ( cid
                                    , 0
                                    , 0
                                    , 0
                                    , `COSIM_CMD_ACK_ERR
                                    , 0
                                    , tid
                                    , addr
                                    , axi_dataRB);
             end
      endcase
     end // if
     end // always
     //-------------------------------------------------------------------------
endmodule
//------------------------------------------------------------------------------
// Revision History
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//------------------------------------------------------------------------------
