module FinalDesign (
	////////////////////////////////////
	// FPGA Pins
	////////////////////////////////////

	// Clock pin
	CLOCK,
	control_to_FPGA,
	control_to_HPS
);

// Clock pin
input CLOCK;
input [31:0] control_to_FPGA;
output reg control_to_HPS;

//=======================================================
//  REG/WIRE declarations
//=======================================================
reg 	 [63:0] 	inputA, inputB, inputA_c, inputB_c;
reg 				waitrequest;
reg 				start;
reg 				reset;
reg 				tpu_read;
reg unsigned [8:0] 	tpu_address;
reg	unsigned [8:0]	tpu_addressplusone;
reg unsigned 	[13:0] 	blocks;
reg unsigned 	[17:0] 	completed, completed_c;
reg	unsigned	[17:0]	completedplusone;
wire					done;
wire 	 		[31:0] 	tpu_output;
//=======================================================
// Controls for Qsys sram slave exported in system module
//=======================================================
wire 	[31:0] 	sram_readdata = 32'b0000_0100_0000_0011_0000_0010_0000_0001;
wire 	[31:0] 	sram_writedata;
reg unsigned 	[11:0] 	sram_address, sram_address_c;
reg unsigned	[11:0]	sram_addressplusone;
reg 				sram_write;
// wire 				sram_clken = 1'b1;
// wire 				sram_chipselect = 1'b1;
reg 	[5:0] 	state;
reg 	[5:0]	next_state;

wire 	[4:0]	largematrix;
//wire CLOCK;

assign sram_writedata = tpu_output;

assign largematrix = control_to_FPGA[31:27];

module RAM(input CLOCK, input [11:0] sram_address, input [31:0] sram_writedata, input sram_write);
reg [31:0] RAM1[63:0];
always @(posedge CLOCK) begin
	if (sram_write) RAM1[sram_address] <= sram_readdata;
end
endmodule
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
		next_state <= ((completed == 13'd4096 && largematrix >= 4) ||
					(completed == 13'd3072 && largematrix >= 3) ||
					(completed == 12'd2048 && largematrix >= 2) || 
					(completed == 11'd1024 && largematrix >= 1)) ? 6'd13 : (completed == blocks) ? 6'd9 : 6'd2;
		
		
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
		// Begin Writing to SRAm
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
endmodule 