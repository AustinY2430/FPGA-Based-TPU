
module systemfinal (
	clock_bridge_1_out_clk_clk,
	control_to_fpga_export,
	control_to_hps_export,
	memory_mem_a,
	memory_mem_ba,
	memory_mem_ck,
	memory_mem_ck_n,
	memory_mem_cke,
	memory_mem_cs_n,
	memory_mem_ras_n,
	memory_mem_cas_n,
	memory_mem_we_n,
	memory_mem_reset_n,
	memory_mem_dq,
	memory_mem_dqs,
	memory_mem_dqs_n,
	memory_mem_odt,
	memory_mem_dm,
	memory_oct_rzqin,
	onship_sram_s1_address,
	onship_sram_s1_clken,
	onship_sram_s1_chipselect,
	onship_sram_s1_write,
	onship_sram_s1_readdata,
	onship_sram_s1_writedata,
	onship_sram_s1_byteenable,
	system_pll_ref_clk_clk,
	system_pll_ref_reset_reset);	

	output		clock_bridge_1_out_clk_clk;
	output	[31:0]	control_to_fpga_export;
	input		control_to_hps_export;
	output	[12:0]	memory_mem_a;
	output	[2:0]	memory_mem_ba;
	output		memory_mem_ck;
	output		memory_mem_ck_n;
	output		memory_mem_cke;
	output		memory_mem_cs_n;
	output		memory_mem_ras_n;
	output		memory_mem_cas_n;
	output		memory_mem_we_n;
	output		memory_mem_reset_n;
	inout	[7:0]	memory_mem_dq;
	inout		memory_mem_dqs;
	inout		memory_mem_dqs_n;
	output		memory_mem_odt;
	output		memory_mem_dm;
	input		memory_oct_rzqin;
	input	[11:0]	onship_sram_s1_address;
	input		onship_sram_s1_clken;
	input		onship_sram_s1_chipselect;
	input		onship_sram_s1_write;
	output	[31:0]	onship_sram_s1_readdata;
	input	[31:0]	onship_sram_s1_writedata;
	input	[3:0]	onship_sram_s1_byteenable;
	input		system_pll_ref_clk_clk;
	input		system_pll_ref_reset_reset;
endmodule
