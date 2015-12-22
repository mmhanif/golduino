#include <Arduino.h>

#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 32

void clear_state(byte state[MATRIX_WIDTH][MATRIX_HEIGHT]);

void generate_next_state(byte current[MATRIX_WIDTH][MATRIX_HEIGHT], byte next[MATRIX_WIDTH][MATRIX_HEIGHT]);

bool should_live(byte state[MATRIX_WIDTH][MATRIX_HEIGHT], int x, int y);

bool is_alive(byte state[MATRIX_WIDTH][MATRIX_HEIGHT], int x, int y);

int num_living_neighbors(byte state[MATRIX_WIDTH][MATRIX_HEIGHT], int x, int y);


