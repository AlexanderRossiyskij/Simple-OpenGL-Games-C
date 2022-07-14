#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include <stdio.h>

#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define MAP_WIDTH        10
#define MAP_HEIGHT       20
#define CNT_CELLS_TETRAD 4
#define NUM_FIGURES      7

typedef enum {IN_MENU, IN_PROGRESS, WON_GAME, LOST_GAME} GameStatus;
typedef enum {RIGHT = 0, DOWN, LEFT, UP} RotateDirection;
typedef enum {CUBE = 0, LINE, LFIGURE, JFIGURE, SFIGURE, ZFIGURE, TFIGURE} Figure;

GameStatus g_game_status;

typedef struct {
  int x;
  int y;
} Point2f;

typedef struct {
  int red;
  int green;
  int blue;
} Color3f;

typedef struct {
  Point2f         points[CNT_CELLS_TETRAD];
  Color3f         color;
  RotateDirection direction;
  Figure          figure;
} Player;

typedef struct {
  bool    is_block;
  Color3f color;
} GameGrid;

GameGrid g_game_grid[MAP_WIDTH][MAP_HEIGHT];
Player   g_player;

void initGame();
void initPlayer();
void initFigure();
void initCube();
void initLine();
void initLFigure();
void initJFigure();
void initSFigure();
void initZFigure();
void initTFigure();
int  randInt(int);
bool isInGameGridXCollision(int);
bool isInGameGridYCollision(int);
bool moveDownCollision();
bool moveRightCollision();
bool moveLeftCollision();
void timer(int);
void updateGame();
void pushCellDown();
void checkField();
void clearLine(int);
void moveKeyboard(int, int, int);
void rotateKeyboard(unsigned char, int, int);
bool rotatable();
void rotatePlayer();
void rotate();
void rotateLine();
void rotateLFigure();
void rotateJFigure();
void rotateSFigure();
void rotateZFigure();
void rotateTFigure();
void display();
void displayGame();
void displayField();
void displayBlock(Color3f*);
void displayPlayer();

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  initGame();
  glutCreateWindow("Tetris");
  glutInitWindowSize(300, 600);
  glutInitWindowPosition(40, 40);
  glutDisplayFunc(display);
  glutKeyboardFunc(rotateKeyboard);
  glutSpecialFunc(moveKeyboard);
  glutTimerFunc(1000, timer, 0);
  glutMainLoop();

  return 0;
}

void initGame() {
  srand(time(NULL));

  memset(g_game_grid, 0, sizeof(g_game_status));
  initPlayer();

  g_game_status = IN_PROGRESS;
}

void initPlayer() {
  g_player.color = (Color3f) {.red = randInt(255), .green = randInt(255), .blue = randInt(255)};
  g_player.direction = RIGHT;

  static int prev_random = -1;
  static int cnt_prev_random_in_raw = 0;
  int x;
  do {
    x = randInt(NUM_FIGURES - 4);
    if(x == prev_random)
      ++cnt_prev_random_in_raw;
    else {
      prev_random = x;
      cnt_prev_random_in_raw = 1;
    }
  } while (cnt_prev_random_in_raw > 3);
  g_player.figure = 1;
  initFigure();
}

void initFigure() {
  switch (g_player.figure) {
    case CUBE:
      initCube();
      break;
    case LINE:
      initLine();
      break;
    case LFIGURE:
      initLFigure();
      break;
    case JFIGURE:
      initJFigure();
      break;
    case SFIGURE:
      initSFigure();
      break;
    case ZFIGURE:
      initZFigure();
      break;
    case TFIGURE:
      initTFigure();
      break;
  }
}

void initCube() { 
  for(int y = 0, index = 0; y < CNT_CELLS_TETRAD / 2; ++y)
    for(int x = 0; x < CNT_CELLS_TETRAD / 2; ++x, ++index)
      g_player.points[index] = (Point2f) {.x = MAP_WIDTH / 2 - 1 + x, .y = MAP_HEIGHT - y - 1};
}

void initLine() {
  for(int x = 0; x < CNT_CELLS_TETRAD; ++x)
    g_player.points[x] = (Point2f) {.x = MAP_WIDTH / 3 + x, .y = MAP_HEIGHT - 2};
}

void initLFigure() {
  g_player.points[0] = (Point2f) {.x = MAP_WIDTH / 2 - 1, .y = MAP_HEIGHT - 1};
  for(int x = 1; x < CNT_CELLS_TETRAD; ++x)
    g_player.points[x] = (Point2f) {.x = MAP_WIDTH / 2 - 2 + x, .y = MAP_HEIGHT - 2};
}

void initJFigure() {
  for(int x = 0; x < CNT_CELLS_TETRAD - 1; ++x)
    g_player.points[x] = (Point2f) {.x = MAP_WIDTH / 2 - 1 + x, .y = MAP_HEIGHT - 2};
  g_player.points[3] = (Point2f) {.x = MAP_WIDTH / 2 + 1, .y = MAP_HEIGHT - 1};
}

void initSFigure() {
  for(int y = -1, index = 0; y < CNT_CELLS_TETRAD / 2 - 1; ++y)
    for(int x = 0; x < CNT_CELLS_TETRAD / 2; ++x, ++index)
      g_player.points[index] = (Point2f) {.x = MAP_WIDTH / 2 + x + y, . y = MAP_HEIGHT + y - 1};
}

void initZFigure() {
  for(int y = 0, index = 0; y > -(CNT_CELLS_TETRAD / 2); --y)
    for(int x = 0; x < CNT_CELLS_TETRAD / 2; ++x, ++index)
      g_player.points[index] = (Point2f) {.x = MAP_WIDTH / 2 + x - y - 1, .y = MAP_HEIGHT + y - 1};
}

void initTFigure() {
  for(int x = 0; x < CNT_CELLS_TETRAD - 1; ++x)
    g_player.points[x] = (Point2f) {.x = MAP_WIDTH / 2 - 1 + x, .y = MAP_HEIGHT - 2};
  g_player.points[3] = (Point2f) {.x = MAP_WIDTH / 2, .y = MAP_HEIGHT - 1};
}

int randInt(int limit) {
  int out;
  do 
    out = rand() / ((RAND_MAX + 1u) / limit);
  while (out > limit);
  return out;
}

bool isInGameGridXCollision(int x) {
  return (0 <= x) && (x < MAP_WIDTH);
}

bool isInGameGridYCollision(int y) {
  return y >= 0;
}

bool moveDownCollision() {
  for(int i = 0; i < CNT_CELLS_TETRAD; ++i)
    if(g_game_grid[g_player.points[i].x][g_player.points[i].y - 1].is_block || !isInGameGridYCollision(g_player.points[i].y - 1))
      return true;
  return false;
}

bool moveRightCollision() {
  for(int i = 0; i < CNT_CELLS_TETRAD; ++i)
    if(g_game_grid[g_player.points[i].x + 1][g_player.points[i].y].is_block || !isInGameGridXCollision(g_player.points[i].x + 1))
      return true;
  return false;
}

bool moveLeftCollision() {
  for(int i = 0; i < CNT_CELLS_TETRAD; ++i)
    if(g_game_grid[g_player.points[i].x - 1][g_player.points[i].y].is_block || !isInGameGridXCollision(g_player.points[i].x - 1))
      return true;
  return false;
}

void timer(int v) {
  updateGame();
  glutPostRedisplay();
  glutTimerFunc(500, timer, v);
}

void updateGame() {
  pushCellDown();
}

void pushCellDown() {
  if(!moveDownCollision())
    for(int i = 0; i < CNT_CELLS_TETRAD; ++i)
      --g_player.points[i].y;
  else {
    for(int i = 0; i < CNT_CELLS_TETRAD; ++i) {
      g_game_grid[g_player.points[i].x][g_player.points[i].y].is_block = true;
      g_game_grid[g_player.points[i].x][g_player.points[i].y].color = g_player.color;
    }
    checkField();
    initPlayer();
  }
}

void checkField() {
  for(int y = 0; y < MAP_HEIGHT; ++y) {
    int cnt_blocks = 0;
    for(int x = 0; x <  MAP_WIDTH; ++x)
      if(g_game_grid[x][y].is_block)
        cnt_blocks++;
    if(cnt_blocks == MAP_WIDTH) {
      clearLine(y);
      --y;
    }
    else if(cnt_blocks == 0)
      break;
  }
}

void clearLine(int y) {
  for(int x = 0; x < MAP_WIDTH; ++x)
    g_game_grid[x][y].is_block = false;
  
  for(int line = y + 1; line < MAP_HEIGHT; ++y, ++line)
    for(int x = 0; x < MAP_WIDTH; ++x) {
      GameGrid tmp = g_game_grid[x][y];
      g_game_grid[x][y] = g_game_grid[x][line];
      g_game_grid[x][line] = tmp;
    }
}

void moveKeyboard(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_LEFT:
      if(!moveLeftCollision())
        for(int i = 0; i < CNT_CELLS_TETRAD; ++i)
          --g_player.points[i].x;
      break;
    case GLUT_KEY_RIGHT:
      if(!moveRightCollision())
        for(int i = 0; i < CNT_CELLS_TETRAD; ++i)
          ++g_player.points[i].x;
      break;
    case GLUT_KEY_DOWN:
      if(!moveDownCollision())
        for(int i = 0; i < CNT_CELLS_TETRAD; ++i)
          --g_player.points[i].y;
      break;
  }
  glutPostRedisplay();
}

void rotateKeyboard(unsigned char key, int x, int y) {
  switch (key) {
    case ' ':
      rotatePlayer();
      break;
  }
  glutPostRedisplay();
}

bool rotatable() {
  switch (g_player.direction) {
    case RIGHT: {
        int cnt_cells_out_of_gird = 0;
        for(int i = 0; i < CNT_CELLS_TETRAD; ++i) 
          if(g_player.points[i].x >= MAP_WIDTH)
            ++cnt_cells_out_of_gird;
        for(int i = cnt_cells_out_of_gird; i > 0; --i)
          for(int j = 0; j < CNT_CELLS_TETRAD; ++j)
            --g_player.points[j].x;
        for(int i = 0; i < CNT_CELLS_TETRAD; ++i)
          if(g_game_grid[g_player.points[i].x][g_player.points[i].y].is_block)
            return false;
      } 
      break;
    case DOWN:
      for(int i = 0; i < CNT_CELLS_TETRAD; ++i) {
        if(g_game_grid[g_player.points[i].x][g_player.points[i].y].is_block || 
           !isInGameGridYCollision(g_player.points[i].y))
          return false;
      }
      break;
    case LEFT: {
        int cnt_cells_out_of_gird = 0;
        for(int i = 0; i < CNT_CELLS_TETRAD; ++i) 
          if(g_player.points[i].x < 0)
            ++cnt_cells_out_of_gird;
        for(int i = cnt_cells_out_of_gird; i > 0; --i)
          for(int j = 0; j < CNT_CELLS_TETRAD; ++j)
            ++g_player.points[j].x;
        for(int i = 0; i < CNT_CELLS_TETRAD; ++i)
          if(g_game_grid[g_player.points[i].x][g_player.points[i].y].is_block)
            return false;
      }
      break;
    case UP:
      for(int i = 0; i < CNT_CELLS_TETRAD; ++i)
        if(g_player.points[i].y >= MAP_HEIGHT)
          return false;
      break;
  }
  return true;
}

void rotatePlayer() {
  Player tmp;
  memcpy(&tmp, &g_player, sizeof(tmp));
  if(g_player.direction == UP)
    g_player.direction = RIGHT;
  else
    ++g_player.direction;
  rotate();
  if(!rotatable())
    memcpy(&g_player, &tmp, sizeof(tmp));
}

void rotate() {
  switch (g_player.figure) {
    case CUBE:
      break;
    case LINE:
      rotateLine();
      break;
    case LFIGURE:
      rotateLFigure();
      break;
    case JFIGURE:
      rotateJFigure();
      break;
    case SFIGURE:
      rotateSFigure();
      break;
    case ZFIGURE:
      rotateZFigure();
      break;
    case TFIGURE:
      rotateTFigure();
      break;
  }
}

void rotateLine() {
  switch (g_player.direction) {
    case RIGHT:
      for(int i = 1; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x += i; g_player.points[i].y -= i;
      }
      break;
    case DOWN:
      for(int i = 1; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x -= i; g_player.points[i].y -= i;
      }
      break;
    case LEFT:
      for(int i = 1; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x -= i; g_player.points[i].y += i;
      }
      break;
    case UP:
      for(int i = 1; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x += i; g_player.points[i].y += i;
      }
      break;
  }
}

void rotateLFigure() {
  g_player.points[0].x = g_player.points[2].x;
  g_player.points[0].y = g_player.points[2].y;
  switch (g_player.direction) {
    case RIGHT:
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x = g_player.points[1].x + i - 1;
        g_player.points[i].y = g_player.points[1].y;
      }
      break;
    case DOWN:
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x = g_player.points[1].x;
        g_player.points[i].y = g_player.points[1].y - i + 1;
      }
      break;
    case LEFT:
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x = g_player.points[1].x - i + 1;
        g_player.points[i].y = g_player.points[1].y;
      }
      break;
    case UP:
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x = g_player.points[1].x;
        g_player.points[i].y = g_player.points[1].y + i - 1;
      }
      break;
  }
}

void rotateJFigure() {
  switch (g_player.direction) {
    case RIGHT:
      for(int i = 0; i < CNT_CELLS_TETRAD - 1; ++i) {
        g_player.points[i].x += i; g_player.points[i].y -= i;
      }
      g_player.points[3].x = g_player.points[2].x; 
      g_player.points[3].y = g_player.points[2].y + 1;
      break;
    case DOWN:
      for(int i = 0; i < CNT_CELLS_TETRAD - 1; ++i) {
        g_player.points[i].x -= i; g_player.points[i].y -= i;
      }
      g_player.points[3].x = g_player.points[2].x + 1; 
      g_player.points[3].y = g_player.points[2].y;
      break;
    case LEFT:
      for(int i = 0; i < CNT_CELLS_TETRAD - 1; ++i) {
        g_player.points[i].x -= i; g_player.points[i].y += i;
      }
      g_player.points[3].x = g_player.points[2].x; 
      g_player.points[3].y = g_player.points[2].y - 1;
      break;
    case UP:
      for(int i = 0; i < CNT_CELLS_TETRAD - 1; ++i) {
        g_player.points[i].x += i; g_player.points[i].y += i;
      }
      g_player.points[3].x = g_player.points[2].x - 1; 
      g_player.points[3].y = g_player.points[2].y;
      break;
  }
}

void rotateSFigure() {
  switch (g_player.direction) {
    case RIGHT:
      g_player.points[1].x = g_player.points[0].x + 1;
      g_player.points[1].y = g_player.points[0].y;
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x = g_player.points[1].x + i - 2;
        g_player.points[i].y = g_player.points[1].y + 1;
      }
      break;
    case DOWN:
      g_player.points[1].x = g_player.points[0].x;
      g_player.points[1].y = g_player.points[0].y - 1;
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x = g_player.points[1].x + 1;
        g_player.points[i].y = g_player.points[1].y - i + 2;
      }
      break;
    case LEFT:
      g_player.points[1].x = g_player.points[0].x - 1;
      g_player.points[1].y = g_player.points[0].y;
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x = g_player.points[1].x - i + 2;
        g_player.points[i].y = g_player.points[1].y - 1;
      }
      break;
    case UP:
      g_player.points[1].x = g_player.points[0].x;
      g_player.points[1].y = g_player.points[0].y + 1;
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x = g_player.points[1].x - 1;
        g_player.points[i].y = g_player.points[1].y + i - 2;
      }
      break;
  }
}

void rotateZFigure() {
  switch (g_player.direction) {
    case RIGHT:
      ++g_player.points[1].x; --g_player.points[1].y;
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x += i - 2; g_player.points[i].y -= i;
      }
      break;
    case DOWN:
      --g_player.points[1].x; --g_player.points[1].y;
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x -= i; g_player.points[i].y -= i - 2;
      }
      break;
    case LEFT:
      --g_player.points[1].x; ++g_player.points[1].y;
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x -= i - 2; g_player.points[i].y += i;
      }
      break;
    case UP:
      ++g_player.points[1].x; ++g_player.points[1].y;
      for(int i = 2; i < CNT_CELLS_TETRAD; ++i) {
        g_player.points[i].x += i; g_player.points[i].y += i - 2;
      }
      break;
  }
}

void rotateTFigure() {
  switch (g_player.direction) {
    case RIGHT:
      for(int i = 1; i < CNT_CELLS_TETRAD - 1; ++i) {
        g_player.points[i].x += i; g_player.points[i].y -= i;
      }
      g_player.points[3].x = g_player.points[1].x;
      g_player.points[3].y = g_player.points[1].y + 1;
      break;
    case DOWN:
      for(int i = 1; i < CNT_CELLS_TETRAD - 1; ++i) {
        g_player.points[i].x -= i; g_player.points[i].y -= i;
      }
      g_player.points[3].x = g_player.points[1].x + 1;
      g_player.points[3].y = g_player.points[1].y;
      break;
    case LEFT:
      for(int i = 1; i < CNT_CELLS_TETRAD - 1; ++i) {
        g_player.points[i].x -= i; g_player.points[i].y += i;
      }
      g_player.points[3].x = g_player.points[1].x ;
      g_player.points[3].y = g_player.points[1].y - 1;
      break;
    case UP:
      for(int i = 1; i < CNT_CELLS_TETRAD - 1; ++i) {
        g_player.points[i].x += i; g_player.points[i].y += i;
      }
      g_player.points[3].x = g_player.points[1].x - 1;
      g_player.points[3].y = g_player.points[1].y;
      break;
  }
}

void display() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  
  displayGame();

  glFlush();
}

void displayGame() {
  glLoadIdentity();
  glScalef(2.0f / MAP_WIDTH, 2.0f / MAP_HEIGHT, 1.0f);
  glTranslatef(-MAP_WIDTH * 0.5f, -MAP_HEIGHT * 0.5f, 0.0f);

  for (int j = 0; j < MAP_HEIGHT; ++j) {
    for (int i = 0; i < MAP_WIDTH; ++i) {
      glPushMatrix();
      glTranslatef(i, j, 0);
      if(!g_game_grid[i][j].is_block) 
        displayField();
      else
        displayBlock(&g_game_grid[i][j].color);
      glPopMatrix();
    }
  }
  displayPlayer();
}

void displayField() {
  glBegin(GL_TRIANGLE_STRIP);
    glColor3f(0.8f, 0.8f, 0.8f); glVertex2f(0.0f, 1.0f);
    glColor3f(0.7f, 0.7f, 0.7f); glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 0.0f);
    glColor3f(0.6f, 0.6f, 0.6f); glVertex2f(1.0f, 0.0f);
  glEnd();
}

void displayBlock(Color3f* color) {
  glBegin(GL_QUADS);
    glColor3f(color->red / 255.0f, color->green / 255.0f, color->blue / 255.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(1.0f, 0.0f);
  glEnd();
  glLineWidth(2);
  glBegin(GL_LINE_LOOP);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(1.0f, 0.0f);
  glEnd();
}

void displayPlayer() {
  for(int i = 0; i < CNT_CELLS_TETRAD; ++i) {
    glPushMatrix();
    glTranslatef(g_player.points[i].x, g_player.points[i].y, 0.0f);
    displayBlock(&g_player.color);
    glPopMatrix();
  }
}
