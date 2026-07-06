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
#define cellSize 6
#define rangeX 40
#define rangeY 40
#define totalCell 1600

int radius = cellSize;

#define JOYSTICK_SW 10
const int VRX_PIN = 1;
const int VRY_PIN = 0;

int snake_len;
int sx[totalCell];
int sy[totalCell];

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Create a 16-bit color canvas buffer in RAM matching your exact screen size
GFXcanvas16 canvas(screenWidth, screenHeight);


int px, py;
int vx, vy;

int fx, fy;
int f_place_time;

bool is_play;
int current_time;
int reward;

long lastPressTime = 0;

bool snake_is_alive;

void place_fruit() {
  fx = random(4, rangeX - 4);
  fy = random(4, rangeY - 4);
  for(int i = 0;i<snake_len; i++) {
    if ((sx[i] == fx) && (sy[i] == fy)) {
      place_fruit();
    }
  }
  f_place_time = current_time;
}

void init_game() {
  px = rangeX / 5;
  py = rangeY / 2;
  vx = 1;  // moving right
  vy = 0;
  snake_len = 1;
  sx[0] = px;
  sy[0] = py;

  current_time = 0;
  reward = 0;
  is_play = false;

  place_fruit();

  snake_is_alive = true;

  lastPressTime = 0;
}

void handleButtonPress() {
  if (millis() - lastPressTime > 200) {
    is_play = !is_play;
    lastPressTime = millis();
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(JOYSTICK_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(JOYSTICK_SW), handleButtonPress, FALLING);

  SPI.end(); 
  // 1. OVERCLOCK THE HARDWARE BUS: 
  // We change the final parameter to 'false' or leave it to allow 
  // dedicated high-speed DMA/Hardware channels to link pins 4 and 6.
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, TFT_CS); 

  tft.init(screenWidth, screenHeight); 
  
  // 2. MAX OUT THE SPI BUS FREQUENCY:
  // The ESP32-C3 can natively push data at 40MHz or even 80MHz over short wires. 
  // Bumping this from 24MHz to 40MHz cuts the image transmission time nearly in half!
  tft.setSPISpeed(20000000);
  
  tft.fillScreen(ST77XX_BLACK);

  init_game();

  tft.setRotation(2);

}

void draw_snake() {
  for(int i=0; i< snake_len; i++) {
    canvas.fillCircle(sx[i] * cellSize + minX + cellSize/2, sy[i] * cellSize + minY  + cellSize/2, radius, ST77XX_GREEN);
  }
}

void loop() {
  if (snake_is_alive == false) {
    canvas.setCursor(40, 200);
    canvas.setTextSize(3);
    canvas.setTextColor(ST77XX_RED);
    canvas.print("GAME OVER");
    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), screenWidth, screenHeight);

    delay(3000);
    init_game();
  }

  // put your main code here, to run repeatedly:
  canvas.fillScreen(ST77XX_BLACK);
  canvas.drawRoundRect(minX, minY, maxX - minX, maxY - minY, 16, ST77XX_BLUE);

  canvas.fillCircle(fx*cellSize + minX + cellSize/2, fy*cellSize + minY  + cellSize/2, radius, ST77XX_RED);

  draw_snake();

  canvas.setTextSize(2);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setCursor(30, 2);
  canvas.print(current_time/5);

  canvas.setCursor(190, 2);
  canvas.print(reward);

  if (is_play) {
    canvas.fillTriangle(127, 10, 115, 3, 115, 17, ST77XX_WHITE);
  } else {
    canvas.fillRect(115, 5, 10, 10, ST77XX_WHITE);
  }

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), screenWidth, screenHeight);

  if (is_play) {
    // game control code
    get_direction();
    update_snake();
    current_time ++;
  }

}

void get_direction() {

  int vrx = analogRead(VRX_PIN);
  int vry = analogRead(VRY_PIN);

  if ((vrx<1000) && (vx==0)) {vx = -1; vy = 0;}
  if ((vrx>3000) && (vx==0)) {vx = 1; vy = 0;}
  if ((vry<1000) && (vy==0)) {vx = 0; vy = -1;}
  if ((vry>3000) && (vy==0)) {vx = 0; vy = 1;}
}

void update_snake() {
  px = px + vx;
  py = py + vy;

  check_snake_survival();

  bool eaten = check_fruit_eaten();

  if (eaten) snake_len ++;

  for(int i = snake_len - 1; i >= 1; i--) {
    sx[i] = sx[i-1];
    sy[i] = sy[i-1];
  }

  sx[0] = px;
  sy[0] = py;
}


void check_snake_survival() {
  if((px>= rangeX)||(py>= rangeY)||(px<0)||(py<0)) {
    snake_is_alive = false;
    return;
  }
  for(int i = 0; i<snake_len; i++) {
    if((sx[i]==px) && (sy[i]==py)) {
      snake_is_alive = false;
      return;
    }
  }
}

bool check_fruit_eaten() {

  if((px - fx) * (px - fx) + (py - fy)*(py - fy) <= 2.0 * radius * radius / cellSize / cellSize) {
    reward = reward + 1;
    place_fruit();
    return true;
  }

  if(current_time - f_place_time > 120) {
    reward = reward - 1;
    place_fruit();
  }
  return false;
}





