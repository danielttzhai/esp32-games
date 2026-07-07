
// =========================================================================
// GAME 1: BREAKOUT
// =========================================================================
namespace Breakout {
  constexpr int ballRadius = 6;
  constexpr int wBoard = 75;
  constexpr int hBoard = 8;
  constexpr int yBoard = (screenHeight-hBoard);
  constexpr int speedBoard = 15;

  int x, y;
  int reward;
  int xBoard;
  float xspeed, yspeed, speed;

  void init_game() {
    x = 120; y = 265;
    reward = 0;
    xBoard = 95;
    xspeed = 5; yspeed = -5;
    speed = 7.071;
  }

  void game_loop() {
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

    int xvalue = analogRead(VRX_PIN);

    if ((xvalue>3000) && (xBoard < screenWidth - wBoard)) xBoard += speedBoard;
    if ((xvalue<1000) && (xBoard > 0)) xBoard -= speedBoard;

    x = x + round(xspeed);
    y = y + round(yspeed);

    // bounce on left/right wall
    if((x>=screenWidth - ballRadius )||(x <= ballRadius)) xspeed = -xspeed;
    // bound on ceiling
    if(y<= ballRadius) yspeed = -yspeed;

    // bound on board
    if ((x>=xBoard) && (x<=xBoard+wBoard) && (y + ballRadius >= screenHeight - hBoard)) {
      int xRel = x - xBoard;
      if (xRel < xBoard / 3) xspeed = xspeed - 2;
      else if (xRel < 2 * xBoard / 3) { }
      else xspeed = xspeed + 2;
 
      if (xspeed > speed - 0.4) xspeed = speed - 0.4;
      else if (xspeed < - speed + 0.4) xspeed = -speed + 0.4;
     
      yspeed = -sqrt(speed*speed - xspeed*xspeed);  
      reward++;
      if (reward%10 == 0) speed = speed * 1.2;
    }

    if (y > screenHeight - hBoard) {
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(30,120);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_RED);
      tft.println("GAME OVER");
      delay(3000);
     
      // RETURN TO MENU
      currentState = MENU;
      sw_pressed = false;
    }
  }
}