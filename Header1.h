#ifndef _Arkanoid
#define _Arkanoid
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>

#define inWidth 9
#define inHeight 5
#define blocksNumber 45

//___________________________________GLOBAL______________________________
//window
extern const int WINDOW_WIDTH = 640;
extern const int WINDOW_HEIGHT = 480;

extern SDL_Window* window = NULL;
extern SDL_Renderer* renderer = NULL;

//textures used https://opengameart.org/content/puzzle-game-art
extern SDL_Texture* background_texture = NULL;
extern SDL_Texture* ball_texture = NULL;
extern SDL_Texture* paddle_texture = NULL;
extern SDL_Texture* block_grey_texture = NULL;
extern SDL_Texture* block_red_texture = NULL;
extern SDL_Texture* block_green_texture = NULL;
extern SDL_Texture* block_yellow_texture = NULL;
extern SDL_Texture* block_blue_texture = NULL;
extern SDL_Texture* block_purple_texture = NULL;

extern Mix_Music* music = MUS_NONE;
extern Mix_Chunk* sound_gameOver = MUS_NONE;
extern Mix_Chunk* sound_win = MUS_NONE;
extern Mix_Chunk* sound_bounce = MUS_NONE;

//ball
extern const int BALL_SIZE = 12;
//platform
extern const int PADDLE_WIDTH = 10;
extern const int PADDLE_LENGTH = 90;
//block
extern const int BLOCK_WIDTH = 68;
extern const int BLOCK_HEIGHT = 30;
extern const int PAUSE = 2;
extern const int xMargin = 6;
//game
extern bool start = false;
extern bool paused = false;
extern int mouse_x = 0;
extern int numberOfSolid = 10;
//________________________________STRUCTURES_________________________________________

typedef struct Ball {
    float x;
    float y;
    float xSpeed;
    float ySpeed;
    float xySpeed;
    int size;
} Ball;

typedef struct Paddle {
    float x;
    float y;
    int w;
    int h;
} Paddle;

typedef struct Block {
    float x;
    float y;
    int w;
    int h;
    bool isSolid;
    bool isDestroyed;
} Block;


//___________________________________________FUNCTIONS________________________________________
bool Initialize(void);
void Update(float elapsed);
void ShutDown(void);

SDL_Texture* LoadTexture(char* file);
void LoadSounds(void);
void Initial(Ball *ball, Paddle *paddle, Block blocks[blocksNumber]);
void CollisionDetect(Ball* ball, Paddle* paddle, Block blocks[blocksNumber], float previousLocation_X, float previousLocation_Y);


//ball
int initialDirection(void);
void Direction(Ball*, Paddle*);
void RandomizeBouncing(Ball*, int range);
void UpdateBall(Ball* ball, Paddle*, float elapsed);
void BallStartPosition(Ball*, Paddle*);
Ball NewBall(int size, const Paddle*);
void RenderBall(const Ball*);

//platform
Paddle NewPaddle(int length, int width);
void RenderPaddle(const Paddle*);
void Control(Paddle*);

//blocks

void SolidBlocks(Block blocks[blocksNumber]);
Block NewBlock(int height, int width, int y_num, int x_num);
void NewBlocks(Block blocks[blocksNumber]);
void RenderBlock(Block*, int y_num);
void RenderBlocks(Block blocks[blocksNumber]);

bool is_reachable(Block blocks[blocksNumber]);

#endif