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
#include "TPU_IO.h"

#define M_size(M)			((M % 8) ? (((M/8) + 1)*8) : (M))

int main(void)
{
	// seeds rng
    srand(time(NULL));
	// External File to save TPU outputs to in C-M/T-M Order
	FILE *out_file_TCMO, *out_file_HR;
	// Internal char arrays to store input matrices A and B
	uint8_t* matrix_a;
	uint8_t* matrix_b;

	// Input Matrix Sizes
	int AW = 0, AH = 0, BW = 0, BH = 0;
	// hardcoded TPU Systolic Array dimensions
	int tile_size = 8;

	// IO variables
	char user_response[32];
	char mode;
	int retval;

	// Global Benchmarking Stats
	unsigned long long acc_hps_time = 0, acc_fpga_time = 0;
	int empty_columns_a = 0;
	int empty_rows_b = 0;
	int ignored_doublewords = 0;
	int written_doublewords = 0;

	int OA_W = 1;
	
	// Begin IO and initialize matrices A and B to pass to TPU
	printf("Enter M for Matrix Multiplication\nEnter C for Convolution\n");
	if(gets(user_response)[0] == 'M')
		MM_full_setup(&matrix_a, &matrix_b, &AH, &AW, &BH, &BW);
	else if(user_response[0] == 'C')
		OA_W = conv_full_setup(&matrix_a, &matrix_b, tile_size, &AH, &AW, &BH, &BW);
	else
	{
		printf("Invalid Mode %s\n", user_response); 
		exit(-1);   
	}

	mode = user_response[0];
	memset(user_response, 0, sizeof user_response);

	out_file_TCMO = fopen("out_file.bin", "wb+");
	fclose(out_file_TCMO);
	// Open at 0 to modify

	out_file_TCMO = fopen("out_file.bin", "rb+");
	if(out_file_TCMO == NULL)
	{
		printf("unable to open file out_file.bin\n"); 
		exit(-1);   
	}

    // Send initialized matrices to TPU and return output to out_file.txt
	retval = HPS_ENGINE(&matrix_a, &matrix_b, tile_size, AH, AW, BH, BW, &acc_hps_time, &acc_fpga_time, 
		&empty_columns_a, &empty_rows_b, &ignored_doublewords, &written_doublewords, &out_file_TCMO, 1, 0);
	if(retval)
	{
		printf("Invalid TPU Cycle\n"); 
		exit(-1);   
	}

	fclose(out_file_TCMO);
	printf("output to out_file.bin\n");
	out_file_TCMO = fopen("out_file.bin", "rb");	
	out_file_HR = fopen("out_fileHR.txt", "w+");

	if(mode == 'C')
		fread_output_mat((BW - AW + 1), (BH - AH + 1), OA_W, &out_file_TCMO, &out_file_HR,'c');
	if(mode == 'M')
		fread_output_mat(BW, AH, OA_W, &out_file_TCMO, &out_file_HR,'m');


	printf("ignored_doublewords: %d, written_doublewords: %d\n", ignored_doublewords, written_doublewords);
	printf("\033[0;33m%dx%d Matrix Results :\033[0m\n", M_size(AH), M_size(BW));
	printf("\033[0;33mHPS format T=%f S\n\r", ((float)acc_hps_time)/1000000);
	printf("FPGA read, calculate, and write T=%f S \n\r", ((float)acc_fpga_time)/1000000);
	printf("Empty columns in A: %d, Empty rows in B: %d, \n", empty_columns_a, empty_rows_b);
	printf("Percent of memory accesses skipped: %f\n", ((float)ignored_doublewords/(float)(written_doublewords + ignored_doublewords))*100);
	printf("Output to out_file.txt\033[0m\n\r");

	return (0);
}