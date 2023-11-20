module SYSMAC
#(parameter Data_Width=8)
(

	input signed [(Data_Width-1):0] inLeft,
	input signed [(Data_Width-1):0] inTop,
	input clk,
	input rst,
	input waitrequest,
	
	output reg signed [(Data_Width-1):0] outRight,
	output reg signed [(Data_Width-1):0] outBottom,
	output wire signed [31:0] outResult
	
);

reg signed [7:0] outRight_c;
reg signed [7:0] outBottom_c;
reg signed [31:0] outResult_c;

wire signed [31:0] a_in;
wire signed [15:0] multiplyc;

always @(negedge clk) begin
	outRight_c <= outRight;
	outBottom_c <= outBottom;
end

always @(posedge clk) begin
	if (rst) begin
		outRight <= 8'd0;
		outBottom <= 8'd0;
		outResult_c <= 32'd0;
	end
	else if (waitrequest) begin
		outRight <= outRight_c;
		outBottom <= outBottom_c;
		outResult_c <= outResult;
	end
	else begin
		outRight <= inLeft;
		outBottom <= inTop;
		outResult_c <= outResult;
	end
end

assign multiplyc = inLeft * inTop;

assign a_in = (waitrequest||rst) ? 32'd0 : {{16{multiplyc[15]}}, multiplyc};

NewAdd adder(.dataa(a_in),.datab(outResult_c),.result(outResult));

endmodule


