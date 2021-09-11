//----------------------------------------------------------------
//  Copyright (c) 2021 by Ando Ki.
//  All rights are reserved by Ando Ki.
//----------------------------------------------------------------
// top.v
//----------------------------------------------------------------
// VERSION: 2013.02.03.
//----------------------------------------------------------------
`timescale 1ns/1ns

`ifndef WIDTH_AD
`define WIDTH_AD   32 // address width
`endif
`ifndef WIDTH_DA
`define WIDTH_DA   32 // data width
`endif
`ifndef ADDR_LENGTH
`define ADDR_LENGTH 12
`endif
`ifndef DELAY_WRITE_SETUP
`define DELAY_WRITE_SETUP 0
`endif
`ifndef DELAY_WRITE_BURST
`define DELAY_WRITE_BURST 0
`endif
`ifndef DELAY_READ_SETUP
`define DELAY_READ_SETUP  0
`endif
`ifndef DELAY_READ_BURST
`define DELAY_READ_BURST  0
`endif

`ifndef COSIM_CHAN_ID
`define COSIM_CHAN_ID  0
`endif
`ifndef COSIM_VERBOSE
`define COSIM_VERBOSE  0
`endif

module top ;
   //---------------------------------------------------------
   `ifdef VCD
   initial begin
       $dumpfile("wave.vcd");
   end
   `endif
   //---------------------------------------------------------
   localparam WIDTH_CID   = 0    // Channel ID width in bits; it should be 0 since no AXI matrix
            , WIDTH_ID    = 4    // ID width in bits
            , WIDTH_AD    =`WIDTH_AD    // address width
            , WIDTH_DA    =`WIDTH_DA    // data width
            , WIDTH_DS    =(WIDTH_DA/8)  // data strobe width
            , WIDTH_SID   =WIDTH_CID+WIDTH_ID // ID for slave
            , WIDTH_AWUSER=1  // Write-address user path
            , WIDTH_WUSER =1  // Write-data user path
            , WIDTH_BUSER =1  // Write-response user path
            , WIDTH_ARUSER=1  // read-address user path
            , WIDTH_RUSER =1; // read-data user path
   localparam ADDR_BASE   =32'h0000_0000
            , ADDR_LENGTH =`ADDR_LENGTH;
   localparam COSIM_CHAN_ID=`COSIM_CHAN_ID
            , COSIM_VERBOSE=`COSIM_VERBOSE;
   //---------------------------------------------------------------------------
   `ifdef __ICARUS__
   `define NET_DELAY
   `else
   `define NET_DELAY  #(1)
   `endif
   //---------------------------------------------------------
   reg                       ARESETn    ;
   reg                       ACLK       ;
   //--------------------------------------------------------------
   wire  [WIDTH_ID-1:0]     `NET_DELAY  AWID    ;
   wire  [WIDTH_AD-1:0]     `NET_DELAY  AWADDR  ;
   `ifdef AMBA_AXI4
   wire  [ 7:0]             `NET_DELAY  AWLEN   ;
   wire                     `NET_DELAY  AWLOCK  ;
   `else
   wire  [ 3:0]             `NET_DELAY  AWLEN   ;
   wire  [ 1:0]             `NET_DELAY  AWLOCK  ;
   `endif
   wire  [ 2:0]             `NET_DELAY  AWSIZE  ;
   wire  [ 1:0]             `NET_DELAY  AWBURST ;
   `ifdef AMBA_AXI_CACHE
   wire  [ 3:0]             `NET_DELAY  AWCACHE ;
   `endif
   `ifdef AMBA_AXI_PROT
   wire  [ 2:0]             `NET_DELAY  AWPROT  ;
   `endif
   wire                     `NET_DELAY  AWVALID ;
   wire                     `NET_DELAY  AWREADY ;
   `ifdef AMBA_QOS
   wire  [ 3:0]             `NET_DELAY  AWQOS   ;
   wire  [ 3:0]             `NET_DELAY  AWREGION;
   `endif
   `ifdef AMBA_AXI_AWUSER
   wire  [WIDTH_AWUSER-1:0] `NET_DELAY  AWUSER  ;
   `endif
   `ifndef AMBA_AXI4
   wire  [WIDTH_ID-1:0]     `NET_DELAY  WID     ;
   `endif
   wire  [WIDTH_DA-1:0]     `NET_DELAY  WDATA   ;
   wire  [WIDTH_DS-1:0]     `NET_DELAY  WSTRB   ;
   wire                     `NET_DELAY  WLAST   ;
   wire                     `NET_DELAY  WVALID  ;
   wire                     `NET_DELAY  WREADY  ;
   `ifdef AMBA_AXI_WUSER
   wire  [WIDTH_WUSER-1:0]  `NET_DELAY  WUSER   ;
   `endif
   wire  [WIDTH_ID-1:0]     `NET_DELAY  BID     ;
   wire  [ 1:0]             `NET_DELAY  BRESP   ;
   wire                     `NET_DELAY  BVALID  ;
   wire                     `NET_DELAY  BREADY  ;
   `ifdef AMBA_AXI_BUSER
   wire  [WIDTH_BUSER-1:0]  `NET_DELAY  BUSER   ;
   `endif
   wire  [WIDTH_ID-1:0]     `NET_DELAY  ARID    ;
   wire  [WIDTH_AD-1:0]     `NET_DELAY  ARADDR  ;
   `ifdef AMBA_AXI4
   wire  [ 7:0]             `NET_DELAY  ARLEN   ;
   wire                     `NET_DELAY  ARLOCK  ;
   `else
   wire  [ 3:0]             `NET_DELAY  ARLEN   ;
   wire  [ 1:0]             `NET_DELAY  ARLOCK  ;
   `endif
   wire  [ 2:0]             `NET_DELAY  ARSIZE  ;
   wire  [ 1:0]             `NET_DELAY  ARBURST ;
   `ifdef AMBA_AXI_CACHE
   wire  [ 3:0]             `NET_DELAY  ARCACHE ;
   `endif
   `ifdef AMBA_AXI_PROT
   wire  [ 2:0]             `NET_DELAY  ARPROT  ;
   `endif
   wire                     `NET_DELAY  ARVALID ;
   wire                     `NET_DELAY  ARREADY ;
   `ifdef AMBA_QOS
   wire  [ 3:0]             `NET_DELAY  ARQOS   ;
   wire  [ 3:0]             `NET_DELAY  ARREGION;
   `endif
   `ifdef AMBA_AXI_ARUSER
   wire  [WIDTH_ARUSER-1:0] `NET_DELAY  ARUSER  ;
   `endif
   wire  [WIDTH_ID-1:0]     `NET_DELAY  RID     ;
   wire  [WIDTH_DA-1:0]     `NET_DELAY  RDATA   ;
   wire  [ 1:0]             `NET_DELAY  RRESP   ;
   wire                     `NET_DELAY  RLAST   ;
   wire                     `NET_DELAY  RVALID  ;
   wire                     `NET_DELAY  RREADY  ;
   `ifdef AMBA_AXI_RUSER
   wire  [WIDTH_RUSER-1:0]  `NET_DELAY  RUSER   ;
   `endif
   //---------------------------------------------------------
   wire          IRQ    =1'b0;
   wire  [31:0]  GPINOUT;
   //---------------------------------------------------------
   cosim_bfm_axi #(.AXI_WIDTH_ID (WIDTH_ID ) // ID width in bits
                  ,.AXI_WIDTH_AD (WIDTH_AD ) // address width
                  ,.AXI_WIDTH_DA (WIDTH_DA ) // data width
                  ,.COSIM_CHAN_ID(COSIM_CHAN_ID)
                  ,.COSIM_VERBOSE(COSIM_VERBOSE))
   u_bfm_axi(
         .ARESETn              ( ARESETn  )
       , .ACLK                 ( ACLK     )
       , .M_AWID               ( AWID     )
       , .M_AWADDR             ( AWADDR   )
       , .M_AWLEN              ( AWLEN    )
       , .M_AWLOCK             ( AWLOCK   )
       , .M_AWSIZE             ( AWSIZE   )
       , .M_AWBURST            ( AWBURST  )
       `ifdef AMBA_AXI_CACHE
       , .M_AWCACHE            ( AWCACHE  )
       `endif
       `ifdef AMBA_AXI_PROT
       , .M_AWPROT             ( AWPROT   )
       `endif
       , .M_AWVALID            ( AWVALID  )
       , .M_AWREADY            ( AWREADY  )
       `ifdef AMBA_QOS
       , .M_AWQOS              ( AWQOS    )
       , .M_AWREGION           ( AWREGION )
       `endif
       `ifndef AMBA_AXI4
       , .M_WID                ( WID      )
       `endif
       , .M_WDATA              ( WDATA    )
       , .M_WSTRB              ( WSTRB    )
       , .M_WLAST              ( WLAST    )
       , .M_WVALID             ( WVALID   )
       , .M_WREADY             ( WREADY   )
       , .M_BID                ( BID      )
       , .M_BRESP              ( BRESP    )
       , .M_BVALID             ( BVALID   )
       , .M_BREADY             ( BREADY   )
       , .M_ARID               ( ARID     )
       , .M_ARADDR             ( ARADDR   )
       , .M_ARLEN              ( ARLEN    )
       , .M_ARLOCK             ( ARLOCK   )
       , .M_ARSIZE             ( ARSIZE   )
       , .M_ARBURST            ( ARBURST  )
       `ifdef AMBA_AXI_CACHE
       , .M_ARCACHE            ( ARCACHE  )
       `endif
       `ifdef AMBA_AXI_PROT
       , .M_ARPROT             ( ARPROT   )
       `endif
       , .M_ARVALID            ( ARVALID  )
       , .M_ARREADY            ( ARREADY  )
       `ifdef AMBA_QOS
       , .M_ARQOS              ( ARQOS    )
       , .M_ARREGION           ( ARREGION )
       `endif
       , .M_RID                ( RID      )
       , .M_RDATA              ( RDATA    )
       , .M_RRESP              ( RRESP    )
       , .M_RLAST              ( RLAST    )
       , .M_RVALID             ( RVALID   )
       , .M_RREADY             ( RREADY   )
       , .IRQ   (IRQ    )
       , .GPIN  (GPINOUT)
       , .GPOUT (GPINOUT)
   );
   //---------------------------------------------------------
   mem_axi_beh #(.AXI_WIDTH_SID  (WIDTH_SID)// Channel ID width in bits
                ,.AXI_WIDTH_AD   (WIDTH_AD )// address width
                ,.AXI_WIDTH_DA   (WIDTH_DA )// data width
                ,.AXI_WIDTH_DS   (WIDTH_DS )// data strobe width
                ,.P_SIZE_IN_BYTES(1<<ADDR_LENGTH) // effective addre bits
                ,.P_DELAY_WRITE_SETUP  (`DELAY_WRITE_SETUP)
                ,.P_DELAY_WRITE_BURST  (`DELAY_WRITE_BURST)
                ,.P_DELAY_READ_SETUP   (`DELAY_READ_SETUP )
                ,.P_DELAY_READ_BURST   (`DELAY_READ_BURST ))
   u_mem_axi  (
          .ARESETn            ( ARESETn  )
        , .ACLK               ( ACLK     )
        , .AWID               ( AWID     )
        , .AWADDR             ( AWADDR   )
        , .AWLEN              ( AWLEN    )
        , .AWLOCK             ( AWLOCK   )
        , .AWSIZE             ( AWSIZE   )
        , .AWBURST            ( AWBURST  )
        `ifdef AMBA_AXI_CACHE
        , .AWCACHE            ( AWCACHE  )
        `endif
        `ifdef AMBA_AXI_PROT
        , .AWPROT             ( AWPROT   )
        `endif
        , .AWVALID            ( AWVALID  )
        , .AWREADY            ( AWREADY  )
        `ifdef AMBA_QOS
        , .AWQOS              ( AWQOS    )
        , .AWREGION           ( AWREGION )
        `endif
        `ifndef AMBA_AXI4
        , .WID                ( WID      )
        `endif
        , .WDATA              ( WDATA    )
        , .WSTRB              ( WSTRB    )
        , .WLAST              ( WLAST    )
        , .WVALID             ( WVALID   )
        , .WREADY             ( WREADY   )
        , .BID                ( BID      )
        , .BRESP              ( BRESP    )
        , .BVALID             ( BVALID   )
        , .BREADY             ( BREADY   )
        , .ARID               ( ARID     )
        , .ARADDR             ( ARADDR   )
        , .ARLEN              ( ARLEN    )
        , .ARLOCK             ( ARLOCK   )
        , .ARSIZE             ( ARSIZE   )
        , .ARBURST            ( ARBURST  )
        `ifdef AMBA_AXI_CACHE
        , .ARCACHE            ( ARCACHE  )
        `endif
        `ifdef AMBA_AXI_PROT
        , .ARPROT             ( ARPROT   )
        `endif
        , .ARVALID            ( ARVALID  )
        , .ARREADY            ( ARREADY  )
        `ifdef AMBA_QOS
        , .ARQOS              ( ARQOS    )
        , .ARREGION           ( ARREGION )
        `endif
        , .RID                ( RID      )
        , .RDATA              ( RDATA    )
        , .RRESP              ( RRESP    )
        , .RLAST              ( RLAST    )
        , .RVALID             ( RVALID   )
        , .RREADY             ( RREADY   )
   );
   //---------------------------------------------------------
   always #5 ACLK = ~ACLK;
   initial begin
       ACLK    = 0;
       ARESETn = 0;
       repeat (2) @ (posedge ACLK);
       ARESETn = 1;
       repeat (5) @ (posedge ACLK);
     //$finish(2);
   end
   //---------------------------------------------------------
   `ifdef VCD
   initial begin
       $dumpvars(0);
   end
   `endif
   //---------------------------------------------------------
endmodule
//----------------------------------------------------------------
// Revision History
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//----------------------------------------------------------------
