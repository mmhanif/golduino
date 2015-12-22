/*
    SmartMatrix Features Demo - Louis Beaudoin (Pixelmatix)
    This example code is released into the public domain
*/

#include <SmartMatrix3.h>
#include "gol.h"
#include "seeds.h"

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
const int defaultScrollOffset = 6;
//const rgb24 defaultBackgroundColor = {0x40, 0, 0};
const rgb24 defaultBackgroundColor = {0, 0, 0};

// Teensy 3.0 has the LED on pin 13
const int ledPin = 13;

//#define DEBUG 1
#define True 1
#define False 0

#define NUM_SEEDS 6
#define ITERATION_INTERVAL 250  // milliseconds
enum ColorChoice { Red = 0, Green, Blue, Random, Continuous_Random};
ColorChoice colorChoice = Red;
rgb24 color_new;
rgb24 color_old;

byte state_0[MATRIX_WIDTH][MATRIX_HEIGHT];
byte state_1[MATRIX_WIDTH][MATRIX_HEIGHT];
int current_state = 0;
int static_count = 0;
int current_seed = 0;
int iterations = 0;
int max_iterations = 300;
bool step = False;

// the setup() method runs once, when the sketch starts
void setup() {
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);

  Serial.begin(38400);

  matrix.addLayer(&backgroundLayer); 
  matrix.begin();
  matrix.setBrightness(defaultBrightness);
  backgroundLayer.enableColorCorrection(true);

  // clear screen
  backgroundLayer.fillScreen(defaultBackgroundColor);
  backgroundLayer.swapBuffers();

  reset_state(random(NUM_SEEDS));
}

void reset_state(int seed) {
  clear_state(state_0);
  clear_state(state_1);
  current_state = 0;
  static_count = 0;
  iterations = 0;
  current_seed = seed;
  colorChoice = (ColorChoice)random(5);
  set_colors(colorChoice);
  switch (seed) {
    case 0:
      seed_r_pentamino(state_0);
      max_iterations = 475;
      break;
    case 1:
      seed_acorn(state_0);
      max_iterations = 100;
      break;
    case 2:
      seed_diehard(state_0);
      max_iterations = 200;
      break;
    case 3:
      seed_pattern_10_Living(state_0);
      max_iterations = 300;
      break;
    case 4:
      seed_pattern_5_x_5(state_0);
      max_iterations = 250;
      break;
    case 5:
    default:
      seed_random(state_0);
      max_iterations = 256;
      break;
      
  }
}

// the loop() method runs over and over again,
// as long as the board has power
void loop() {

  while (step and (Serial.available() > 0)) {

    char c = Serial.read();
    if (c == '\n') {
      next();
    }
    if (c == 'r') {
      reset_state(current_seed);
    }
    if (c == 'n') {
      int seed = Serial.parseInt();
      reset_state(seed);
    }
    if (c == 'c') {
      step = False;
      break;
    }
  }

  if (!step) {
    next();
  }

  if (iterations > max_iterations) {
    reset_state(random(NUM_SEEDS));
  }
  
  delay(ITERATION_INTERVAL);  // wait for half a second
}

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
    reset_state(random(NUM_SEEDS));
  }

  iterations += 1;

#ifdef DEBUG
  Serial.print("static_count = ");
  Serial.println(static_count);
#endif
}

bool is_static() {
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      if (state_0[x][y] != state_1[x][y]) {
        return False;
      }
    } 
  }
  return True; 
}

void draw_state(byte current[MATRIX_WIDTH][MATRIX_HEIGHT], byte last[MATRIX_WIDTH][MATRIX_HEIGHT]) {
  if (colorChoice == Continuous_Random) {
    set_colors(colorChoice);
  }
  rgb24 *color;

  backgroundLayer.fillScreen(defaultBackgroundColor);

  // Draw state:
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

void set_colors(ColorChoice colorChoice) {
  switch (colorChoice) {
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
    case Random:
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

