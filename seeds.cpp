#include "seeds.h"

void seed_r_pentamino(byte state[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  state[15][15] = 1;
  state[16][15] = 1;
  state[16][14] = 1;  
  state[17][15] = 1;
  state[17][16] = 1;
}

void seed_vertical_line(byte state[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  state[15][15] = 1;
  state[16][15] = 1;
  state[17][15] = 1;
}

void seed_diehard(byte state[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  state[13][13] = 1;
  state[13][17] = 1;
  state[13][18] = 1;
  state[13][19] = 1;
  state[14][12] = 1;
  state[14][13] = 1;
  state[15][18] = 1;
}

void seed_acorn(byte state[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  state[13][13] = 1;
  state[13][14] = 1;
  state[13][17] = 1;
  state[13][18] = 1;
  state[13][19] = 1;
  state[14][16] = 1;
  state[15][14] = 1;
}

void seed_pattern_10_Living(byte state[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  int origin = 14;
  state[origin  ][origin  ] = 1;
  state[origin  ][origin+2] = 1;
  state[origin+1][origin+2] = 1;
  state[origin+2][origin+4] = 1;
  state[origin+3][origin+4] = 1;
  state[origin+3][origin+6] = 1;
  state[origin+4][origin+4] = 1;
  state[origin+4][origin+6] = 1;
  state[origin+4][origin+7] = 1;
  state[origin+5][origin+6] = 1;
}

void seed_pattern_5_x_5(byte state[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  int origin = 14;
  state[origin  ][origin  ] = 1;
  state[origin  ][origin+2] = 1;
  state[origin  ][origin+4] = 1;
  state[origin+1][origin+1] = 1;
  state[origin+1][origin+2] = 1;
  state[origin+1][origin+4] = 1;
  state[origin+2][origin+3] = 1;
  state[origin+2][origin+4] = 1;
  state[origin+3][origin  ] = 1;
  state[origin+4][origin  ] = 1;
  state[origin+4][origin+1] = 1;
  state[origin+4][origin+2] = 1;
  state[origin+4][origin+4] = 1;
}

void seed_random(byte state[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  const int density = random(50);  // 0 - 50%
  int max_num_living = MATRIX_WIDTH * MATRIX_HEIGHT * density / 100;
  for (int i=0; i<max_num_living; i++) {
    int x = random(MATRIX_WIDTH);
    int y = random(MATRIX_HEIGHT);
    state[x][y] = 1;
  }
}



