#include <Adafruit_GFX.h>    
#include <Adafruit_ST7789.h> 
#include <SPI.h>

// Your working hardware pin setup
#define TFT_CS         7
#define TFT_DC         2
#define TFT_RST        3
#define SPI_MOSI       6
#define SPI_SCK        4
#define SPI_MISO      -1 

#define screenWidth 240
#define screenHeight 280

#define minX 0
#define maxX 240
#define minY 20
#define maxY 260
#define cellSize 8
#define rangeX 30
#define rangeY 30


//in cpp, we need to explicitly specify the size of the static array. "Static" means its size will not change
#define JOYSTICK_SW 10
const int VRX_PIN = 0;
const int VRY_PIN = 1;
const int PLAY_BUTTON = 9;

int cursor_x;  // store index coordinate, ranging from 0 to rangeX-1
int cursor_y;   // store index coordinate, ranging from 0 to rangeY-1
bool is_play;

int gen; 

int current[rangeX][rangeY]; 
int next[rangeX][rangeY];

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Create a 16-bit color canvas buffer in RAM matching your exact screen size
GFXcanvas16 canvas(screenWidth, screenHeight);

void handlePlayButtonPress() {
  is_play = !is_play;
}

void init_game() {
  for(int i=0; i<rangeX; i++) {
    for(int j=0; j<rangeY; j++) {
      current[i][j] = 0;
      next[i][j] = 0;
    }
  }
  cursor_x = rangeX / 2;
  cursor_y = rangeY / 2;
  is_play = false;
  gen = 0;
}

void setup() {
  Serial.begin(9600);
  pinMode(JOYSTICK_SW, INPUT_PULLUP);
  pinMode(PLAY_BUTTON, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PLAY_BUTTON), handlePlayButtonPress, FALLING);

  SPI.end(); 
  // 1. OVERCLOCK THE HARDWARE BUS: 
  // We change the final parameter to 'false' or leave it to allow 
  // dedicated high-speed DMA/Hardware channels to link pins 4 and 6.
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, TFT_CS); 

  tft.init(screenWidth, screenHeight); 
  
  // 2. MAX OUT THE SPI BUS FREQUENCY:
  // The ESP32-C3 can natively push data at 40MHz or even 80MHz over short wires. 
  // Bumping this from 24MHz to 40MHz cuts the image transmission time nearly in half!
  tft.setSPISpeed(40000000);
  
  tft.fillScreen(ST77XX_BLACK);

  init_game();
  tft.setRotation(2);
}


void loop() {

  // update control
  update_cursor();
  place_life();

  if (is_play) {
    // compute next generation
    evolve();
  }

  // drawing the canvas
  // clear canvas
  canvas.fillScreen(ST77XX_BLACK);
  draw_life();
  draw_cursor(cursor_x, cursor_y);
  display_info();

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), screenWidth, screenHeight);
  delay(50);

}

void display_info() {
  canvas.setTextSize(2);
  canvas.setTextColor(ST77XX_CYAN);
  canvas.setCursor(40, 2);
  canvas.print(gen);
  canvas.setCursor(140, 2);
  if (is_play) {
    canvas.print("Play");
  } else {
    canvas.print("Paused");
  }
}

void update_cursor() {

  int xvalue = analogRead(VRX_PIN);  // 0-4096
  int yvalue = analogRead(VRY_PIN);

  if ((xvalue<1000) && (cursor_x < rangeX - 1)) {
    cursor_x = cursor_x+1;
  }

  if ((xvalue>3000) && (cursor_x > 0)) {
    cursor_x = cursor_x-1;
  }

  if ((yvalue<1000) && (cursor_y > 0)) {
    cursor_y = cursor_y-1;
  }

  if ((yvalue>3000) && (cursor_y < rangeY - 1)) {
    cursor_y = cursor_y+1;
  }
}


void draw_cursor(int i, int j) {
  int x = i*cellSize + minX, y=j*cellSize + minY;
  canvas.drawRect(x, y, cellSize, cellSize, ST77XX_BLUE);
}

void place_life() {
  int sw = digitalRead(JOYSTICK_SW);

  if(sw == 0) {
    current[cursor_x][cursor_y] = 1 - current[cursor_x][cursor_y];
  }
}

void draw_life() {
  for(int a = 0; a < rangeX; a++ ) {
    for(int b = 0; b < rangeY; b++) {
      if (current[a][b] == 1) {
        canvas.fillRect(a*cellSize+minX, b*cellSize+minY, cellSize, cellSize, ST77XX_WHITE);
      }
    }
  }
}

// define a dedicated function to compute neighbors
int count_living_neighbors(int grid[][rangeY], int i, int j) {
  int count = 0;
  for(int u=i-1;u<=i+1;u++) {  // we iterate u through i-1, i, i+1
    for(int v=j-1;v<=j+1;v++) {  // we iterate v through j-1, j, j+1
      // now we have index-coordinate as (u, v) being one of the 3x3 cells
      //if ((u>=0) && (v>=0) && (u<rangeX) && (v<rangeY)) {
      //  count = count + grid[u][v];
      //}
      int safe_u = (u + rangeX) % rangeX;  // u mod rangeX
      int safe_v = (v + rangeY) % rangeY;  // v mod rangeY
      count = count + grid[safe_u][safe_v];
    }
  }

  count = count - grid[i][j];

  return count;
}

void evolve() {
  int i, j;
  for(i=0;i<rangeX;i++){  // looping i=0, 1, ..., rangeX-1
    for(j=0;j<rangeY;j++) {  // looping j=0, 1, ..., rangeY-1
      int num_neighbors = count_living_neighbors(current, i, j);
        if (current[i][j] == 1) {  
          // if there is a life at (i, j) right now
          // check num_neighbors and decide to keep the life survive or not
          if ((num_neighbors == 2)||(num_neighbors == 3)){
            next[i][j] = 1;
          } else {
            next[i][j] = 0;
          }
      } else {
          if ((num_neighbors == 3)){
            next[i][j] = 1; // a life is born!
          } else {
            next[i][j] = 0;
          }
      }
    }
  }

  // copy the next generation status to the current generation
  for(i=0;i<rangeX;i++){  // looping i=0, 1, ..., rangeX-1
    for(j=0;j<rangeY;j++) {  // looping j=0, 1, ..., rangeY-1
      current[i][j] = next[i][j];
    }
  }

  gen ++;
}












