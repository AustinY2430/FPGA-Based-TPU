	systemfinal u0 (
		.clock_bridge_1_out_clk_clk (<connected-to-clock_bridge_1_out_clk_clk>), // clock_bridge_1_out_clk.clk
		.control_to_fpga_export     (<connected-to-control_to_fpga_export>),     //        control_to_fpga.export
		.control_to_hps_export      (<connected-to-control_to_hps_export>),      //         control_to_hps.export
		.memory_mem_a               (<connected-to-memory_mem_a>),               //                 memory.mem_a
		.memory_mem_ba              (<connected-to-memory_mem_ba>),              //                       .mem_ba
		.memory_mem_ck              (<connected-to-memory_mem_ck>),              //                       .mem_ck
		.memory_mem_ck_n            (<connected-to-memory_mem_ck_n>),            //                       .mem_ck_n
		.memory_mem_cke             (<connected-to-memory_mem_cke>),             //                       .mem_cke
		.memory_mem_cs_n            (<connected-to-memory_mem_cs_n>),            //                       .mem_cs_n
		.memory_mem_ras_n           (<connected-to-memory_mem_ras_n>),           //                       .mem_ras_n
		.memory_mem_cas_n           (<connected-to-memory_mem_cas_n>),           //                       .mem_cas_n
		.memory_mem_we_n            (<connected-to-memory_mem_we_n>),            //                       .mem_we_n
		.memory_mem_reset_n         (<connected-to-memory_mem_reset_n>),         //                       .mem_reset_n
		.memory_mem_dq              (<connected-to-memory_mem_dq>),              //                       .mem_dq
		.memory_mem_dqs             (<connected-to-memory_mem_dqs>),             //                       .mem_dqs
		.memory_mem_dqs_n           (<connected-to-memory_mem_dqs_n>),           //                       .mem_dqs_n
		.memory_mem_odt             (<connected-to-memory_mem_odt>),             //                       .mem_odt
		.memory_mem_dm              (<connected-to-memory_mem_dm>),              //                       .mem_dm
		.memory_oct_rzqin           (<connected-to-memory_oct_rzqin>),           //                       .oct_rzqin
		.onship_sram_s1_address     (<connected-to-onship_sram_s1_address>),     //         onship_sram_s1.address
		.onship_sram_s1_clken       (<connected-to-onship_sram_s1_clken>),       //                       .clken
		.onship_sram_s1_chipselect  (<connected-to-onship_sram_s1_chipselect>),  //                       .chipselect
		.onship_sram_s1_write       (<connected-to-onship_sram_s1_write>),       //                       .write
		.onship_sram_s1_readdata    (<connected-to-onship_sram_s1_readdata>),    //                       .readdata
		.onship_sram_s1_writedata   (<connected-to-onship_sram_s1_writedata>),   //                       .writedata
		.onship_sram_s1_byteenable  (<connected-to-onship_sram_s1_byteenable>),  //                       .byteenable
		.system_pll_ref_clk_clk     (<connected-to-system_pll_ref_clk_clk>),     //     system_pll_ref_clk.clk
		.system_pll_ref_reset_reset (<connected-to-system_pll_ref_reset_reset>)  //   system_pll_ref_reset.reset
	);

