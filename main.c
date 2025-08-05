#include "raylib.h"
#define SCREEN_WIDTH 1020
#define SCREEN_HEIGHT 700

int main(void) {
  Rectangle random_rec = {510, 350, 30, 30};
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pac Xon");

  SetTargetFPS(60);

  while(!WindowShouldClose()){

    BeginDrawing();
    DrawRectangleRec(random_rec, WHITE);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
