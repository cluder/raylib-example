
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"

#include "raylib.h"
#include "rlgl.h"

typedef struct _Player {
	Texture currentTexture;
	Texture tex;
	Texture texFlipped;
	float x; float y;
	float speed;
	bool isMovingLeft;
	bool isMovingRight;
	Sound shootFx;
} Player;

typedef struct _Enemy {
	float health;
	float maxHealth;
	Texture currentTexture;
	Texture texture;
	Texture textureFlipped;
	float x; float y;
	float hSpeed;
	float vSpeed;
	bool isMovingLeft;
	bool isMovingRight;
	bool isDead;
} Enemy;

typedef struct _Bullet {
	Vector2 pos;
	Vector2 size;
	bool isAlive;
} Bullet;

void initGame();
void initEnemies();
void resetGame();

void drawPlayer();
void drawBullets();
void drawBG();
void drawGame();
void drawEnemies();
void drawLevelInfo();

void handleInput();
void handlePlayerInput();
void shoot();
void updatePlayer(float tpf);
void updateBullets(float tpf);
void updateGame(float tpf);
void updateEnemies(float tpf);

void detectCollision();
bool isCollidingBE(Bullet b, Enemy e);
bool isCollidingPE(Player p, Enemy e);
bool checkPixelCollisionPE(Rectangle r, Player p, Enemy e);

float calcFrameTime();
Color getEnemyTint(Enemy e);

void gameOver();
void drawGameOverScreen();
void checkLevelEnd();
void nextLevel();
void testLevel();

// textures
Texture getOrLoadGhostTex(bool flipped);
Texture getOrLoadPlayerTex(bool flipped);

typedef enum _GameState {
	GAME,
	GAME_OVER
} GameState;

GameState gameState;

// globals  \o/
Player player = {0};

// bullets 
#define MAX_BULLETS 5
Bullet bullets[MAX_BULLETS];
const float bulletSpeed = 300;
const Vector2 bulletSize = (Vector2){2,12};

// enemy
#define NUM_ENEMIES 15 
const int numRows = 3;
int numEnemiesPerRow = NUM_ENEMIES/numRows;
Enemy enemies[NUM_ENEMIES];

int currentLevel = 1;

#define TARGET_FPS 30

int main() 
{
	const int WIDTH = 800;
	const int HEIGHT = 600;

	InitWindow(WIDTH, HEIGHT, "Raylib Example");
	SetTargetFPS(TARGET_FPS);

	// initial positions etc.
	initGame();

	while(!WindowShouldClose()) {
	
		BeginDrawing();
		drawBG();	
		
		updateGame(calcFrameTime());
		drawGame();
		
		EndDrawing();
	} 

	CloseWindow();

	return 0;
}

// TODO limit max frame time (stutter / debugging)
float calcFrameTime() {
	float tpf = GetFrameTime();

	return tpf;
}

void initGame() {
	gameState = GAME;
	
	// audio
	InitAudioDevice();	
	SetMasterVolume(0.5f);
	
	Sound pew = LoadSound("pewpew.ogg");	
	Texture playerTex = getOrLoadPlayerTex(false);
	Texture playerTexFlipped = getOrLoadPlayerTex(true);

	// init Player
	player.tex = playerTex;
	player.texFlipped = playerTexFlipped;
	player.x = GetScreenWidth()/2 -player.tex.width/2;
	player.y = GetScreenHeight()-(20+player.tex.height);
	// movingLeft/right default: false
	player.speed = 150;
	player.shootFx = pew;

	initEnemies();
}

void resetGame() {
	initEnemies();
	gameState = GAME;
}

Texture getOrLoadGhostTex(bool flipped) {
	static bool isLoaded = false;
	static Texture ghostTex; 
	static Texture ghostTexFlipped;

	if (isLoaded == false) {
		ghostTex = LoadTexture("ghost.png");
		Image ghostImgFlipped = LoadImage("ghost.png");
		ImageFlipHorizontal(&ghostImgFlipped);
		ghostTexFlipped = LoadTextureFromImage(ghostImgFlipped);
		UnloadImage(ghostImgFlipped);
		isLoaded = true;
	}

	return flipped?ghostTexFlipped:ghostTex;
}

// toto fix copy pasta
Texture getOrLoadPlayerTex(bool flipped) {
	static bool isLoaded = false;
	static Texture playerTex; 
	static Texture playerTexFlipped;

	if (isLoaded == false) {
		playerTex = LoadTexture("player.png");
		Image playerImgFlipped = LoadImage("player.png");
		ImageFlipHorizontal(&playerImgFlipped);
		playerTexFlipped = LoadTextureFromImage(playerImgFlipped);
		UnloadImage(playerImgFlipped);
		isLoaded = true;
	}

	return flipped?playerTexFlipped:playerTex;
}

void initEnemies() {
	const float baseHealth = 3;
	float healthLevelModifier = 0.4;

	const float baseHSpeed = 100;
	float hSpeedLevelModifier = 0.4;
	
	float health = baseHealth;
	float hSpeed = baseHSpeed;

	health = baseHealth + (baseHealth * healthLevelModifier * currentLevel);
	hSpeed = baseHSpeed + (baseHSpeed * hSpeedLevelModifier * currentLevel);

	fprintf(stderr, "Level:%d: health:%f, hspeed:%f\n",
			currentLevel, health, hSpeed);
	
	Texture ghostTex = getOrLoadGhostTex(false);
	for (int i = 0; i < NUM_ENEMIES; i++) {
		enemies[i].texture = ghostTex;
		enemies[i].textureFlipped = getOrLoadGhostTex(true);
		enemies[i].hSpeed = hSpeed;
		enemies[i].vSpeed = 0;
		enemies[i].health = health;
		enemies[i].maxHealth = health;
	}
	
	// init enemy positions
	int xSpace = 20;	
	int ySpace = 20;
	int xOffs = (GetScreenWidth() - numEnemiesPerRow*ghostTex.width 
					- numEnemiesPerRow*xSpace)/2;
	int yOffs = 100;
	for (int row = 0; row < numRows; row++) {
		for (int col = 0; col < numEnemiesPerRow; col++) {
			int idx = (row*numEnemiesPerRow)+col;
			enemies[idx].x = ghostTex.width * col + col*xSpace +xOffs;
			enemies[idx].y = ghostTex.height * row + row*ySpace +yOffs;
			enemies[idx].isDead = false;
		}
	}
}


void drawBG() {
	ClearBackground(RAYWHITE);
}

void handleInput() {
	switch(gameState) {
	case GAME:
		handlePlayerInput();
		break;
	case GAME_OVER:
		if (IsKeyDown(KEY_SPACE)) {
			resetGame();
		}
		break;
	default:
		break;
	}
}

void handlePlayerInput() {
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
	// debug
	if (IsKeyPressed(KEY_X)) {
		gameOver();
	}
	if (IsKeyPressed(KEY_S)) {
		nextLevel();
	}
	if (IsKeyPressed(KEY_T)) {
		testLevel();
	}
}

void updateGame(float tpf) {
	handleInput();

	updateEnemies(tpf);
	updatePlayer(tpf);
	updateBullets(tpf);
	detectCollision();

	checkLevelEnd(); // check if all enemies are dead
}

void detectCollision() {
	// check for colisions between bullets and enemies
	for (int bIdx = 0; bIdx < MAX_BULLETS; bIdx++) {
		for (int eIdx = 0; eIdx < NUM_ENEMIES; eIdx++) {
			if (enemies[eIdx].isDead || bullets[bIdx].isAlive == false) {
				continue;
			}
			bool collision = isCollidingBE(bullets[bIdx], enemies[eIdx]);
			if (collision) {
				enemies[eIdx].health --;
				enemies[eIdx].hSpeed *= 1.05f;
				enemies[eIdx].vSpeed += 10;
				if (enemies[eIdx].health <= 0) {
					enemies[eIdx].isDead = true;
				}
				bullets[bIdx].isAlive = false;
			}
		}
	}

	// check for collision between enemies and player
	for (int eIdx = 0; eIdx < NUM_ENEMIES; eIdx++) {
		bool collision = isCollidingPE(player, enemies[eIdx]);
		if (collision) {
			gameOver();
		}
	}
}

// check collision (bullet point vs enemy rectangle)
bool isCollidingBE(Bullet b, Enemy e) {
	Rectangle enemyRect;
	enemyRect.x = e.x;
	enemyRect.y = e.y;
	enemyRect.width = e.texture.width;
	enemyRect.height= e.texture.height;

	return CheckCollisionPointRec(b.pos, enemyRect);
}

// check for collision between player and enemy
// 1. check rectangles for collision
// 2. check pixel perfect
bool isCollidingPE(Player p, Enemy e) {
	Rectangle enemyRect;
	enemyRect.x = e.x;
	enemyRect.y = e.y;
	enemyRect.width = e.texture.width;
	enemyRect.height= e.texture.height;
	
	Rectangle playerRect;
	playerRect.x = p.x;
	playerRect.y = p.y;
	playerRect.width = p.tex.width;
	playerRect.height= p.tex.height;
	
	Rectangle colRect = GetCollisionRec(playerRect, enemyRect);
	if (colRect.width > 0 && colRect.height > 0) {
		// pixel perfect collision check
		return checkPixelCollisionPE(colRect, p, e);
	}
	return false;
}

Color getPixelColor(Image img, int imgX, int imgY, int screenX, int screenY)
{
	Color *imgData = GetImageData(img);	
		
	for (int y = 0; y < img.height; y++) {
		for (int x = 0; x < img.width; x++) {
			int imgPixelPosX = imgX + x;
			int imgPixelPosY = imgY + y;
			
			if (imgPixelPosX == screenX && 
				imgPixelPosY == screenY) {
				return imgData[y*img.width + x];
			}
		}
	}
	// return transparent color if pixel is out of range
	return BLANK;
}

// check if non-transparent player and enemy pixels overlap
bool checkPixelCollisionPE(Rectangle rect, Player p, Enemy e)
{
	// get player img  TODO
	Image pImg =  GetTextureData(p.currentTexture);

	// get enemy img
	Image eImg =  GetTextureData(e.currentTexture);

	// for each pixel in the collision rectangle, 
	// check if a non-transparent player and enemy pixel exists
	for (int y = 0; y < rect.height; y++) {
		for (int x = 0; x < rect.width; x++) {
			// for the img at loc x/y get the color at screen loc x/y
			Color playerPixelCol = getPixelColor(pImg, p.x, p.y,
				rect.x+x, rect.y+y);
			Color enemyPixelCol = getPixelColor(eImg, e.x, e.y, 
				rect.x+x, rect.y+y);

			if (playerPixelCol.a != 0 && enemyPixelCol.a != 0) {
				return true;
			}
		}
	}
	return false;
}

void updatePlayer(float tpf) {
	if (player.isMovingLeft) {
		player.x -= player.speed *tpf;
	}
	if (player.isMovingRight) {
		player.x += player.speed *tpf;
	}
}

// TODO add ability to shoot horizontally ?
void shoot() {
	static int bulletIdx = 0;	

	bullets[bulletIdx].pos.x = player.x + player.tex.width/2;
	bullets[bulletIdx].pos.y = player.y;
	bullets[bulletIdx].size = bulletSize;
	bullets[bulletIdx].isAlive = true;

	bulletIdx++;
	bulletIdx %= MAX_BULLETS;
	PlaySound(player.shootFx);
}

void drawGame() {
	drawPlayer();
	drawBullets();
	drawEnemies();
	drawLevelInfo();

	if (gameState == GAME_OVER) {
		drawGameOverScreen();
	}
}	

void drawPlayer() {
	if (player.isMovingLeft) {
		player.currentTexture = player.texFlipped;
	} else {
		player.currentTexture = player.tex;
 	}

	DrawTexture (player.currentTexture, player.x, player.y, 
			GRAY);
}

void updateEnemies(float tpf) {
	int xBoundary = 30;	

	for (int i = 0; i < NUM_ENEMIES; i++) {
		if (enemies[i].isDead) {
			continue;
		}
		enemies[i].x += enemies[i].hSpeed*tpf;	
		enemies[i].y += enemies[i].vSpeed*tpf;	
		
		// if any enemy touches the boundary - invert speed	
		if (enemies[i].x <= 0+xBoundary) {
			enemies[i].hSpeed *= -1;
		}
		if (enemies[i].x >= GetScreenWidth()-(xBoundary + 
									enemies[i].texture.width)) {
			enemies[i].hSpeed *= -1;
		}

		// check if enemy reaches bottom
		if (enemies[i].y >= GetScreenHeight()) {
			enemies[i].isDead = true;
			gameOver();
		}
	}
}

void updateBullets(float tpf) {

	for (int i = 0; i < MAX_BULLETS; i++) {
		if (bullets[i].isAlive) {
			bullets[i].pos.y -= bulletSpeed * tpf;
		}

		// bullet reaches top border
		if (bullets[i].pos.y <= 0) {
			bullets[i].isAlive = false;
		}
	}
}

void drawBullets() {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (bullets[i].isAlive) {
			DrawRectangleV(bullets[i].pos,bullets[i].size, BLACK);
		}
	}
}

void drawEnemies() {
	for (int i = 0; i < NUM_ENEMIES; i++) {
		if (enemies[i].isDead) {
			continue;
		}
		
		Color tint = getEnemyTint(enemies[i]);

		if (enemies[i].hSpeed > 0) {
			// horizontal flip
			enemies[i].currentTexture = enemies[i].textureFlipped;
		} else {
			enemies[i].currentTexture = enemies[i].texture;
		}
		DrawTexture (enemies[i].currentTexture, 
				enemies[i].x, enemies[i].y, tint);
	}
}

void drawLevelInfo() {
	int fontSize = 30;
	const char* txt = "Level:%d";
	const char *levelInfo = TextFormat(txt, currentLevel);
	int txtLen = MeasureText(levelInfo, fontSize);
	DrawFPS(20, 20);
	DrawText(levelInfo, GetScreenWidth()/2 -txtLen/2, 20, fontSize, RED);
}


Color getEnemyTint(Enemy e) {
	Color tint = WHITE;	
	
	// inverted health percent
	float healthPercent = 1-(e.health / e.maxHealth);

	tint.r = 255 - (25*healthPercent);
	tint.g = 255 - (255*healthPercent);
	tint.b = 255 - (255*healthPercent);

	return tint;	
}

void gameOver() {
	gameState = GAME_OVER;

	// reset enemies v speed
	for (int i = 0; i < NUM_ENEMIES; i++) {
		enemies[i].vSpeed = 0;
	}
	
	// stop player
	player.isMovingLeft = false;
	player.isMovingRight = false;
}

void drawGameOverScreen() {
	int fontSize = 70;
	int fontSizeInfo = 35;
	const char* txtGameOver = "GAME OVER";
	const char* txtRestart = "press SPACE to restart";

	int textWidthGameOver = MeasureText(txtGameOver, fontSize);
	int textWidthRestart = MeasureText(txtRestart, fontSizeInfo);

	DrawText(txtGameOver, GetScreenWidth()/2 - textWidthGameOver/2, 
		GetScreenHeight()/2, fontSize, RED);
	DrawText(txtRestart, GetScreenWidth()/2 - textWidthRestart/2, 
		GetScreenHeight()/2 + 100, fontSizeInfo, RED);
}

void checkLevelEnd() {
	bool enemiesAlive = false;
	for (int i = 0; i < NUM_ENEMIES; i++) {
		if (enemies[i].isDead == false) {
			enemiesAlive = true;	
		}
	}
	if (enemiesAlive == false) {
		currentLevel++;
		resetGame();
	}
}

void nextLevel() {
	currentLevel++;
	resetGame();
}


// simple test level with one enemy
void testLevel() {
	for (int i = 0; i < NUM_ENEMIES; i++) {
		enemies[i].isDead = true;
	}
	enemies[0].isDead = false;
	enemies[0].vSpeed = 0;
	enemies[0].hSpeed = -10;
	enemies[0].x = player.x -200;
	enemies[0].y = player.y;
}
