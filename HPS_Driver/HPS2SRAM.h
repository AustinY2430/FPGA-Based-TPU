#ifndef _HPS2SRAM_H
#define _HPS2SRAM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Called before writing each tile to TPU's SRAM, counts how many 16B values */
/* to write per tile according to the sparse matrices generated from earlier */
int sparse_element_prediction(int sparse_a_column, int sparse_b_row, int TilesPerOp, char *sparse_a, char *sparse_b);

/* Accumulates bytes into a word, writes to SRAM and flushes buffer when full. */
void SRAM_ACC_Write(volatile unsigned int **sram_ptr_ptr,
    uint32_t *ACC, uint8_t ByteToACC);

/* Updates Offsets of the desired row and col positions to be in for */
/* Matrix A, Matrix B, and their Sparse Representation Counterparts. */
void tile_offset_updater(int *a_column, int *a_row, int *sparse_a_column,
    int *b_column, int *b_row, int *sparse_b_row, int AH, int AW, int BH,
        int *a_offset, int *b_offset, int OutputTile);

/* Contains the main operation of the TPU sending tile data, and reading results. */
int HPS_ENGINE(uint8_t **matrix_a, uint8_t **matrix_b, int tile_size,
	int AH, int AW, int BH, int BW, unsigned long long *acc_hps_time,
		unsigned long long *acc_fpga_time, int *empty_columns_a, int *empty_rows_b,
            int *ignored_doublewords, int *written_doublewords, FILE *out_file);
#endif /* _HPS2SRAM_H */
