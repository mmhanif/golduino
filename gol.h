/*
 * Basic Game of Life functions
 */

#include <Arduino.h>

#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 32

/*
 *  Given current state, set values in next state
 */
void generate_next_state(byte current[MATRIX_WIDTH][MATRIX_HEIGHT], byte next[MATRIX_WIDTH][MATRIX_HEIGHT]);

/*
 *  Set all cells in given state to 0 (dead)
 */
void clear_state(byte state[MATRIX_WIDTH][MATRIX_HEIGHT]);

/*
 * Private functions
 */
bool _should_live(byte state[MATRIX_WIDTH][MATRIX_HEIGHT], int x, int y);

bool _is_alive(byte state[MATRIX_WIDTH][MATRIX_HEIGHT], int x, int y);

int _num_living_neighbors(byte state[MATRIX_WIDTH][MATRIX_HEIGHT], int x, int y);


