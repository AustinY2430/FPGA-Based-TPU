#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
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
	// Input Matrix Sizes
	int AW = 0; 
	int AH = 0;
	int BW = 0;
	int BH = 0;

	int AWi = 0;
	int AHi = 0;
	int BWi = 0;
	int BHi = 0;
	// hardcoded TPU Systolic Array dimensions
	int tile_size = 8;

	FILE* out_file, *file_a, *file_b;
	// IO variables
	char user_response[32];
	char mat_a_name[32], mat_b_name[32];
	char mode;
	int retval;

	// Global Benchmarking Stats
	unsigned long long acc_hps_time = 0, acc_fpga_time = 0;
	int empty_columns_a = 0;
	int empty_rows_b = 0;
	int ignored_doublewords = 0;
	int written_doublewords = 0;
	int total_empty_columns = 0;
	int total_empty_rows = 0;

	// Number of input Channels
	int c = 0;
	// Number of output Channels
	int k = 0;
	// Number of batches
	int b = 0;

	int g = 0;
	int h = 0;
	int i = 0;
	
	char mat_a[34];
	char mat_b[34];
	char mat_c[34];
	
	// Begin IO for Convolution
	Conv_IO(&AH, &AW, &BH, &BW);

	printf("Convolution in (H) Hardware or (S) Software\n");
	if(gets(user_response)[0] == 'H')
	{
		memset(user_response, 0, sizeof user_response);
		mode = 'H';
	}
	else if((user_response)[0] == 'S')
	{
		memset(user_response, 0, sizeof user_response);
		mode = 'S';
	}
	else
	{
		printf("Invalid Input: %s\nChoose H or S\n", user_response); 
		exit(-1);   
	}
	printf("Enter M for Multi-Channel Convolution\nEnter O for Multi-OA Convolution\nEnter B for Batch Processing\n");
	if(gets(user_response)[0] == 'M')
	{
		memset(user_response, 0, sizeof user_response);
		printf("Enter the number of input channels:\n");
		if(atoi(gets(user_response)))
			c = atoi(user_response);
		memset(user_response, 0, sizeof user_response);
		k = 1;
		b = 1;
	}
	else if(user_response[0] == 'O')
	{
		memset(user_response, 0, sizeof user_response);
		printf("Enter the number of input channels:\n");
		if(atoi(gets(user_response)))
			c = atoi(user_response);
		memset(user_response, 0, sizeof user_response);
		printf("Enter the number of output channels:\n");
		if(atoi(gets(user_response)))
			k = atoi(user_response);
		memset(user_response, 0, sizeof user_response);
		b = 1;
	}
	else if(user_response[0] == 'B')
	{
		memset(user_response, 0, sizeof user_response);
		printf("Enter the number of input channels:\n");
		if(atoi(gets(user_response)))
			c = atoi(user_response);
		memset(user_response, 0, sizeof user_response);
		printf("Enter the number of output channels:\n");
		if(atoi(gets(user_response)))
			k = atoi(user_response);
		memset(user_response, 0, sizeof user_response);
		printf("Enter the number of batches:\n");
		if(atoi(gets(user_response)))
			b = atoi(user_response);
		memset(user_response, 0, sizeof user_response);
	}
	else
	{
		printf("Invalid Input: %s\nChoose M, O, or B\n", user_response); 
		exit(-1);   
	}
	// Generates desired number of Input Kernels
	for(h = 0; h < k; h++)
	{
		for(i = 0; i < c; i++)
		{
			sprintf(mat_a,"k%01d_c%01d_", h, i);
			// Create a random Kernel and Input Activation of specified size
			rand_mat(AW, AH, strcat(mat_a, "kernel.txt"), mat_a_name, 0, 'b', h, 0);
			memset(mat_a, 0, sizeof mat_a);
		}
	}
	// Generates the desired number of Input Activations 
	for(g = 0; g < b; g++)
	{
		for(i = 0; i < c; i++)
		{
			sprintf(mat_b,"b%01d_c%01d_", i, g);
			// Create a random Kernel and Input Activation of specified size
			rand_mat(BW, BH, strcat(mat_b, "IA.txt"), mat_b_name, 0, 'i', 0, g);
			memset(mat_b, 0, sizeof mat_b);
		}
	}
	// Preserve initial dimensions to initialize future files
	AWi = AW;
	AHi = AH;
	BHi = BH;
	BWi = BW;
	//For every batch of Input Activations
	for(g = 0; g < b; g++)
	{
		printf("\033[0;33m Batch %d, IA incrementing from %d to %d in C-MO\033[0m\n", g, g, ((BHi*BWi) + g));
		//For every output channel
		for(h = 0; h < k; h++)
		{
			printf("\033[0;33m Output Channel %d, Kernel all %ds\033[0m\n", h, h + 1);
			// Open desired outfile to send read data onto
            sprintf(mat_c,"k%01d_", h);
            // Create File
            out_file = fopen(strcat(mat_c, "out_file.bin"), "wb+");
            fclose(out_file);
            // Open at 0 to modify
			out_file = fopen(mat_c, "rb+");
			// For every channel
			for(i = 0; i < c; i++)
			{
				printf("\033[0;33m Weight Channel %d Begin\033[0m\n", i);
				// Matrix A and B to calloc onto
				uint8_t* matrix_a;
				uint8_t* matrix_b;

				// Reset Dimensions to properly initialize inputs
				AW = AWi;
				AH = AHi;
				BH = BHi;
				BW = BWi;

				sprintf(mat_a,"k%01d_c%01d_", h, i);
				sprintf(mat_b,"b%01d_c%01d_", i, g);
				// Inititialize matrix A and B with corresponding channel data
				if(mode == 'H')
				{
					retval = conv_init(&matrix_a, &matrix_b, strcat(mat_a, "kernel.txt"), strcat(mat_b, "IA.txt"),
						&AH, &AW, &BH, &BW, tile_size);
                    if(retval == 0)
                    {
                        printf("Conv Init failed.\n");
						exit(-1);
                    }
				}
				if(mode == 'S')
				{
					file_a = fopen(strcat(mat_a, "kernel.txt"), "rb");
					file_b = fopen(strcat(mat_b, "IA.txt"), "rb");

					// Initializes Matrix A and B arrays by callocing padded size on the heap
					matrix_a = (uint8_t*)calloc((size_t)(AW*AH), sizeof(uint8_t));
					if (matrix_a == NULL)
					{
						printf("Memory allocation failed.\n");
						exit(-1);
					}
					matrix_b = (uint8_t*)calloc((size_t)(BW*BH), sizeof(uint8_t));
					if (matrix_b == NULL)
					{
						printf("Memory allocation failed.\n");
						exit(-1);
					}
					if(!fread(matrix_a, AH*AW, 1, file_a))
					{
						printf("unable to read matrix from file\n");
						exit(-1);
					}
					if(!fread(matrix_b, BH*BW, 1, file_b))
					{
						printf("unable to read matrix from file\n");
						exit(-1);
					}
					fclose(file_a);
					fclose(file_b);
				}

				// Send input matrices to TPU, receive output in out_file.txt, return !0 if error
				// Send to hardware
				printf("mode = %c\n", mode);
				if(mode == 'H')
				{
					retval = HPS_ENGINE(&matrix_a, &matrix_b, tile_size, AH, AW, BH, BW, &acc_hps_time, &acc_fpga_time, 
						&empty_columns_a, &empty_rows_b, &ignored_doublewords, &written_doublewords, &out_file, 0, i);

					total_empty_columns += empty_columns_a;
					total_empty_rows += empty_rows_b;		
					if(retval)
					{
						printf("Invalid TPU Cycle\n"); 
						exit(-1);   
					}
				}
				// Simulate in software
				if(mode == 'S')
				{
					conv_sim(&matrix_a, &matrix_b, AHi, AWi, BHi, BWi, &acc_hps_time, &out_file, i);
				}
				// Return to the beginning of the file to accumulate following channels onto Output Activation
				// Buffer to collect out_file contents onto

				memset(mat_a, 0, sizeof mat_a);
				memset(mat_b, 0, sizeof mat_b);
				// Free Matrix A and B for next run
				free(matrix_a);
				free(matrix_b);
                fseek(out_file, 0, SEEK_SET);
			}
            printf("Output Matrix Printed to %s\n", mat_c);
		}
	}
	printf("\033[0;33m%dx%d Matrix Results :\033[0m\n", M_size(AH), M_size(BW));
	printf("\033[0;33mHPS format T=%f S\n\r", ((float)acc_hps_time)/1000000);
	printf("FPGA read, calculate, and write T=%f S \n\r", ((float)acc_fpga_time)/1000000);
	printf("Empty columns in A: %d, Empty rows in B: %d, \n", total_empty_columns, total_empty_rows);
	printf("Percent of memory accesses skipped: %f\n", ((float)ignored_doublewords/(float)(written_doublewords + ignored_doublewords))*100);
	printf("Output to out_file.txt\033[0m\n\r");

	return (0);
}
