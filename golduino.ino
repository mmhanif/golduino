/*
    Conway's Game of Life implemented for the SmartMatrix Display (http://docs.pixelmatix.com/SmartMatrix/postkick.html)

    The program reads commands from the serial port to configure various parameters and then run the simulation
    
    Based on code from SmartMatrix Features Demo - Louis Beaudoin (Pixelmatix)
*/

#include <SmartMatrix3.h>
#include "gol.h"
#include "seeds.h"
#include "debugutils.h"

/* 
 *  These lines come straight from the SmartMatrix Features Demo code
 */

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 32;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

const int defaultBrightness = 100*(255/100);    // full brightness
//const int defaultBrightness = 15*(255/100);    // dim: 15% brightness
const rgb24 defaultBackgroundColor = {0, 0, 0};

// Uncomment line below for debug printouts
#define DEBUG


/*
 *  Enums
 */
enum Mode { Configure, Running_Step, Running_Continuous };
enum SeedChoice { Input_Seed = 0, R_Pentamino, Acorn, Diehard, Pattern_10, Pattern_5x5, Random_Seed, Random_Choice};
enum ColorChoice { Input_Color = 0, Red, Green, Blue, Random_Color, Continuous_Random};


/* 
 *  These variables can be directly set by instructions received via the Serial port
 */
Mode mode = Configure;                        // Program switches between Configuration mode and two Running modes
SeedChoice seed_choice = Random_Choice;       // Which seed state did we initialize the first iteration with 
ColorChoice color_choice = Random_Color;      // Use one of Red, Green, Blue or a Random color, or Random color each iteration
int iteration_interval = 250;                 // Delay between each iteration when running in continuous mode
rgb24 input_color = {255,0,0};                // Stores color input directly as a parameter via serial port


/*
 *  State variables used when running the simulation
 *  These are reset by the reset_state() function
 */
byte state_0[MATRIX_WIDTH][MATRIX_HEIGHT]; // state of cells: 1 for alive, 0 for dead 
byte state_1[MATRIX_WIDTH][MATRIX_HEIGHT]; // We toggle between state_0 and state_1 as we iterate
int current_state = 0;                     // 0 => state state_0, 1 => state_1
int static_count = 0;                      // number of times there has been no change from one state to the next
int iterations = 0;                        // How many iterations have we done so far

int max_iterations = 300;                  // Max number of iterations before we reset (changes depending on seed)
rgb24 color_new;                           // Color to use for newly alive cells
rgb24 color_old;                           // Color to use for cells were alive in last iteration

/*
 * Perform initial set of the matix and make sure all parameters are reset
 */
void setup() {
  Serial.begin(38400);

  matrix.addLayer(&backgroundLayer); 
  matrix.begin();
  matrix.setBrightness(defaultBrightness);
  backgroundLayer.enableColorCorrection(true);

  reset_state();
}

/*
 *  Read parameters from serial port and run next iteration
 *  If we have gone past max iterations, reset with a new random seed
 */
void loop() {
  char c;
  
  switch (mode) {
    case Configure:
       read_config();
       break;

    case Running_Step:
      // Wait for instruction to move to take action
      c = read_char(true);
      if (c == 'n') {
        next();
      } 
      if (c == 'c') {
        mode = Configure;
        DEBUG_PRINTLN("Mode set to Configure");
        return;
      }
      if (c == 't') {
        mode = Running_Continuous;
        return;
      }
      break;

    case Running_Continuous:
      c = read_char(false);
      if (c == 'c') {
        mode = Configure;
        DEBUG_PRINTLN("Mode set to Configure");
        return;
      }
      if (c == 's') {
        mode = Running_Step;
        return;
      }
      next();
      break;
  }  

  if (iterations > max_iterations) {
    reset_state();
    init_simulation();
  }
  
  delay(iteration_interval);  // wait for half a second
}


/*
 * 
 */
void read_config() {
  char c = read_char(true);
  int s_choice, c_choice, x, y;

  switch (c) {
    case 's':
      DEBUG_PRINTLN("Mode set to Running_Step");
      mode = Running_Step;
      break;

    case 't':
      DEBUG_PRINTLN("Mode set to Running_Continuous");
      mode = Running_Continuous;
      break;
      
    case 'e':
      DEBUG_PRINTLN("Attempting to set seed");
      // Set seed
      reset_state();
      s_choice = Serial.parseInt();
      if (s_choice > (int)Random_Choice) {
        s_choice = (int)Random_Choice; 
      }
      seed_choice = (SeedChoice)s_choice;
      DEBUG_PRINT("Seed = ");
      DEBUG_PRINTLN(seed_choice);
      if (seed_choice == 0) {
        while(true) {
          x = Serial.parseInt();
          y = Serial.parseInt();
          if (x == -1) {
            break;
          }
          state_0[x][y] = 1;
          DEBUG_PRINT("(");
          DEBUG_PRINT(x);
          DEBUG_PRINT(",");
          DEBUG_PRINT(y);
          DEBUG_PRINTLN(")");
        }
        #ifdef DEBUG
        print_state(state_0);
        #endif
      }
      seed_state();
      break;

    case 'l':
      DEBUG_PRINTLN("Attempting to set color choice");
      // Set color_choice
      c_choice = Serial.parseInt();
      if (c_choice > (int)Continuous_Random) {
        c_choice = (int)Continuous_Random; 
      }
      color_choice = (ColorChoice)c_choice;
      DEBUG_PRINT("color choice = ");
      DEBUG_PRINTLN(color_choice);
      if (color_choice == Input_Color) {
        input_color.red   = Serial.parseInt(); 
        input_color.green = Serial.parseInt(); 
        input_color.blue  = Serial.parseInt(); 
        DEBUG_PRINT("(");
        DEBUG_PRINT(input_color.red);   DEBUG_PRINT(",");
        DEBUG_PRINT(input_color.green); DEBUG_PRINT(",");
        DEBUG_PRINT(input_color.blue);  DEBUG_PRINT(")");        
      }
      set_colors();
      break;

    case 'i':
      DEBUG_PRINTLN("Attempting to set iteration interval");
      // Set iteration interval
      iteration_interval = Serial.parseInt();
      DEBUG_PRINT("iteration interval = ");
      DEBUG_PRINTLN(iteration_interval);
      break;

    case 0:  // Timeout
    default:
      mode = Running_Continuous;
      break;
  }
}


/*
 * Reset variables for new simulation
 */
void reset_state() {
  clear_state(state_0);
  clear_state(state_1);
  current_state = 0;
  static_count = 0;
  iterations = 0;

  // clear screen
  backgroundLayer.fillScreen(defaultBackgroundColor);
  backgroundLayer.swapBuffers();
}

void init_simulation() {
  seed_state();
  set_colors();
}

void seed_state() {
  SeedChoice seed = seed_choice;
  if (seed == Random_Choice) {
    // Exclude Input_Seed(0) and Random_Choice(7)
    seed = (SeedChoice) (random(5) + 1);
  }
  _seed_state(seed);
}

void _seed_state(SeedChoice seed) {
  //enum SeedChoice { Input_Seed = 0, R_Pentamino, Acorn, Diehard, Pattern_10, Pattern_5x5, Random_Seed, Random_Choice};
  switch (seed) {
    case Input_Seed:
      // Seed state already set from input;
      max_iterations = 256;
      break;
    case R_Pentamino:
      seed_r_pentamino(state_0);
      max_iterations = 475;
      break;
    case Acorn:
      seed_acorn(state_0);
      max_iterations = 100;
      break;
    case Diehard:
      seed_diehard(state_0);
      max_iterations = 200;
      break;
    case Pattern_10:
      seed_pattern_10_Living(state_0);
      max_iterations = 300;
      break;
    case Pattern_5x5:
      seed_pattern_5_x_5(state_0);
      max_iterations = 250;
      break;
    case Random_Seed:
    default:
      seed_random(state_0);
      max_iterations = 256;
      break;
  }
}


/*
 * Iterate game of life
 * If we determine we are in a static state then reset
 */
void next() {
  if (current_state == 0) {
    draw_state(state_0, state_1);
    generate_next_state(state_0, state_1);
    current_state = 1;
  }
  else {
    draw_state(state_1, state_0);
    generate_next_state(state_1, state_0);
    current_state = 0;
  }

  if (is_static()) {
    static_count += 1;
  }
  else {
    static_count = 0;
  }
  if (static_count > 3) {
    reset_state();
    init_simulation();
  }

  iterations += 1;

  DEBUG_PRINT("static_count = ");
  DEBUG_PRINTLN(static_count);
}

/*
 * Returns true if state_0 == state_1
 */
bool is_static() {
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      if (state_0[x][y] != state_1[x][y]) {
        return false;
      }
    } 
  }
  return true; 
}

/* 
 *  Draw given state on SmartMatrix
 */
void draw_state(byte current[MATRIX_WIDTH][MATRIX_HEIGHT], byte last[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  // If color_choice is Continuous_Random then we reset the calor each iteration,
  // otherwise it is only set when we call reset_state()
  if (color_choice == Continuous_Random) {
    set_colors();
  }
  rgb24 *color;

  // Fill layer with the background color
  backgroundLayer.fillScreen(defaultBackgroundColor);

  // Color in living cells
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      if (current[x][y] == 1) {
        color = (last[x][y] == 1) ? &color_old : &color_new;
        backgroundLayer.drawPixel(x, y, *color);
      }
    }
  }

  backgroundLayer.swapBuffers();      
}

/*
 * Set colors depending on color_choice
 */
void set_colors() {
  switch (color_choice) {
    case Red:
      color_new.red = 255; color_new.green = 0; color_new.blue = 0;
      color_old.red = 100; color_old.green = 0; color_old.blue = 0;
      break;
    case Green:
      color_new.red = 0; color_new.green = 255; color_new.blue = 0;
      color_old.red = 0; color_old.green = 100; color_old.blue = 0;
      break;
    case Blue:
      color_new.red = 0; color_new.green = 0; color_new.blue = 255;
      color_old.red = 0; color_old.green = 0; color_old.blue = 100;
      break;
    case Random_Color:
    case Continuous_Random:
    default:
      int red_val = random(156);
      int green_val = random(156);
      int blue_val = random(156);
      color_new.red = red_val+100; color_new.green = green_val+100; color_new.blue = blue_val+100;
      color_old.red = red_val; color_old.green = green_val; color_old.blue = blue_val;
      break;
    }   
}

/*
 *  Helper function for reading from the serial port 
 *  if block = true, then loop until we receive a char or we timout
 *  Returns 0 if no character read
 */
char read_char(bool block) {
  const unsigned long MAX_WAIT_TIME = 30000;  // 10 seconds
  char c = 0;
  unsigned long start_time, wait_time;
  start_time = millis();
  while (block && (Serial.available() == 0)) {
    wait_time = millis() - start_time;
    if (wait_time > MAX_WAIT_TIME) {
      break;
    }
  }
  if (Serial.available()) {
    c = Serial.read();
    DEBUG_PRINT("Read ");
    DEBUG_PRINTLN(c);
  }
  return c;
}

/*
 *  Helper function to print set of living cells in a given state matrix
 */
void print_state(byte state[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  Serial.print("[");
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      if (state[x][y]) {
        Serial.print("(");
        Serial.print(x);
        Serial.print(",");
        Serial.print(y);
        Serial.print("),");
      }
    } 
  }
  Serial.println("]");; 
}



