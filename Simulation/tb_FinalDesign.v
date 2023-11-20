`timescale 1ns/10ps

// Test bench module
module tb_FinalDesign#(parameter Matrix_Size=8, parameter Data_Width=8);

// Input Array
/////////////////////////////////////////////////////////
//                  Test Bench Signals                 //
/////////////////////////////////////////////////////////

reg clk;
integer i,j,k;
reg [31:0] control_to_FPGA;
wire control_to_HPS;
// Matrices


/////////////////////////////////////////////////////////
//                  I/O Declarations                   //
/////////////////////////////////////////////////////////
// declare variables to hold signals going into submodule


/////////////////////////////////////////////////////////
//              Submodule Instantiation                //
/////////////////////////////////////////////////////////

/*****************************************
|------|    RENAME TO MATCH YOUR MODULE */
FinalDesign Top
(
    .CLOCK(clk),
    .control_to_FPGA(control_to_FPGA),
    .control_to_HPS(control_to_HPS)
);

initial begin
  clk = 1'b0;
  control_to_FPGA = 32'b0000_0000_0000_0000_0000_0000_0000_0001;
  repeat(2) @(posedge clk);
  control_to_FPGA = 32'b0000_1000_0000_0000_0100_0000_1010_0010;
  repeat(1) @(posedge control_to_HPS);
  repeat(3) @(posedge clk);
  control_to_FPGA = 32'b0000_1000_0000_0000_0100_0000_1010_0110;
  repeat(1) @(posedge clk);
  control_to_FPGA = 32'b0000_1000_0000_0000_0100_0000_1010_0010;
  repeat(1) @(posedge control_to_HPS);
  repeat(3) @(posedge clk);
  control_to_FPGA = 32'b0000_0000_0000_0000_0000_0000_0000_0001;
  repeat(2) @(posedge clk);
  control_to_FPGA = 32'b0000_1000_0000_0000_0100_0000_1010_0010;
  repeat(1) @(posedge control_to_HPS);
  repeat(3) @(posedge clk);
  control_to_FPGA = 32'b0000_1000_0000_0000_0100_0000_1010_0110;
  repeat(1) @(posedge clk);
  control_to_FPGA = 32'b0000_1000_0000_0000_0100_0000_1010_0010;
  repeat(1) @(posedge control_to_HPS);
  $stop;


  // $display("Running Time = %d clock cycles",clock_count);

  //$stop; // End Simulation
end

// Clock
always begin
   #10;           // wait for initial block to initialize clock
   clk = ~clk;
end

endmodule
