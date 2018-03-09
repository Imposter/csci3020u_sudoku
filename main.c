#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_LENGTH 256
#define GRID_SIZE 9 // 9x9

typedef struct {
    char values[GRID_SIZE][GRID_SIZE];
    char row_result;
    char column_result;
} puzzle_t;

typedef struct {
    char values[GRID_SIZE / 3][GRID_SIZE / 3];
    char grid_result;
} grid_t;

void *row_thread_worker(void *param) {
    puzzle_t *puzzle = (puzzle_t *)param;
    puzzle->row_result = 1;

    for (char r = 0; r < GRID_SIZE; r++) {
        char *row = puzzle->values[r];
        for (char i = 0; i < GRID_SIZE; i++) {
            for (char j = 0; j < GRID_SIZE; j++) {
                if (i == j) continue;
                if (row[i] == row[j] && row[i] != 0 && row[j] != 0) {
                    puzzle->row_result = 0;
                    return NULL;
                }
            }
        }
    }

    return NULL;
}

void *column_thread_worker(void *param) {
    puzzle_t *puzzle = (puzzle_t *)param;
    puzzle-> column_result = 1;

    for (char c = 0; c < GRID_SIZE; c++) {
        for (char i = 0; i < GRID_SIZE; i++) {
            for (char j = 0; j < GRID_SIZE; j++) {
                if (i == j) continue;
                if (puzzle->values[i][c] == puzzle->values[j][c] && puzzle->values[i][c] != 0 && puzzle->values[j][c] != 0) {
                    puzzle->column_result = 0;
                    return NULL;
                }
            }
        }
    }

    return NULL;
}

void *sub_grid_thread_worker(void *param) {
	grid_t *grid = (grid_t *)param;
	grid->grid_result = 1;
    
    // Loop through each cell in the grid
    for (char x = 0; x < GRID_SIZE / 3; x++) {
        for (char y = 0; y < GRID_SIZE / 3; y++) {
            // Calculate the index at which the current cell resides
			char i = (x * (GRID_SIZE / 3)) + y;

            // Loop through every cell except the current one in the grid
			for (char x2 = 0; x2 < GRID_SIZE / 3; x2++) {
				for (char y2 = 0; y2 < GRID_SIZE / 3; y2++) {
					char i2 = (x2 * (GRID_SIZE / 3)) + y2;
					if (i2 == i) continue;

                    // Check if the two cells have the same values, if they do, the grid is invalid
					if (grid->values[x][y] == grid->values[x2][y2] && grid->values[x][y] != 0 && grid->values[x2][y2] != 0) {
						grid->grid_result = 0;
						return NULL;
					}
				}
			}
        }
	}

    return NULL;
}

char load_puzzle(puzzle_t *puzzle, const char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) return 0;

    char buffer[BUFFER_LENGTH];
    char row = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        char len = strlen(buffer);
        char col = 0;
        for (char i = 0; i < len && col < GRID_SIZE; i++) {
            char c = buffer[i];
            if (c == ' ') continue; // Ignore spaces
            puzzle->values[row][col] = atoi(&c);
            col++;
        }
        row++;
    }

    fclose(file);
    return 1;
}

void split_grid(char grid[GRID_SIZE][GRID_SIZE], char new_grid[GRID_SIZE / 3][GRID_SIZE / 3], char x, char y) {
	const char new_size = GRID_SIZE / 3;
	for (char r = x; r < x + new_size; r++) {
        for (char c = y; c < y + new_size; c++) {
            new_grid[r - x][c - y] = grid[r][c];
        }
    }
}

int main(int argc, char **argv) {
    puzzle_t puzzle;
    load_puzzle(&puzzle, "puzzle.txt");

    // Threads
    pthread_t threads[GRID_SIZE + 2];

    // Row thread
    pthread_create(&threads[0], NULL, row_thread_worker, (void *)&puzzle);

    // Column thread
    pthread_create(&threads[1], NULL, column_thread_worker, (void *)&puzzle);

    // Sub grid thread
    grid_t grid_args[GRID_SIZE];
    for (char x = 0; x < GRID_SIZE / 3; x++) {
		for (char y = 0; y < GRID_SIZE / 3; y++) {
			char i = (x * (GRID_SIZE / 3)) + y;
			grid_t *grid = &grid_args[i];
			split_grid(puzzle.values, grid->values, x * 3, y * 3);
			pthread_create(&threads[i + 2], NULL, sub_grid_thread_worker, (void *)grid);
		}
    }

    // Wait for results
    for (char i = 0; i < GRID_SIZE + 2; i++) {
        pthread_join(threads[i], NULL);
    }

	// Prchar results
    if (puzzle.row_result == 0 || puzzle.column_result == 0) {
		printf("invalid\r\n");
		return 1;
    } else {
		for (char i = 0; i < GRID_SIZE; i++) {
			if (grid_args[i].grid_result == 0) {
				printf("invalid\r\n");
				return 1;
			}
		}
	}

	printf("valid\r\n");
    return 0;
}