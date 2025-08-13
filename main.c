
#include <stdio.h>
#include <stdbool.h>
#include "raylib.h"
#include "raymath.h"
#define SCREEN_WIDTH 1020
#define SCREEN_HEIGHT 730
#define GRID_ROWS 23
#define GRID_COLS 34
#define CELL_SIZE 30
#define MAX_BALLS 5

typedef struct {
  Vector2 pos;
  Vector2 vel;
  float rad;
  float mass;
  Color color;

  //extra for the player
  int cell_x, cell_y;
  int target_x, target_y;
  bool moving;
  float speed;
} Ball;

typedef struct {
  int lives;
  int score;
  int level;
  float percentage_filled;
  bool game_over;
  bool level_complete;
  bool on_border;
  Vector2 trail_start;
} GameState;

GameState game_state;

//WALLS
typedef enum {
  WALL_BLANK,
  WALL_BUILDING,
  WALL_SOLID,
  WALL_VISITED
} WallState;


WallState grid[GRID_ROWS][GRID_COLS];


// ** FUNCTIONS ** //
//ball
Ball ball_init(Vector2 pos, Vector2 vel, float rad, float mass, Color color);
Ball ball_update(Ball ball, float dt, WallState grid[GRID_ROWS][GRID_COLS], Ball *player);
Vector2 get_attraction_force(Ball a, Ball b, float G);
void apply_force(Ball *ball, Vector2 force, float dt);
//grid
void draw_grid(Ball ball);
void init_grid();
//player
void player_init(Ball *player, int start_cell_x, int start_cell_y, float speed,float rad, Color color);
void player_move(Ball *player, int dx, int dy);
void player_update(Ball *player, float dt, Ball balls[], int num_balls);
static inline bool is_solid_cell(int row, int col);
//player mech
void complete_area(Ball balls[], int num_balls);
void reset_player_to_border(Ball *player);
bool is_on_border(int x, int y);

void flood_fill(WallState temp_grid[GRID_ROWS][GRID_COLS],int row, int col, Ball balls[], int num_balls);
//game mech
void init_game_state();
void update_percentage();
void draw_ui();

void reset_level(Ball *player, Ball balls[], int num_balls);
void next_level(Ball *player, Ball balls[], int num_balls);

bool check_collision_with_balls(Ball *player, Ball balls[], int num_balls);
bool player_on_border = true;


int main(void) {
  //float G = 1000.0f;
  Ball balls[MAX_BALLS];
  int num_balls = 1;
  balls[0] = ball_init((Vector2){200.0f, 300.0f}, (Vector2){4.0f, 5.0f}, 20, 20, RED);

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pac Xon");
  init_grid();
  init_game_state();

  Ball player;
  player_init(&player, 0, 0, 220.0f, CELL_SIZE * 0.4f, DARKBLUE);

  SetTargetFPS(60);

  while(!WindowShouldClose()){
    float dt = GetFrameTime();

    if (game_state.game_over) {
      if (IsKeyPressed(KEY_R)) {
	init_game_state();
	reset_level(&player, balls, num_balls);
	num_balls = 1;
      }
    } else if (game_state.level_complete) {
      if (IsKeyPressed(KEY_SPACE)){
	next_level(&player, balls, num_balls);
	if (game_state.level % 3 == 0 && num_balls < MAX_BALLS) {
	  num_balls++;
	}
      }
    } else {
      if (!player.moving)
	{
	  if (IsKeyDown(KEY_RIGHT)) player_move(&player, +1,  0);
	  if (IsKeyDown(KEY_LEFT))  player_move(&player, -1,  0);
	  if (IsKeyDown(KEY_UP))    player_move(&player,  0, -1);
	  if (IsKeyDown(KEY_DOWN))  player_move(&player,  0, +1);
	}

      player_update(&player, dt, balls, num_balls);
      
      if (!player_on_border) {
	if (check_collision_with_balls(&player, balls, 1)) {
	  game_state.lives--;
	  if (game_state.lives <= 0) {
	    game_state.game_over = true;
	  } else {
	    reset_player_to_border(&player);
	  }
	}
      }
      for (int i = 0; i < num_balls; i++) {
	balls[i] = ball_update(balls[i], dt, grid, &player);
      }
      
      update_percentage();

      if (game_state.percentage_filled >= 80.0f && !game_state.level_complete) {
	game_state.level_complete = true;
	game_state.score += (int)(game_state.percentage_filled * 10) + game_state.level * 100;
      }
    }
    
    BeginDrawing();
    ClearBackground(BLACK);

    draw_grid(player);

    for (int i = 0; i < num_balls; i++){
    DrawCircleV(balls[i].pos, balls[i].rad, balls[i].color);
    }
    
    DrawCircleV(player.pos, player.rad, player.color);
    
    draw_ui();

    if (game_state.game_over) {
      DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 200});
      DrawText("GAME OVER (~_~)", SCREEN_WIDTH/2 - 175, (SCREEN_HEIGHT - 40)/2, 40, RED);
      DrawText("Press R to restart", SCREEN_WIDTH/2 - 80,( SCREEN_HEIGHT - 40)/2 + 50, 20, WHITE);
    }

    if (game_state.level_complete) {
      DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 200});
      DrawText("LEVEL COMPLETE!", SCREEN_WIDTH/2 - 150, (SCREEN_HEIGHT - 40)/2, 40, GREEN);
      DrawText("Press SPACE for next level", SCREEN_WIDTH/2 - 100, (SCREEN_HEIGHT - 40)/2 + 40, 20, WHITE);
    }
    
    EndDrawing();
  }

  CloseWindow();

  return 0;
}


// # 1
Ball ball_init(Vector2 pos, Vector2 vel, float rad, float mass, Color color) {
  return (Ball){pos, vel, rad, mass, color, false};
}

// # 2
Ball ball_update(Ball ball, float dt, WallState grid[GRID_ROWS][GRID_COLS], Ball *player ) {

  Vector2 next_pos = {
    ball.pos.x + ball.vel.x,
    ball.pos.y + ball.vel.y
  };

  
  int next_cell_x = next_pos.x / CELL_SIZE;
  int next_cell_y = next_pos.y / CELL_SIZE;

  int cell_x = ball.pos.x / CELL_SIZE;
  int cell_y = ball.pos.y / CELL_SIZE;

  WallState current = grid[cell_y][cell_x];


  if(grid[cell_y][next_cell_x] == WALL_SOLID)
    {
      ball.vel.x = -ball.vel.x;
    }
  if(grid[next_cell_y][cell_x] == WALL_SOLID)
    {
      ball.vel.y = -ball.vel.y;
    }

  
  if(grid[next_cell_y][cell_x] == WALL_BUILDING || grid[next_cell_y][cell_x] == WALL_BUILDING ) {
    game_state.lives--;
    if (game_state.lives <= 0) {
      game_state.game_over = true;
    } else {
      reset_player_to_border(player);
    }
  }
  
  ball.pos.x += ball.vel.x ;
  ball.pos.y += ball.vel.y ;
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
void draw_grid(Ball ball) {
  
  for(int row = 0; row < GRID_ROWS; row++) {
    for(int col = 0; col < GRID_COLS; col++) {
      WallState state = grid[row][col];
      Color c ;

      switch(state) {
      case WALL_BLANK: c = BLACK; break;
      case WALL_BUILDING: c = YELLOW; break;
      case WALL_SOLID: c = GRAY; break;
      }

      DrawRectangle(col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE, c);
      DrawRectangleLines(col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE, DARKGRAY);
    }
  }
    
}

// # 6
void init_grid() {

  for(int row = 0; row < GRID_ROWS; row++) {
    for(int col = 0; col < GRID_COLS; col++) {
      if(row == 0 || row == GRID_ROWS - 1 || col == 0 || col == GRID_COLS - 1)
	grid[row][col] = WALL_SOLID;
      else
	grid[row][col] = WALL_BLANK;
    }
  }
}

// # 7

void player_init(Ball *player, int start_cell_x, int start_cell_y, float speed,
                 float rad, Color color)
{
  player->cell_x = start_cell_x;
  player->cell_y = start_cell_y;
  player->target_x = start_cell_x;
  player->target_y = start_cell_y;
  player->pos = (Vector2){start_cell_x * CELL_SIZE + CELL_SIZE * 0.5f,
			  start_cell_y * CELL_SIZE + CELL_SIZE * 0.5f};
  player->vel = (Vector2){0, 0};
  player->rad = rad;
  player->mass = 100.0f;
  player->color = color;
  player->moving = false;
  player->speed = speed;
}

// # 8
void player_move(Ball *player, int dx, int dy) {
  if (player->moving) return;
  int nx = player->cell_x + dx;
  int ny = player->cell_y + dy;

  if (nx < 0 || nx >= GRID_COLS || ny < 0 || ny >= GRID_ROWS) return;

  player->target_x = nx;
  player->target_y = ny;
  player->moving = true;
}


// # 9
void player_update(Ball *player, float dt, Ball balls[], int num_balls)
{
  if (!player->moving) return;

  Vector2 target_pos = (Vector2) {
    player->target_x * CELL_SIZE + CELL_SIZE * 0.5f,
    player->target_y * CELL_SIZE + CELL_SIZE * 0.5f
  };

  Vector2 dir = Vector2Subtract(target_pos, player->pos);
  float dist = Vector2Length(dir);

  if (dist <= player->speed * dt) {
    player->pos = target_pos;
    player->cell_x = player->target_x;
    player->cell_y = player->target_y;
    player->moving = false;
    
    bool was_on_border = player_on_border;
    bool now_on_border = is_on_border(player->cell_x, player->cell_y);

    player_on_border = now_on_border;

    // if moving from border to non border
    if (was_on_border && !now_on_border) {
      game_state.trail_start = (Vector2){player->cell_x, player->cell_y};
      grid[player->cell_y][player->cell_x] = WALL_BUILDING;
    }
    //if building and reach border 
    else if (!was_on_border && now_on_border){
      complete_area(balls, num_balls);
    }
    //if building continue trail
    else if (!was_on_border && !now_on_border) {
      grid[player->cell_y][player->cell_x] = WALL_BUILDING;
    }


  } else {
    Vector2 step = Vector2Scale(Vector2Normalize(dir), player->speed * dt);
    player->pos = Vector2Add(player->pos, step);
  }
}

// # 10
bool is_on_border(int x, int y) {
  return (x == 0 || x == GRID_COLS - 1 || y == 0 || y == GRID_ROWS - 1) || grid[y][x] == WALL_SOLID;
}

// # 11
void complete_area(Ball balls[], int num_balls) {
  //create a temp copy of the grid
  WallState temp_grid[GRID_ROWS][GRID_COLS];
  for (int i = 0; i< GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      temp_grid[i][j] = grid[i][j];
    }
  }
  //convert building to wall in temp  grid
  for (int i = 0; i< GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      if (temp_grid[i][j] == WALL_BUILDING) {
	temp_grid[i][j] = WALL_SOLID;
      }
    }
  }

  for (int b = 0; b < num_balls; b++) {
    int cx = balls[b].pos.x / CELL_SIZE;
    int cy = balls[b].pos.y / CELL_SIZE;

    flood_fill(temp_grid, cy, cx, balls, num_balls);
  } 

  for (int i = 0; i < GRID_ROWS; i++) {
    if (temp_grid[i][0] == WALL_BLANK) {
      flood_fill(temp_grid, i, 0, balls, 1);
    }
    if (temp_grid[i][GRID_COLS - 1] == WALL_BLANK) {
      flood_fill(temp_grid, i, GRID_COLS - 1, balls, 1);
    }
  }

  for (int j = 0; j < GRID_COLS; j++) {
    if (temp_grid[0][j] == WALL_BLANK) {
      flood_fill(temp_grid, 0, j, balls, 1);
    }
    if (temp_grid[GRID_ROWS - 1][j] == WALL_BLANK){
      flood_fill(temp_grid, GRID_ROWS - 1, j, balls, 1);
    }
  }
  //copy the result back
  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      if (grid[i][j] == WALL_BUILDING) {
	grid[i][j] = WALL_SOLID;
      } else if (grid[i][j] == WALL_BLANK && temp_grid[i][j] == WALL_BLANK){
	grid[i][j] = WALL_SOLID;
	game_state.score += 10;
      }
    }
  }
}

// # 12
void init_game_state() {
  game_state.lives = 3;
  game_state.score = 0;
  game_state.level = 1;
  game_state.percentage_filled = 0.0f;
  game_state.level_complete = false;
  game_state.game_over = false;
  game_state.trail_start = (Vector2){0, 0};
}

// # 13
bool check_collision_with_balls(Ball *player, Ball balls[], int num_balls) {
  for (int i = 0; i < num_balls; i++) {
    float dist = Vector2Distance(player->pos, balls[i].pos);
    if (dist < player->rad + balls[i].rad) {
      return true;
    }
  }
  return false;
}

// # 14

void reset_player_to_border(Ball *player)
{
  for (int i = 0; i < GRID_ROWS; i++)
    {
      for(int j = 0; j < GRID_COLS; j++)
	{
	  if(grid[i][j] == WALL_BUILDING) grid[i][j] = WALL_BLANK;
	}
    }

  player_init(player, 1, 1, player->speed, player->rad, player->color);
  player_on_border = true;
}

// # 15
void draw_ui() {
  char text[100];
  sprintf(text, "lives: %d", game_state.lives);
  DrawText(text, 10, SCREEN_HEIGHT - 35, 20, WHITE);

  sprintf(text, "score: %d", game_state.score);
  DrawText(text, 40 + 3 * CELL_SIZE,  SCREEN_HEIGHT - 35, 20, WHITE);

  sprintf(text,"Filled: %.1f%%", game_state.percentage_filled);
  DrawText(text, 70 + 9 * CELL_SIZE ,  SCREEN_HEIGHT - 35, 20, WHITE);

  sprintf(text, "Level: %d", game_state.level);
  DrawText(text, 100 + 14 * CELL_SIZE,  SCREEN_HEIGHT - 35, 20, WHITE);

  sprintf(text, "Target: 80%%");
  DrawText(text, 130 + 17 * CELL_SIZE,  SCREEN_HEIGHT - 35, 20, WHITE);
}

// # 16
void update_percentage() {
  int total_cells = 0;
  int filled_cells = 0;

  for (int i = 1; i < GRID_ROWS - 1; i++) {
    for (int j = 1; j < GRID_COLS - 1; j++){
      total_cells++;
      if (grid[i][j] == WALL_SOLID) {
	filled_cells++;
      }
    }
  }

  game_state.percentage_filled = (float)filled_cells / total_cells * 100.0f;
}

// # 17
void reset_level(Ball *player, Ball balls[], int num_balls) {
  init_grid();
  player_init(player, 0, 0, player->speed, player->rad, player->color);
  balls[0] = ball_init((Vector2){200.0f, 300.0f}, (Vector2){4.0f, 5.0f}, 15, 20, RED);
  if (num_balls > 1) {
    balls[1] = ball_init((Vector2){600.0f, 400.0f}, (Vector2){-3.0f, 4.0f}, 15, 20, ORANGE);
  }
  if (num_balls > 2) {
    balls[2] = ball_init((Vector2){400.0f, 200.0f}, (Vector2){5.0f, -3.0f}, 15, 20, PINK);
  }
}

// # 18
void next_level(Ball *player, Ball balls[], int num_balls) {
  game_state.level++;
  game_state.level_complete = false;

  reset_level(player, balls, num_balls);

  for (int i = 0; i < num_balls; i++) {
    balls[i].vel.x *= 1.1f;
    balls[i].vel.y *= 1.1f;
  }
}

// # 19
void flood_fill(WallState temp_grid[GRID_ROWS][GRID_COLS], int row, int col, Ball balls[], int num_balls) {
  if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) return;
  if (temp_grid[row][col] != WALL_BLANK) return;

  temp_grid[row][col] = WALL_VISITED;

  flood_fill(temp_grid, row + 1, col, balls, num_balls);
  flood_fill(temp_grid, row - 1, col, balls, num_balls);
  flood_fill(temp_grid, row , col + 1, balls, num_balls);
  flood_fill(temp_grid, row , col - 1, balls, num_balls);
}
