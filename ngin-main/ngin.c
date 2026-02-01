#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define MAP_WIDTH 24
#define MAP_HEIGHT 24

#define cwin SDL_CreateWindow 
#define cren SDL_CreateRenderer
#define tick SDL_GetTicks

#define FAST_INV(x) ((x) > 0 ? (1.0 / (x)) : (1.0 / -(x)))

#define RGB_Red     (ColorRGB){255, 0, 0}
#define RGB_Green   (ColorRGB){0, 255, 0}
#define RGB_Blue    (ColorRGB){0, 0, 255}
#define RGB_White   (ColorRGB){255, 255, 255}
#define RGB_Yellow  (ColorRGB){255, 255, 0}

// fov = 2*atan(planeY/planeX)
//gcc -o ngin ngin.c -lSDL2 -lm

typedef struct{
    double posX,posY,dirX,dirY,planeX,planeY;
    double rotSpeed,moveSpeed; // can be const
    int mapX,mapY;
}playerInfo;

typedef struct{
    Uint32 frameStart;
    Uint32 frameTime;
    int frameCount;
    float fps;
    Uint32 fpsTimer;
}fpsInfo;

typedef struct{
    bool running;
    double cameraX, rayDirX, rayDirY, deltaDistX, deltaDistY, perpWallDist, sideDistX, sideDistY;
    int mapX, mapY, hit, side, stepX, stepY, lineHeight, drawStart, drawEnd;
}gameState;

typedef struct {
    Uint8 r, g, b;
} ColorRGB;

int worldMap[MAP_WIDTH][MAP_HEIGHT] = {
  {1,1,1,1,1,2,2,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
  {4,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
  {4,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
  {4,0,0,0,0,1,1,1,1,3,2,1,0,0,1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

void initPlayerInfo(playerInfo *player, double rotSpeedPlayer, double moveSpeedPlayer, double playerFOV){
    player->posX = 22;
    player->posY = 12;
    player->dirX = -1;
    player->dirY = 0;
    player->planeX = 0;
    player->planeY = playerFOV;
    player->rotSpeed = rotSpeedPlayer;
    player->moveSpeed = moveSpeedPlayer;
}

void initFPSInfo(fpsInfo *fps){
    fps->frameStart = tick(); // Initialize here
    fps->frameTime;
    fps->frameCount = 0;
    fps->fps = 0.0f;
    fps->fpsTimer = tick(); 
}

void calcFPS(fpsInfo *fps, SDL_Window* window){
    // FPS calculation - fix the timing
        Uint32 currentTime = tick();
        fps->frameCount++;
        currentTime = tick();
        if (currentTime - fps->fpsTimer >= 1000) {
            fps->fps = fps->frameCount * 1000.0f / (currentTime - fps->fpsTimer);
            char title[256];
            snprintf(title, sizeof(title), "Raycasting - FPS: %.2f", fps->fps);
            SDL_SetWindowTitle(window, title);
            fps->fpsTimer = currentTime;
            fps->frameCount = 0;
        }
}

void frameRateCap(fpsInfo *fps){
    fps->frameTime = tick() - fps->frameStart;
        if (fps->frameTime < 16) {
            SDL_Delay(16 - fps->frameTime); // Cap at ~60 FPS
        }
}

void playerMoveFunc(playerInfo *player, double deltaTime){
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    double frameSpeed = player->moveSpeed * deltaTime;
    
    // Pre-calculate rotation values only once per frame
    double frameRotSpeed = player->rotSpeed * deltaTime;
    double cosRot = cos(frameRotSpeed);
    double sinRot = sin(frameRotSpeed);
    double cosRotNeg = cos(-frameRotSpeed);
    double sinRotNeg = sin(-frameRotSpeed);
    
    if (keystate[SDL_SCANCODE_W]) {
        if (worldMap[(int)(player->posX + player->dirX * frameSpeed)][(int)player->posY] == 0) player->posX += player->dirX * frameSpeed;
        if (worldMap[(int)player->posX][(int)(player->posY + player->dirY * frameSpeed)] == 0) player->posY += player->dirY * frameSpeed;
    }
    if (keystate[SDL_SCANCODE_S]) {
        if (worldMap[(int)(player->posX - player->dirX * frameSpeed)][(int)player->posY] == 0) player->posX -= player->dirX * frameSpeed;
        if (worldMap[(int)player->posX][(int)(player->posY - player->dirY * frameSpeed)] == 0) player->posY -= player->dirY * frameSpeed;
    }
    if (keystate[SDL_SCANCODE_D]) {
        double oldDirX = player->dirX;
        player->dirX = player->dirX * cosRotNeg - player->dirY * sinRotNeg;
        player->dirY = oldDirX * sinRotNeg + player->dirY * cosRotNeg;
        double oldPlaneX = player->planeX;
        player->planeX = player->planeX * cosRotNeg - player->planeY * sinRotNeg;
        player->planeY = oldPlaneX * sinRotNeg + player->planeY * cosRotNeg;
    }
    if (keystate[SDL_SCANCODE_A]) {
        double oldDirX = player->dirX;
        player->dirX = player->dirX * cosRot - player->dirY * sinRot;
        player->dirY = oldDirX * sinRot + player->dirY * cosRot;
        double oldPlaneX = player->planeX;
        player->planeX = player->planeX * cosRot - player->planeY * sinRot;
        player->planeY = oldPlaneX * sinRot + player->planeY * cosRot;
    }
}

void DDA(gameState *state){
    while (!state->hit) {
        if (state->sideDistX < state->sideDistY) {
            state->sideDistX += state->deltaDistX;
            state->mapX += state->stepX;
            state->side = 0;
        } else {
            state->sideDistY += state->deltaDistY;
            state->mapY += state->stepY;
            state->side = 1;
        }
        if (worldMap[state->mapX][state->mapY] > 0) state->hit = 1;
    }
}

void initRayCast(gameState *state, playerInfo *player, int x){
    state->cameraX = 2 * x / (double)SCREEN_WIDTH - 1;
    state->rayDirX = player->dirX + player->planeX * state->cameraX;
    state->rayDirY = player->dirY + player->planeY * state->cameraX;

    state->mapX = (int)player->posX;
    state->mapY = (int)player->posY;

    // Use fast inverse approximation instead of fabs(1/x)
    state->deltaDistX = FAST_INV(state->rayDirX);
    state->deltaDistY = FAST_INV(state->rayDirY);
    state->hit = 0;
}

void calcDistRayCast(gameState *state, playerInfo *player){
    if (state->rayDirX < 0) {
        state->stepX = -1;
        state->sideDistX = (player->posX - state->mapX) * state->deltaDistX;
    } else {
        state->stepX = 1;
        state->sideDistX = (state->mapX + 1.0 - player->posX) * state->deltaDistX;
    }
    if (state->rayDirY < 0) {
        state->stepY = -1;
        state->sideDistY = (player->posY - state->mapY) * state->deltaDistY;
    } else {
        state->stepY = 1;
        state->sideDistY = (state->mapY + 1.0 - player->posY) * state->deltaDistY;
    }
}

void calcWallHeight(gameState *state, playerInfo *player){
    if (state->side == 0)
        state->perpWallDist = (state->mapX - player->posX + (1 - state->stepX) / 2) / state->rayDirX;
    else
        state->perpWallDist = (state->mapY - player->posY + (1 - state->stepY) / 2) / state->rayDirY;

    state->lineHeight = (int)(SCREEN_HEIGHT / state->perpWallDist);
    state->drawStart = -state->lineHeight / 2 + SCREEN_HEIGHT / 2;
    state->drawEnd = state->lineHeight / 2 + SCREEN_HEIGHT / 2;
    
    if (state->drawStart < 0) state->drawStart = 0;
    if (state->drawEnd >= SCREEN_HEIGHT) state->drawEnd = SCREEN_HEIGHT - 1;
}

ColorRGB divideColor(ColorRGB color, int divisor) {
    return (ColorRGB){color.r / divisor, color.g / divisor, color.b / divisor};
}

void colorWall(ColorRGB *color, gameState *state){
    switch(worldMap[state->mapX][state->mapY])
            {
                case 1:  *color = RGB_Red;    break; //red
                case 2:  *color = RGB_Green;  break; //green
                case 3:  *color = RGB_Blue;   break; //blue
                case 4:  *color = RGB_White;  break; //white
                default: *color = RGB_Yellow; break; //yellow
            }

            // Give x and y sides different brightness
            if (state->side == 1) {
                *color = divideColor(*color, 2);
            }
}

int main(void) {
    playerInfo player;
    fpsInfo fps;
    gameState state;
    ColorRGB color;

    SDL_Window* window = cwin("Raycasting", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = cren(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Init(SDL_INIT_VIDEO);
    initPlayerInfo(&player, 2.0, 3.0, 0.88); // More reasonable speeds for delta-time movement
    initFPSInfo(&fps);

    state.running = true;
    SDL_Event event;
    Uint32 lastTime = tick();
    

    while (state.running) {
        Uint32 currentTime = tick();
        fps.frameStart = currentTime;
        
        // Calculate delta time in seconds (optimized)
        static Uint32 lastTime = 0;
        if (lastTime == 0) lastTime = currentTime; // Initialize on first frame
        double deltaTime = (currentTime - lastTime) * 0.001; // Multiply instead of divide
        lastTime = currentTime;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) state.running = false;
        }

        playerMoveFunc(&player, deltaTime);

        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255); // Sky blue
        SDL_RenderClear(renderer);

        for (int x = 0; x < SCREEN_WIDTH; x++) {
            initRayCast(&state,&player,x);
        
            calcDistRayCast(&state,&player);

            DDA(&state);

            calcWallHeight(&state,&player);

            colorWall(&color,&state);

            // Set the render color and draw the wall line
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            SDL_RenderDrawLine(renderer, x, state.drawStart, x, state.drawEnd);
        }

        calcFPS(&fps,window);

        SDL_RenderPresent(renderer);
        
        
        // frameRateCap(&fps); // - Frame rate limiting
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
