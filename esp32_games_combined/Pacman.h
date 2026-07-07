
// =========================================================================
// GAME 3: PACMAN
// =========================================================================
namespace Pacman {
  constexpr int minX = 0;
  constexpr int maxX = 240;
  constexpr int minY = 20;
  constexpr int maxY = 260;
  constexpr int cellSize = 6;
  constexpr int rangeX = 40;
  constexpr int rangeY = 40;
  constexpr int radius = cellSize;

  int px, py, vx, vy;
  int fx, fy, f_place_time;
  bool is_play;
  int current_time, reward;
  bool pac_man_is_alive;

  void place_fruit() {
    fx = random(4, rangeX - 4);
    fy = random(4, rangeY - 4);
    f_place_time = current_time;
  }

  void init_game() {
    px = rangeX / 5; py = rangeY / 2;
    vx = 1; vy = 0;
    current_time = 0; reward = 0;
    is_play = false;
    place_fruit();
    pac_man_is_alive = true;
  }

  void game_loop() {
    if (!pac_man_is_alive) {
      canvas.setCursor(40, 200);
      canvas.setTextSize(3);
      canvas.setTextColor(ST77XX_RED);
      canvas.print("GAME OVER");
      tft.drawRGBBitmap(0, 0, canvas.getBuffer(), screenWidth, screenHeight);
      delay(3000);
      currentState = MENU; // Return to menu
      sw_pressed = false;
      return;
    }

    if (sw_pressed) {
      is_play = !is_play;
      sw_pressed = false;
    }

    canvas.fillScreen(ST77XX_BLACK);
    canvas.drawRoundRect(minX, minY, maxX - minX, maxY - minY, 16, ST77XX_BLUE);
    canvas.fillCircle(fx*cellSize + minX + cellSize/2, fy*cellSize + minY  + cellSize/2, radius, ST77XX_RED);
    canvas.fillCircle(px * cellSize + minX + cellSize/2, py * cellSize + minY  + cellSize/2, radius, ST77XX_YELLOW);

    canvas.setTextSize(2);
    canvas.setTextColor(ST77XX_WHITE);
    canvas.setCursor(30, 2);
    canvas.print(current_time/5);
    canvas.setCursor(190, 2);
    canvas.print(reward);

    if (is_play) canvas.fillTriangle(127, 10, 115, 3, 115, 17, ST77XX_WHITE);
    else canvas.fillRect(115, 5, 10, 10, ST77XX_WHITE);

    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), screenWidth, screenHeight);

    if (is_play) {
      int vrx = analogRead(VRX_PIN);
      int vry = analogRead(VRY_PIN);
     
      // Map to your pacman original movement
      if (vrx<1000) {vx = -1; vy = 0;}
      if (vrx>3000) {vx = 1; vy = 0;}
      if (vry<1000) {vx = 0; vy = -1;}
      if (vry>3000) {vx = 0; vy = 1;}

      px += vx; py += vy;

      if((px>= rangeX)||(py>= rangeY)||(px<0)||(py<0)) pac_man_is_alive = false;

      if((px - fx)*(px - fx) + (py - fy)*(py - fy) <= 2.0 * radius * radius / cellSize / cellSize) {
        reward++; place_fruit();
      }
      if(current_time - f_place_time > 120) {
        reward--; place_fruit();
      }
      current_time ++;
    }
  }
}
