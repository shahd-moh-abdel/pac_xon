
#include "raylib.h"
#define SCREEN_WIDTH 1020
#define SCREEN_HEIGHT 700

int main(void) {
  Vector2 ball_pos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
  Vector2 ball_speed = {5.0f, 4.0f};
  int ball_rad = 20;
  
  Rectangle random_rec = {510, 350, 30, 30};
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pac Xon");

  SetTargetFPS(60);

  while(!WindowShouldClose()){

      ball_pos.x += ball_speed.x;
      ball_pos.y += ball_speed.y;

      if(ball_pos.x >= (SCREEN_WIDTH - ball_rad) || ball_pos.x <= (0 + ball_rad)) ball_speed.x = -ball_speed.x;
      if(ball_pos.y >= (SCREEN_HEIGHT - ball_rad) || ball_pos.y <= (0 + ball_rad)) ball_speed.y = -ball_speed.y;

    
    BeginDrawing();
    ClearBackground(BLACK);
    DrawRectangleRec(random_rec, WHITE);
    DrawCircle(ball_pos.x, ball_pos.y, ball_rad, RED);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
