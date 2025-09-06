#include <SDL2/SDL.h>
#include <iostream>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PLAYER_SIZE = 32;
const int PLAYER_SPEED = 5;
const int ATTACK_DURATION = 200; // milliseconds

enum Direction { UP, DOWN, LEFT, RIGHT };

struct Player {
    int x, y;
    Direction dir;
};

struct Attack {
    bool active;
    SDL_Rect rect;
    Uint32 startTime;
};

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("Simple SDL Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!win) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!ren) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    Player player{SCREEN_WIDTH / 2 - PLAYER_SIZE / 2, SCREEN_HEIGHT / 2 - PLAYER_SIZE / 2, DOWN};
    Attack attack{false, {0, 0, PLAYER_SIZE, PLAYER_SIZE}, 0};

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_w:
                        player.y -= PLAYER_SPEED;
                        player.dir = UP;
                        break;
                    case SDLK_s:
                        player.y += PLAYER_SPEED;
                        player.dir = DOWN;
                        break;
                    case SDLK_a:
                        player.x -= PLAYER_SPEED;
                        player.dir = LEFT;
                        break;
                    case SDLK_d:
                        player.x += PLAYER_SPEED;
                        player.dir = RIGHT;
                        break;
                    case SDLK_j:
                        attack.active = true;
                        attack.startTime = SDL_GetTicks();
                        switch (player.dir) {
                            case UP:
                                attack.rect = {player.x, player.y - PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE};
                                break;
                            case DOWN:
                                attack.rect = {player.x, player.y + PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE};
                                break;
                            case LEFT:
                                attack.rect = {player.x - PLAYER_SIZE, player.y, PLAYER_SIZE, PLAYER_SIZE};
                                break;
                            case RIGHT:
                                attack.rect = {player.x + PLAYER_SIZE, player.y, PLAYER_SIZE, PLAYER_SIZE};
                                break;
                        }
                        break;
                }
            }
        }

        if (attack.active && SDL_GetTicks() - attack.startTime > ATTACK_DURATION) {
            attack.active = false;
        }

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        SDL_Rect playerRect = {player.x, player.y, PLAYER_SIZE, PLAYER_SIZE};
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderFillRect(ren, &playerRect);

        if (attack.active) {
            SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
            SDL_RenderFillRect(ren, &attack.rect);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
