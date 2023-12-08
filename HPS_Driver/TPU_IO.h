#ifndef _TPU_IO_H
#define _TPU_IO_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* IO to select files from previous, new, random, or test. */
int file_selection_IO(char *mat_a_name, char *mat_b_name, int AH, int AW, int BH, int BW);



/* IO to accept MM Dimensions from User. */
void MM_IO(int *AH, int *AW, int *BH, int *BW);

/* Method to intitialize MM matrices with file data depending on input sizes. */
void MM_init(uint8_t **matrix_a, uint8_t **matrix_b, char *mat_a_name,
    char *mat_b_name, int AH, int AW, int BH, int BW);

/* complete IO to accept matrix A and B dimensions. Calls file_selection_IO and MM_init */
void MM_full_setup(uint8_t **matrix_a, uint8_t **matrix_b, int *AH, int *AW, int *BH, int *BW);

/* IO to accept Convolution Dimensions from User. */
void Conv_IO(int *AH, int *AW, int *BH, int *BW);

/* Method to intitialize convolution matrices with file data depending on input sizes. */
void conv_init(uint8_t **matrix_a, uint8_t **matrix_b, char *mat_a_name,
    char *mat_b_name, int *AH, int *AW, int *BH, int *BW, int tile_size);

/* complete IO to accept kernel and IA dimensions. Calls file_selection_IO and conv_init */
int conv_full_setup(uint8_t **matrix_a, uint8_t **matrix_b,
    int tile_size, int *AH, int *AW, int *BH, int *BW);

#endif /* _TPU_IO_H */