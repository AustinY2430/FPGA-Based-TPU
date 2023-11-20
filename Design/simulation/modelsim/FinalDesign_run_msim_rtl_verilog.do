transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -vlog01compat -work work +incdir+C:/Users/Kille/Desktop/Design {C:/Users/Kille/Desktop/Design/SYSMAC.v}
vlog -vlog01compat -work work +incdir+C:/Users/Kille/Desktop/Design {C:/Users/Kille/Desktop/Design/TPU8.v}
vlog -vlog01compat -work work +incdir+C:/Users/Kille/Desktop/Design {C:/Users/Kille/Desktop/Design/FinalDesign.v}
vlog -vlog01compat -work work +incdir+C:/Users/Kille/Desktop/Design {C:/Users/Kille/Desktop/Design/NewAdd.v}
vlib systemfinal
vmap systemfinal systemfinal
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_mem_if_hhp_qseq_synth_top.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_reset_controller.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_reset_synchronizer.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_up_avalon_reset_from_locked_signal.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/hps_sdram.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/hps_sdram_p0_acv_hard_addr_cmd_pads.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/hps_sdram_p0_acv_hard_io_pads.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/hps_sdram_p0_acv_hard_memphy.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/hps_sdram_p0_acv_ldc.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/hps_sdram_p0_altdqdqs.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/hps_sdram_p0_clock_pair_generator.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/hps_sdram_p0_generic_ddio.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_arm_a9_hps.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_arm_a9_hps_hps_io.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_onship_sram.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_system_pll.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_system_pll_sys_pll.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_control_to_fpga.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_control_to_hps.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_0.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_0_avalon_st_adapter.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_1.v}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal {c:/users/kille/desktop/design/db/ip/systemfinal/systemfinal.v}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altdq_dqs2_acv_connect_to_hard_phy_cyclonev.sv}
vlog -vlog01compat -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_avalon_sc_fifo.v}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_mem_if_dll_cyclonev.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_mem_if_hard_memory_controller_top_cyclonev.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_mem_if_oct_cyclonev.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_merlin_address_alignment.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_merlin_arbitrator.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_merlin_axi_master_ni.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_merlin_burst_adapter.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_merlin_burst_adapter_13_1.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_merlin_burst_uncompressor.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_merlin_slave_agent.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_merlin_slave_translator.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_merlin_traffic_limiter.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/altera_merlin_width_adapter.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/hps_sdram_p0.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/hps_sdram_pll.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_arm_a9_hps_fpga_interfaces.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_arm_a9_hps_hps_io_border.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_irq_mapper.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_0_avalon_st_adapter_error_adapter_0.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_0_cmd_demux.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_0_cmd_mux.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_0_router.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_0_router_002.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_0_rsp_demux.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_0_rsp_mux.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_1_cmd_demux.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_1_cmd_mux.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_1_router.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_1_router_002.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_1_rsp_demux.sv}
vlog -sv -work systemfinal +incdir+c:/users/kille/desktop/design/db/ip/systemfinal/submodules {c:/users/kille/desktop/design/db/ip/systemfinal/submodules/systemfinal_mm_interconnect_1_rsp_mux.sv}

