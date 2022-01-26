#include "Header1.h"
//napraw kolizje z dwoma blokami jednoczesnie
//menu by sie przydalo
//podzial na moduly

Paddle paddle;
Ball ball;
Block blocks[blocksNumber];

//___________________________________MAIN______________________________

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));
    atexit(&ShutDown);
    if (!Initialize()) {
        exit(1);
    }

    bool quit = false;
    SDL_Event event;

    // get the number of milliseconds since SDL library initialization
    Uint32 last_tick = SDL_GetTicks();

    // game loop
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                if (start) {
                    paused = !paused;
                }
            }
            if (event.type == SDL_MOUSEMOTION)
            {
                SDL_GetMouseState(&mouse_x, NULL);
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
            {
                quit = true;
                ShutDown();
            }
        }
        Uint32 current_tick = SDL_GetTicks();
        Uint32 difference = current_tick - last_tick;
        float elapsed = difference / 1000.0f;
        Update(elapsed);
        last_tick = current_tick;
    }

    SDL_Quit();
    return 0;
}


// INITIALISATION
bool Initialize(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        printf ("Error initializing SDL_mixer: %s\n", Mix_GetError());
        return false;
    }
    //creating window
    window = SDL_CreateWindow("Arkanoid", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        return false;
    }
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    background_texture = LoadTexture("textures\\background.png");
    paddle_texture = LoadTexture("textures\\paddleRed.png");
    ball_texture = LoadTexture("textures\\ballGrey.png");
    block_grey_texture = LoadTexture("textures\\blockGrey.png");
    block_red_texture = LoadTexture("textures\\blockRed.png");
    block_yellow_texture = LoadTexture("textures\\blockYellow.png");
    block_green_texture = LoadTexture("textures\\blockGreen.png");
    block_blue_texture = LoadTexture("textures\\blockBlue.png");
    block_purple_texture = LoadTexture("textures\\blockPurple.png");

    LoadSounds();

    Initial(&ball, &paddle, blocks);

    return true;
}

SDL_Texture* LoadTexture(char* file) {
    SDL_Surface *image = IMG_Load(file);
    SDL_Texture *texture;
    if (!image) {
        printf("Failed to load image.png: %s\n", IMG_GetError());
    }

    texture = SDL_CreateTextureFromSurface(renderer, image);
    SDL_FreeSurface(image);
    image = NULL;

    if (!texture) {
        printf("Error creating texture: %s\n", SDL_GetError());
        return false;
    }

    return  texture;
}

void LoadSounds() {
  //music
  /* music = Mix_LoadMUS("music.wav");
   if (!music) {
       printf ("Error loading music: %s\n", Mix_GetError());
       return false;
   }*/

   // Load sounds
    sound_gameOver = Mix_LoadMUS("sounds/game_over.mp3");
    if (!sound_gameOver) {
        printf("Error loading game over sound: %s\n", Mix_GetError());
        return false;
    }
    sound_win = Mix_LoadMUS("sounds/win.mp3");
    if (!sound_win) {
        printf("Error loading win sound: %s\n", Mix_GetError());
        return false;
    }
    sound_bounce = Mix_LoadMUS("sounds/bounce.mp3");
    if (!sound_bounce) {
        printf("Error loading bounce sound: %s\n", Mix_GetError());
        return false;
    }

    // Play music forever
    //Mix_PlayMusic(music, -1);
}

//UPDATE
void Update(float elapsed) {
    // changing color of the window
    SDL_SetRenderDrawColor(renderer, 5, 10, 20, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, background_texture, NULL, NULL);

    // drawing elements
    float previousLocation_X = ball.x;
    float previousLocation_Y = ball.y;
    
    UpdateBall(&ball, &paddle, elapsed);
    CollisionDetect(&ball, &paddle, blocks, previousLocation_X, previousLocation_Y);
    RenderBlocks(blocks);
    RenderPaddle(&paddle);
    Control(&paddle);
    RenderBall(&ball);

    SDL_RenderPresent(renderer);
}

// SHUTDOWN
void ShutDown(void) {
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    
    Mix_FreeChunk(sound_bounce);
    Mix_FreeChunk(sound_gameOver);
    Mix_FreeChunk(sound_win);
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}


void Initial(Ball *ball, Paddle* paddle, Block blocks[blocksNumber]) {
    NewBlocks(blocks);
    SolidBlocks(blocks);
    while (!is_reachable(blocks)) {
        NewBlocks(blocks);
        SolidBlocks(blocks);
    }
    *paddle = NewPaddle(PADDLE_LENGTH, PADDLE_WIDTH);
    *ball = NewBall(BALL_SIZE, paddle);
    start = false;
}


// BALL
int initialDirection(void) {
    return rand() % 2 == 1 ? 1 : -1;
}

// direction depends on position of the ball relative to the platform 
void Direction(Ball* ball, Paddle* paddle) {
    float xySpeed = ball->xySpeed;

    // position of the ball: -1->right edge, 1-> left edge, 0-> center of the platform
    float delta_x = ((ball->x + ball->size / 2) - (paddle->x + paddle->w / 2)) / (paddle->w / 2); 
    float speedControl = 0.8f;
    
    int sign = delta_x < 0 ? -1 : 1;
    // scaling a range to another
    ball->xSpeed = sign * (((fabs)(delta_x) * speedControl * xySpeed)*((double)xySpeed - 100.0f)/xySpeed + 100); 
    ball->ySpeed = sqrt((double)xySpeed * xySpeed - (double)ball->xSpeed * ball->xSpeed) * (ball->ySpeed > 0 ? -1 : 1);
}

void RandomizeBouncing(Ball* ball, int range) {
    float xySpeed = ball->xySpeed;
    ball->xSpeed += (rand() % 2 == 1 ? 1 : -1) * rand() % range;
    ball->ySpeed = sqrt((double)xySpeed * xySpeed - (double)ball->xSpeed * ball->xSpeed);
}

void UpdateBall(Ball* ball, Paddle *paddle, float elapsed) {
    if (start && !paused) {
        ball->x += ball->xSpeed * elapsed;
        ball->y += ball->ySpeed * elapsed;
    }
}

void BallStartPosition(Ball* ball, Paddle* paddle) {
    ball->x = paddle->x + paddle->w / 2 - ball->size / 2;
}

// COLLISION
void CollisionDetect(Ball* ball, Paddle* paddle, Block blocks[blocksNumber], float previous_X, float previous_Y) {

    // with window
    if ((ball->x + ball->size) > WINDOW_WIDTH) {
        if (!Mix_PlayingMusic())
            Mix_PlayMusic(sound_bounce, 1);
        ball->xSpeed *= -1;
        ball->x = WINDOW_WIDTH - ball->size;
    }
    else if (ball->x < 0) {
        if (!Mix_PlayingMusic())
            Mix_PlayMusic(sound_bounce, 1);
        ball->xSpeed *= -1;
        ball->x = 0;
    }
    if ((ball->y + ball->size) > WINDOW_HEIGHT) {
        // game over
        Mix_PlayMusic(sound_gameOver, 1);
        Initial(ball,paddle,blocks);
    }
    else if (ball->y < 0) {
        if (!Mix_PlayingMusic())
            Mix_PlayMusic(sound_bounce, 1);
        ball->ySpeed *= -1;
        ball->y = 0;
    }
    
    //platform
    if ((paddle->x + paddle->w) > WINDOW_WIDTH) {
        paddle->x = WINDOW_WIDTH - paddle->w;
    }
    else if (paddle->x < 0) {
        paddle->x = 0;
    }
    
    if (!start && !paused) {
        BallStartPosition(ball, paddle);
    }

    //ball with platform
    if (((ball->x + ball->size) > paddle->x) && (ball->x < (paddle->x + paddle->w)) && ((ball->y + ball->size) > paddle->y) && (ball->y < (paddle->y + paddle->h))) { // if collides with platform
        if (!Mix_PlayingMusic())
            Mix_PlayMusic(sound_bounce, 1);
        if ((ball->x + ball->size - ball->size / 2) < paddle->x) { // collides with the left of the platfrom
            ball->xSpeed = -(abs(ball->xSpeed));
            ball->x = paddle->x - ball->size;
        }
        else if ((ball->x + (ball->size) / 5) > (paddle->x + paddle->w)) { // collides with the right of the platform
            ball->xSpeed = -(abs(ball->xSpeed));
            ball->x = paddle->x + paddle->w + 1;
        }
        else {
            Direction(ball, paddle);
            ball->y = paddle->y - ball->size;
        }
    }

    //with blocks_______________________________________________________________________!!!
    bool allDestroyed = true;
    for (int index = 0; index < blocksNumber; index++) {
        if (!(blocks[index].isDestroyed) || blocks[index].isSolid) { // if block exist
            if (((ball->x + ball->size) > (blocks[index].x)) && (ball->x < (blocks[index].x + blocks[index].w)) && ((ball->y + ball->size) > blocks[index].y) && (ball->y < (blocks[index].y + blocks[index].h))) { // if ball collides with block
                // to avoid problem, when the ball collides with two blocks
                if ((((ball->x + ball->size / 2) < (blocks[index].x + blocks[index].w)) && ((ball->x + ball->size / 2) > blocks[index].x)) || (((ball->y + ball->size / 2) < (blocks[index].y + blocks[index].y)) && ((ball->y + ball->size / 2) > blocks[index].y))) {
                     Mix_PlayMusic(sound_bounce, 1);
                    if (((previous_X + ball->size) < blocks[index].x) && (previous_Y > blocks[index].y) && ((previous_Y + ball->size) < (blocks[index].y + blocks[index].h))) { // collides with the left of the block
                        ball->xSpeed *= -1;
                    }
                    else if ((previous_X > (blocks[index].x + blocks[index].w)) && ((previous_Y > blocks[index].y) || ((previous_Y + ball->size) < (blocks[index].y + blocks[index].h)))) { // collides with the right of the block
                        ball->xSpeed *= -1;
                    }
                    else if ((previous_Y > (blocks[index].y + blocks[index].h)) && (((previous_X + ball->size) > blocks[index].x) || (previous_X  < (blocks[index].x + blocks[index].w)))) { // collides with the bottom of the block
                        ball->ySpeed *= -1;
                    }
                    else if (((previous_Y + ball->size) < blocks[index].y) && (((previous_X + ball->size) > blocks[index].x) || (previous_X < (blocks[index].x + blocks[index].w)))) { // collides with the top of the block
                        ball->ySpeed *= -1;
                    }
                    else {
                        ball->ySpeed *= -1;
                        ball->xSpeed *= -1;
                        RandomizeBouncing(ball, 50);
                    }
                    ball->x = previous_X;
                    ball->y = previous_Y;
                    blocks[index].isDestroyed = true;
                }
            }
            if (!blocks[index].isSolid) {
                allDestroyed = false;
            }
        }
    }
    //win
    if (allDestroyed) {
        Mix_PlayMusic(sound_win, 1);
        Initial(ball, paddle, blocks);
    }
}

Ball NewBall(int size, const Paddle* paddle) {
    const float Speed =  270.0f;
    Ball ball = {
        .x = paddle->x + (paddle->w) / 2,
        .y = paddle->y - (paddle->h) - size,
        .size = size,
        .xSpeed = Speed * initialDirection(),
        .ySpeed = -Speed,
        .xySpeed = sqrt(2* (double)Speed * Speed),
    };
    return ball;
}

void RenderBall(const Ball* ball) {
    int size = ball->size;
    int halfSize = size / 2;
    SDL_Rect rect = {
        .x = ball->x,
        .y = ball->y,
        .w = size,
        .h = size,
    };
    SDL_RenderCopy(renderer, ball_texture, NULL, &rect);
}

//PLATFORM
Paddle NewPaddle(int length, int width) {
    Paddle paddle = {
        .x = WINDOW_WIDTH / 2 - length / 2,
        .y = WINDOW_HEIGHT - length,
        .w = length,
        .h = width,
    };
    return paddle;
}

void RenderPaddle(const Paddle* paddle) {
    SDL_Rect rect = {
        .x = paddle->x,
        .y = paddle->y,
        .w = paddle->w,
        .h = paddle->h,
    };
 
    SDL_RenderCopy(renderer, paddle_texture, NULL, &rect);
}

void Control(Paddle* paddle) {
    const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);
   
    if (keyboardState[SDL_SCANCODE_SPACE]) {
        start = true;
    }
    
    if (!paused) {
        paddle->x = mouse_x;
    }
}


//BLOCK
void SolidBlocks(Block blocks[blocksNumber]) {
    int number = (rand() % numberOfSolid/2)+ numberOfSolid/2;
    while (number > 0) {
        if (!blocks[(rand() % (blocksNumber))].isSolid) {
            blocks[(rand() % (blocksNumber))].isSolid = true;
            number--;
        }
    }
}

Block NewBlock(int height, int width, int y_num, int x_num) {
    Block block = {
        .x = xMargin + PAUSE * x_num + BLOCK_WIDTH * x_num,
        .y = PAUSE * (y_num + 1) + BLOCK_HEIGHT * y_num,
        .w = width,
        .h = height,
        .isDestroyed = false,
        .isSolid = false,
    };
    return block;
}


void RenderBlock(Block* block, int y_num) {
    SDL_Rect rect = {
        .x = block->x,
        .y = block->y,
        .w = block->w,
        .h = block->h,
    };
    SDL_Texture* texture = NULL;
    if (block->isSolid) {
        texture = block_grey_texture;
    }
    else {
        switch (y_num)
        {
            case 1:
                texture = block_red_texture;
                break;
            case 2:
                texture = block_yellow_texture;
                break;
            case 3:
                texture = block_green_texture;
                break;
            case 4:
                texture = block_blue_texture;
                break;
            case 5:
                texture = block_purple_texture;
                break;
        }
    }
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    
}

void NewBlocks(Block blocks[blocksNumber]) {
    int h = 0;
    int w = 0;
    for (int index = 0; index < blocksNumber; index++) {
        if (w == inWidth) {
            w = 0;
            h++;
        }
        blocks[index] = NewBlock(BLOCK_HEIGHT, BLOCK_WIDTH, h, w);
        w++;
    }
}

void RenderBlocks(Block blocks[blocksNumber]) {
    int h = 0;
    int w = 0;
    for (int index = 0; index < blocksNumber; index++) {
        if (w == inWidth) {
            w = 0;
            h++;
        }
        if ((!(blocks[index].isDestroyed)) || (blocks[index].isSolid)) {
            RenderBlock(&blocks[index], h + 1);
        }
        w++;
    }
}

bool is_reachable(Block blocks[blocksNumber]) {
    bool reachable[blocksNumber];
    // initialisation of array
    for (int i = 0; i < blocksNumber; i++) {
        if (blocks[i].isSolid || i < (blocksNumber-inWidth)) {
            reachable[i] = false;
        }
        else {
            reachable[i] = true;
        }
    }

    for (int i = (blocksNumber - inWidth - 1); i >= 0; i--) {
        if (((i + inWidth < blocksNumber && reachable[i + inWidth]) || ((i + 1) % inWidth != 0 && reachable[i + 1])) && !blocks[i].isSolid) {
            reachable[i] = true;
        }
    }

    for (int i = 0; i <blocksNumber; i++) {
        if (!reachable[i] && !blocks[i].isSolid) {
            if ((i - inWidth >= 0 && reachable[i - inWidth]) || ((i - 1) > 0 && (i - 1) % inWidth != (inWidth-1) && reachable[i - 1])) {
                reachable[i] = true;
            }
            else {
                return false;
            }
        }
    }
    return true;
}