
#include "raylib.h"
#define SCREEN_WIDTH 1020
#define SCREEN_HEIGHT 700

typedef struct {
  Vector2 pos;
  Vector2 vel;
  float rad;
  float mass;
  Color color;
} Ball;

int main(void) {

  Ball ball;
  ball.pos.x = SCREEN_WIDTH / 2.0f;
  ball.pos.y = SCREEN_HEIGHT / 2.0f;
  ball.rad = 20;
  ball.vel.x = 4.0f;
  ball.vel.y = 5.0f;
  ball.color = RED;
  
  Rectangle random_rec = {510, 350, 30, 30};
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pac Xon");

  SetTargetFPS(60);

  while(!WindowShouldClose()){

      ball.pos.x += ball.vel.x;
      ball.pos.y += ball.vel.y;

      if(ball.pos.x >= (SCREEN_WIDTH - ball.rad) || ball.pos.x <= (0 + ball.rad)) ball.vel.x = -ball.vel.x;
      if(ball.pos.y >= (SCREEN_HEIGHT - ball.rad) || ball.pos.y <= (0 + ball.rad)) ball.vel.y = -ball.vel.y;

    
    BeginDrawing();
    ClearBackground(BLACK);
    DrawRectangleRec(random_rec, WHITE);
    DrawCircle(ball.pos.x, ball.pos.y, ball.rad, ball.color);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
