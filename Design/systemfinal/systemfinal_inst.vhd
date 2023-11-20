	component systemfinal is
		port (
			clock_bridge_1_out_clk_clk : out   std_logic;                                        -- clk
			control_to_fpga_export     : out   std_logic_vector(31 downto 0);                    -- export
			control_to_hps_export      : in    std_logic                     := 'X';             -- export
			memory_mem_a               : out   std_logic_vector(12 downto 0);                    -- mem_a
			memory_mem_ba              : out   std_logic_vector(2 downto 0);                     -- mem_ba
			memory_mem_ck              : out   std_logic;                                        -- mem_ck
			memory_mem_ck_n            : out   std_logic;                                        -- mem_ck_n
			memory_mem_cke             : out   std_logic;                                        -- mem_cke
			memory_mem_cs_n            : out   std_logic;                                        -- mem_cs_n
			memory_mem_ras_n           : out   std_logic;                                        -- mem_ras_n
			memory_mem_cas_n           : out   std_logic;                                        -- mem_cas_n
			memory_mem_we_n            : out   std_logic;                                        -- mem_we_n
			memory_mem_reset_n         : out   std_logic;                                        -- mem_reset_n
			memory_mem_dq              : inout std_logic_vector(7 downto 0)  := (others => 'X'); -- mem_dq
			memory_mem_dqs             : inout std_logic                     := 'X';             -- mem_dqs
			memory_mem_dqs_n           : inout std_logic                     := 'X';             -- mem_dqs_n
			memory_mem_odt             : out   std_logic;                                        -- mem_odt
			memory_mem_dm              : out   std_logic;                                        -- mem_dm
			memory_oct_rzqin           : in    std_logic                     := 'X';             -- oct_rzqin
			onship_sram_s1_address     : in    std_logic_vector(11 downto 0) := (others => 'X'); -- address
			onship_sram_s1_clken       : in    std_logic                     := 'X';             -- clken
			onship_sram_s1_chipselect  : in    std_logic                     := 'X';             -- chipselect
			onship_sram_s1_write       : in    std_logic                     := 'X';             -- write
			onship_sram_s1_readdata    : out   std_logic_vector(31 downto 0);                    -- readdata
			onship_sram_s1_writedata   : in    std_logic_vector(31 downto 0) := (others => 'X'); -- writedata
			onship_sram_s1_byteenable  : in    std_logic_vector(3 downto 0)  := (others => 'X'); -- byteenable
			system_pll_ref_clk_clk     : in    std_logic                     := 'X';             -- clk
			system_pll_ref_reset_reset : in    std_logic                     := 'X'              -- reset
		);
	end component systemfinal;

	u0 : component systemfinal
		port map (
			clock_bridge_1_out_clk_clk => CONNECTED_TO_clock_bridge_1_out_clk_clk, -- clock_bridge_1_out_clk.clk
			control_to_fpga_export     => CONNECTED_TO_control_to_fpga_export,     --        control_to_fpga.export
			control_to_hps_export      => CONNECTED_TO_control_to_hps_export,      --         control_to_hps.export
			memory_mem_a               => CONNECTED_TO_memory_mem_a,               --                 memory.mem_a
			memory_mem_ba              => CONNECTED_TO_memory_mem_ba,              --                       .mem_ba
			memory_mem_ck              => CONNECTED_TO_memory_mem_ck,              --                       .mem_ck
			memory_mem_ck_n            => CONNECTED_TO_memory_mem_ck_n,            --                       .mem_ck_n
			memory_mem_cke             => CONNECTED_TO_memory_mem_cke,             --                       .mem_cke
			memory_mem_cs_n            => CONNECTED_TO_memory_mem_cs_n,            --                       .mem_cs_n
			memory_mem_ras_n           => CONNECTED_TO_memory_mem_ras_n,           --                       .mem_ras_n
			memory_mem_cas_n           => CONNECTED_TO_memory_mem_cas_n,           --                       .mem_cas_n
			memory_mem_we_n            => CONNECTED_TO_memory_mem_we_n,            --                       .mem_we_n
			memory_mem_reset_n         => CONNECTED_TO_memory_mem_reset_n,         --                       .mem_reset_n
			memory_mem_dq              => CONNECTED_TO_memory_mem_dq,              --                       .mem_dq
			memory_mem_dqs             => CONNECTED_TO_memory_mem_dqs,             --                       .mem_dqs
			memory_mem_dqs_n           => CONNECTED_TO_memory_mem_dqs_n,           --                       .mem_dqs_n
			memory_mem_odt             => CONNECTED_TO_memory_mem_odt,             --                       .mem_odt
			memory_mem_dm              => CONNECTED_TO_memory_mem_dm,              --                       .mem_dm
			memory_oct_rzqin           => CONNECTED_TO_memory_oct_rzqin,           --                       .oct_rzqin
			onship_sram_s1_address     => CONNECTED_TO_onship_sram_s1_address,     --         onship_sram_s1.address
			onship_sram_s1_clken       => CONNECTED_TO_onship_sram_s1_clken,       --                       .clken
			onship_sram_s1_chipselect  => CONNECTED_TO_onship_sram_s1_chipselect,  --                       .chipselect
			onship_sram_s1_write       => CONNECTED_TO_onship_sram_s1_write,       --                       .write
			onship_sram_s1_readdata    => CONNECTED_TO_onship_sram_s1_readdata,    --                       .readdata
			onship_sram_s1_writedata   => CONNECTED_TO_onship_sram_s1_writedata,   --                       .writedata
			onship_sram_s1_byteenable  => CONNECTED_TO_onship_sram_s1_byteenable,  --                       .byteenable
			system_pll_ref_clk_clk     => CONNECTED_TO_system_pll_ref_clk_clk,     --     system_pll_ref_clk.clk
			system_pll_ref_reset_reset => CONNECTED_TO_system_pll_ref_reset_reset  --   system_pll_ref_reset.reset
		);

