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

// Memory Offsets for SRAM and LW Bridge
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_SPAN      0x00003FFF
#define FPGA_LW_BASE 		  0xFF200000
#define FPGA_LW_SPAN		  0x00001000

// Lightweight Bridge Interface
void *h2p_lw_virtual_base;

// SRAM Interface
volatile unsigned int *sram_ptr = NULL;
volatile unsigned int *sram_read_ptr = NULL;
void *sram_virtual_base;

// Control Interface
volatile unsigned int *Control_ptr = NULL ;
volatile unsigned int *Control_to_FPGA = NULL;
volatile unsigned int *Control_to_HPS = NULL;
volatile int start;

// /dev/mem file id
int fd;
int done = 0;

// Timer Variables
struct timeval startHPS, endHPS, startFPGA, endFPGA/*, startWrite, endWrite*/;
unsigned long long acc_hps_time = 0, acc_fpga_time = 0, acc_write_time = 0;

/* Generates a random byte file of size M*M. Saves to prev_file. */
void rand_mat(int M, char* filename, char* MAT_AorB, int sparse_density)
{
    FILE* Rand_Mat;
    Rand_Mat = fopen(filename, "w+");
    int i = 0;
    unsigned char rand_c;

    for(i = 0; i < M*M; i++)
    {
        // P of random nonzero value from provided sparsity
		if(((rand() % 100) + 1) > sparse_density)
		{
			rand_c = rand();
			fprintf(Rand_Mat, "%c", rand_c);
		}
        // Q of zero value
		else
			fprintf(Rand_Mat, "%c", 0x00);
    }
    strcpy(MAT_AorB, filename);
    fclose(Rand_Mat);
}
/* Prints input matrices in row major order as     */
/* signed decimals and outputs them to text files. */ 
void print_input_mat(int M, FILE **input_outfile, uint8_t *input_matrix, char letter, int test)
{
	if(M <= 32 && !test)
		printf("\033[0;33mMatrix %c:\033[0m\n", letter);
	int i = 0, row = 0, index = 0;
	for(i = 1; i <= M * M; i++)
	{
		fprintf(*input_outfile, "%d ", (signed char)input_matrix[index]);
		if(M <= 32 && !test)
		{
			printf("\033[0;37m%4d	", (signed char)input_matrix[index]);
		}
		if(i % M == 0)
		{
			// If on the final column, move to the beginning of the next row
			row++;
			index = 0;
			index += row;
			fprintf(*input_outfile, "\n");
			if(M <= 32 && !test)
			{
				printf("\n");
			}
		}
		else
			// Otherwise prints in row major by reading left to right
			index += M;
	}
	fclose(*input_outfile);
	if(M <= 32 && !test)
		printf("\033[0;33mInput Matrix %c output to	input_outfile%c.txt\033[0m\n\r", letter, letter);
}
/* Called after files are opened and before writing first tile. Seeks through*/
/* input matrices recording empty columns in A and rows in B to sparse_matrix*/
/* Returns the amount of empty columns or rows. */
int sparse_mat_create(int M, int TileCount, uint8_t *input_matrix,
    char *sparse_matrix, char AorB)
{
	int sparse_offset = 0;
	int empty_doubleword_count = 0;
	int sparse_element = 0;
	int i = 0;
	int j = 0;
	int row = 0;
	int column = 0;
	
    // Sparse Matrix A is constructed by scanning 8x1 columns in each row
    // Stored in row major order due to compatibility with sparse checks
	if(AorB == 'a')
	{
        // 8 columns for each tile in greater matrix
		for(i = 0; i < TileCount*8; i++)
		{
            // check if each element in column is zero
			for(j = 0; j < 8; j++)
			{
                // If data is nonzero, mark data in sparse_matrix
				if(input_matrix[j + sparse_offset] != 0)
				{
					sparse_matrix[sparse_element] = 1;
					break;
				}
			}
            // Otherwise mark empty in sparse_matrix
			if(sparse_matrix[sparse_element] != 1)
			{
				empty_doubleword_count++;
				sparse_matrix[sparse_element] = 0;
			}
			sparse_element++;
            // If at right edge of matrix, go back to left and move down a row
			if(column == (M-1))
			{
				column = 0;
				row++;
			}
            // Otherwise move right by a column
			else
				column++;
            // Offset calculated using desired row and column
			sparse_offset = column*M + 8*row;
		}
	}
    // Sparse Matrix B is constructed by scanning 1x8 rows in each column
    // Stored in column major order
	if(AorB == 'b')
	{
        // 8 rows for each tile in greater matrix
		for(i = 0; i < TileCount*8; i++)
		{
            // check if each element in column is zero
			for(j = 0; j < 8; j++)
			{
                // If data is nonzero, mark data in sparse_matrix
				if(input_matrix[j*M + sparse_offset] != 0)
				{
					sparse_matrix[sparse_element] = 1;
					break;
				}
			}
            // Otherwise mark empty in sparse_matrix
			if(sparse_matrix[sparse_element] != 1)
			{
				sparse_matrix[sparse_element] = 0;
				empty_doubleword_count++;
			}
			sparse_element++;
            // If at bottom of the matrix, go back
            // to the top and move right one row
			if(row == (M-1))
			{
				column++;
				row = 0;
			}
            // Otherwise move down by a row
			else
				row++;
            // Offset calculated using desired row and column
			sparse_offset = 8*column*M + row;
		}
	}
	return empty_doubleword_count;
}
/* Stops the previously set timer, corrects */
/* for overflow on us and returns delay.    */
unsigned long long TimerStop(struct timeval endTime, struct timeval startTime)
{
	gettimeofday(&endTime, NULL);
	// Corrects potential .tv_usec overflow
	if (endTime.tv_usec < startTime.tv_usec)
	{
		endTime.tv_sec--;
		endTime.tv_usec += 1000000;
	}
	// returns the difference between start and end times
	return (unsigned long long)endTime.tv_sec
    - (unsigned long long)startTime.tv_sec
    + (unsigned long long)endTime.tv_usec
    - (unsigned long long)startTime.tv_usec;
}
/* Accumates bytes into a word, writes to SRAM and flushes buffer when full. */
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
		//printf("%08x\n", *ACC); //Write to SRAM
		*ACC = 0x00000000;
		byte_count = 0;
	}
	else
		byte_count++;
}
/* Prints Output Matrix as decimals in row major order      */
/* from array in DRAM to outfile and to terminal if M<= 32. */
void print_output_mat(int M, FILE **out_file,
	int *matrix_c, int MatrixTileWidth, int test)
{
	int i, j, k, h;
	printf("\033[0;33mFPGA Output:\n");
	int mat_print_offset = 0;
	// For tiled sections of each column
	for(i = 0; i < MatrixTileWidth; i++)
	{
		// For every column section
		for(j = 0; j < 8; j++)
		{
			mat_print_offset = j + i*64;
			// For tiled sections of each row
			for(k = 0; k < MatrixTileWidth; k++)
			{
				// for every section
				for(h = 0; h < 8; h++)
				{
					if(M <= 32 && !test)
						printf("\033[0;37m%10d\033[0m",
							matrix_c[(h*8) + mat_print_offset]);
					fprintf(*out_file, "%d, ",
						matrix_c[(h*8) + mat_print_offset]);
				}
				mat_print_offset = mat_print_offset + MatrixTileWidth*64;
			}
			// Ends Row
			if(M <= 32 && !test)
				printf("\n");
			fprintf(*out_file, "\n");
		}
	}
}
/* Function to calculate trimmed average for a number of trials. */
void trimmed_averager(int m, char *name, unsigned long long *trial_arr,
    int trials, float *avg_arr, float *StdDev_arr,
        unsigned long long *trimmed_avg_arr, int *outlier_count_arr, int trim)
{
	int i = 0;
    float avg = 0;
    // Accumulates results of trials onto an average
    for(i = 0; i < trials; i++)
        avg += (float)trial_arr[i];
    // Computes the average
    avg = avg/trials;
    avg_arr[m] = avg;
    printf("%s average: %f\n", name, avg);
	if(trim)
    {
		float StdDev = 0;
		// Computes Standard Deviation
		for(i = 0; i < trials; i++)
			StdDev += pow((float)trial_arr[i] - (float)avg, 2);
		StdDev = sqrt(StdDev/trials);
		printf("%s standard deviation: %f\n", name, StdDev);
		// Stores Standard Deviation within global array
		StdDev_arr[m] = StdDev;
		// Computes Margin to be trimmed away
		StdDev = StdDev*3;
		int trim_count = 0;
		int outlier_count = 0;
		unsigned long long outlier_arr[trials];
		unsigned long long trimmed_arr[trials];
		// Sorts outliers out of dataset
		for(i = 0; i < trials; i++)
		{
			if((trial_arr[i] < StdDev + avg) && (trial_arr[i] > StdDev - avg))
			{
				trimmed_arr[trim_count] = trial_arr[i];
				trim_count++;
			}
			else
			{
				outlier_arr[outlier_count] = trial_arr[i];;
				outlier_count++;
			}
		}
		float trimmed_avg = 0;
		// Computes Trimmed Average
		for(i = 0; i < trim_count; i++)
			trimmed_avg += trimmed_arr[i];
		trimmed_avg = trimmed_avg/trim_count;
		printf("%s trimmed average: %f\n", name, trimmed_avg);
		trimmed_avg_arr[m] = trimmed_avg;
		// Prints Outliers
		if(outlier_arr[0])
		{
			printf("%s outliers:\n", name);
			for(i = 0; i < outlier_count; i++)
				printf("%llu\n", outlier_arr[i]);
		}
		// Stores Outlier Amount
		outlier_count_arr[m] = outlier_count;

		memset(outlier_arr, 0, sizeof outlier_arr);
	}
	memset(trial_arr, 0, sizeof trial_arr);
}
int main(void)
{
	// UI Variables
	int M = 0, times1024 = 0;
	char mat_a_name[32], mat_b_name[32], user_response[32];
	// I/O File Pointers
	FILE *prev_files, *input_outfileA, *input_outfileB, *file_a, *file_b, *out_file;
    // ========================================================================
    // Begin Test Variables
    // ========================================================================
    // TEST FLAG
    int test = 0;
	// Number of trials to be ran
	int trials = 10;
	// Sizes of M and sparsities S to be tested best for S == M
    int M_arr[9] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};
	int S_arr[9] = {0, 50, 60, 70, 80, 90, 95, 97, 99};
	// Stores results of trials
    unsigned long long HPS_trials[trials], FPGA_trials[trials];
    unsigned long long IT_trials[trials], ET_trials[trials];
    // Stores averages of each size
    float HPS_avg[(sizeof M_arr)/sizeof(int)];
    float FPGA_avg[(sizeof M_arr)/sizeof(int)];
	float IT_avg[(sizeof S_arr)/sizeof(int)];
	float ET_avg[(sizeof S_arr)/sizeof(int)];
	// Stores averages of trials for each value of M
    unsigned long long HPS_trimmed_avg[(sizeof M_arr)/sizeof(int)];
    unsigned long long FPGA_trimmed_avg[(sizeof M_arr)/sizeof(int)];
    unsigned long long IT_trimmed_avg[(sizeof S_arr)/sizeof(int)];
    unsigned long long ET_trimmed_avg[(sizeof S_arr)/sizeof(int)];
    // Stores Standard deviation for each value of M
	float HPS_std_dev[(sizeof M_arr)/sizeof(int)];
	float FPGA_std_dev[(sizeof M_arr)/sizeof(int)];
	float IT_std_dev[(sizeof S_arr)/sizeof(int)];
	float ET_std_dev[(sizeof S_arr)/sizeof(int)];
    // Records each outlier from the tests
	int HPS_outlier_count[(sizeof M_arr)/sizeof(int)];
	int FPGA_outlier_count[(sizeof M_arr)/sizeof(int)];
	int IT_outlier_count[(sizeof S_arr)/sizeof(int)];
	int ET_outlier_count[(sizeof S_arr)/sizeof(int)];
    // Trial Iterator
    int t = 0;
	// Matrix Size Iterator
    int m = 0;
	// Sparsity Iterator
	int s = 0;
    // ========================================================================
    // End Test Variables
    // ========================================================================
	int sparse_density = 0;
		
	int i = 0;
	
	// seeds rng
    srand(time(NULL));

	printf("Enter the size of the input matrices: (8, 16, 32...)\n");
	if(gets(user_response) == NULL)
    {
        printf("Error! Invalid Input %s\n", user_response); 
        exit(-1);
    }
    if(user_response[0] == 't' || user_response[0] == 's')
    {
		if(user_response[0] == 't')
		{
			test = 1;
			printf("\033[0;33mTEST MODE:\nTrials: %d\033[0m\n", trials);
			printf("What percentage of matrix should be zeros? (10, 20, 30...)\n");
			if(gets(user_response) == NULL)
			{
				printf("Error! Invalid Input %s\n", user_response); 
				exit(-1);   
			}
			sparse_density = atoi(user_response);
		}
		if(user_response[0] == 's')
		{
			test = 2;
		}
        // GOTO LABEL (Marks beginning of testing script)
        BEGIN_TEST:
		// Hardcodes matrix size M to be fed by array M_arr
		if(test == 1)
        	M = M_arr[m];
		else // if test == 2
		{
			M = 512;
		}
		if(test == 2)
			sparse_density = S_arr[s];
		// Hardcodes input matrices to be random and of size M
        rand_mat(M, "randomA.bin", mat_a_name, sparse_density);
        rand_mat(M, "randomB.bin", mat_b_name, sparse_density);
    }
    else
    {
        M = atoi(user_response);
        if ((M % 8) != 0)
        {
            printf("Error! Invalid Matrix Size %d\n", M); 
            exit(-1);
        }
        memset(user_response, 0, sizeof user_response);
        printf("Use previous files, new files, or random input? p/n/r\n");
        if(gets(user_response) == NULL)
        {
            printf("Error! Invalid Input %s\n", user_response); 
            exit(-1);   
        }
        if(user_response[0] == 'p')
        {
            // Opens previous file names out of directory
            prev_files = fopen("prev_files.txt", "r");
            if (fgets(mat_a_name, sizeof(mat_a_name), prev_files) == NULL)
            {
                printf("Error! Invalid Previous Files\n"); 
                exit(-1);
            }
            mat_a_name[strlen(mat_a_name)-1] = '\0';
            // Writes over first filename to isolate second filename
            if (fgets(mat_b_name, sizeof(mat_b_name), prev_files) == NULL)
            {
                printf("Error! Invalid Previous Files\n"); 
                exit(-1);
            }
            fclose(prev_files);
        }
        else if(user_response[0] == 'n')
        {
            // Otherwise, requests filenames from user
            printf("Enter filename of Matrix A: \n");
            if(gets(mat_a_name) == NULL)
            {
                printf("Error! Invalid Input %s\n", user_response); 
                exit(-1);   
            }
            printf("Enter filename of Matrix B: \n");
            if(gets(mat_b_name) == NULL)
            {
                printf("Error! Invalid Input %s\n", user_response); 
                exit(-1);   
            }
            // Saves new file names to directory
            prev_files = fopen("prev_files.txt", "w+");
            fprintf(prev_files, "%s\n%s", mat_a_name, mat_b_name);
        }
        else if(user_response[0] == 'r')
        {
			memset(user_response, 0, sizeof user_response);
			printf("What percentage of matrix should be zeros? (10, 20, 30...)\n");
			if(gets(user_response) == NULL)
        	{
            	printf("Error! Invalid Input %s\n", user_response); 
            	exit(-1);   
        	}
			sparse_density = atoi(user_response);
            // Generates random files and saves to prev_files
            rand_mat(M, "randomA.bin", mat_a_name, sparse_density);
            rand_mat(M, "randomB.bin", mat_b_name, sparse_density);
            prev_files = fopen("prev_files.txt", "w+");
            fprintf(prev_files, "randomA.bin\nrandomB.bin");
        }
        else
        {
            printf("Error! Choose p, n, or r\n");
            exit(-1);
        }
        memset(user_response, 0, sizeof user_response);
    }
	uint8_t* matrix_a;
	uint8_t* matrix_b;
	
    // Initializes Matrix A and B arrays by callocing on the heap
	matrix_a = (uint8_t*)calloc((size_t)(M*M), sizeof(uint8_t));
	if (matrix_a == NULL)
	{
		printf("Memory allocation failed.\n");
		return 1; // or handle the failure as appropriate
	}
	matrix_b = (uint8_t*)calloc((size_t)(M*M), sizeof(uint8_t));
	if (matrix_a == NULL)
	{
		printf("Memory allocation failed.\n");
		return 1; // or handle the failure as appropriate
	}
	out_file = fopen("out_file.txt", "w+");

	// Opens files in binary and maps them to calloced arrays on the heap
	file_a = fopen(mat_a_name, "rb");
	if(!fread(matrix_a, M*M, 1, file_a))
	{
		printf("unable to read matrix from file\n");
		exit(-1);
	}
	file_b = fopen(mat_b_name, "rb");
	if(!fread(matrix_b, M*M, 1, file_b))
	{
		printf("unable to read matrix from file\n");
		exit(-1);
	}
    
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

	// Reads input files and prints to out_files and to terminal if M<=32 
	input_outfileA = fopen("input_outfileA.txt", "w+");
	input_outfileB = fopen("input_outfileB.txt", "w+");
	print_input_mat(M, &input_outfileA, matrix_a, 'A', test);
	print_input_mat(M, &input_outfileB, matrix_b, 'B', test);
	
	// Prompts user to start matrix multiplication
    if(test)
        start = 1;
    else
    {
        printf("Enter 1 to Start:");
        if(gets(user_response) == NULL)
        {
            printf("Error! Invalid Input %s\n", user_response); 
            exit(-1);   
        }
        start = atoi(user_response);
        memset(user_response, 0, sizeof user_response);
    }
	// "Macros" representing fundamental tiling values
	int MatrixTileWidth = M/8;
	int TileCount = pow(MatrixTileWidth,2);
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
	int empty_columns_a = 0;
	int empty_rows_b = 0;
	int ignored_doublewords = 0;
	int written_doublewords = 0;
	int sram_limit = 0;
	int sparse_a_pred = 0, sparse_b_pred = 0;

	char* sparse_a;
	char* sparse_b;

	sparse_a = (char*)calloc((size_t)(TileCount*8), sizeof(char));
	if (sparse_a == NULL)
	{
		printf("Memory allocation failed.\n");
		return 1; // or handle the failure as appropriate
	}
	sparse_b = (char*)calloc((size_t)(TileCount*8), sizeof(char));
	if (sparse_b == NULL)
	{
		printf("Memory allocation failed.\n");
		return 1; // or handle the failure as appropriate
	}
    // Generates sparse matrix representations of A and B
	empty_columns_a = sparse_mat_create(M, TileCount, matrix_a, sparse_a, 'a');
	empty_rows_b = sparse_mat_create(M, TileCount, matrix_b, sparse_b, 'b');
	
	if(start)
    {
        // For every tile in input matrices A and B
        for(OutputTile = 0; OutputTile < TileCount; OutputTile++)
        {
			if(OutputTile % 1000 == 0)
				printf("Start Tile: %d\n", OutputTile);
			// Sends reset to FPGA
			*(Control_to_FPGA) = 1;
			times1024 = 0;
			doublewords_to_send = 0;
			// Predicts The amount of doublewords we're sending each tile
			sparse_a_pred = sparse_a_column;
			sparse_b_pred = sparse_b_row;
			for(InputStreamTile = 0; InputStreamTile < MatrixTileWidth; InputStreamTile++)
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
			
			//printf("here\n");
			// For every matrix in input stream (Width of matrix M/tile size (8))
			for(InputStreamTile = 0; InputStreamTile < (MatrixTileWidth*8); InputStreamTile++)
			{
				// For each word in each input stream matrix
            
                // If both input tiles have data
                if(sparse_a[sparse_a_column] == 1 && sparse_b[sparse_b_row] == 1)
                {
                    //Write A Double Word
                    for(byte = 0; byte < 4; byte++)
                    {
                        // Accumulates byte 03_02_01_00 (+3 reads down)
                        SRAM_ACC_Write(&sram_ptr, &ACC, matrix_a[3 + a_offset - byte]);
                    }
                    for(byte = 0; byte < 4; byte++)
                    {
                        // Accumulates byte 07_06_05_04 (+7 reads down)
                        SRAM_ACC_Write(&sram_ptr, &ACC, matrix_a[7 + a_offset - byte]);
                    }
                    // Write B Double Word
                    for(byte = 0; byte < 4; byte++)
                    {
                        // Accumulates bytes 24_16_08_00 (+M*3 reads to the right)
                        SRAM_ACC_Write(&sram_ptr, &ACC, matrix_b[M*3 + b_offset + AB_DoubleWord - byte*M]);
                    }
                    for(byte = 0; byte < 4; byte++)
                    {
                        // Accumulates bytes 56_48_40_32 (+M*7 reads to the right)
                        SRAM_ACC_Write(&sram_ptr, &ACC, matrix_b[M*7 + b_offset + AB_DoubleWord - byte*M]);
                    }				
                    sram_limit++;
                    written_doublewords++;
                }
                else
				{
                    ignored_doublewords++;
				}
                // a_offset jumps to next column of a inputs to read

                a_offset = a_offset + M;
                sparse_a_column++;
                sparse_b_row++;

				// b_offset skips ahead to the next tile input to read
                if ((AB_DoubleWord+1) % 8 == 0) {
                    b_offset = b_offset + 8;
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
					acc_hps_time += TimerStop(endHPS, startHPS);
					gettimeofday(&startFPGA, NULL);
					// Acknowledging that HPS has sent more data
					*(Control_to_FPGA) = ControlSignalACK;
					// Waiting for FPGA to acknowledge (data is ready)
					while(*(Control_to_HPS) == 1);
					// Waiting for FPGA to annouce that its full
					*(Control_to_FPGA) = ControlSignal;
					while(*(Control_to_HPS) == 0);
					acc_fpga_time += TimerStop(endFPGA, startFPGA);
					gettimeofday(&startHPS, NULL);
					//printf("final sram chunk sent\n");
				}
			}
			// Output tiles generated in column major order
			// Same column of B for every row of A
			// If current tile is the last tile in the column
			if((OutputTile + 1) % (MatrixTileWidth) == 0)
			{
				// a_offset always begins at 0 for new column
				a_column = 0;
				a_row = 0;
				sparse_a_column = 0;
				// b_column increments to the next column
				b_column += 1;
				b_row = 0;
				sparse_b_row = b_column * MatrixTileWidth * 8;
			}
			// Otherwise column remains the same, and row increments
			else
			{
				// a_column always starts at 0 (reads left to right)
				a_column = 0;
				// a_row shifts down to the next row
				a_row += 1;
				sparse_a_column = a_row * MatrixTileWidth * 8;
				sparse_b_row = b_column * MatrixTileWidth * 8;
			}
			// Offsets updated from row and column
			a_offset = 8*(a_column*M + a_row);
			b_offset = 8*(b_column*M + b_row);

			// Ends first HPS Timer and starts FPGA Timer
			acc_hps_time += TimerStop(endHPS, startHPS);

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
			acc_fpga_time += TimerStop(endFPGA, startFPGA);
			//============================================
			// End FPGA 
			//============================================
			gettimeofday(&startHPS, NULL);

			// Resets machine for last tile written to sram
			// Resets sram read pointer to top of assigned SRAM
			while(*(Control_to_HPS) == 0);

			sram_read_ptr = (volatile unsigned int *)sram_virtual_base;
			sram_read_ptr++;
			// Stores volatile output into a static global array
			for(i = 0; i < 64; i++)
			{
				fprintf(out_file, "%d\n", *sram_read_ptr);
				sram_read_ptr++;
			}
			// Resets pointers to overwrite in SRAM
			sram_ptr =(volatile unsigned int *)(sram_virtual_base);
			doublewords_to_send = 0;

			// Ends second HPS Timer
			acc_hps_time += TimerStop(endHPS, startHPS);
		}
	}
    if(test == 1)
    {
		// Flushes matrix_C (Throwing away results, not testing accuracy)
		// Records accumulated times into corresponding trial number
        FPGA_trials[t] = acc_fpga_time;
        HPS_trials[t] = acc_hps_time;
		// Resets accumulated times for previous trial
		acc_hps_time = 0;
		acc_fpga_time = 0;
		// If the final trial
        if(t == (trials - 1))
        {
			printf("\n\033[0;33m%dx%d Done\033[0m\n", M_arr[m], M_arr[m]);
            trimmed_averager(m, "HPS", HPS_trials, trials, HPS_avg, HPS_std_dev, HPS_trimmed_avg, HPS_outlier_count, 1);
            trimmed_averager(m, "FPGA", FPGA_trials, trials, FPGA_avg, FPGA_std_dev, FPGA_trimmed_avg, FPGA_outlier_count, 1);
            
			// Start back at trial 0 for the next size of m
			t = 0;
			m += 1;
        }
		else
		{
			// Otherwise, proceed to the next trial for current M size
			t += 1;
		}
        if(m == (sizeof M_arr)/sizeof(int))
        {
			printf("\033[0;33mAverages in us trimmed within 3 standard deviations for %d trials\033[0m\n", trials);
			// If done with all trials for all M sizes, print results
			size_t i = 0;
            for(i = 0; i < (sizeof M_arr)/sizeof(int); i++)
            {
                printf("\033[0;33m%dx%d Matrix: \n", M_arr[i], M_arr[i]);
                printf("HPS avg:\033[0m %f, \033[0;33mHPS trimmed avg:\033[0m %llu \033[0;33mHPS StdDev:\033[0m %f, \033[0;33mHPS Outliers:\033[0m %d \n", HPS_avg[i], HPS_trimmed_avg[i], HPS_std_dev[i], HPS_outlier_count[i]);
                printf("\033[0;33mFPGA avg:\033[0m %f \033[0;33mHPS trimmed avg:\033[0m %llu \033[0;33mFPGA StdDev:\033[0m %f, \033[0;33mFPGA Outliers:\033[0m %d\n", FPGA_avg[i], FPGA_trimmed_avg[i], FPGA_std_dev[i], FPGA_outlier_count[i]);
            }
            return (0);
        }
		// Otherwise Return to beginning of test and repeat
        else
			goto BEGIN_TEST;
    }
	else if(test == 2)
	{
		// Flushes matrix_C (Throwing away results, not testing accuracy)
		// Records accumulated times into corresponding trial number
        FPGA_trials[t] = acc_fpga_time;
        HPS_trials[t] = acc_hps_time;
		IT_trials[t] = ignored_doublewords;
		ET_trials[t] = (float)ignored_doublewords/(float)(written_doublewords + ignored_doublewords);
		// Resets accumulated times for previous trial
		acc_hps_time = 0;
		acc_fpga_time = 0;
		memset(sparse_a, 0, TileCount*8);
		memset(sparse_b, 0, TileCount*8);
		sparse_b_row = 0;
		sparse_a_column = 0;
		ignored_doublewords = 0;
		written_doublewords = 0;

		if(t == (trials - 1))
        {
            printf("\033[0;33m%d%% Sparsity done \033[0;33m\n", S_arr[s]);
            trimmed_averager(s, "HPS", HPS_trials, trials, HPS_avg, HPS_std_dev, HPS_trimmed_avg, HPS_outlier_count, 1);
            trimmed_averager(s, "FPGA", FPGA_trials, trials, FPGA_avg, FPGA_std_dev, FPGA_trimmed_avg, FPGA_outlier_count, 1);
            trimmed_averager(s, "Empty Tiles", ET_trials, trials, ET_avg, ET_std_dev, ET_trimmed_avg, ET_outlier_count, 1);
			trimmed_averager(s, "Ignored Tiles", IT_trials, trials, IT_avg, IT_std_dev, IT_trimmed_avg, IT_outlier_count, 1);
			
			// Start back at trial 0 for the next size of m
			t = 0;
			s += 1;
        }
		else
		{
			// Otherwise, proceed to the next trial for current M size
			t += 1;
		}
        if(s == (sizeof S_arr)/sizeof(int))
        {
			printf("\033[0;33mAverages in us trimmed within 3 standard deviations for %d trials\033[0m\n", trials);
			// If done with all trials for all M sizes, print results
			size_t i = 0;
            for(i = 0; i < (sizeof S_arr)/sizeof(int); i++)
            {
                printf("\033[0;33m %d%% Sparsity for 1024x1024 \n", S_arr[i]);
                printf("HPS avg:\033[0m %f, \033[0;33mHPS trimmed avg:\033[0m %llu \033[0;33mHPS StdDev:\033[0m %f, \033[0;33mHPS Outliers:\033[0m %d \n", HPS_avg[i], HPS_trimmed_avg[i], HPS_std_dev[i], HPS_outlier_count[i]);
                printf("\033[0;33mFPGA avg:\033[0m %f \033[0;33mFPGA trimmed avg:\033[0m %llu \033[0;33mFPGA StdDev:\033[0m %f, \033[0;33mFPGA Outliers:\033[0m %d\n", FPGA_avg[i], FPGA_trimmed_avg[i], FPGA_std_dev[i], FPGA_outlier_count[i]);
				printf("\033[0;33mEmpty Tile avg:\033[0m %f \033[0;33mEmpty Tile trimmed avg:\033[0m %llu \033[0;33mEmpty Tile StdDev:\033[0m %f, \033[0;33mEmpty Tile Outliers:\033[0m %d\n", ET_avg[i], ET_trimmed_avg[i], ET_std_dev[i], ET_outlier_count[i]);
				printf("\033[0;33mIgnored Tile avg:\033[0m %f \033[0;33mIgnored Tile trimmed avg:\033[0m %llu \033[0;33mIgnored Tile StdDev:\033[0m %f, \033[0;33mIgnored Tile Outliers:\033[0m %d\n", IT_avg[i], IT_trimmed_avg[i], IT_std_dev[i], IT_outlier_count[i]);
            }
            return (0);
        }
		// Otherwise Return to beginning of test and repeat
        else
			goto BEGIN_TEST;
	}
    else
    {
        //out_file = fopen("out_file.txt", "w+");
        //print_output_mat(M, &out_file, matrix_c, MatrixTileWidth, test);
        sram_read_ptr = (volatile unsigned int *) sram_virtual_base;

        printf("\033[0;33m%dx%d Matrix at %d Sparsity Results :\033[0m\n", M, M, sparse_density);
        printf("\033[0;33mHPS format T=%f S, HPS Write T=%f \n\r", ((float)acc_hps_time)/1000000, ((float)acc_write_time)/1000000);
        printf("FPGA read, calculate, and write T=%f S \n\r", ((float)acc_fpga_time)/1000000);
		printf("Empty columns in A: %d, Empty rows in B: %d, \n", empty_columns_a, empty_rows_b);
		printf("Percent of memory accesses skipped: %f\n", ((float)ignored_doublewords/(float)(written_doublewords + ignored_doublewords))*100);
        printf("Output to out_file.txt\033[0m\n\r");
		fclose(out_file);
        return (0);
    }
}
