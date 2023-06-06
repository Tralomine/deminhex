#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>


#define WIDTH 42
#define HEIGHT 32
#define MINECOUNT 320


#define OFFSETX 40
#define OFFSETY 40


typedef struct mineTile_st {
  bool isMine;
  bool isDiscovered;
  bool isFlagged;
  struct mineTile_st *neighbors[6];
  Vector2 pos;
} mineTile;

int countNeighbor(mineTile *t) {
  int neighborMines = 0;
  for (size_t i = 0; i < 6; i++) {
    if(t->neighbors[i]) {
      neighborMines += t->neighbors[i]->isMine;
    }
  }
  return neighborMines;
}

int countNeighborFlaggs(mineTile *t) {
  int neighborFlaggs = 0;
  for (size_t i = 0; i < 6; i++) {
    if(t->neighbors[i]) {
      neighborFlaggs += t->neighbors[i]->isFlagged;
    }
  }
  return neighborFlaggs;
}

bool discoverMine(mineTile *t) {
  if (!t || t->isFlagged || t->isDiscovered) return false;
  t->isDiscovered = true;
  if (t->isMine) {
    return true;
  }
  if (!countNeighbor(t))
    for (size_t i = 0; i < 6; i++) {
      discoverMine(t->neighbors[i]);
    }
  return false;
}

void drawMine(mineTile *t) {
  DrawPoly(t->pos, 6, 20, 0, GRAY);

  if (t->isDiscovered) {
    DrawPoly(t->pos, 6, 18, 0, LIGHTGRAY);
    int neighborMines = countNeighbor(t);
    if(t->isMine) {
      DrawCircleV(t->pos, 11, DARKPURPLE);
      DrawCircleV(t->pos, 7, RED);
			if(t->isFlagged) {	//discovered+flagged, only when losing
				DrawPoly(t->pos, 6, 15, 0, GREEN);
			}
    } else if (neighborMines) {
			Color c[] = {WHITE, SKYBLUE, LIME, GOLD, RED, PURPLE, BLACK};
      DrawText(TextFormat("%d", neighborMines), t->pos.x-5, t->pos.y-9, 18, c[neighborMines]);
    }
  } else {
    DrawPoly(t->pos, 6, 18, 0, DARKGRAY);
    if (t->isFlagged) {
      DrawPoly(t->pos, 3, 8, 30+60*((int)(t->pos.x*11+t->pos.y*7)%13), DARKBLUE);
    }
  }
}

int sqdist(Vector2 a, Vector2 b) {
  return (a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y);
}

void generateMap(mineTile *tileMap, size_t width, size_t height) {
	for (size_t x = 0; x<width; x++) {
    for (size_t y = 0; y<height; y++) {
      memset(tileMap[x*height+y].neighbors, 0, 6*sizeof(mineTile*));
      if (x>0) tileMap[x*height+y].neighbors[0] = tileMap+(x-1)*height+y;
      if (x<width-1) tileMap[x*height+y].neighbors[4] = tileMap+(x+1)*height+y;

      if (y%2) {
        Vector2 pos = {34.64*x+OFFSETX, 30*y+OFFSETY};
        tileMap[x*height+y].pos = pos;

        if (x>0 && y>0) tileMap[x*height+y].neighbors[1] = tileMap+(x-1)*height+y-1;
        if (y>0) tileMap[x*height+y].neighbors[2] = tileMap+(x)*height+y-1;
        if (y<height-1) tileMap[x*height+y].neighbors[3] = tileMap+(x)*height+y+1;
        if (x>0 && y<height-1) tileMap[x*height+y].neighbors[5] = tileMap+(x-1)*height+y+1;
      } else {
        Vector2 pos = {34.64*x+17.32+OFFSETX, 30*y+OFFSETY};
        tileMap[x*height+y].pos = pos;

        if (x<width-1 && y>0) tileMap[x*height+y].neighbors[1] = tileMap+(x+1)*height+y-1;
        if (y>0) tileMap[x*height+y].neighbors[2] = tileMap+(x)*height+y-1;
        if (y<height-1) tileMap[x*height+y].neighbors[3] = tileMap+(x)*height+y+1;
        if (x<width-1 && y<height-1) tileMap[x*height+y].neighbors[5] = tileMap+(x+1)*height+y+1;
      }
    }
  }
}

void resetMap(mineTile *tileMap, size_t width, size_t height, int mineCount) {
	for (size_t i = 0; i < width*height; i++) {
		tileMap[i].isFlagged = false;
		tileMap[i].isMine = false;
		tileMap[i].isDiscovered = false;
	}
	for (size_t i = 0; i < mineCount; i++) {
		size_t pos = rand()%(height*width);
		if(tileMap[pos].isMine) {
			i--;
		} else {
			tileMap[pos].isMine = true;
		}
	}

}

int main(void) {
  const int screenWidth = WIDTH*34.64+65;
  const int screenHeight = HEIGHT*30+120;

  srand(time(NULL));

  mineTile *tileMap = calloc(WIDTH*HEIGHT, sizeof(mineTile));

	generateMap(tileMap, WIDTH, HEIGHT);
	resetMap(tileMap, WIDTH, HEIGHT, MINECOUNT);

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(screenWidth, screenHeight, "Hex Mineswepper");
  SetTargetFPS(60);

  bool lost = false;
  bool win = false;

	float timeElapsed = 0;
	bool startedPlaying = false;
	float lastMousePress = 100;

  while (!WindowShouldClose()) {

		lastMousePress += GetFrameTime()*100;

    size_t nearestMine = -1;
    size_t nearestDistSq = 2147483647;
    Vector2 mousePos = GetMousePosition();
    if (!lost && !win) {
			if (startedPlaying) timeElapsed += GetFrameTime();
      for (size_t i = 0; i < WIDTH*HEIGHT; i++) {
        int sqDistCur = sqdist(mousePos, tileMap[i].pos);
        if (sqDistCur < nearestDistSq) {
          nearestMine = i;
          nearestDistSq = sqDistCur;
        }
      }

      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
				startedPlaying = true;
        if (nearestDistSq < 400) {
					lost |= discoverMine(&tileMap[nearestMine]);
					if (lastMousePress < 18 && tileMap[nearestMine].isDiscovered && countNeighbor(&tileMap[nearestMine]) == countNeighborFlaggs(&tileMap[nearestMine])) {
						for (size_t i = 0; i < 6; i++) {
							if (tileMap[nearestMine].neighbors[i]) {
								lost |= discoverMine(tileMap[nearestMine].neighbors[i]);
							}
						}
					}
				}
				lastMousePress = 0;
				if (lost) {
					for (size_t i = 0; i < WIDTH*HEIGHT; i++) {
						if (tileMap[i].isMine) tileMap[i].isDiscovered = true;
					}
				}
      }
      if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
				startedPlaying = true;
        if (nearestDistSq < 400 && !tileMap[nearestMine].isDiscovered)
          tileMap[nearestMine].isFlagged = !tileMap[nearestMine].isFlagged;
      }
    } else {
			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
				resetMap(tileMap, WIDTH, HEIGHT, MINECOUNT);
				win = false;
				lost = false;
				timeElapsed = 0;
			}
		}
    win = true;
		int mines = 0;
    for (size_t i = 0; i < WIDTH*HEIGHT; i++) {
      if(!tileMap[i].isDiscovered && !tileMap[i].isMine) win = false;
			if(tileMap[i].isMine) mines++;
			if(tileMap[i].isFlagged) mines--;
    }

    BeginDrawing();

      ClearBackground(RAYWHITE);

      for (size_t i = 0; i < WIDTH*HEIGHT; i++) {
        drawMine(&tileMap[i]);
      }

      if (nearestDistSq < 400) {
        DrawPoly(tileMap[nearestMine].pos, 6, 20, 0, Fade(WHITE, 0.5));
      }

      if (lost) {
				DrawRectangleRec((Rectangle){0, screenHeight/2-60, screenWidth, 110}, Fade(BLACK, 0.7));
				DrawText("Game Over !", (screenWidth-MeasureText("Game Over !", 50))/2, screenHeight/2-40, 50, MAROON);
				DrawText("Click to start a new game", (screenWidth-MeasureText("Click to start a new game", 20))/2, screenHeight/2+20, 20, RED);
			}
      if (win) {
				DrawRectangleRec((Rectangle){0, screenHeight/2-60, screenWidth, 110}, Fade(BLACK, 0.7));
				DrawText("You Won !", (screenWidth-MeasureText("You Won !", 50))/2, screenHeight/2-40, 50, DARKGREEN);
				DrawText("Click to start a new game", (screenWidth-MeasureText("Click to start a new game", 50))/2, screenHeight/2+20, 20, GREEN);
			}
			const char* mineTxt = TextFormat("Mines left : %d", mines);
			const char* timeTxt = TextFormat("Time : %ds", (int)timeElapsed);
			DrawText(mineTxt, (screenWidth-MeasureText(mineTxt, 30))/2, screenHeight-70, 30, (mines<0)?RED:DARKBLUE);
			DrawText(timeTxt, (screenWidth-MeasureText(timeTxt, 15))/2, screenHeight-30, 15, (mines<0)?RED:DARKBLUE);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
