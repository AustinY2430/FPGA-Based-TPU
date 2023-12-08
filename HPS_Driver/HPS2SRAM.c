#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "matrix_methods.h"
#include "benchmarking.h"
#include "HPS2SRAM.h"

// Memory Offsets for SRAM and LW Bridge
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_SPAN      0x00003FFF
#define FPGA_LW_BASE 		  0xFF200000
#define FPGA_LW_SPAN		  0x00001000

#define M_size(M)			((M % 8) ? (((M/8) + 1)*8) : (M))

int sparse_element_prediction(int sparse_a_column, int sparse_b_row, int TilesPerOp, char *sparse_a, char *sparse_b)
{
	int InputStreamTile, AB_DoubleWord = 0;
	int sparse_a_pred = sparse_a_column;
	int sparse_b_pred = sparse_b_row;
	int doublewords_to_send = 0;
	// For each input tile required to calculate output
	for(InputStreamTile = 0; InputStreamTile < TilesPerOp; InputStreamTile++)
	{
		// For each word in each input stream matrix
		for(AB_DoubleWord = 0; AB_DoubleWord < 8; AB_DoubleWord++)
		{
			// If both input tiles have data
			if(sparse_a[sparse_a_pred] == 1 && sparse_b[sparse_b_pred] == 1)
			{
				doublewords_to_send++;
			}
			// Increment A column and B row
			sparse_a_pred++;
			sparse_b_pred++;
		}
	}
	return(doublewords_to_send);
}

void SRAM_ACC_Write(volatile unsigned int **sram_ptr_ptr,
    uint32_t *ACC, uint8_t ByteToACC)
{
	static int byte_count = 0;
	// Accumulates byte onto buffer 
    *ACC = (*ACC << 8) + ByteToACC;
	if(byte_count == 3)
	{
		// Writing new 32 bit integer at current SRAM address
		**sram_ptr_ptr = *ACC;
		//printf("%08x", (unsigned int)*sram_ptr_ptr); //Write to SRAM
		// Increment SRAM pointer by 4 bytes
		*sram_ptr_ptr = *sram_ptr_ptr + 1;
		// Flushes buffer
		// Print to console
		// printf("%08x\n", *ACC); //Write to SRAM
		*ACC = 0x00000000;
		byte_count = 0;
	}
	else
		byte_count++;
}
void tile_offset_updater(int *a_column, int *a_row, int *sparse_a_column,
    int *b_column, int *b_row, int *sparse_b_row, int AH, int AW, int BH,
        int *a_offset, int *b_offset, int OutputTile)
{
    // Output tiles generated in column major order
    // Same column of B for every row of A
    // If current tile is the last tile in the column
    // printf("a_row: %d, a_col: %d, b_row: %d, b_col: %d\n", a_row, a_column, b_row, b_column);
    if((OutputTile + 1) % (M_size(AH)/8) == 0)
    {
        // printf("current tile is lowest tile in column\n");
        // a_offset always begins at 0 for new column
        *a_column = 0;
        *a_row = 0;
        *sparse_a_column = 0;
        // b_column increments to the next column
        *b_column = *b_column + 1;
        *b_row = 0;
        *sparse_b_row = *b_column * M_size(BH);
    }
    // Otherwise column remains the same, and row increments
    else
    {
        // printf("current tile is not lowest tile in column\n");
        // a_column always starts at 0 (reads left to right)
        *a_column = 0;
        // a_row shifts down to the next row
        *a_row = *a_row + 1;
        *sparse_a_column = *a_row * M_size(AW);
        *sparse_b_row = *b_column * M_size(BH);
    }
    // Offsets updated from row and column
    *a_offset = 8*(*a_column*M_size(AH) + *a_row);
    *b_offset = 8*(*b_column*M_size(BH) + *b_row);
}

int HPS_ENGINE(uint8_t **matrix_a, uint8_t **matrix_b, int tile_size,
	int AH, int AW, int BH, int BW, unsigned long long *acc_hps_time,
		unsigned long long *acc_fpga_time, int *empty_columns_a, int *empty_rows_b,
            int *ignored_doublewords, int *written_doublewords, FILE *out_file)
{
	// Lightweight Bridge Interface
	void *h2p_lw_virtual_base;

	// SRAM Interface
	volatile unsigned int *sram_ptr = NULL;
	volatile unsigned int *sram_read_ptr = NULL;
	void *sram_virtual_base;

	// Control Interface
	// volatile unsigned int *Control_ptr = NULL ;
	volatile unsigned int *Control_to_FPGA = NULL;
	volatile unsigned int *Control_to_HPS = NULL;
	volatile int start;

	// /dev/mem file id
	int fd;

	// Timer Variables
	struct timeval startHPS, endHPS, startFPGA, endFPGA/*, startWrite, endWrite*/;

	// "Macros" representing fundamental tiling values
	int TileCountA = (M_size(AW)/tile_size)*(M_size(AH)/tile_size);
	int TileCountB = (M_size(BW)/tile_size)*(M_size(BH)/tile_size);
	int TileCountC = (M_size(AH)/tile_size)*(M_size(BW)/tile_size);
	int TilesPerOp = M_size(AW)/tile_size;
	// Descriptive Iterators for SRAM writing algorithm
	int OutputTile = 0;
	int InputStreamTile = 0;
    int AB_DoubleWord = 0;
    int byte = 0;
	// Offset values to keep track of matrix navigation
	int a_column = 0, a_row = 0;
	int b_column = 0, b_row = 0;
	int a_offset = 0, b_offset = 0;
	// Control signal to start FPGA and inform of matrix size
	int ControlSignal = 0;
	int ControlSignalACK = 0;
	// Buffer to accumulate bytes onto a word
	uint32_t ACC = 0x00000000;
	//int mem_offset = 0;
    int sparse_a_column = 0, sparse_b_row = 0;
	int doublewords_to_send = 0;
	int sram_limit = 0;
	int times1024 = 0;

	int tile_print = 0;
    int tile_print_row = 0;
	int tile_print_col = 0;

	char user_response[32];

	char* sparse_a;
	char* sparse_b;

	
	// === get FPGA addresses ==================
    // Open /dev/mem
	if((fd = open( "/dev/mem", ( O_RDWR | O_SYNC))) == -1)
	{
		printf("ERROR: could not open \"/dev/mem\"...\n");
		return(1);
	}
    
	//============================================
    // get virtual addr that maps to physical
	// for light weight bus
	// FIFO status registers	
	h2p_lw_virtual_base = mmap( NULL, FPGA_LW_SPAN,
		(PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_LW_BASE);
	if(h2p_lw_virtual_base == MAP_FAILED)
	{
		printf("ERROR: mmap1() failed...\n");
		close(fd);
		return(1);
	}
	// Initalizing Control Interface
	Control_to_FPGA = (unsigned int *)(h2p_lw_virtual_base);
	Control_to_HPS = (unsigned int *)(h2p_lw_virtual_base + 0x10);

	//============================================
	// scratch RAM FPGA parameter addr 
	sram_virtual_base = mmap(NULL, FPGA_ONCHIP_SPAN,
		(PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_ONCHIP_BASE);
	if(sram_virtual_base == MAP_FAILED)
	{
		printf( "ERROR: mmap2() failed...\n" );
		close( fd );
		return(1);
	}
   // Initialize SRAM Interface
	sram_ptr =(unsigned int *)(sram_virtual_base);

	// Prompts user to start matrix multiplication

	printf("Enter 1 to Start:");
	if(gets(user_response) == NULL)
	{
		printf("Error! Invalid Input %s\n", user_response); 
		exit(-1);   
	}
	start = atoi(user_response);
	memset(user_response, 0, sizeof user_response);

	// Generates sparse matrix representations of A and B
	*empty_columns_a = sparse_mat_create(M_size(AH), M_size(AW), TileCountA, *matrix_a, &sparse_a, 'a', tile_size);
	*empty_rows_b = sparse_mat_create(M_size(BH), M_size(BW), TileCountB, *matrix_b, &sparse_b, 'b', tile_size);

	if(start)
    {
		// For every tile in the output matrix C
		for(OutputTile = 0; OutputTile < TileCountC; OutputTile++)
		{
			// Only 1000 tiles worth of data fit into SRAM
			if(OutputTile % 1000 == 0)
				printf("Start Tile: %d\n", OutputTile);
			// Sends reset to FPGA
			*(Control_to_FPGA) = 1;
			times1024 = 0;
			doublewords_to_send = 0;
			// Predicts The amount of doublewords we're sending each tile	
			doublewords_to_send = sparse_element_prediction(sparse_a_column, sparse_b_row, TilesPerOp, sparse_a, sparse_b);

			// Predicts how many writes it will take to write entirety to sram
			int temp_sram_limit = doublewords_to_send;
			while(temp_sram_limit > 1024)
			{
				temp_sram_limit -= 1024;
				times1024++;
			}
			int times1024_remaining = times1024; 
			// calculates control signals using predictions
			ControlSignal = (times1024 << 27) + (doublewords_to_send << 4) + 0b0010;
			ControlSignalACK = (times1024 << 27) + (doublewords_to_send << 4) + 0b0110;
			//==========================================================================================
			// Start first HPS Timer
			gettimeofday(&startHPS, NULL);

			AB_DoubleWord = 0;
			
			// For every pair of matrices required to calculate output tile
			for(InputStreamTile = 0; InputStreamTile < (TilesPerOp*tile_size); InputStreamTile++)
			{
				// For each word in each input stream matrix
				// If both input tiles have data
				// printf("Input_tile: %d\n", InputStreamTile);
				if(sparse_a[sparse_a_column] == 1 && sparse_b[sparse_b_row] == 1)
				{
					//Write A Double Word
					for(byte = 0; byte < 4; byte++)
					{
						// Accumulates byte 03_02_01_00 (+3 reads down)
						SRAM_ACC_Write(&sram_ptr, &ACC, (*matrix_a)[3 + a_offset - byte]);
					}
					for(byte = 0; byte < 4; byte++)
					{
						// Accumulates byte 07_06_05_04 (+7 reads down)
						SRAM_ACC_Write(&sram_ptr, &ACC, (*matrix_a)[7 + a_offset - byte]);
					}
					// Write B Double Word
					for(byte = 0; byte < 4; byte++)
					{
						// Accumulates bytes 24_16_08_00 (increments column, by adding B height)
						SRAM_ACC_Write(&sram_ptr, &ACC, (*matrix_b)[M_size(BH)*3 + b_offset + AB_DoubleWord - byte*M_size(BH)]);
					}
					for(byte = 0; byte < 4; byte++)
					{
						// Accumulates bytes 56_48_40_32 (increments column, by adding B height)
						SRAM_ACC_Write(&sram_ptr, &ACC, (*matrix_b)[M_size(BH)*7 + b_offset + AB_DoubleWord - byte*M_size(BH)]);
					}				
					sram_limit++;
					*written_doublewords = *written_doublewords + 1;
				}
				else
				{
					*ignored_doublewords = *ignored_doublewords + 1;
				}
				// a_offset jumps to next column of a inputs to read

				a_offset = a_offset + M_size(AH);
				sparse_a_column++;
				sparse_b_row++;

				// b_offset skips ahead to the next tile input to read
				if ((AB_DoubleWord+1) % tile_size == 0) {
					b_offset = b_offset + tile_size;
					AB_DoubleWord = 0;
				}
				else {
					AB_DoubleWord++;
				}
				// If data to write to SRAM wont fit, then fill sram, and figure
				// out how many more times to write remaining data

				//printf("sram write done, sram_limit: %d\n", sram_limit);
				if((sram_limit == 1024) && (times1024_remaining > 0))
				{
					times1024_remaining--;
					// Set sram pointer to top of sram, and reset sram_limit
					sram_ptr =(volatile unsigned int *)(sram_virtual_base);
					sram_limit = 0;
					*acc_hps_time += TimerStop(endHPS, startHPS);
					gettimeofday(&startFPGA, NULL);
					// Acknowledging that HPS has sent more data
					*(Control_to_FPGA) = ControlSignalACK;
					// Waiting for FPGA to acknowledge (data is ready)
					while(*(Control_to_HPS) == 1);
					// Waiting for FPGA to annouce that its full
					*(Control_to_FPGA) = ControlSignal;
					while(*(Control_to_HPS) == 0);
					*acc_fpga_time += TimerStop(endFPGA, startFPGA);
					gettimeofday(&startHPS, NULL);
					//printf("final sram chunk sent\n");
				}
			}
			// Update Tile Offsets to select the next required tile for cycle
			tile_offset_updater(&a_column, &a_row, &sparse_a_column, &b_column,
				&b_row, &sparse_b_row, AH, AW, BH, &a_offset, &b_offset, OutputTile);

			// Ends first HPS Timer and starts FPGA Timer
			*acc_hps_time += TimerStop(endHPS, startHPS);

			// Control signal for remainder of memory
			//============================================
			// Begin FPGA 
			//============================================
			gettimeofday(&startFPGA, NULL);
			if(times1024 > 0)
			{
				// Acknowledging that HPS has sent more data
				*(Control_to_FPGA) = ControlSignalACK;
				// Waiting for FPGA to acknowledge (data is ready)
				while(*(Control_to_HPS) == 1);
			}
			// Waiting for FPGA to annouce that its full
			*(Control_to_FPGA) = ControlSignal;
			while(*(Control_to_HPS) == 0);
			// Ends FPGA Timer and starts second HPS Timer
			*acc_fpga_time += TimerStop(endFPGA, startFPGA);
			//============================================
			// End FPGA 
			//============================================
			gettimeofday(&startHPS, NULL);

			// Resets machine for last tile written to sram
			// Resets sram read pointer to top of assigned SRAM
			while(*(Control_to_HPS) == 0);

			sram_read_ptr = (volatile unsigned int *)sram_virtual_base;
			sram_read_ptr++;
			// Stores volatile output into an external text file

			// Write output from TPU to outfile.txt with padded zeros trimmed
			fwrite_output_mat(&sram_read_ptr, out_file, &tile_print_row, &tile_print_col, AH, BW);

			tile_print++;
			// Tile all zeros, resulting from additional padded tile
			// Resets pointers to overwrite in SRAM
			sram_ptr =(volatile unsigned int *)(sram_virtual_base);
			doublewords_to_send = 0;

			// Ends second HPS Timer
			*acc_hps_time += TimerStop(endHPS, startHPS);
		}
	}
	sram_read_ptr = (volatile unsigned int *) sram_virtual_base;
	free(sparse_a);
	free(sparse_b);

	return (0);
}