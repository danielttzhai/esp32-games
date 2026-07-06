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

// const int VRY_PIN = 0;
const int VRX_PIN = 1;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define screenWidth 240
#define screenHeight 280

#define ballRadius 6
#define wBoard 75
#define hBoard 8
#define yBoard (screenHeight-hBoard)
#define speedBoard 15

// Create a 16-bit color canvas buffer in RAM matching your exact screen size
GFXcanvas16 canvas(screenWidth, screenHeight);


int x, y;
int reward;
int xBoard;
float xspeed, yspeed, speed;

void reset_game() {
  x = 120;
  y = 265;

  reward = 0;

  xBoard = 95;

  xspeed = 5;
  yspeed = -5;
  speed = 7.071;
}


void setup() {
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

  // tft.setRotation(2);
  reset_game();
}


void loop() {
  // 1. CLEAR THE RAM CANVAS
  canvas.fillScreen(ST77XX_BLACK);
  canvas.drawRect(0, 0, screenWidth, screenHeight, ST77XX_CYAN);

  canvas.drawCircle(x, y, ballRadius, ST77XX_GREEN);
  canvas.drawRect(xBoard, yBoard, wBoard, hBoard, ST77XX_CYAN);

  canvas.setCursor(20,10);
  canvas.setTextSize(2);
  canvas.setTextColor(ST77XX_BLUE);
  canvas.print("reward:");
  canvas.print(reward);

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), screenWidth, screenHeight);

  int xvalue = analogRead(VRX_PIN);  // 0-4096

  if ((xvalue>3000) && (xBoard < screenWidth - wBoard)) {
    xBoard = xBoard + speedBoard;
  }

  if ((xvalue<1000) && (xBoard > 0)) {
    xBoard = xBoard - speedBoard;
  }

  x = x + round(xspeed);
  y = y + round(yspeed);

  // bounce on left/right wall
  if((x>=screenWidth - ballRadius )||(x <= ballRadius)) {
    //tone(buzzerPin, 4000, 5); 
    xspeed = -xspeed;
  }

  // bound on ceiling
  if(y<= ballRadius) {
    //tone(buzzerPin, 4000, 5); 
    yspeed = -yspeed;
  }


  // bound on board
  if ((x>=xBoard) && (x<=xBoard+wBoard) && (y + ballRadius >= screenHeight - hBoard)) {
    //tone(buzzerPin, 2000, 5); 

    int xRel = x - xBoard;
    if (xRel < xBoard / 3) {
      xspeed = xspeed - 2;  // hit left 1/3
    } else if (xRel < 2 * xBoard / 3) {  // hit middle

    } else {  // hit right
      xspeed = xspeed + 2;
    }
  
    if (xspeed > speed - 0.4) {
      xspeed = speed - 0.4;
    } else if (xspeed < - speed + 0.4) {
      xspeed = -speed + 0.4;
    }
    yspeed = -sqrt(speed*speed - xspeed*xspeed);  
    reward++;
    if (reward%10 == 0) {
      speed = speed * 1.2;
    }
  }

  if (y>screenHeight - hBoard) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(30,120);
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_RED);
    tft.println("GAME OVER");

    delay(3000);
  
    reset_game();
    
  }
}
