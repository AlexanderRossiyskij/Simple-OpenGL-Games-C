#include <GL/gl.h>
#include <GL/glut.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Define width and height of game map
#define MAP_WIDTH  10
#define MAP_HEIGHT 10

typedef struct {
  bool is_mine;
  bool is_flag;
  bool is_open;
  int  mines_around_cell;
} GameGrid;

int      g_cnt_mines;
int      g_cnt_closed_cells;
bool     g_is_failed;
GameGrid g_map[MAP_WIDTH][MAP_HEIGHT];

bool isCellInMap(int, int);
void openFields(int, int);
void mouse(int, int, int, int);
void init();
void displayLine(float, float, float, float);
void displayNumber(int);
void displayMine();
void displayField();
void displayFieldOpen();
void displayFlag();
void displayGame();
void display();

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  
  init();
  glutCreateWindow("Minesweeper");
  glutInitWindowSize(300, 300);
  glutInitWindowPosition(40, 40);
  glutDisplayFunc(display);
  glutMouseFunc(mouse);
  glutMainLoop();

  return 0;
}

bool isCellInMap(int x, int y) {
  return (0 <= x) && (x < MAP_WIDTH) && (0 <= y) && (y < MAP_HEIGHT);
}

void openFields(int x, int y) {
  if(!isCellInMap(x, y) || g_map[x][y].is_open) return;
  g_map[x][y].is_open = true;
  g_cnt_closed_cells--;
  if(g_map[x][y].mines_around_cell == 0)
    for(int dx = -1; dx < 2; ++dx)
      for(int dy = -1; dy < 2; ++dy)
        openFields(x + dx, y + dy);

  if(g_map[x][y].is_mine) {
    g_is_failed = true;
    for(int j = 0; j < MAP_HEIGHT; ++j)
      for(int i = 0; i < MAP_WIDTH; ++i)
        g_map[j][i].is_open = true;
  }
}

void mouse(int button, int state, int mousex, int mousey) {
  int windowX = glutGet(GLUT_WINDOW_WIDTH);
  int windowY = glutGet(GLUT_WINDOW_HEIGHT);

  int mapX = mousex / (windowX / MAP_WIDTH);
  int mapY = MAP_HEIGHT - mousey / (windowY / MAP_HEIGHT) - 1;
  if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    if(g_is_failed)
      init();
    else {
      if(isCellInMap(mapX, mapY) && !g_map[mapX][mapY].is_flag)
          openFields(mapX, mapY);
    }
  } else if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
    if(!g_map[mapX][mapY].is_open) {
      if(!g_map[mapX][mapY].is_flag)
        g_map[mapX][mapY].is_flag = true;
      else
        g_map[mapX][mapY].is_flag = false;
    }
  }
  glutPostRedisplay();
}

void init() {
  srand(time(NULL));
  memset(g_map, 0, sizeof(g_map));
  
  g_is_failed = false;
  g_cnt_mines = 10;
  g_cnt_closed_cells = MAP_WIDTH * MAP_HEIGHT;
  for(int i = 0; i < g_cnt_mines; ++i) {
    int x, y;
    do
      x = rand() / ((RAND_MAX + 1u) / MAP_WIDTH);
    while(x > MAP_WIDTH);
    do
      y = rand() / ((RAND_MAX + 1u) / MAP_HEIGHT);
    while(y > MAP_HEIGHT);

    if(!g_map[x][y].is_mine) {
      g_map[x][y].is_mine = true;
      for(int dx = -1; dx < 2; ++dx)
        for(int dy = -1; dy < 2; ++dy)
          if(isCellInMap(x + dx, y + dy))
            g_map[x + dx][y + dy].mines_around_cell += 1;
    }
    else
      --i;
  }
}

void displayLine(float x1, float y1, float x2, float y2) {
  glVertex2f(x1, y1);
  glVertex2f(x2, y2);
}

void displayNumber(int num) {
  glLineWidth(1);
  glColor3f(1.0f, 1.0f, 0.0f);
  glBegin(GL_LINES);
    if((num != 1) && (num != 4)) displayLine(0.3f, 0.85f, 0.7f, 0.85f);
    if((num != 0) && (num != 1) && (num != 7)) displayLine(0.3f, 0.5f, 0.7f, 0.5f);
    if((num != 1) && (num != 4) && (num != 7)) displayLine(0.3f, 0.15f, 0.7f, 0.15f);

    if((num != 5) && (num != 6)) displayLine(0.7f, 0.5f, 0.7f, 0.85f);
    if((num != 2)) displayLine(0.7f, 0.5f, 0.7f, 0.15f);

    if((num != 1) && (num != 2) && (num != 3) && (num != 7)) displayLine(0.3f, 0.5f, 0.3f, 0.85f);
    if((num == 0) || (num == 2) || (num == 6) || (num == 8)) displayLine(0.3f, 0.5f, 0.3f, 0.15f);
  glEnd();
}

void displayMine() {
  glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(0.3f, 0.3f);
    glVertex2f(0.7f, 0.3f);
    glVertex2f(0.7f, 0.7f);
    glVertex2f(0.3f, 0.7f);
  glEnd();
}

void displayField() {
  glBegin(GL_TRIANGLE_STRIP);
    glColor3f(0.8f, 0.8f, 0.8f); glVertex2f(0.0f, 1.0f);
    glColor3f(0.7f, 0.7f, 0.7f); glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 0.0f);
    glColor3f(0.6f, 0.6f, 0.6f); glVertex2f(1.0f, 0.0f);
  glEnd();
}

void displayFieldOpen() {
  glBegin(GL_TRIANGLE_STRIP);
    glColor3f(0.3f, 0.7f, 0.3f); glVertex2f(0.0f, 1.0f);
    glColor3f(0.3f, 0.6f, 0.3f); glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 0.0f);
    glColor3f(0.3f, 0.5f, 0.3f); glVertex2f(1.0f, 0.0f);
  glEnd();
}

void displayFlag() {
  glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(0.25f, 0.75f);
    glVertex2f(0.85f, 0.5f);
    glVertex2f(0.25f, 0.25f);
  glEnd();
  glLineWidth(5);
  glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(0.25f, 0.75f);
    glVertex2f(0.25f, 0.0f);
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
      if(g_map[i][j].is_open) {
        displayFieldOpen();
        if(g_map[i][j].is_mine)
           displayMine();
        else if (g_map[i][j].mines_around_cell > 0)
          displayNumber(g_map[i][j].mines_around_cell);
      } else {
        displayField();
        if(g_map[i][j].is_flag)
          displayFlag();
      }
      glPopMatrix();
    }
}

void display() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  
  if(g_cnt_mines == g_cnt_closed_cells)
    init();
  displayGame();

  glFlush();
}

