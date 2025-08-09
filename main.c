
#include "raylib.h"
#include "raymath.h"
#define SCREEN_WIDTH 1020
#define SCREEN_HEIGHT 700
#define GRID_ROWS 23
#define GRID_COLS 34
#define CELL_SIZE 30

typedef struct {
  Vector2 pos;
  Vector2 vel;
  float rad;
  float mass;
  Color color;
} Ball;

//WALLS
typedef enum {
  WALL_BLANK,
  WALL_BUILDING,
  WALL_SOLID
} WallState;

// grid
WallState grid[GRID_ROWS][GRID_COLS];

//functions
Ball ball_init(Vector2 pos, Vector2 vel, float rad, float mass, Color color);
Ball ball_update(Ball ball, float dt, WallState grid[GRID_ROWS][GRID_COLS]);
Vector2 get_attraction_force(Ball a, Ball b, float G);
void apply_force(Ball *ball, Vector2 force, float dt);
void draw_grid(Ball ball);
void init_grid();

int main(void) {
  float G = 1000.0f;
  Ball ball = ball_init(
			(Vector2){44.0f, 55.0f},
			(Vector2){4.0f, 5.0f},
			20,
			20,
			GREEN
			);

  //Ball attractor = ball_init((Vector2){(SCREEN_WIDTH / 2.0f), (SCREEN_HEIGHT / 2.0f)},(Vector2){0, 0}, 40, 100000, RED); 
  
  Rectangle random_rec = {510, 350, 30, 30};
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pac Xon");
  init_grid();

  SetTargetFPS(60);

  while(!WindowShouldClose()){
    float dt = GetFrameTime();
    //Vector2 force = get_attraction_force(ball, attractor, G);
    //apply_force(&ball, force, dt);


    ball = ball_update(ball, dt, grid);
    //attractor = ball_update(attractor, dt);
    BeginDrawing();
    ClearBackground(BLACK);
    draw_grid(ball);
    DrawCircle(ball.pos.x, ball.pos.y, ball.rad, ball.color);
    //DrawCircle(attractor.pos.x, attractor.pos.y, attractor.rad, attractor.color);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}


// # 1
Ball ball_init(Vector2 pos, Vector2 vel, float rad, float mass, Color color) {
  return (Ball){pos, vel, rad, mass, color};
}

// # 2
Ball ball_update(Ball ball, float dt, WallState grid[GRID_ROWS][GRID_COLS]) {
  
  int cell_x = ball.pos.x / CELL_SIZE;
  int cell_y = ball.pos.y / CELL_SIZE;
  WallState current = grid[cell_y][cell_x];


  ball.pos.x += ball.vel.x ;//* dt;
  ball.pos.y += ball.vel.y ;//* dt;

  if(current)
    {
      
    }


  if(ball.pos.x >= (SCREEN_WIDTH - ball.rad) || ball.pos.x <= (0 + ball.rad))
    {
      ball.vel.x = -ball.vel.x;
    }
  if(ball.pos.y >= (SCREEN_HEIGHT - ball.rad) || ball.pos.y <= (0 + ball.rad))
    {
      ball.vel.y = -ball.vel.y;
    }
  return ball ;
}

// # 3
Vector2 get_attraction_force(Ball a, Ball b, float G)
{
  Vector2 direction = Vector2Subtract(b.pos, a.pos);
  float distance = Vector2Length(direction);
  float mag = G * (a.mass * b.mass)/(distance * distance);
  return Vector2Scale(Vector2Normalize(direction), mag);
}

// # 4
void apply_force(Ball *ball, Vector2 force, float dt)
{
  Vector2 acc = Vector2Scale(force, 1.0f / ball->mass);
  ball->vel = Vector2Add(ball->vel, Vector2Scale(acc, dt));
}

// # 5
void draw_grid(Ball ball)
{
  Vector2 next_pos = {
    ball.pos.x + ball.vel.x,
    ball.pos.y + ball.vel.y
  };

  int next_cell_x = next_pos.x / CELL_SIZE;
  int next_cell_y = next_pos.y / CELL_SIZE;
  
  for(int row = 0; row < GRID_ROWS; row++)
    {
      for(int col = 0; col < GRID_COLS; col++)
	{
	  WallState state = grid[row][col];
	  Color c ;

	  switch(state)
	    {
	    case WALL_BLANK: c = BLACK; break;
	    case WALL_BUILDING: c = YELLOW; break;
	    case WALL_SOLID: c = GRAY; break;
	    }

	  if(row == next_cell_y && col == next_cell_x){
	    c = PINK;
	  }

	  DrawRectangle(col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE, c);
	}
    }
    
}

// # 6
void init_grid() {
  
  for(int row = 0; row < GRID_ROWS; row++)
    {
      for(int col = 0; col < GRID_COLS; col++)
	{
	  if(row == 0 || row == GRID_ROWS - 1 || col == 0 || col == GRID_COLS - 1)
	    grid[row][col] = WALL_SOLID;
	  else
	    grid[row][col] = WALL_BLANK;
	}
    }
  
}
