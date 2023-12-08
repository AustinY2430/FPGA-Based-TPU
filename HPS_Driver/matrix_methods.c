#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define M_size(M)			((M % 8) ? (((M/8) + 1)*8) : (M))

void rand_mat(int W, int H, char* filename, char* MAT_AorB,
    int sparse_density, char mode)
{
    FILE* Rand_Mat;
    Rand_Mat = fopen(filename, "w+");
    int i = 0;
    unsigned char rand_c;

	if(mode == 'r') {
		for(i = 0; i < W*H; i++) {
			// P of random nonzero value from provided sparsity
			if(((rand() % 100) + 1) > sparse_density) {
				rand_c = rand();
				fprintf(Rand_Mat, "%c", rand_c);
			}
			// Q of zero value
			else
				fprintf(Rand_Mat, "%c", 0x00);
		}
	}
	// Prints file of ones
	if(mode == 'b') {
        for(i = 0; i < W*H; i++)
		    fprintf(Rand_Mat, "%c", 0x01);
	}
	// Prints file of indices
	if(mode == 'i') {
		for(i = 0; i < W*H; i++)
			fprintf(Rand_Mat, "%c", 0x01*i);
	}
    strcpy(MAT_AorB, filename);
    fclose(Rand_Mat);
}

/* Prints input matrices in row major order as     */
/* signed decimals and outputs them to text files. */ 
void print_input_mat(int W, int H, FILE **input_outfile, uint8_t *input_matrix, char letter)
{
	if(W <= 32)
		printf("\033[0;33mMatrix %c:\033[0m\n", letter);
	int i = 0, row = 0, index = 0;
	for(i = 1; i <= W * H; i++)
	{
		fprintf(*input_outfile, "%d ", (signed char)input_matrix[index]);
		if(W <= 32)
			printf("\033[0;37m%4d	", (signed char)input_matrix[index]);
        // If on the final column, move to the beginning of the next row
		if(i % W == 0)
		{
			row++;
			index = row;
			fprintf(*input_outfile, "\n");
			if(W <= 32)
				printf("\n");
		}
        // Otherwise prints in row major by reading left to right
		else
			index += H;
	}
	fclose(*input_outfile);
	if(W <= 32)
		printf("\033[0;33mInput Matrix %c output to	input_outfile%c.txt\033[0m\n\r", letter, letter);
}

/* Called after files are opened and before writing first tile. Seeks through*/
/* input matrices recording empty columns in A and rows in B to sparse_matrix*/
/* Returns the amount of empty columns or rows. */
int sparse_mat_create(int H, int W, int TileCount, uint8_t *input_matrix,
    char **sparse_matrix, char AorB, int tile_size)
{
	int sparse_offset = 0;
	int empty_doubleword_count = 0;
	int sparse_element = 0;
	int i = 0;
	int j = 0;
	int row = 0;
	int column = 0;

	*sparse_matrix = (char*)calloc((size_t)(TileCount*tile_size), sizeof(char));
	if (*sparse_matrix == NULL)
	{
		printf("Memory allocation failed.\n");
		return 1; // or handle the failure as appropriate
	}
	
    // Sparse Matrix A is constructed by scanning 8x1 columns in each tile row
    // Stored in row major order due to compatibility with sparse checks
	if(AorB == 'a')
	{
        // 8 columns for each tile in greater matrix
		for(i = 0; i < TileCount*tile_size; i++)
		{
            // check if each element in column is zero
			for(j = 0; j < tile_size; j++)
			{
                // If data is nonzero, mark data in sparse_matrix
				if(input_matrix[j + sparse_offset] != 0)
				{
					(*sparse_matrix)[sparse_element] = 1;
					// printf("%d", input_matrix[j + sparse_offset]);
					break;
				}
			}
			// printf("\n");
            // Otherwise mark empty in sparse_matrix
			if((*sparse_matrix)[sparse_element] != 1)
			{
				empty_doubleword_count++;
				(*sparse_matrix)[sparse_element] = 0;
			}
			// printf("a%d: %d\n", i + j, sparse_matrix[sparse_element]);
			sparse_element++;
            // If at right edge of matrix, go back to left and move down a row
			if(column == (W-1))
			{
				column = 0;
				row++;
			}
            // Otherwise move right by a column
			else
				column++;
            // Offset calculated using desired row and column
			sparse_offset = column*H + tile_size*row;
		}
	}
    // Sparse Matrix B is constructed by scanning 1x8 rows in each column
    // Stored in column major order
	if(AorB == 'b')
	{
        // 8 rows for each tile in greater matrix
		for(i = 0; i < TileCount*tile_size; i++)
		{
            // check if each element in column is zero
			for(j = 0; j < tile_size; j++)
			{
                // If data is nonzero, mark data in sparse_matrix
				if(input_matrix[j*H + sparse_offset] != 0)
				{
					// printf("%d", input_matrix[j + sparse_offset]);
					(*sparse_matrix)[sparse_element] = 1;
					break;
				}
			}
			// printf("\n");
            // Otherwise mark empty in sparse_matrix
			if((*sparse_matrix)[sparse_element] != 1)
			{
				(*sparse_matrix)[sparse_element] = 0;
				empty_doubleword_count++;
			}
			// printf("b%d: %d\n", i + j, sparse_matrix[sparse_element]);
			sparse_element++;
            // If at bottom of the matrix, go back
            // to the top and move right one row
			if(row == (H-1))
			{
				column++;
				row = 0;
			}
            // Otherwise move down by a row
			else
				row++;
            // Offset calculated using desired row and column
			sparse_offset = tile_size*column*H + row;
		}
	}
	return empty_doubleword_count;
}

/* Prints Output Matrix from outfile.txt in row major order */
/* Stored in tile-column major order                        */
int fread_output_mat(int W, int H, int C_W, char mode)
{
    // Output File to read from
	FILE* out_file;
    out_file = fopen("out_file.txt", "r");
    // Iterators
	int i, j, k = 0;
	// Initialize Buffer to read outfile.txt into
	signed int* matrix_c = (signed int*)calloc((size_t)(W*H), sizeof(signed int));
	for(i = 0; i < H*W; i++)
	{
		if(!fscanf(out_file, "%d", &matrix_c[i]))
		{
			printf("Error! Unable to read out of outfile\n"); 
			exit(-1);
		}
	}
	int print_num = 0;
    int tile_size = 8;
    int complete_tiles_H = H/tile_size;
    int complete_tiles_W = W/tile_size;
    int complete_tile_size = tile_size*tile_size;
    int partial_H = H % tile_size;
    int partial_W = W % tile_size;
    int partial_lower_amount_per_tile,  partial_rightmost_amount = 0;
    int tile_offset = 0;
	// If Output Matrix is wider than a tile and taller than but not
    // divisible by 8, Output Matrix includes a partial lower amount
    if((H > tile_size) && (W >= tile_size) && partial_H)
    {
		// Partial lower amount is the width of a full tile times
        // the height of the partial remaining amount
        partial_lower_amount_per_tile = partial_H*tile_size;
    }
	// If Output Matrix is wider than a tile, but only one tile tall
	else if((W >= tile_size) && (H < tile_size))
		// Partial lower amount is the width of a full tile times
        // the complete height of the output matrix
		partial_lower_amount_per_tile = H*tile_size;
	// Otherwise, the output is smaller than a single tile
	else
        // Partial lower amount is just the complete tile size
		partial_lower_amount_per_tile = partial_H * W;
	// If Output Matrix is taller than a tile and wider but not
    // divisible by tile size, Output Matrix includes a partial right amount
    if((W > tile_size) && (H >= tile_size) && partial_W)
		// Partial right amount is height of a full tile times
        // the width of the partial remaining amount;
        partial_rightmost_amount = partial_W*tile_size;
	// If Output Matrix is taller than a tile, but only one tile wide
    else if((H >= tile_size) && (W < tile_size))
    {
		// partial rightmost amount is the height of a full tile times
        // the complete width of the output matrix
        partial_rightmost_amount = W*tile_size;
    }
	// Otherwise, the output is smaller than a single tile, no partial
    else
    {
        partial_rightmost_amount = partial_W * H;
    }
    // printf("partial_lower_amount_per_tile: %d\n", partial_lower_amount_per_tile);
    // printf("partial_right_amount: %d\n", partial_rightmost_amount);
    // printf("complete tiles w: %d\n", complete_tiles_W);
    // printf("complete tiles h: %d\n", complete_tiles_H);

	if(complete_tiles_H)
	{
		// For every row in the height of the complete tiles
		for(i = 0; i < complete_tiles_H*tile_size; i++)
		{
			// For every complete tile in the width of the row
			for(j = 0; j < complete_tiles_W; j++)
			{
				// For every element in each complete tile
				for(k = 0; k < tile_size; k++)
					printf("\033[0;37m%4d	", (signed int)matrix_c[k*tile_size + (i % tile_size) + tile_offset]);
				tile_offset += (complete_tile_size*complete_tiles_H + partial_lower_amount_per_tile);

			}
			// For the partial right amount of the end of the row
			if(partial_rightmost_amount)
			{
				tile_offset = (complete_tile_size*complete_tiles_H + partial_lower_amount_per_tile)*complete_tiles_W + (i / tile_size)*partial_rightmost_amount;
				for(k = 0; k < partial_W; k++)
					printf("\033[0;37m%4d	", (signed int)matrix_c[k*tile_size + (i % tile_size) + tile_offset]);
			}
			printf("\n");
			// If the last row in the current tile row, increment to the next
			tile_offset = complete_tile_size*((i + 1) / tile_size);
		}
	}
    if(partial_lower_amount_per_tile)
    {
		// For every row in the partial below the complete tiles
        for(i = 0; i < partial_H; i++)
        {
            // For every complete tile in the width of the row
            for(j = 0; j < complete_tiles_W; j++)
            {
                for(k = 0; k < tile_size; k++)
                {
                    printf("\033[0;37m%4d	", (signed int)matrix_c[k*partial_H + (i % tile_size) + tile_offset]);
					print_num++;
					if(((print_num % C_W) == 0) && (mode == 'c'))
						printf("\n");
                }
                tile_offset += (complete_tile_size*complete_tiles_H + partial_lower_amount_per_tile);
            }
			// Bottom Right Corner
            if(partial_rightmost_amount)
            {
				tile_offset = (complete_tile_size*complete_tiles_H + partial_lower_amount_per_tile)*complete_tiles_W + complete_tiles_H*partial_rightmost_amount;
                for(k = 0; k < partial_W; k++)
                {
                    printf("\033[0;37m%4d	", (signed int)matrix_c[k*partial_H + (i % tile_size) + tile_offset]);
					print_num++;
					if(((print_num % C_W) == 0) && (mode == 'c'))
						printf("\n");
                }
            }
            if(mode != 'c')
				printf("\n");
            tile_offset = complete_tile_size*complete_tiles_H;
        }
    }
    // If Output is simply a single tile, print in R-MO by adding height
	else if(H > tile_size && W > tile_size)
	{
		for(i = 0; i < H; i++)
		{
			for(j = 0; j < W; j++)
			{
				printf("\033[0;37m%4d	", (signed int)matrix_c[i + j*H]);
				print_num++;
				if(((print_num % C_W) == 0) && (mode == 'c'))
					printf("\n");
			}
			if(mode != 'c')
				printf("\n");
		}
	}
    return(1);
}

void fwrite_output_mat(volatile unsigned int **sram_read_ptr, FILE *out_file, int *tile_print_row, int *tile_print_col, int AH, int BW){
    int i, j = 0;
    // Last tile is incomplete on bottom and right
    if(((*tile_print_row + 1) == M_size(AH)/8) && ((*tile_print_col + 1) == M_size(BW)/8) && (AH % 8) && (BW % 8))
    {
        // printf(" is the bottom right corner\n");
        for(i = 0; i < BW % 8; i++)
        {
            // Only record values
            for(j = 0; j < (AH % 8); j++)
            {
                fprintf(out_file, "%d\n", **sram_read_ptr);
                *sram_read_ptr = *sram_read_ptr + 1;
            }
            // Skip past zeros
            *sram_read_ptr = *sram_read_ptr + (8 - AH % 8);
        }
        // Skip past zeros
        *sram_read_ptr = *sram_read_ptr + (8 - BW % 8)*8;
    }
    // If tile in bottom row above padded tiles, tile is incomplete on bottom
    else if(((*tile_print_row + 1) == M_size(AH)/8) && (AH % 8))
    {
        // printf(" is lowest partial row\n");
        for(i = 0; i < 8; i++)
        {
            // Only record values
            for(j = 0; j < AH % 8; j++)
            {
                fprintf(out_file, "%d\n", **sram_read_ptr);
                *sram_read_ptr = *sram_read_ptr + 1;
            }
            // Skip past zeros
            *sram_read_ptr = *sram_read_ptr + (8 - AH % 8);
        }
    }
    // If tile in final column, tile is incomplete to the right
    else if(((*tile_print_col + 1) == M_size(BW)/8) && (BW % 8))
    {
        // printf(" is rightmost partial row\n");
        for(i = 0; i < BW % 8; i++)
        {
            // Only record values
            for(j = 0; j < 8; j++)
            {
                fprintf(out_file, "%d\n", **sram_read_ptr);
                *sram_read_ptr = *sram_read_ptr + 1;
            }
        }
        // Skip past zeros
        *sram_read_ptr = *sram_read_ptr + (8 - BW % 8)*8;
    }
    // Otherwise a completely full tile
    else
    {
        // printf(" is completely full\n");
        // 8x8 values in a full tile
        for(i = 0; i < 64; i++)
        {
            fprintf(out_file, "%d\n", **sram_read_ptr);
            *sram_read_ptr = *sram_read_ptr + 1;
        }
    }
    if(((*tile_print_row + 1) % (M_size(AH)/8)) == 0)
    {
        *tile_print_col = *tile_print_col + 1;
        *tile_print_row = 0;
    }
    else
    {
        *tile_print_row = *tile_print_row + 1;
    }
}

void kernel_transform(int kernel_len, int AH, int AW, uint8_t *matrix_a, uint8_t *kernel)
{
	int i, j, k = 0;
	// For every element in the kernel
	for(i = 0; i < kernel_len; i++)
	{
		// For every row in Kernel file
		for(j = 0; j < AH; j++)
		{
			// For every element of the row
			for(k = 0; k < AW; k++)
			{
				// Map in Row-Major Order to the top of the matrix_a file
				matrix_a[i*8] = kernel[j + k*AH];
			}
		}
	}
}

// For every row of window shifts in IA
void IA_transform(int kernel_windows_h, int kernel_windows_w, int AH, int AW, int BH, int kernel_len, uint8_t *matrix_b, uint8_t *IA)
{
	int h, i, j, k = 0;
	int print_index = 0;
	for(h = 0; h < kernel_windows_h; h++)
	{
		// for every position of window in row of IA
		for(i = 0; i < kernel_windows_w; i++)
		{
			// For every row of window file
			for(j = 0; j < AH; j++)
			{
				// For every element of row of window
				for(k = 0; k < AW; k++)
				{
					// Map to column of matrix_B
					matrix_b[print_index] = IA[j + k*BH + h + i*BH];
					print_index++;
				}
			}
			print_index += (8 - kernel_len % 8);
		}
	}
}