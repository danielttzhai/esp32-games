#include <Adafruit_GFX.h>  
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <math.h>

// --- GLOBAL HARDWARE & PIN SETUP ---
#define TFT_CS         7
#define TFT_DC         2
#define TFT_RST        3
#define SPI_MOSI       6
#define SPI_SCK        4
#define SPI_MISO      -1

// Standardized Joystick Pins
#define VRX_PIN        1
#define VRY_PIN        0
#define JOYSTICK_SW    10
#define PLAY_BUTTON    9  // Specifically used by Game of Life

#define screenWidth 240
#define screenHeight 280

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(screenWidth, screenHeight);

// --- STATE MACHINE SETUP ---
enum GameState { MENU, BREAKOUT, LIFE, PACMAN, SNAKE };
GameState currentState = MENU;

volatile bool sw_pressed = false;
void IRAM_ATTR handleButtonPress() {
  static unsigned long lastPressTime = 0;
  if (millis() - lastPressTime > 200) {
    sw_pressed = true;
    lastPressTime = millis();
  }
}

#include "Breakout.h"
#include "Life.h"
#include "Pacman.h"
#include "Snake.h"

int menuSelection = 0;

void menu_loop() {
  canvas.fillScreen(ST77XX_BLACK);
  canvas.setTextSize(3);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setCursor(15, 30);
  canvas.print("SELECT GAME");

  const char* games[] = {"Breakout", "Life", "Pacman", "Snake"};
 
  for (int i=0; i<4; i++) {
    if (i == menuSelection) {
      canvas.setTextColor(ST77XX_GREEN);
      canvas.setCursor(30, 90 + i*40);
      canvas.print("> ");
      canvas.print(games[i]);
    } else {
      canvas.setTextColor(ST77XX_WHITE);
      canvas.setCursor(65, 90 + i*40);
      canvas.print(games[i]);
    }
  }
  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), screenWidth, screenHeight);

  int yvalue = analogRead(VRY_PIN);
  if (yvalue < 1000) { // Joystick Up
    menuSelection--;
    if (menuSelection < 0) menuSelection = 3;
    delay(200); // Simple debounce
  } else if (yvalue > 3000) { // Joystick Down
    menuSelection++;
    if (menuSelection > 3) menuSelection = 0;
    delay(200); // Simple debounce
  }

  if (sw_pressed) {
    sw_pressed = false;
    // Launch selected game
    if (menuSelection == 0) { Breakout::init_game(); currentState = BREAKOUT; }
    else if (menuSelection == 1) { Life::init_game(); currentState = LIFE; }
    else if (menuSelection == 2) { Pacman::init_game(); currentState = PACMAN; }
    else if (menuSelection == 3) { Snake::init_game(); currentState = SNAKE; }
  }
}


// =========================================================================
// ARDUINO SETUP & LOOP
// =========================================================================
void setup() {
  Serial.begin(9600);
 
  pinMode(JOYSTICK_SW, INPUT_PULLUP);
  pinMode(PLAY_BUTTON, INPUT_PULLUP);
 
  // Attach the single global interrupt
  attachInterrupt(digitalPinToInterrupt(JOYSTICK_SW), handleButtonPress, FALLING);

  SPI.end();
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, TFT_CS);
  tft.init(screenWidth, screenHeight);
  tft.setSPISpeed(40000000);
 
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(2); // Set globally so all games and menu are oriented correctly
}

void loop() {
  switch (currentState) {
    case MENU:      menu_loop(); break;
    case BREAKOUT:  Breakout::game_loop(); break;
    case LIFE:      Life::game_loop(); break;
    case PACMAN:    Pacman::game_loop(); break;
    case SNAKE:     Snake::game_loop(); break;
  }
}
