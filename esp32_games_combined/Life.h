
// =========================================================================
// GAME 2: GAME OF LIFE
// =========================================================================
namespace Life {
  constexpr int minX = 0;
  constexpr int maxX = 240;
  constexpr int minY = 20;
  constexpr int maxY = 260;
  constexpr int cellSize = 8;
  constexpr int rangeX = 30;
  constexpr int rangeY = 30;

  int cursor_x;  
  int cursor_y;  
  bool is_play;
  int gen;
  int current[rangeX][rangeY];
  int nextArr[rangeX][rangeY]; // Renamed slightly to avoid C++ standard library conflicts

  void init_game() {
    for(int i=0; i<rangeX; i++) {
      for(int j=0; j<rangeY; j++) {
        current[i][j] = 0;
        nextArr[i][j] = 0;
      }
    }
    cursor_x = rangeX / 2;
    cursor_y = rangeY / 2;
    is_play = false;
    gen = 0;
  }

  int count_living_neighbors(int grid[][rangeY], int i, int j) {
    int count = 0;
    for(int u=i-1;u<=i+1;u++) {  
      for(int v=j-1;v<=j+1;v++) {  
        int safe_u = (u + rangeX) % rangeX;  
        int safe_v = (v + rangeY) % rangeY;  
        count = count + grid[safe_u][safe_v];
      }
    }
    return count - grid[i][j];
  }

  void evolve() {
    for(int i=0;i<rangeX;i++){  
      for(int j=0;j<rangeY;j++) {  
        int num_neighbors = count_living_neighbors(current, i, j);
        if (current[i][j] == 1) {  
          if ((num_neighbors == 2)||(num_neighbors == 3)) nextArr[i][j] = 1;
          else nextArr[i][j] = 0;
        } else {
          if ((num_neighbors == 3)) nextArr[i][j] = 1;
          else nextArr[i][j] = 0;
        }
      }
    }
    for(int i=0;i<rangeX;i++){  
      for(int j=0;j<rangeY;j++) {  
        current[i][j] = nextArr[i][j];
      }
    }
    gen ++;
  }

  void game_loop() {
    // Check external play button (Pin 9)
    static long lastPlayBtn = 0;
    if (digitalRead(PLAY_BUTTON) == LOW && millis() - lastPlayBtn > 200) {
      is_play = !is_play;
      lastPlayBtn = millis();
    }

    // Exit mechanic: Double press the joystick button quickly to leave
    if (sw_pressed) {
      sw_pressed = false;
      static long lastSwPress = 0;
      if (millis() - lastSwPress < 500) { // Double click detected
        currentState = MENU;
        sw_pressed = false;
        delay(500);
        return;
      }
      lastSwPress = millis();
     
      // Place life logic
      current[cursor_x][cursor_y] = 1 - current[cursor_x][cursor_y];
    }

    int xvalue = analogRead(VRX_PIN);  
    int yvalue = analogRead(VRY_PIN);

    // Using your Life VRX/VRY mapping
    if ((xvalue>3000) && (cursor_x < rangeX - 1)) cursor_x++;
    if ((xvalue<1000) && (cursor_x > 0)) cursor_x--;
    if ((yvalue<1000) && (cursor_y > 0)) cursor_y--;
    if ((yvalue>3000) && (cursor_y < rangeY - 1)) cursor_y++;

    if (is_play) evolve();

    canvas.fillScreen(ST77XX_BLACK);
   
    // Draw Life
    for(int a = 0; a < rangeX; a++ ) {
      for(int b = 0; b < rangeY; b++) {
        if (current[a][b] == 1) {
          canvas.fillRect(a*cellSize+minX, b*cellSize+minY, cellSize, cellSize, ST77XX_WHITE);
        }
      }
    }
   
    // Draw Cursor
    canvas.drawRect(cursor_x*cellSize + minX, cursor_y*cellSize + minY, cellSize, cellSize, ST77XX_BLUE);

    // Display Info
    canvas.setTextSize(2);
    canvas.setTextColor(ST77XX_CYAN);
    canvas.setCursor(40, 2);
    canvas.print(gen);
    canvas.setCursor(140, 2);
    if (is_play) canvas.print("Play");
    else canvas.print("Paused");

    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), screenWidth, screenHeight);
    delay(50);
  }
}