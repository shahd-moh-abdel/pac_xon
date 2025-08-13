
#include <stdio.h>
#include <stdbool.h>
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

  //extra for the player
  int cell_x, cell_y;
  int target_x, target_y;
  bool moving;
  float speed;
} Ball;

typedef struct {
  int lives;
  int score;
  bool game_over;
} GameState;
GameState game_state;

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
void player_init(Ball *player, int start_cell_x, int start_cell_y, float speed,
                 float rad, Color color);
void player_move(Ball *player, int dx, int dy);
void player_update(Ball *player, float dt);
bool is_on_border(int x, int y);
void complete_area();
void init_game_state();
bool check_collision_with_balls(Ball *player, Ball balls[], int num_balls);
void reset_player_to_border(Ball *player);
static inline bool is_solid_cell(int row, int col);
bool player_on_border = true;


int main(void) {
  float G = 1000.0f;
  Ball balls[1];
  balls[0] = ball_init((Vector2){200.0f, 300.0f}, (Vector2){4.0f, 5.0f}, 20, 20, RED);

  //Ball attractor = ball_init((Vector2){(SCREEN_WIDTH / 2.0f), (SCREEN_HEIGHT / 2.0f)},(Vector2){0, 0}, 40, 100000, RED); 
  
  Rectangle random_rec = {510, 350, 30, 30};
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pac Xon");
  init_grid();
  init_game_state();

  Ball player;
  player_init(&player, 2, 2, 220.0f, CELL_SIZE * 0.4f, DARKGREEN);

  SetTargetFPS(60);

  while(!WindowShouldClose()){
    float dt = GetFrameTime();
    //Vector2 force = get_attraction_force(ball, attractor, G);
    //apply_force(&ball, force, dt);
    if (!player.moving)
      {
	if (IsKeyPressed(KEY_RIGHT)) player_move(&player, +1,  0);
	if (IsKeyPressed(KEY_LEFT))  player_move(&player, -1,  0);
	if (IsKeyPressed(KEY_UP))    player_move(&player,  0, -1);
	if (IsKeyPressed(KEY_DOWN))  player_move(&player,  0, +1);
      }
    if (!player_on_border) {
      if (check_collision_with_balls(&player, balls, 1)) {
	game_state.lives--;
	printf("lives remaining: %d\n", game_state.lives);
	if (game_state.lives <= 0) {
	  game_state.game_over = true;
	  printf("game over :(\n");
	} else {
	  reset_player_to_border(&player);
	}
	}
    }

    player_update(&player, dt);
    balls[0] = ball_update(balls[0], dt, grid);
    //attractor = ball_update(attractor, dt);
    BeginDrawing();
    ClearBackground(BLACK);
    draw_grid(balls[0]);
    DrawCircleV(balls[0].pos, balls[0].rad, balls[0].color);
    DrawCircleV(player.pos, player.rad, player.color);
    //DrawCircle(attractor.pos.x, attractor.pos.y, attractor.rad, attractor.color);
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
Ball ball_update(Ball ball, float dt, WallState grid[GRID_ROWS][GRID_COLS]) {

  Vector2 next_pos = {
    ball.pos.x + ball.vel.x,
    ball.pos.y + ball.vel.y
  };

  
  int next_cell_x = next_pos.x / CELL_SIZE;
  int next_cell_y = next_pos.y / CELL_SIZE;

  
  int cell_x = ball.pos.x / CELL_SIZE;
  int cell_y = ball.pos.y / CELL_SIZE;
  WallState current = grid[cell_y][cell_x];


  // if(ball.pos.x >= (SCREEN_WIDTH - ball.rad) || ball.pos.x <= (0 + ball.rad))
  if(grid[cell_y][next_cell_x] == WALL_SOLID)
    {
      ball.vel.x = -ball.vel.x;
    }
  //if(ball.pos.y >= (SCREEN_HEIGHT - ball.rad) || ball.pos.y <= (0 + ball.rad))
  if(grid[next_cell_y][cell_x] == WALL_SOLID)
    {
      ball.vel.y = -ball.vel.y;
    }
  
  ball.pos.x += ball.vel.x ;//* dt;
  ball.pos.y += ball.vel.y ;//* dt;
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
void player_update(Ball *player, float dt)
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

    //if (grid[player->cell_y][player->cell_x] == WALL_BLANK) make_tile();
  } else {
    Vector2 step = Vector2Scale(Vector2Normalize(dir), player->speed * dt);
    player->pos = Vector2Add(player->pos, step);
  }
  bool was_on_border = player_on_border;
  bool now_on_border = is_on_border(player->cell_x, player->cell_y);
  player_on_border = now_on_border;

  //debug
  if (was_on_border != now_on_border){
    printf("Player moved %s border\n", now_on_border ? "to" : "frome");
  }
  if (was_on_border && !now_on_border) {
    grid[player->cell_y][player->cell_x] = WALL_BUILDING;
    printf("Started building");
  } else if (!was_on_border && !now_on_border) {
    grid[player->cell_y][player->cell_x] = WALL_BUILDING;
  }
  if (!was_on_border && now_on_border){
    complete_area();
  }
}

// # 10
bool is_on_border(int x, int y) {
  return (x == 0 || x == GRID_COLS - 1 || y == 0 || y == GRID_ROWS - 1) || grid[y][x] == WALL_SOLID;
}

// # 11
void complete_area() {
  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      if (grid[i][j] == WALL_BUILDING) {
	grid[i][j] = WALL_SOLID;
      }
    }
  }
  printf("area completed \n");
}

// # 12
void init_game_state() {
  game_state.lives = 3;
  game_state.score = 0;
  game_state.game_over = false;
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

