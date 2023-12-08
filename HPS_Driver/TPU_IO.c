#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "matrix_methods.h"

#define M_size(M)			((M % 8) ? (((M/8) + 1)*8) : (M))

int file_selection_IO(char *mat_a_name, char *mat_b_name, int AH, int AW, int BH, int BW)
{
    FILE *prev_files;
    int sparse_density = 0;
    char user_response[32];
    printf("Use previous files, new files, or random input? p/n/r\n");
    if(gets(user_response) == NULL)
    {
        printf("Error! Invalid Input %s\n", user_response); 
        exit(-1);   
    }
    // User selects previous files to be reused
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
    // User selects new files to be reused
    else if(user_response[0] == 'n')
    {
        // Otherwise, requests filenames from user
        printf("Enter filename of Matrix A: \n");
        if(fgets(mat_a_name, 32, stdin) == NULL)
        {
            printf("Error! Invalid Input %s\n", user_response); 
            exit(-1);   
        }
        printf("Enter filename of Matrix B: \n");
        if(fgets(mat_b_name, 32, stdin) == NULL)
        {
            printf("Error! Invalid Input %s\n", user_response); 
            exit(-1);   
        }
        // Saves new file names to directory
        prev_files = fopen("prev_files.txt", "w+");
        fprintf(prev_files, "%s\n%s", mat_a_name, mat_b_name);
        fclose(prev_files);
    }
    // User selects random files to be reused
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
        rand_mat(AW, AH, "randomA.bin", mat_a_name, sparse_density, 'r');
        rand_mat(BW, BH, "randomB.bin", mat_b_name, sparse_density, 'r');
        prev_files = fopen("prev_files.txt", "w+");
        fprintf(prev_files, "randomA.bin\nrandomB.bin");
        fclose(prev_files);
        
    }
    else if(user_response[0] == 'b')
    {
        memset(user_response, 0, sizeof user_response);
        // Generates a file A of 1s and a file B of indices
        rand_mat(AW, AH, "binaryA.bin", mat_a_name, 0, 'b');
        rand_mat(BW, BH, "indexedB.bin", mat_b_name, 0, 'i');
        prev_files = fopen("prev_files.txt", "w+");
        fprintf(prev_files, "binaryA.bin\nindexedB.bin");
        fclose(prev_files);
    }
    else
    {
        printf("Error! Choose p, n, or r\n");
        exit(-1);
    }
    return(0);
}

void MM_IO(int *AH, int *AW, int *BH, int *BW)
{
    char user_response[32];

    printf("Enter the height of input matrix A:\n");
    if(atoi(gets(user_response)))
        *AH = atoi(user_response);
    else
    {
        printf("Error! Invalid Matrix A height %s\n", user_response); 
        exit(-1);
    }
    memset(user_response, 0, sizeof user_response);
    printf("Enter the width of input matrix A:\n");
    if(atoi(gets(user_response)))
        *AW = atoi(user_response);
    else
    {
        printf("Error! Invalid Matrix A width %s\n", user_response); 
        exit(-1);
    }
    memset(user_response, 0, sizeof user_response);
    printf("Enter the height of input matrix B:\n");
    if(atoi(gets(user_response)))
        *BH = atoi(user_response);
    else
    {
        printf("Error! Invalid Matrix B height %s\n", user_response); 
        exit(-1);
    }
    memset(user_response, 0, sizeof user_response);
    printf("Enter the width of input matrix B:\n");
    if(atoi(gets(user_response)))
        *BW = atoi(user_response);
    else
    {
        printf("Error! Invalid Matrix B width %s\n", user_response); 
        exit(-1);
    }
    // Incompatible if A width d.n.e. B height
    if(*AW != *BH)
    {
        printf("Error! Incompatible matrix sizes! %d and %d\n", *AW, *BH); 
        exit(-1);
    }
}

void MM_init(uint8_t **matrix_a, uint8_t **matrix_b, char *mat_a_name,
    char *mat_b_name, int AH, int AW, int BH, int BW)
{
    uint8_t* matrix_a_buff;
    uint8_t* matrix_b_buff;

    FILE *file_a, *file_b;

    int i = 0;

    matrix_a_buff = (uint8_t*)calloc((size_t)(AH), sizeof(uint8_t));
    if (matrix_a_buff == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(-1);
    }
    matrix_b_buff = (uint8_t*)calloc((size_t)(BH), sizeof(uint8_t));
    if (matrix_b_buff == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(-1);
    }
    // Initializes Matrix A and B arrays by callocing padded size on the heap
    *matrix_a = (uint8_t*)calloc((size_t)(M_size(AW)*M_size(AH)), sizeof(uint8_t));
    if (*matrix_a == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(-1);
    }
    *matrix_b = (uint8_t*)calloc((size_t)(M_size(BW)*M_size(BH)), sizeof(uint8_t));
    if (*matrix_b == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(-1);
    }

    // Opens files in binary and maps them to calloced arrays on the heap
    file_a = fopen(mat_a_name, "rb");
    // For every column in column major order input
    for(i = 0; i < AW; i++)
    {
        // Read from occupied portions of input file
        if(!fread(matrix_a_buff, AH, 1, file_a))
        {
            printf("unable to read matrix from file\n");
            exit(-1);
        }
        // Write buffer to region of padded file
        memcpy(*matrix_a + i*M_size(AH), matrix_a_buff, AH);
    }
    file_b = fopen(mat_b_name, "rb");
    for(i = 0; i < BW; i++)
    {
        // Read from occupied portions of input file
        if(!fread(matrix_b_buff, BH, 1, file_b))
        {
            printf("unable to read matrix from file\n");
            exit(-1);
        }
        // Write buffer to region of padded file
        memcpy(*matrix_b + i*M_size(BH), matrix_b_buff, BH);
    }
    free(matrix_a_buff);
    free(matrix_b_buff);

    fclose(file_a);
    fclose(file_b);
}

void MM_full_setup(uint8_t **matrix_a, uint8_t **matrix_b, int *AH, int *AW, int *BH, int *BW)
{
    int retval = 0;
    FILE *input_outfileA, *input_outfileB;

    char mat_a_name[32], mat_b_name[32];

    MM_IO(AH, AW, BH, BW);

    // Requests files from user (previous, new, random, or test matrices)
    retval = file_selection_IO(mat_a_name, mat_b_name, *AH, *AW, *BH, *BW);
    if (retval)
    {
        printf("File Selection Failed.\n");
        exit(-1);
    }

    //Initialize matrix a and b arrays and read input files in accordingly
    MM_init(matrix_a, matrix_b, mat_a_name, mat_b_name, *AH, *AW, *BH, *BW);

	input_outfileA = fopen("input_outfileA.txt", "w+");
	input_outfileB = fopen("input_outfileB.txt", "w+");

    print_input_mat(M_size(*AW), M_size(*AH), &input_outfileA, *matrix_a, 'A');
    print_input_mat(M_size(*BW), M_size(*BH), &input_outfileB, *matrix_b, 'B');
}

void Conv_IO(int *AH, int *AW, int *BH, int *BW)
{
    char user_response[32];

    printf("Enter the height of the kernel:\n");
    if(atoi(gets(user_response)))
        *AH = atoi(user_response);
    else
    {
        printf("Error! Invalid kernel height %s\n", user_response); 
        exit(-1);
    }
    memset(user_response, 0, sizeof user_response);
    printf("Enter the width of the kernel:\n");
    if(atoi(gets(user_response)))
        *AW = atoi(user_response);
    else
    {
        printf("Error! Invalid kernel width %s\n", user_response); 
        exit(-1);
    }
    memset(user_response, 0, sizeof user_response);
    printf("Enter the height of Input Activation Matrix:\n");
    if(atoi(gets(user_response)))
        *BH = atoi(user_response);
    else
    {
        printf("Error! Invalid Input Activation Matrix height %s\n", user_response); 
        exit(-1);
    }
    memset(user_response, 0, sizeof user_response);
    printf("Enter the width of Input Activation Matrix:\n");
    if(atoi(gets(user_response)))
        *BW = atoi(user_response);
    else
    {
        printf("Error! Invalid Input Activation Matrix width %s\n", user_response); 
        exit(-1);
    }
}

void conv_init(uint8_t **matrix_a, uint8_t **matrix_b, char *mat_a_name,
    char *mat_b_name, int *AH, int *AW, int *BH, int *BW, int tile_size)
{
    FILE *file_a, *file_b;
    
    uint8_t* matrix_a_buff;
    uint8_t* matrix_b_buff;

    uint8_t* kernel;
    uint8_t* IA;

    int kernel_len = (*AH)*(*AW);
	int kernel_windows_w = (*BW - *AW) + 1;
	int kernel_windows_h = (*BH - *AH) + 1;
	int window_locations = kernel_windows_w*kernel_windows_h;

    // Buffer to read each element of kernel one at a time, in row major order
    matrix_a_buff = (uint8_t*)calloc((size_t)(1), sizeof(uint8_t));
    if (matrix_a_buff == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(-1); // or handle the failure as appropriate
    }
    // Buffer to accumulate the window (size of kernel) for every position in Input Activation Matrix
    matrix_b_buff = (uint8_t*)calloc((size_t)((M_size(kernel_len))), sizeof(uint8_t));
    if (matrix_b_buff == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(-1); // or handle the failure as appropriate
    }

    // Open kernel file into kernel matrix to transform into matrix A
    kernel = (uint8_t*)calloc((size_t)((*AH)*(*AW)), sizeof(uint8_t));
    if (kernel == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(-1); // or handle the failure as appropriate
    }
    // Open IA file into IA matrix to transform into matrix B
    IA = (uint8_t*)calloc((size_t)((*BH)*(*BW)), sizeof(uint8_t));
    if (IA == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(-1); // or handle the failure as appropriate
    }

    // Flattened Kernel is one tile in height, and the length of every element n (1xn)
    *matrix_a = (uint8_t*)calloc((size_t)(M_size(kernel_len)*tile_size), sizeof(uint8_t));
    if (*matrix_a == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(-1); // or handle the failure as appropriate
    }
    // Transformed Input Activation is of size Flattened_Kernel * Flattened_Kernel
    *matrix_b = (uint8_t*)calloc((size_t)((M_size(kernel_len))*M_size(window_locations)), sizeof(uint8_t));
    if (*matrix_b == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(-1); // or handle the failure as appropriate
    }

    // Opens files in binary and maps them to calloced arrays on the heap
    file_a = fopen(mat_a_name, "rb");

    // Read Kernel File into Kernel Matrix
    if(!fread(kernel, (*AH)*(*AW), 1, file_a))
    {
        printf("unable to read kernel from file\n");
        exit(-1);
    }
    file_b = fopen(mat_b_name, "rb");
    // Read IA File into IA Matrix
    if(!fread(IA, (*BH)*(*BW), 1, file_b))
    {
        printf("unable to read input activation from file\n");
        exit(-1);
    }
    kernel_transform(kernel_len, *AH, *AW, *matrix_a, kernel);
    // For every row of window shifts in IA
    IA_transform(kernel_windows_h, kernel_windows_w, *AH, *AW, *BH, kernel_len, *matrix_b, IA);
    *AH = 1;
    *AW = kernel_len;
    *BH = kernel_len;
    *BW = window_locations;

    free(matrix_a_buff);
    free(matrix_b_buff);
    free(kernel);
    free(IA);

    fclose(file_a);
    fclose(file_b);
}

int conv_full_setup(uint8_t **matrix_a, uint8_t **matrix_b,
    int tile_size, int *AH, int *AW, int *BH, int *BW)
{

    char mat_a_name[32], mat_b_name[32];

    FILE *input_outfileA, *input_outfileB;

    int retval = 0;

    Conv_IO(AH, AW, BH, BW);

    int kernel_len = (*AH)*(*AW);
	int kernel_windows_w = (*BW - *AW) + 1;
	int kernel_windows_h = (*BH - *AH) + 1;
	int window_locations = kernel_windows_w*kernel_windows_h;

    retval = file_selection_IO(mat_a_name, mat_b_name, *AH, *AW, *BH, *BW);
    if (retval)
    {
        printf("File Selection Failed.\n");
        return 1; // or handle the failure as appropriate
    }

    conv_init(matrix_a, matrix_b, mat_a_name, mat_b_name, AH, AW, BH, BW, tile_size);

	input_outfileA = fopen("input_outfileA.txt", "w+");
	input_outfileB = fopen("input_outfileB.txt", "w+");

    print_input_mat(M_size(kernel_len), tile_size, &input_outfileA, *matrix_a, 'A');
    print_input_mat(M_size(window_locations), M_size(kernel_len), &input_outfileB, *matrix_b, 'B');
    return(kernel_windows_w);
}