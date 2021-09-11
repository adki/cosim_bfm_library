`ifndef COSIM_BFM_AXI_TASKS_V
`define COSIM_BFM_AXI_TASKS_V
//------------------------------------------------------------------------------
//  Copyright (c) 2021 by Ando Ki.
//  All right reserved by Ando KI.
//------------------------------------------------------------------------------
// cosim_bfm_axi_tasks.v
//------------------------------------------------------------------------------
`ifdef COSIM_DPI
bit [7:0] axi_dataWB[0:`COSIM_DATA_BNUM-1];
bit [7:0] axi_dataRB[0:`COSIM_DATA_BNUM-1];
`elsif COSIM_VPI
reg [7:0] axi_dataWB[0:`COSIM_DATA_BNUM-1];
reg [7:0] axi_dataRB[0:`COSIM_DATA_BNUM-1];
`endif
//------------------------------------------------------------------------------
task axi_read_task;
     input [31:0]             tid;  // transaction id
     input [AXI_WIDTH_AD-1:0] addr;
     input [31:0]             size; // 1 ~ 128 byte in a beat
     input [31:0]             leng; // 1 ~ 16  beats in a burst
     input [31:0]             burst; // burst type
begin
     fork
     axi_read_address_channel(tid,addr,size,leng,burst);
     axi_read_data_channel(tid,addr,size,leng,burst);
     join
end
endtask
//------------------------------------------------------------------------------
task axi_read_address_channel;
     input [31:0]             tid;
     input [AXI_WIDTH_AD-1:0] addr;
     input [31:0]             size; // 1 ~ 128 byte in a beat
     input [31:0]             leng; // 1 ~ 16  beats in a burst
     input [31:0]             burst; // burst type
begin
     @ (posedge ACLK);
     M_ARID    <= #1 tid;
     M_ARADDR  <= #1 addr;
     M_ARLEN   <= #1 leng-1;
     M_ARLOCK  <= #1 'b0;
     M_ARSIZE  <= #1 axi_get_size(size);
     M_ARBURST <= #1  burst[1:0];
     `ifdef AMBA_AXI_PROT
     M_ARPROT  <= #1 'h0; // data, secure, normal
     `endif
     M_ARVALID <= #1 'b1;
     @ (posedge ACLK);
     while (M_ARREADY==1'b0) @ (posedge ACLK);
     M_ARVALID <= #1 'b0;
     @ (negedge ACLK);
end
endtask
//------------------------------------------------------------------------------
task axi_read_data_channel;
     input [31:0]             tid;
     input [AXI_WIDTH_AD-1:0] addr;
     input [31:0]             size; // 1 ~ 128 byte in a beat
     input [31:0]             leng; // 1 ~ 16  beats in a burst
     input [31:0]             burst; // burst type
     reg   [AXI_WIDTH_AD-1:0] naddr;
     reg   [AXI_WIDTH_DS-1:0] strb;
   //reg   [AXI_WIDTH_DA-1:0] maskT;
     reg   [AXI_WIDTH_DA-1:0] dataR;
     integer idx, idy, idz;
begin
     idz = 0;
     naddr  = addr;
     @ (posedge ACLK);
     M_RREADY <= #1 1'b1;
     for (idx=0; idx<leng; idx=idx+1) begin
          @ (posedge ACLK);
          while (M_RVALID==1'b0) @ (posedge ACLK);
          strb = axi_get_strb(naddr, size);
          dataR = M_RDATA;
          for (idy=0; idy<AXI_WIDTH_DS; idy=idy+1) begin
               //if (strb[idy]) axi_dataRB[naddr-addr+idy] = dataR&8'hFF;
               if (strb[idy]) begin
                   axi_dataRB[idz] = dataR&8'hFF; // justified
                   idz = idz + 1;
               end
               dataR = dataR>>8;
          end
          //for (idy=(AXI_WIDTH_DS-1); idy>=0; idy=idy-1) begin
          //     if (strb[idy]) maskT = (maskT<<8)|{8{1'b1}};
          //     else           maskT = (maskT<<8);
          //end
          //dataR[idx] = (dataR[idx]&~maskT)|(M_RDATA&maskT);
          if (tid!=M_RID) begin
             $display($time,,"%m Error tid/RID mis-match for read-data-channel", tid, M_RID);
          end
          if (idx==leng-1) begin
             if (M_RLAST==1'b0) begin
                 $display($time,,"%m Error RLAST expected for read-data-channel");
             end
          end else begin
              @ (negedge ACLK);
              naddr = axi_get_next_addr( naddr  // current address
                                   , size  // num of bytes in a beat
                                   , burst);// type of burst
          end
     end
     M_RREADY <= #1 'b0;
     @ (negedge ACLK);
end
endtask
//------------------------------------------------------------------------------
task axi_write_task;
     input [31:0]             tid;  // transaction id
     input [AXI_WIDTH_AD-1:0] addr;
     input [31:0]             size; // 1 ~ 128 byte in a beat
     input [31:0]             leng; // 1 ~ 16  beats in a burst
     input [31:0]             burst; // burst type
begin
     fork
     axi_write_address_channel(tid,addr,size,leng,burst);
     axi_write_data_channel(tid,addr,size,leng,burst);
     axi_write_resp_channel(tid);
     join
end
endtask
//------------------------------------------------------------------------------
task axi_write_address_channel;
     input [31:0]             tid;
     input [AXI_WIDTH_AD-1:0] addr;
     input [31:0]             size; // 1 ~ 128 byte in a beat
     input [31:0]             leng; // 1 ~ 16  beats in a burst
     input [31:0]             burst; // burst type
begin
     @ (posedge ACLK);
     M_AWID    <= #1 tid;
     M_AWADDR  <= #1 addr;
     M_AWLEN   <= #1 leng-1;
     M_AWLOCK  <= #1 'b0;
     M_AWSIZE  <= #1 axi_get_size(size);
     M_AWBURST <= #1  burst[1:0];
     `ifdef AMBA_AXI_PROT
     M_AWPROT  <= #1 'h0; // data, secure, normal
     `endif
     M_AWVALID <= #1 'b1;
     @ (posedge ACLK);
     while (M_AWREADY==1'b0) @ (posedge ACLK);
     M_AWVALID <= #1 'b0;
     @ (negedge ACLK);
end
endtask
//------------------------------------------------------------------------------
task axi_write_data_channel;
     input [31:0]             tid;
     input [AXI_WIDTH_AD-1:0] addr;
     input [31:0]             size; // 1 ~ 128 byte in a beat
     input [31:0]             leng; // 1 ~ 16  beats in a burst
     input [31:0]             burst; // burst type
     reg   [AXI_WIDTH_AD-1:0] naddr;
     integer idx;
begin
     naddr  = addr;
     @ (posedge ACLK);
     `ifndef AMBA_AXI4
     M_WID    <= #1 tid;
     `endif
     M_WVALID <= #1 1'b1;
     for (idx=0; idx<leng; idx=idx+1) begin
          M_WDATA <= #1 axi_get_data(addr, naddr, size);
          M_WSTRB <= #1 axi_get_strb(naddr, size);
          M_WLAST <= #1 (idx==(leng-1));
          naddr <= axi_get_next_addr(naddr, size, burst);
          @ (posedge ACLK);
          while (M_WREADY==1'b0) @ (posedge ACLK);
     end
     M_WLAST  <= #1 'b0;
     M_WVALID <= #1 'b0;
     @ (negedge ACLK);
end
endtask
//------------------------------------------------------------------------------
task axi_write_resp_channel;
     input [31:0] tid;
begin
     M_BREADY <= #1 'b1;
     @ (posedge ACLK);
     while (M_BVALID==1'b0) @ (posedge ACLK);
     if (tid!=M_BID) begin
        $display($time,,"%m Error tid mis-match for write-resp-channel 0x%x/0x%x", tid, M_BID);
     end else begin
         case (M_BRESP)
         `ifdef DEBUG
         2'b00: begin
                $display($time,,"%m OK response for write-resp-channel: OKAY");
                end
         `endif
         2'b01: $display($time,,"%m OK response for write-resp-channel: EXOKAY");
         2'b10: $display($time,,"%m Error response for write-resp-channel: SLVERR");
         2'b11: $display($time,,"%m Error response for write-resp-channel: DECERR");
         endcase
     end
     M_BREADY <= #1 'b0;
     @ (negedge ACLK);
end
endtask
//------------------------------------------------------------------------------
// input: num of bytes
// output: AxSIZE[2:0] code
function [2:0] axi_get_size;
   input [7:0] size;
begin
   case (size)
     1: axi_get_size = 0;
     2: axi_get_size = 1;
     4: axi_get_size = 2;
     8: axi_get_size = 3;
    16: axi_get_size = 4;
    32: axi_get_size = 5;
    64: axi_get_size = 6;
   128: axi_get_size = 7;
   default: axi_get_size = 0;
   endcase
end
endfunction
//------------------------------------------------------------------------------
function [AXI_WIDTH_DS-1:0] axi_get_strb;
    input [31:0] addr;
    input [31:0] size; // num of bytes in a beat
    integer offset;
    reg   [127:0] bit_size;
begin
    offset   = addr%AXI_WIDTH_DS;
    case (size)
      1: bit_size = {  1{1'b1}};
      2: bit_size = {  2{1'b1}};
      4: bit_size = {  4{1'b1}};
      8: bit_size = {  8{1'b1}};
     16: bit_size = { 16{1'b1}};
     32: bit_size = { 32{1'b1}};
     64: bit_size = { 64{1'b1}};
    128: bit_size = {128{1'b1}};
    default: bit_size = 0;
    endcase
    axi_get_strb = bit_size<<offset;
end
endfunction
//------------------------------------------------------------------------------
function [AXI_WIDTH_AD-1:0] axi_get_next_addr;
    input [31:0] addr; // current address
    input [31:0] size; // num of bytes in a beat
    input [31:0] burst; // type of burst
    integer offset;
begin
    case (burst[1:0])
    2'b00: axi_get_next_addr = addr; // fixed
    2'b01: begin // increment
           offset = addr%AXI_WIDTH_DS;
           if ((offset+size)<=AXI_WIDTH_DS) begin
               axi_get_next_addr = addr + size;
           end else begin // (offset+size)>nb
               axi_get_next_addr = addr + AXI_WIDTH_DS - size;
           end
           end
    2'b10: begin // wrap
           if ((addr%size)!=0) begin
              $display($time,,"%m wrap-burst not aligned");
              axi_get_next_addr = addr;
           end else begin
               offset = addr%AXI_WIDTH_DS;
               if ((offset+size)<=AXI_WIDTH_DS) begin
                   axi_get_next_addr = addr + size;
               end else begin // (offset+size)>nb
                   axi_get_next_addr = addr + AXI_WIDTH_DS - size;
               end
           end
           end
    default: $display($time,,"%m Error un-defined burst-type: %2b", burst);
    endcase
end
endfunction
//------------------------------------------------------------------------------
// axi_dataWB[0]   = saddr + 0;
// axi_dataWB[1]   = saddr + 1;
// axi_dataWB[2]   = saddr + 2;
//
function [AXI_WIDTH_DA-1:0] axi_get_data;
    input [AXI_WIDTH_AD-1:0] saddr; // start address
    input [AXI_WIDTH_AD-1:0] addr;  // current address
    input [31:0]             size;
    reg   [ 7:0]             data[0:AXI_WIDTH_DS-1];
    integer offset, idx, idy, idz, ids;
begin
    `ifdef RIGOR
    for (idx=0; idx<AXI_WIDTH_DS; idx=idx+1) begin
         data[idx] = 'bX;
    end
    `endif
    offset = addr%AXI_WIDTH_DS;
    ids = 0;
    for (idx=addr%AXI_WIDTH_DS; (idx<AXI_WIDTH_DS)&&(ids<size); idx=idx+1) begin
         idz = addr+(idx-offset)-saddr;
         data[idx] = axi_dataWB[idz];
//$display($time,,"%m sa=%04x ad=%04x idx=%03d idz=%03d", saddr, addr, idx, idz);
         ids = ids + 1;
    end
    axi_get_data = 0;
    for (idy=0; idy<AXI_WIDTH_DS; idy=idy+1) begin
         axi_get_data = axi_get_data|(data[idy]<<(8*idy));
    end
    //for (idy=AXI_WIDTH_DS-1; idy>=0; idy=idy-1) begin
    //     axi_get_data = (axi_get_data<<8)|data[idy];
    //end
end
endfunction
//------------------------------------------------------------------------------
// Revision History
//
// 2021.07.01: Started by Ando Ki (andoki@gmail.com)
//------------------------------------------------------------------------------
`endif
