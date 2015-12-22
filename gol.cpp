#include "gol.h"

void clear_state(byte state[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      state[x][y] = 0;
    }
  }  
}

void generate_next_state(byte current[MATRIX_WIDTH][MATRIX_HEIGHT], byte next[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  clear_state(next);

  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        next[x][y] = should_live(current, x, y) ? 1 : 0;
    }
  }
}

bool should_live(byte state[MATRIX_WIDTH][MATRIX_HEIGHT], int x, int y) {
  int living_neighbors = num_living_neighbors(state, x, y);
  bool should_live = (   ((living_neighbors == 2) and is_alive(state, x, y)) 
                      or  (living_neighbors == 3));
#ifdef DEBUG
  if ((living_neighbors > 0) or is_alive(state, x, y)) {
    Serial.print(x);
    Serial.print(",");
    Serial.print(y);
    Serial.print("   is_alive = ");
    Serial.print(is_alive(state,x,y));
    Serial.print("   living neighbors = ");
    Serial.println(living_neighbors);
  }
#endif

  return should_live;
}

bool is_alive(byte state[MATRIX_WIDTH][MATRIX_HEIGHT], int x, int y) {
  return (state[x][y] == 1);
}

int num_living_neighbors(byte state[MATRIX_WIDTH][MATRIX_HEIGHT], int x, int y) {
  int count = 0;
  for (int i = -1; i < 2; i++) {
    int nx = x+i;
    if ((nx < 0) or (nx >= MATRIX_WIDTH)) continue;
    for (int j = -1; j < 2; j++) {
      int ny = y+j;
      if ((ny < 0) or (ny >= MATRIX_HEIGHT)) continue;
      if ((nx == x) and (ny == y)) continue;
      if (is_alive(state, nx, ny)) {
        count += 1; 
      }
    }
  }
  return count;
}




