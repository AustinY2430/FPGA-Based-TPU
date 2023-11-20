module FinalDesign (
	////////////////////////////////////
	// FPGA Pins
	////////////////////////////////////

	// Clock pin
	CLOCK_50,
	
	////////////////////////////////////
	// HPS Pins
	////////////////////////////////////
	
	// DDR3 SDRAM
	HPS_DDR3_ADDR,
	HPS_DDR3_BA,
	HPS_DDR3_CAS_N,
	HPS_DDR3_CKE,
	HPS_DDR3_CK_N,
	HPS_DDR3_CK_P,
	HPS_DDR3_CS_N,
	HPS_DDR3_DM,
	HPS_DDR3_DQ,
	HPS_DDR3_DQS_N,
	HPS_DDR3_DQS_P,
	HPS_DDR3_ODT,
	HPS_DDR3_RAS_N,
	HPS_DDR3_RESET_N,
	HPS_DDR3_RZQ,
	HPS_DDR3_WE_N
);

////////////////////////////////////
// FPGA Pins
////////////////////////////////////

// Clock pin
input CLOCK_50;

////////////////////////////////////
// HPS Pins
////////////////////////////////////

// DDR3 SDRAM
output	[14:0]	HPS_DDR3_ADDR;
output	[2:0] 	HPS_DDR3_BA;
output				HPS_DDR3_CAS_N;
output				HPS_DDR3_CKE;
output				HPS_DDR3_CK_N;
output				HPS_DDR3_CK_P;
output				HPS_DDR3_CS_N;
output	[3:0]		HPS_DDR3_DM;
inout		[31:0]	HPS_DDR3_DQ;
inout		[3:0]		HPS_DDR3_DQS_N;
inout		[3:0]		HPS_DDR3_DQS_P;
output				HPS_DDR3_ODT;
output				HPS_DDR3_RAS_N;
output				HPS_DDR3_RESET_N;
input					HPS_DDR3_RZQ;
output				HPS_DDR3_WE_N;

//=======================================================
//  REG/WIRE declarations
//=======================================================
reg 	 [63:0] 	inputA, inputB, inputA_c, inputB_c;
reg 				waitrequest;
reg 				start;
reg 				reset;
reg 				tpu_read;
reg unsigned [8:0] 	tpu_address, tpu_addressplusone, tpu_address_c;
reg 		[31:0] 	control_to_FPGA;
reg 				control_to_HPS;
wire  	[13:0] 	blocks;
reg unsigned 	[17:0] 	completed, completedplusone, completed_c;
wire					done;
wire 	 		[31:0] 	tpu_output;
//=======================================================
// Controls for Qsys sram slave exported in system module
//=======================================================
wire 	[31:0] 	sram_readdata;
wire 	[31:0] 	sram_writedata;
reg unsigned 	[11:0] 	sram_address, sram_addressplusone, sram_address_c;
reg 				sram_write;
wire 				sram_clken = 1'b1;
wire 				sram_chipselect = 1'b1;
reg 	[5:0] 	state;
reg 	[5:0]	next_state;

wire [4:0] largematrix;
reg [4:0] largematrix_count;
reg [4:0] largematrix_count_c;
wire CLOCK;

assign sram_writedata = tpu_output;
assign largematrix = control_to_FPGA[31:27];
assign blocks = (control_to_FPGA[1]) ? control_to_FPGA[17:4] : 14'd0;

// =======================================================
// TPU Module
// =======================================================
TPU8 tpu(
	.clk(CLOCK), 
	.start(start), 
	.resetSystem(reset), 
	.waitrequest(waitrequest), 
	.read(tpu_read), 
	.output_address(tpu_address), 
	.inputA(inputA), 
	.inputB(inputB),
	.blocks(blocks), 
	.done(done), 
	.DataOutput(tpu_output)
);

// FSM Control
always @(negedge CLOCK) begin
	if (control_to_FPGA[0]) state <= 6'd0;
	else state <= next_state;
	tpu_addressplusone <= tpu_address + 1'b1;
	sram_addressplusone <= sram_address + 1'b1;
	completedplusone <= completed + 1'b1;
	completed_c <= completed;
	sram_address_c <= sram_address;
	inputA_c <= inputA;
	inputB_c <= inputB;
	largematrix_count <= largematrix_count_c;
end

// FSM Next State
always @(posedge CLOCK) begin
case (state)
	6'd0: begin 
		// Reset
		waitrequest <= 1'b1;
		control_to_HPS <= 1'b0;
		sram_write <= 1'b0;
		sram_address <= 12'd0;
		tpu_address <= 9'd0;
		tpu_read <= 1'b0;
		completed <= 18'd0;
		inputA <= 64'd0;
		inputB <= 64'd0;
		start <= (control_to_FPGA[1]) ? 1'b1 : 1'b0;
		reset <= (control_to_FPGA[1]) ? 1'b0 : 1'b1;
		next_state <= (control_to_FPGA[1]) ? 1'b1 : 1'b0;
	end
	6'd1: begin
		// Start Reading: Wait 1 Cycle
		waitrequest <= 1'b1;
		next_state <= ((completed == 11'd1024*largematrix_count) && (largematrix != 1'b0)) ? 6'd13 : (completed == blocks) ? 6'd9 : 6'd2;
		largematrix_count_c <= ((completed == 11'd1024*largematrix_count)) ? (largematrix_count + 1'b1) : largematrix_count;
		
		// Not Used	
		completed <= completed_c;
		sram_address <= sram_address_c;
		tpu_address <= 9'd0;
		control_to_HPS <= 1'b0;
		sram_write <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
		inputA <= inputA_c;
		inputB <= inputB_c;
		tpu_read <= 1'b0;
	end
	6'd2: begin
		// Read A[31:0]
		waitrequest <= 1'b1;
		sram_address <= sram_addressplusone;
		inputA[31:0] <= sram_readdata;
		next_state <= 6'd3;
		
		
		// Not Used	
		completed <= completed_c;
		tpu_address <= 9'd0;
		control_to_HPS <= 1'b0;
		sram_write <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
		inputA[63:32] <= inputA_c[63:32];
		inputB <= inputB_c;
		tpu_read <= 1'b0;
	end
	6'd3: begin
		// Wait 1 Cycle
		waitrequest <= 1'b1;
		next_state <= 6'd4;
		
		
		// Not Used	
		completed <= completed_c;
		sram_address <= sram_address_c;
		tpu_address <= 9'd0;
		control_to_HPS <= 1'b0;
		sram_write <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
		inputA <= inputA_c;
		inputB <= inputB_c;
		tpu_read <= 1'b0;
	end
	6'd4: begin
		// Read A[63:32]
		waitrequest <= 1'b1;
		sram_address <= sram_addressplusone;
		inputA[63:32] <= sram_readdata;
		next_state <= 6'd5;
		
		
		// Not Used	
		completed <= completed_c;
		tpu_address <= 9'd0;
		control_to_HPS <= 1'b0;
		sram_write <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
		inputA[31:0] <= inputA_c[31:0];
		inputB <= inputB_c;
		tpu_read <= 1'b0;
	end
	6'd5: begin
		// Wait 1 Cycle
		waitrequest <= 1'b1;
		next_state <= 6'd6;
		
		
		// Not Used	
		completed <= completed_c;
		sram_address <= sram_address_c;
		tpu_address <= 9'd0;
		control_to_HPS <= 1'b0;
		sram_write <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
		inputA <= inputA_c;
		inputB <= inputB_c;
		tpu_read <= 1'b0;
	end
	6'd6: begin
		// Read B[31:0]
		waitrequest <= 1'b1;
		sram_address <= sram_addressplusone;
		inputB[31:0] <= sram_readdata;
		next_state <= 6'd7;
		
		
		// Not Used	
		completed <= completed_c;
		tpu_address <= 9'd0;
		control_to_HPS <= 1'b0;
		sram_write <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
		inputA <= inputA_c;
		inputB[63:32] <= inputB_c[63:32];
		tpu_read <= 1'b0;
	end
	6'd7: begin
		// Wait 1 Cycle
		waitrequest <= 1'b1;
		next_state <= 6'd8;
		
		
		// Not Used	
		completed <= completed_c;
		sram_address <= sram_address_c;
		tpu_address <= 9'd0;
		control_to_HPS <= 1'b0;
		sram_write <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
		inputA <= inputA_c;
		inputB <= inputB_c;
		tpu_read <= 1'b0;
	end
	6'd8: begin
		// Read B[63:32], Start TPU
		waitrequest <= 1'b0;
		sram_address <= sram_addressplusone;
		completed <= completedplusone;
		inputB[63:32] <= sram_readdata;
		next_state <= 6'd1;
		
		
		// Not Used	
		tpu_address <= 9'd0;
		control_to_HPS <= 1'b0;
		sram_write <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
		inputA <= inputA_c;
		inputB[31:0] <= inputB_c[31:0];
		tpu_read <= 1'b0;
	end
	6'd9: begin
		// Wait for TPU to Finish
		inputA <= 64'd0;
		inputB <= 64'd0;
		sram_address <= 12'd0;
		tpu_address <= 9'd0;
		waitrequest <= 1'b0;
		next_state <= (done) ? 6'd10 : 6'd9;
		
		
		// Not Used	
		completed <= completed_c;
		control_to_HPS <= 1'b0;
		sram_write <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
		tpu_read <= 1'b0;
	end
	6'd10: begin
		// Begin Writing to Sram
		inputA <= 64'd0;
		inputB <= 64'd0;
		waitrequest <= 1'b1;
		sram_address <= 12'd0;
		tpu_address <= 9'd0;
		tpu_read <= 1'b1;
		sram_write <= 1'b1;
		next_state <= 6'd11;
		
		
		// Not Used	
		completed <= completed_c;
		control_to_HPS <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
	end
	6'd11: begin
		// Write to SRAM
		inputA <= 64'd0;
		inputB <= 64'd0;
		waitrequest <= 1'b1;
		tpu_read <= 1'b1;
		sram_write <= 1'b1;
		tpu_address <= tpu_addressplusone;
		sram_address <= sram_addressplusone;
		next_state <= (tpu_address == 8'd63) ? 6'd12 : 6'd11;
		
		// Not Used	
		completed <= completed_c;
		control_to_HPS <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
	end
	6'd12: begin
		// Done
		inputA <= 64'd0;
		inputB <= 64'd0;
		waitrequest <= 1'b1;
		control_to_HPS <= 1'b1;
		tpu_read <= 1'b0;
		sram_write <= 1'b0;
		
		// Not Used	
		completed <= completed_c;
		sram_address <= sram_address_c;
		tpu_address <= 9'd0;
		start <= 1'b1;
		reset <= 1'b0;
	end
	6'd13: begin
		// Larger than 1024
		sram_address <= 12'd0;
		control_to_HPS <= 1'b1;
		next_state <= (control_to_FPGA[2]) ? 6'd14 : 6'd13;
		
		// Not Used	
		completed <= completed_c;
		tpu_address <= 9'd0;
		sram_write <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
		inputA <= inputA_c;
		inputB <= inputB_c;
		tpu_read <= 1'b0;
	end
	6'd14: begin
		// Wait 1 Cycle
		control_to_HPS <= 1'b0;
		next_state <= 6'd2;
		
		// Not Used	
		completed <= completed_c;
		sram_address <= sram_address_c;
		tpu_address <= 9'd0;
		sram_write <= 1'b0;
		start <= 1'b1;
		reset <= 1'b0;
		inputA <= inputA_c;
		inputB <= inputB_c;
		tpu_read <= 1'b0;
	end
	default: next_state <= 6'd0;
endcase
end

systemfinal u0(
	// Global Signals
   .system_pll_ref_clk_clk     (CLOCK_50),     //    system_pll_ref_clk.clk
   .system_pll_ref_reset_reset (1'b0), //  system_pll_ref_reset.reset

	// HPS DDR3 SDRAM
	.memory_mem_a			(HPS_DDR3_ADDR),
	.memory_mem_ba			(HPS_DDR3_BA),
	.memory_mem_ck			(HPS_DDR3_CK_P),
	.memory_mem_ck_n		(HPS_DDR3_CK_N),
	.memory_mem_cke		(HPS_DDR3_CKE),
	.memory_mem_cs_n		(HPS_DDR3_CS_N),
	.memory_mem_ras_n		(HPS_DDR3_RAS_N),
	.memory_mem_cas_n		(HPS_DDR3_CAS_N),
	.memory_mem_we_n		(HPS_DDR3_WE_N),
	.memory_mem_reset_n	(HPS_DDR3_RESET_N),
	.memory_mem_dq			(HPS_DDR3_DQ),
	.memory_mem_dqs		(HPS_DDR3_DQS_P),
	.memory_mem_dqs_n		(HPS_DDR3_DQS_N),
	.memory_mem_odt		(HPS_DDR3_ODT),
	.memory_mem_dm			(HPS_DDR3_DM),
	.memory_oct_rzqin		(HPS_DDR3_RZQ),
	
	// Clock Bridge	
	.clock_bridge_1_out_clk_clk(CLOCK),
	 
	// FPGA SRAM
   .onship_sram_s1_address     (sram_address),
   .onship_sram_s1_clken       (sram_clken),
   .onship_sram_s1_chipselect  (sram_chipselect),
   .onship_sram_s1_write       (sram_write),
   .onship_sram_s1_readdata    (sram_readdata),
   .onship_sram_s1_writedata   (sram_writedata),
   .onship_sram_s1_byteenable  (4'b1111),
	 
	// HPS & FPGA Control Signals
   .control_to_hps_export      (control_to_HPS),
   .control_to_fpga_export     (control_to_FPGA)
);
endmodule // end top level
