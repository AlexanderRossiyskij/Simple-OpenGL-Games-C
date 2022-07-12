#include <GL/gl.h>
#include <GL/glut.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAP_WIDTH  10
#define MAP_HEIGHT 10

typedef enum {SNAKE_MOVE_UP, SNAKE_MOVE_DOWN, SNAKE_MOVE_RIGHT, SNAKE_MOVE_LEFT} SnakeDirectionMoving;

typedef struct {
  int x;
  int y;
} Point;

typedef struct {
  bool is_food;
  bool is_wall;
} GameGrid;

typedef struct {
  int                  length;
  SnakeDirectionMoving direction;
  Point                head;
  Point                body[MAP_WIDTH * MAP_HEIGHT];
} Snake;

int      g_game_status;
GameGrid g_map[MAP_WIDTH][MAP_HEIGHT];
Snake    g_snake;
bool     g_key_input_was;

void initGame();
void updateGame();
bool isPartOfSnake(int, int);
bool isInGameGrid(int, int);
bool doesSnakeEatItself();
bool isWin();
void updateSnakeBody(Point);
void addSnakeCell(Point);
int  randInt(int);
void timer(int);
void keyboard(int, int, int);
void display();
void displayGame();
void displayWin();
void displayLoose();
void displayField();
void displaySnake();
void displaySnakeHead();
void displaySnakeBody();
void displaySnakeCell(void (*)());
void displayApple();

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  initGame();

  glutCreateWindow("Snake");
  glutInitWindowSize(300, 300);
  glutInitWindowPosition(40, 40);
  glutSpecialFunc(keyboard);
  glutDisplayFunc(display);
  glutTimerFunc(250, timer, 0);
  glutMainLoop();

  return 0;
}

void initGame() {
  srand(time(NULL));
  memset(g_map, 0, sizeof(g_map));
  memset(&g_snake, 0, sizeof(g_snake));

  g_game_status = 0;
  g_key_input_was = false;

  int x = randInt(MAP_WIDTH);
  int y = randInt(MAP_HEIGHT);
  g_map[x][y].is_food = true;
  do {
    x = randInt(MAP_WIDTH);
  } while(g_map[x][1].is_food);
  g_snake.head = (Point) {.x = x, .y = 1};
  g_snake.direction = SNAKE_MOVE_UP;
}


void updateGame() {
  Point head = g_snake.head;
  switch(g_snake.direction) {
    case SNAKE_MOVE_UP:
      ++g_snake.head.y;
      break;
    case SNAKE_MOVE_DOWN:
      --g_snake.head.y;
      break;
    case SNAKE_MOVE_RIGHT:
      ++g_snake.head.x;
      break;
    case SNAKE_MOVE_LEFT:
      --g_snake.head.x;
      break;
  }

  g_key_input_was = false;
  if(!isInGameGrid(g_snake.head.x, g_snake.head.y)) 
    g_game_status = -1;
  else if(doesSnakeEatItself())
    g_game_status = -1;
  else if(g_map[g_snake.head.x][g_snake.head.y].is_food) {
    g_map[g_snake.head.x][g_snake.head.y].is_food = false;
    addSnakeCell(head);
    ++g_snake.length;
    updateSnakeBody(head);
    int x;
    int y;
    do {
      x = randInt(MAP_WIDTH);
      y = randInt(MAP_HEIGHT);
    } while(isPartOfSnake(x, y));
    g_map[x][y].is_food = true;
  } else
    updateSnakeBody(head);
  if(isWin())
    g_game_status = 1;
}

bool isWin() {
  return g_snake.length == MAP_WIDTH * MAP_HEIGHT - 1;
}

bool doesSnakeEatItself() {
  for(int i = 0; i < g_snake.length; ++i) {
    if(g_snake.head.x == g_snake.body[i].x && g_snake.head.y == g_snake.body[i].y)
      return true;
  }
  return false;
}

bool isInGameGrid(int x, int y) {
  return (0 <= x) && (x < MAP_WIDTH) && (0 <= y) && (y < MAP_HEIGHT);
}

bool isPartOfSnake(int x, int y) {
  if(x == g_snake.head.x && y == g_snake.head.y)
    return true;

  for(int i = 0; i < g_snake.length; ++i)
    if(x == g_snake.body[i].x && y == g_snake.body[i].y)
      return true;
  return false;
}

void addSnakeCell(Point head) {
  if(g_snake.length == 0)
    g_snake.body[g_snake.length] = head;
  else {
    Point cell1 = g_snake.length == 1 ? g_snake.head : g_snake.body[g_snake.length - 2];
    Point cell2 = g_snake.body[g_snake.length - 1];
    int dx = cell1.x - cell2.x;
    int dy = cell1.y - cell2.y;
    
    if (dx > 0)
      --cell2.x;
    else if (dx < 0)
      ++cell2.x;

    if (dy > 0)
      --cell2.y;
    else if (dy < 0)
      ++cell2.y;

    g_snake.body[g_snake.length] = cell2;
  }
}

void updateSnakeBody(Point head) {
  for(int i = 0; i < g_snake.length; ++i) {
    Point tmp_head = g_snake.body[i];
    g_snake.body[i] = head;
    head = tmp_head;
  }
}

int randInt(int limit) {
  int out;
  do 
    out = rand() / ((RAND_MAX + 1u) / limit);
  while (out > limit);
  return out;
}

void timer(int v) {
  if(g_game_status == 0)
    updateGame();
  glutPostRedisplay();
  glutTimerFunc(250, timer, v);
}

void keyboard(int key, int x, int y) {
  if(!g_key_input_was)
    switch(key) {
      case 100:
        if(g_snake.direction == SNAKE_MOVE_RIGHT)
          break;
        g_key_input_was = true;
        g_snake.direction = SNAKE_MOVE_LEFT;
        break;
      case 101:
        if(g_snake.direction == SNAKE_MOVE_DOWN)
          break;
        g_key_input_was = true;
        g_snake.direction = SNAKE_MOVE_UP;
        break;
      case 102:
        if(g_snake.direction == SNAKE_MOVE_LEFT)
          break;
        g_key_input_was = true;
        g_snake.direction = SNAKE_MOVE_RIGHT;
        break;
      case 103:
        if(g_snake.direction == SNAKE_MOVE_UP)
          break;
        g_key_input_was = true;
        g_snake.direction = SNAKE_MOVE_DOWN;
        break;
    }
}

void display() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  if(g_game_status == 0)
    displayGame();
  else if(g_game_status == 1)
    displayWin();
  else 
    displayLoose();

  glFlush();
}

void displayWin() {
  glLoadIdentity();
  glColor3f(1.0f, 1.0f, 0.0f);
  glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);
  glEnd();
}

void displayLoose() {
  glLoadIdentity();
  glColor3f(1.0f, 0.0f, 0.0f);
  glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);
  glEnd();
}

void displayGame() {
  glLoadIdentity();
  glScalef(2.0f / MAP_WIDTH, 2.0f / MAP_HEIGHT, 1.0f);
  glTranslatef(-MAP_WIDTH * 0.5, -MAP_HEIGHT * 0.5, 0);

  for(int j = 0; j < MAP_HEIGHT; ++j)
    for(int i = 0; i < MAP_WIDTH; ++i) {
      glPushMatrix();
      glTranslatef(i, j, 0);
      if (g_map[i][j].is_food)
        displayApple();
      else
        displayField();
      glPopMatrix();
    }
  displaySnake();
}

void displayField() {
  glBegin(GL_TRIANGLE_STRIP);
    glColor3f(0.8f, 0.8f, 0.8f); glVertex2f(0.0f, 1.0f);
    glColor3f(0.7f, 0.7f, 0.7f); glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 0.0f);
    glColor3f(0.6f, 0.6f, 0.6f); glVertex2f(1.0f, 0.0f);
  glEnd();
}

void displaySnake() {
  glPushMatrix();
  glTranslatef(g_snake.head.x, g_snake.head.y, 0.0f);
  displaySnakeCell(displaySnakeHead);
  glPopMatrix();
  for(int i = 0; i < g_snake.length; ++i) {
    glPushMatrix();
    glTranslatef(g_snake.body[i].x, g_snake.body[i].y, 0.0f);
    displaySnakeCell(displaySnakeBody);
    glPopMatrix();
  }
}

void displaySnakeCell(void (*displayPartOfSnake)()) {
  displayPartOfSnake();
  glBegin(GL_LINE_LOOP);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(1.0f, 0.0f);
  glEnd();
}

void displaySnakeHead() {
  glBegin(GL_TRIANGLE_STRIP);
    glColor3f(0.01f, 0.20f, 0.13f); glVertex2f(0.0f, 1.0f); 
    glColor3f(0.01f, 0.29f, 0.19f); glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 0.0f);
    glColor3f(0.01f, 0.20f, 0.13f); glVertex2f(1.0f, 0.0f);
  glEnd();
}

void displaySnakeBody() {
  glBegin(GL_TRIANGLE_STRIP);
    glColor3f(0.01f, 0.39f, 0.25f); glVertex2f(0.0f, 1.0f); 
    glColor3f(0.01f, 0.49f, 0.31f); glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 0.0f);
    glColor3f(0.01f, 0.39f, 0.25f); glVertex2f(1.0f, 0.0f);
  glEnd();
}

void displayApple() {
  glBegin(GL_TRIANGLE_STRIP);
    glColor3f(0.7f, 0.0f, 0.0f); glVertex2f(0.0f, 1.0f);
    glColor3f(0.8f, 0.0f, 0.0f); glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 0.0f);
    glColor3f(0.6f, 0.0f, 0.0f); glVertex2f(1.0f, 0.0f);
  glEnd();
}
