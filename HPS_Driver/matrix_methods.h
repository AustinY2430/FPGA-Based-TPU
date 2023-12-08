#ifndef _MATRIX_METHODS_H
#define _MATRIX_METHODS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Generates a random binary file of chars size W*H. Saves to prev_file. */
/* Modes allow for Randomized (r), Ones (b), or C-MO Indexed (i) arrays. */
void rand_mat(int W, int H, char* filename, char* MAT_AorB,
    int sparse_density, char mode);

/* Prints input matrices to terminal in row major order as        */
/* signed decimals and outputs them to text files for validation. */ 
void print_input_mat(int W, int H, FILE **input_outfile,
    uint8_t *input_matrix, char letter);

/* Called after files are opened and before writing first tile. Seeks through*/
/* input matrices recording empty columns in A and rows in B to sparse_matrix*/
/* Returns the amount of empty columns or rows. */
int sparse_mat_create(int H, int W, int TileCount, uint8_t *input_matrix,
    char **sparse_matrix, char AorB, int tile_size);

/* Prints Output Matrix from outfile.txt in row major order */
/* Stored in tile-column major order.                       */
int fread_output_mat(int W, int H, int C_W, char mode);

/* Reads Input from TPU and writes into an external file. */
/* Trims zeros from tile according to tile position.      */
void fwrite_output_mat(volatile unsigned int **sram_read_ptr, FILE *out_file,
    int *tile_print_row, int *tile_print_col, int AH, int BW);

/* Converts Matrix A into the flattened Matrix required for Convolution. */
void kernel_transform(int kernel_len, int AH, int AW, uint8_t *matrix_a,
    uint8_t *kernel);

/* Converts Matrix B into the transformed Matrix required for Convolution. */
void IA_transform(int kernel_windows_h, int kernel_windows_w, int AH, 
    int AW, int BH, int kernel_len, uint8_t *matrix_b, uint8_t *IA);

#endif /* _MATRIX_METHODS_H */