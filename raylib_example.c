#include "raylib.h"
#include "rlgl.h"
#include "stdio.h"

typedef struct _Player {
	Texture tex;
	Texture texFlipped;
	float x; float y;
	float speed;
	bool isMovingLeft;
	bool isMovingRight;
	Sound shootFx;
} Player;

typedef struct _Enemy {
	Texture texture;
	Texture textureFlipped;
	float x; float y;
	float hSpeed;
	float vSpeed;
	bool isMovingLeft;
	bool isMovingRight;
} Enemy;

typedef struct _Bullet {
	Vector2 pos;
	Vector2 size;
	bool alive;
} Bullet;

void initGame();

void drawPlayer();
void drawBullets();
void drawBG();
void drawGame();
void drawEnemies();

void handlInput();
void updatePlayer();
void updateBullets();
void shoot();
void updateGame();
void updateEnemies();

// globals  \o/
Player player = {0};

// bullets 
#define MAX_BULLETS 5
Bullet bullets[MAX_BULLETS];
const float bulletSpeed = 300;
const Vector2 bulletSize = (Vector2){2,10};

// enemy
#define NUM_ENEMIES 21
const int numRows = 3;
int numEnemiesPerRow = NUM_ENEMIES/numRows;
Enemy enemies[NUM_ENEMIES];

int main() 
{
	const int WIDTH = 800;
	const int HEIGHT = 600;

	InitWindow(WIDTH, HEIGHT, "Raylib Example");
	SetTargetFPS(30);

	// initial positions etc.
	initGame();

	while(!WindowShouldClose()) {
	
		BeginDrawing();
		drawBG();	
		
		updateGame();
		drawGame();
		
		EndDrawing();
	} 

	CloseWindow();

	return 0;
}

void initGame() {
	InitAudioDevice();	
	SetMasterVolume(0.3f);
	Sound pew = LoadSound("pewpew.ogg");	
	Image playerImg = LoadImage("player.png");
	Image playerImgFlipped = LoadImage("player.png");
	ImageFlipHorizontal(&playerImg);
	Texture playerTex = LoadTextureFromImage(playerImg);
	Texture playerTexFlipped = LoadTextureFromImage(playerImgFlipped);

	// init Player
	player.tex = playerTex;
	player.texFlipped = playerTexFlipped;
	player.x = GetScreenWidth()/2 -player.tex.width/2;
	player.y = GetScreenHeight()-(20+player.tex.height);
	// movingLeft/right default: false
	player.speed = 150;
	player.shootFx = pew;

	// init enemy
	Image ghostImg = LoadImage("ghost.png");
	Image ghostImgFlipped = LoadImage("ghost.png");
	ImageFlipHorizontal(&ghostImgFlipped);
	Texture ghostTex= LoadTextureFromImage(ghostImg);
	Texture ghostTexFlipped = LoadTextureFromImage(ghostImgFlipped);

	for (int i = 0; i < NUM_ENEMIES; i++) {
		enemies[i].texture = ghostTex;
		enemies[i].textureFlipped = ghostTexFlipped;
		enemies[i].hSpeed = 100;
	}
	
	// init enemy positions
	int xSpace = 20;	
	int ySpace = 20;
	int xOffs = (GetScreenWidth() - numEnemiesPerRow*ghostImg.width 
					- numEnemiesPerRow*xSpace)/2;
	int yOffs = 50;
	for (int row = 0; row < numRows; row++) {
		for (int col = 0; col < numEnemiesPerRow; col++) {
			int idx = (row*numEnemiesPerRow)+col;
			enemies[idx].x = ghostImg.width * col + col*xSpace +xOffs;
			enemies[idx].y = ghostImg.height * row + row*ySpace +yOffs;
		}
	}
}

void drawBG() {
	ClearBackground(RAYWHITE);
}

void handleInput() {
	player.isMovingLeft = false;
	player.isMovingRight= false;

	if (IsKeyDown(KEY_LEFT)) {
		player.isMovingLeft = true;
	}
	if (IsKeyDown(KEY_RIGHT)) {
		player.isMovingRight= true;
	}
	if (IsKeyPressed(KEY_UP)) {
		shoot();
	}
}

void updateGame() {
	handleInput();

	updateEnemies();
	updatePlayer();
	updateBullets();
}

void updatePlayer() {
	if (player.isMovingLeft) {
		player.x -= player.speed *GetFrameTime();
	}
	if (player.isMovingRight) {
		player.x += player.speed *GetFrameTime();
	}
}
 
void shoot() {
	static int bulletIdx = 0;	

	bullets[bulletIdx].pos.x = player.x + player.tex.width/2;
	bullets[bulletIdx].pos.y = player.y;
	bullets[bulletIdx].size = bulletSize;
	bullets[bulletIdx].alive = true;

	bulletIdx++;
	bulletIdx %= MAX_BULLETS;
	PlaySound(player.shootFx);
}

void drawGame() {
	drawPlayer();
	drawBullets();
	drawEnemies();
}	

void drawPlayer() {
	if (player.isMovingLeft) {
		DrawTexture (player.tex, player.x, player.y, 
			GRAY);
	} else {
		DrawTexture (player.texFlipped, player.x, player.y, 
			GRAY);
 	}
}

void updateEnemies() {
	int xBoundary = 30;	

	for (int i = 0; i < NUM_ENEMIES; i++) {
		enemies[i].x += enemies[i].hSpeed*GetFrameTime();	
		
		// if any enemy touches the boundary - invert speed	
		if (enemies[i].x <= 0+xBoundary) {
			enemies[i].hSpeed *= -1;
		}

		if (enemies[i].x >= GetScreenWidth()-(xBoundary + 
									enemies[i].texture.width)) {
			enemies[i].hSpeed *= -1;
		}
	}
}

void updateBullets() {

	for (int i = 0; i < MAX_BULLETS; i++) {
		if (bullets[i].alive) {
			bullets[i].pos.y -= bulletSpeed * GetFrameTime();
		}
	}
}

void drawBullets() {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (bullets[i].alive) {
			DrawRectangleV(bullets[i].pos,bullets[i].size, BLACK);
		}
	}
}
void drawEnemies() {
	for (int i = 0; i < NUM_ENEMIES; i++) {
		if (enemies[i].hSpeed > 0) {
			// horizontal flip
			DrawTexture (enemies[i].textureFlipped, 
				enemies[i].x, enemies[i].y, WHITE);
		} else {
			DrawTexture (enemies[i].texture, 
				enemies[i].x, enemies[i].y, WHITE);
		}
	}
}

