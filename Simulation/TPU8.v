// This works and includes systolic setup. Matrix sizes multiple of 8.
module TPU8
#(parameter Matrix_Size=8, parameter Data_Width=8)
(

	input	clk,
	input	start,
	input resetSystem,
	input	waitrequest,
	input	read,
	input [8:0]	output_address,
	input [63:0] inputA,
	input [63:0] inputB,
	input [13:0] blocks, // Base is 8. block 1 = 8x8. block 3 = 24x24


	output done,
	output reg [31:0] DataOutput

);


// * Internal Signals

genvar i;
genvar k;

//reg started;

// Internal Timer
reg [16:0] internal_timer;
reg [16:0] internal_timerplusone;
reg [16:0] internal_timer_c;

// Output Buffer
reg signed [31:0] Buffer[63:0];

// Input A & B
reg signed [(Data_Width-1):0] inA[(Matrix_Size-1):0], inB[(Matrix_Size-1):0];
reg signed [7:0] inA_c[7:0], inB_c[7:0];

reg signed [(Data_Width-1):0] inA1, inA1_c;
reg signed [(Data_Width-1):0] inA2[1:0], inA2_c[1:0];
reg signed [(Data_Width-1):0] inA3[2:0], inA3_c[2:0];
reg signed [(Data_Width-1):0] inA4[3:0], inA4_c[3:0];
reg signed [(Data_Width-1):0] inA5[4:0], inA5_c[4:0];
reg signed [(Data_Width-1):0] inA6[5:0], inA6_c[5:0];
reg signed [(Data_Width-1):0] inA7[6:0], inA7_c[6:0];

reg signed [(Data_Width-1):0] inB1, inB1_c;
reg signed [(Data_Width-1):0] inB2[1:0], inB2_c[1:0];
reg signed [(Data_Width-1):0] inB3[2:0], inB3_c[2:0];
reg signed [(Data_Width-1):0] inB4[3:0], inB4_c[3:0];
reg signed [(Data_Width-1):0] inB5[4:0], inB5_c[4:0];
reg signed [(Data_Width-1):0] inB6[5:0], inB6_c[5:0];
reg signed [(Data_Width-1):0] inB7[6:0], inB7_c[6:0];

// Temporary buffer from MAC
wire signed [31:0] result[63:0];

// Done Flag
assign done = (internal_timer >= (blocks + 5'd14)) ? 1'b1 : 1'b0;

// Timer
always @(posedge clk) begin
	if (resetSystem) internal_timer <= 17'd0;
	else if (~waitrequest && ~done) internal_timer <= internal_timerplusone;
	else internal_timer <= internal_timer_c;
end
always @(negedge clk) begin
	internal_timerplusone <= internal_timer + 1'b1;
	internal_timer_c <= internal_timer;
end


// ========================================================================================
// Input Row/Column 0
// ========================================================================================
always @(posedge clk) begin
	if (resetSystem) begin
			inA[0] <= 8'd0;
			inB[0] <= 8'd0;
	end
	else if (~waitrequest) begin
		inA[0] <= inputA[((1*Data_Width) - 1):0];
		inB[0] <= inputB[((1*Data_Width) - 1):0];
	end
	else begin
		inA[0] <= inA_c[0];
		inB[0] <= inB_c[0];
	end
end

// ========================================================================================
// Input Row/Column 1
// ========================================================================================
always @(posedge clk) begin
	if (resetSystem) begin
		inA[1] <= 8'd0;
		inB[1] <= 8'd0;
		inA1 <= 8'd0;
		inB1 <= 8'd0;
	end
	else if (~waitrequest) begin
		inA1 <= inputA[((2*Data_Width) - 1):(1*Data_Width)];
		inB1 <= inputB[((2*Data_Width) - 1):(1*Data_Width)];
		inA[1] <= inA1;
		inB[1] <= inB1;
	end
	else begin
		inA[1] <= inA_c[1];
		inB[1] <= inB_c[1];
		inA1 <= inA1_c;
		inB1 <= inB1_c;
	end
end

// ========================================================================================
// Input Row/Column 2
// ========================================================================================
always @(posedge clk) begin
	if (resetSystem) begin
			inA[2] <= 8'd0;
			inB[2] <= 8'd0;
			inA2[0] <= 8'd0;
			inB2[0] <= 8'd0;
			inA2[1] <= 8'd0;
			inB2[1] <= 8'd0;
	end
	else if (~waitrequest) begin
			inA2[1] <= inputA[((3*Data_Width) - 1):(2*Data_Width)];
			inB2[1] <= inputB[((3*Data_Width) - 1):(2*Data_Width)];
			inA2[0] <= inA2[1];
			inB2[0] <= inB2[1];
			inA[2] <= inA2[0];
			inB[2] <= inB2[0];
	end
	else begin
		inA[2] <= inA_c[2];
		inB[2] <= inB_c[2];
		inA2[0] <= inA2_c[0];
		inA2[1] <= inA2_c[1];
		inB2[0] <= inB2_c[0];
		inB2[1] <= inB2_c[1];
	end
end

// ========================================================================================
// Input Row/Column 3
// ========================================================================================
always @(posedge clk) begin
	if (resetSystem) begin
		inA[3] <= 8'd0;
		inB[3] <= 8'd0;
		inA3[0] <= 8'd0;
		inB3[0] <= 8'd0;
		inA3[1] <= 8'd0;
		inB3[1] <= 8'd0;
		inA3[2] <= 8'd0;
		inB3[2] <= 8'd0;
	end
	else if (~waitrequest) begin
		inA3[2] <= inputA[((4*Data_Width) - 1):(3*Data_Width)];
		inB3[2] <= inputB[((4*Data_Width) - 1):(3*Data_Width)];
		inA3[1] <= inA3[2];
		inB3[1] <= inB3[2];
		inA3[0] <= inA3[1];
		inB3[0] <= inB3[1];
		inA[3] <= inA3[0];
		inB[3] <= inB3[0];
	end
	else begin
		inA[3] <= inA_c[3];
		inB[3] <= inB_c[3];
		inA3[0] <= inA3_c[0];
		inA3[1] <= inA3_c[1];
		inA3[2] <= inA3_c[2];
		inB3[0] <= inB3_c[0];
		inB3[1] <= inB3_c[1];
		inB3[2] <= inB3_c[2];
	end
end

// ========================================================================================
// Input Row/Column 4
// ========================================================================================
always @(posedge clk) begin
	if (resetSystem) begin
		inA[4] <= 8'd0;
		inB[4] <= 8'd0;
		inA4[0] <= 8'd0;
		inB4[0] <= 8'd0;
		inA4[1] <= 8'd0;
		inB4[1] <= 8'd0;
		inA4[2] <= 8'd0;
		inB4[2] <= 8'd0;
		inA4[3] <= 8'd0;
		inB4[3] <= 8'd0;
	end
	else if (~waitrequest) begin
		inA4[3] <= inputA[((5*Data_Width) - 1):(4*Data_Width)];
		inB4[3] <= inputB[((5*Data_Width) - 1):(4*Data_Width)];
		inA4[2] <= inA4[3];
		inB4[2] <= inB4[3];
		inA4[1] <= inA4[2];
		inB4[1] <= inB4[2];
		inA4[0] <= inA4[1];
		inB4[0] <= inB4[1];
		inA[4] <= inA4[0];
		inB[4] <= inB4[0];
	end
	else begin
		inA[4] <= inA_c[4];
		inB[4] <= inB_c[4];
		inA4[0] <= inA4_c[0];
		inA4[1] <= inA4_c[1];
		inA4[2] <= inA4_c[2];
		inA4[3] <= inA4_c[3];
		inB4[0] <= inB4_c[0];
		inB4[1] <= inB4_c[1];
		inB4[2] <= inB4_c[2];
		inB4[3] <= inB4_c[3];
	end
end

// ========================================================================================
// Input Row/Column 5
// ========================================================================================
always @(posedge clk) begin
	if (resetSystem) begin
		inA[5] <= 8'd0;
		inB[5] <= 8'd0;
		inA5[0] <= 8'd0;
		inB5[0] <= 8'd0;
		inA5[1] <= 8'd0;
		inB5[1] <= 8'd0;
		inA5[2] <= 8'd0;
		inB5[2] <= 8'd0;
		inA5[3] <= 8'd0;
		inB5[3] <= 8'd0;
		inA5[4] <= 8'd0;
		inB5[4] <= 8'd0;
	end
	else if (~waitrequest) begin
		inA5[4] <= inputA[((6*Data_Width) - 1):(5*Data_Width)];
		inB5[4] <= inputB[((6*Data_Width) - 1):(5*Data_Width)];
		inA5[3] <= inA5[4];
		inB5[3] <= inB5[4];
		inA5[2] <= inA5[3];
		inB5[2] <= inB5[3];
		inA5[1] <= inA5[2];
		inB5[1] <= inB5[2];
		inA5[0] <= inA5[1];
		inB5[0] <= inB5[1];
		inA[5] <= inA5[0];
		inB[5] <= inB5[0];
	end
	else begin
		inA[5] <= inA_c[5];
		inB[5] <= inB_c[5];
		inA5[0] <= inA5_c[0];
		inA5[1] <= inA5_c[1];
		inA5[2] <= inA5_c[2];
		inA5[3] <= inA5_c[3];
		inA5[4] <= inA5_c[4];
		inB5[0] <= inB5_c[0];
		inB5[1] <= inB5_c[1];
		inB5[2] <= inB5_c[2];
		inB5[3] <= inB5_c[3];
		inB5[4] <= inB5_c[4];
	end
end

// ========================================================================================
// Input Row/Column 6
// ========================================================================================
always @(posedge clk) begin
	if (resetSystem) begin
		inA[6] <= 8'd0;
		inB[6] <= 8'd0;
		inA6[0] <= 8'd0;
		inB6[0] <= 8'd0;
		inA6[1] <= 8'd0;
		inB6[1] <= 8'd0;
		inA6[2] <= 8'd0;
		inB6[2] <= 8'd0;
		inA6[3] <= 8'd0;
		inB6[3] <= 8'd0;
		inA6[4] <= 8'd0;
		inB6[4] <= 8'd0;
		inA6[5] <= 8'd0;
		inB6[5] <= 8'd0;
	end
	else if (~waitrequest) begin
		inA6[5] <= inputA[((7*Data_Width) - 1):(6*Data_Width)];
		inB6[5] <= inputB[((7*Data_Width) - 1):(6*Data_Width)];
		inA6[4] <= inA6[5];
		inB6[4] <= inB6[5];
		inA6[3] <= inA6[4];
		inB6[3] <= inB6[4];
		inA6[2] <= inA6[3];
		inB6[2] <= inB6[3];
		inA6[1] <= inA6[2];
		inB6[1] <= inB6[2];
		inA6[0] <= inA6[1];
		inB6[0] <= inB6[1];
		inA[6] <= inA6[0];
		inB[6] <= inB6[0];
	end
	else begin
		inA[6] <= inA_c[6];
		inB[6] <= inB_c[6];
		inA6[0] <= inA6_c[0];
		inA6[1] <= inA6_c[1];
		inA6[2] <= inA6_c[2];
		inA6[3] <= inA6_c[3];
		inA6[4] <= inA6_c[4];
		inA6[5] <= inA6_c[5];
		inB6[0] <= inB6_c[0];
		inB6[1] <= inB6_c[1];
		inB6[2] <= inB6_c[2];
		inB6[3] <= inB6_c[3];
		inB6[4] <= inB6_c[4];
		inB6[5] <= inB6_c[5];
	end
end

// ========================================================================================
// Input Row/Column 7
// ========================================================================================
always @(posedge clk) begin
	if (resetSystem) begin
		inA[7] <= 8'd0;
		inB[7] <= 8'd0;
		inA7[0] <= 8'd0;
		inB7[0] <= 8'd0;
		inA7[1] <= 8'd0;
		inB7[1] <= 8'd0;
		inA7[2] <= 8'd0;
		inB7[2] <= 8'd0;
		inA7[3] <= 8'd0;
		inB7[3] <= 8'd0;
		inA7[4] <= 8'd0;
		inB7[4] <= 8'd0;
		inA7[5] <= 8'd0;
		inB7[5] <= 8'd0;
		inA7[6] <= 8'd0;
		inB7[6] <= 8'd0;	
	end
	else if (~waitrequest) begin
		inA7[6] <= inputA[((8*Data_Width) - 1):(7*Data_Width)];
		inB7[6] <= inputB[((8*Data_Width) - 1):(7*Data_Width)];
		inA7[5] <= inA7[6];
		inB7[5] <= inB7[6];
		inA7[4] <= inA7[5];
		inB7[4] <= inB7[5];
		inA7[3] <= inA7[4];
		inB7[3] <= inB7[4];
		inA7[2] <= inA7[3];
		inB7[2] <= inB7[3];
		inA7[1] <= inA7[2];
		inB7[1] <= inB7[2];
		inA7[0] <= inA7[1];
		inB7[0] <= inB7[1];
		inA[7] <= inA7[0];
		inB[7] <= inB7[0];
	end
	else begin
		inA[7] <= inA_c[7];
		inB[7] <= inB_c[7];
		inA7[0] <= inA7_c[0];
		inA7[1] <= inA7_c[1];
		inA7[2] <= inA7_c[2];
		inA7[3] <= inA7_c[3];
		inA7[4] <= inA7_c[4];
		inA7[5] <= inA7_c[5];
		inA7[6] <= inA7_c[6];
		inB7[0] <= inB7_c[0];
		inB7[1] <= inB7_c[1];
		inB7[2] <= inB7_c[2];
		inB7[3] <= inB7_c[3];
		inB7[4] <= inB7_c[4];
		inB7[5] <= inB7_c[5];
		inB7[6] <= inB7_c[6];
	end
end

// ========================================================================================
// Latches
// ========================================================================================
generate for (i = 0; i < 8; i = i + 1) begin : Sys1
always @(negedge clk) begin
	inA_c[i] = inA[i];
	inB_c[i] = inB[i];
end
end
endgenerate

always @(negedge clk) begin
	inA1_c = inA1;
	inB1_c = inB1;
end

generate for (i = 0; i < 2; i = i + 1) begin : Sys2
always @(negedge clk) begin
	inA2_c[i] = inA2[i];
	inB2_c[i] = inB2[i];
end
end
endgenerate
generate for (i = 0; i < 3; i = i + 1) begin : Sys3
always @(negedge clk) begin
	inA3_c[i] = inA3[i];
	inB3_c[i] = inB3[i];
end
end
endgenerate
generate for (i = 0; i < 4; i = i + 1) begin : Sys4
always @(negedge clk) begin
	inA4_c[i] = inA4[i];
	inB4_c[i] = inB4[i];
end
end
endgenerate
generate for (i = 0; i < 5; i = i + 1) begin : Sys5
always @(negedge clk) begin
	inA5_c[i] = inA5[i];
	inB5_c[i] = inB5[i];
end
end
endgenerate
generate for (i = 0; i < 6; i = i + 1) begin : Sys6
always @(negedge clk) begin
	inA6_c[i] = inA6[i];
	inB6_c[i] = inB6[i];
end
end
endgenerate
generate for (i = 0; i < 7; i = i + 1) begin : Sys7
always @(negedge clk) begin
	inA7_c[i] = inA7[i];
	inB7_c[i] = inB7[i];
end
end
endgenerate

// ========================================================================================
// Output from
// ========================================================================================
always @(posedge clk) begin
	if (read) begin
		DataOutput[31:0] <= Buffer[output_address];
	end
	else DataOutput <= 32'd0;
end

// ========================================================================================
// Results to Buffer
// ========================================================================================
generate for (i = 0; i < 8; i = i + 1) begin : Column0Buffer
	always @(posedge clk) begin
		if (resetSystem) Buffer[i] <= 32'd0;
		else if (internal_timer == (blocks + i)) Buffer[i] <= result[i];
	end
end
endgenerate

generate for (i = 0; i < 8; i = i + 1) begin : Column1Buffer
	always @(posedge clk) begin
		if (resetSystem) Buffer[i+8] <= 32'd0;
		else if (internal_timer == (blocks + i + 2'd1)) Buffer[i+8] <= result[i+8];
	end
end
endgenerate

generate for (i = 0; i < 8; i = i + 1) begin : Column2Buffer
	always @(posedge clk) begin
		if (resetSystem) Buffer[i+16] <= 32'd0;
		else if (internal_timer == (blocks + i  + 2'd2)) Buffer[i+16] <= result[i+16];
	end
end
endgenerate

generate for (i = 0; i < 8; i = i + 1) begin : Column3Buffer
	always @(posedge clk) begin
		if (resetSystem) Buffer[i+24] <= 32'd0;
		else if (internal_timer == (blocks + i  + 3'd3)) Buffer[i+24] <= result[i+24];
	end
end
endgenerate

generate for (i = 0; i < 8; i = i + 1) begin : Column4Buffer
	always @(posedge clk) begin
		if (resetSystem) Buffer[i+32] <= 32'd0;
		else if (internal_timer == (blocks + i + 3'd4)) Buffer[i+32] <= result[i+32];
	end
end
endgenerate

generate for (i = 0; i < 8; i = i + 1) begin : Column5Buffer
	always @(posedge clk) begin
		if (resetSystem) Buffer[i+40] <= 32'd0;
		else if (internal_timer == (blocks + i + 3'd5)) Buffer[i+40] <= result[i+40];
	end
end
endgenerate

generate for (i = 0; i < 8; i = i + 1) begin : Column6Buffer
	always @(posedge clk) begin
		if (resetSystem) Buffer[i+48] <= 32'd0;
		else if (internal_timer == (blocks + i + 3'd6)) Buffer[i+48] <= result[i+48];
	end
end
endgenerate

generate for (i = 0; i < 8; i = i + 1) begin : Column7Buffer
	always @(posedge clk) begin
		if (resetSystem) Buffer[i+56] <= 32'd0;
		else if (internal_timer == (blocks + i + 4'd7)) Buffer[i+56] <= result[i+56];
	end
end
endgenerate

// ========================================================================================
// Instantiate Wires and SYSMACs
// ========================================================================================
wire signed [(Data_Width-1):0] inLeft[((Matrix_Size*Matrix_Size)-1):0];
wire signed [(Data_Width-1):0] inTop[((Matrix_Size*Matrix_Size)-1):0];

// ========================================================================================
// 0x0 MAC
// ========================================================================================
SYSMAC #(Data_Width) mac(.inLeft(inA[0]), .inTop(inB[0]), .clk(clk), .rst(resetSystem), .waitrequest(waitrequest), .outRight(inLeft[0]), .outBottom(inTop[0]), .outResult(result[0]));

// ========================================================================================
// Row 0 MACs minus 0x0 MAC
// ========================================================================================
generate for (i = 0; i < Matrix_Size-1; i = i + 1) begin : RowZero
	SYSMAC #(Data_Width) mac(.inLeft(inLeft[i*Matrix_Size]), .inTop(inB[i+1]), .clk(clk), .rst(resetSystem), .waitrequest(waitrequest), .outRight(inLeft[(i+1)*Matrix_Size]), .outBottom(inTop[(i+1)*Matrix_Size]), .outResult(result[(i+1)*Matrix_Size]));
end
endgenerate

// ========================================================================================
// Column 0 MACs minus 0x0 MAC
// ========================================================================================
generate for (i = 0; i < Matrix_Size-1; i = i + 1) begin : ColumnZero
	SYSMAC #(Data_Width) mac(.inLeft(inA[i+1]), .inTop(inTop[i]), .clk(clk), .rst(resetSystem), .waitrequest(waitrequest), .outRight(inLeft[i+1]), .outBottom(inTop[i+1]), .outResult(result[i+1]));
end
endgenerate

// ========================================================================================
// MACs by column k and row i. no 0x0 row/column
// ========================================================================================
generate for (k = 0; k < Matrix_Size-1; k = k + 1) begin : RestofMACsone//Column 
		 for (i = 0; i < Matrix_Size-1; i = i + 1) begin : RestofMacstwo//Row
			SYSMAC #(Data_Width) mac(.inLeft(inLeft[i+1+(k*Matrix_Size)]), .inTop(inTop[i+Matrix_Size+(k*Matrix_Size)]), .clk(clk), .rst(resetSystem), .waitrequest(waitrequest), .outRight(inLeft[i+Matrix_Size+1+(k*Matrix_Size)]), .outBottom(inTop[i+Matrix_Size+1+(k*Matrix_Size)]), .outResult(result[i+Matrix_Size+1+(k*Matrix_Size)]));
	end
end
endgenerate

endmodule
