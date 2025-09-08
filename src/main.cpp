#if defined(__EMSCRIPTEN__) || defined(_WIN32)
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include "bitmap_font.h"

// Platform-specific main function handling
#ifdef __EMSCRIPTEN__
#define SDL_MAIN_HANDLED
#include <emscripten.h>
#endif

#ifdef __APPLE__
#define SDL_MAIN_HANDLED
#endif

#ifdef __linux__
#define SDL_MAIN_HANDLED
#endif

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PLAYER_SIZE = 16;
const int PLAYER_SPEED = 5;
const int ATTACK_DURATION = 200; // milliseconds
const int ENEMY_BASE_SIZE = 12;
const float ENEMY_SPEED = 1.5f;
const int MAX_ENEMIES = 20;
const int ENEMY_SPAWN_RATE = 2000; // milliseconds between spawns
const int MAX_ENEMY_LEVEL = 10;
const float ENEMY_KNOCKBACK_DISTANCE = 2.0f; // multiplier of enemy size
const int SHARD_SIZE = 8;
const int MAX_SHARDS = 50;
const int SHARD_LIFETIME = 10000; // milliseconds
const int MAGNET_SIZE = 12;
const int MAX_MAGNETS = 5;
const int MAGNET_LIFETIME = 15000; // milliseconds
const int MAGNET_DROP_CHANCE = 1; // 1 in 100 chance

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

struct Enemy {
    int x, y;
    float speed;
    bool active;
    Uint32 spawnTime;
    int level;
    float knockbackX, knockbackY;
    Uint32 knockbackEndTime;
};

struct Shard {
    int x, y;
    bool active;
    Uint32 spawnTime;
    int value;
    SDL_Color color;
};

struct Magnet {
    int x, y;
    bool active;
    Uint32 spawnTime;
};

// Function to render text using bitmap font
void renderText(SDL_Renderer* renderer, BitmapFont* font, const std::string& text, int x, int y, SDL_Color color) {
    if (!font) {
        // If no font is available, skip text rendering
        return;
    }
    
    font->renderText(renderer, text, x, y, color);
}

// Function to get enemy size based on level
int getEnemySize(int level) {
    return ENEMY_BASE_SIZE + (level - 1) * 2; // Each level adds 2 pixels
}

// Function to get shard properties based on enemy level
void getShardProperties(int enemyLevel, int& value, SDL_Color& color) {
    if (enemyLevel >= 8) {
        // Level 8+ enemies drop purple shards (highest value)
        value = 25;
        color = {128, 0, 128, 255}; // Purple
    } else if (enemyLevel >= 6) {
        // Level 6-7 enemies drop blue shards
        value = 20;
        color = {0, 0, 255, 255}; // Blue
    } else if (enemyLevel >= 4) {
        // Level 4-5 enemies drop green shards
        value = 15;
        color = {0, 255, 0, 255}; // Green
    } else if (enemyLevel >= 2) {
        // Level 2-3 enemies drop orange shards
        value = 10;
        color = {255, 165, 0, 255}; // Orange
    } else {
        // Level 1 enemies drop yellow shards (base value)
        value = 5;
        color = {255, 255, 0, 255}; // Yellow
    }
}

// Global variables for the game loop
SDL_Renderer* g_renderer = nullptr;
SDL_Window* g_window = nullptr;
BitmapFont* g_font = nullptr;
SDL_Texture* g_playerTexture = nullptr;
SDL_Texture* g_enemyTextures[MAX_ENEMY_LEVEL + 1] = {nullptr};
Player g_player{0, 0, DOWN};
Attack g_attack{false, {0, 0, PLAYER_SIZE, PLAYER_SIZE}, 0};
Enemy g_enemies[MAX_ENEMIES];
Shard g_shards[MAX_SHARDS];
Magnet g_magnets[MAX_MAGNETS];
Uint32 g_lastEnemySpawn = 0;
int g_score = 0;
Uint32 g_magnetEffectEndTime = 0;
bool g_quit = false;

// Game loop function for Emscripten
void gameLoop() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            g_quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    g_quit = true;
                    break;
                case SDLK_j:
                    g_attack.active = true;
                    g_attack.startTime = SDL_GetTicks();
                    switch (g_player.dir) {
                        case UP:
                            g_attack.rect = {g_player.x, g_player.y - PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE};
                            break;
                        case DOWN:
                            g_attack.rect = {g_player.x, g_player.y + PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE};
                            break;
                        case LEFT:
                            g_attack.rect = {g_player.x - PLAYER_SIZE, g_player.y, PLAYER_SIZE, PLAYER_SIZE};
                            break;
                        case RIGHT:
                            g_attack.rect = {g_player.x + PLAYER_SIZE, g_player.y, PLAYER_SIZE, PLAYER_SIZE};
                            break;
                    }
                    break;
            }
        }
    }

    // Handle continuous movement with keyboard state
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    float moveX = 0, moveY = 0;
    
    if (keystate[SDL_SCANCODE_W]) {
        moveY -= 1;
        g_player.dir = UP;
    }
    if (keystate[SDL_SCANCODE_S]) {
        moveY += 1;
        g_player.dir = DOWN;
    }
    if (keystate[SDL_SCANCODE_A]) {
        moveX -= 1;
        g_player.dir = LEFT;
    }
    if (keystate[SDL_SCANCODE_D]) {
        moveX += 1;
        g_player.dir = RIGHT;
    }
    
    // Normalize diagonal movement
    if (moveX != 0 && moveY != 0) {
        float length = sqrt(moveX * moveX + moveY * moveY);
        moveX /= length;
        moveY /= length;
    }
    
    // Apply movement
    g_player.x += moveX * PLAYER_SPEED;
    g_player.y += moveY * PLAYER_SPEED;

    if (g_attack.active && SDL_GetTicks() - g_attack.startTime > ATTACK_DURATION) {
        g_attack.active = false;
    }

    // Enemy spawning
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - g_lastEnemySpawn > ENEMY_SPAWN_RATE) {
        // Find an inactive enemy to spawn
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!g_enemies[i].active) {
                // Calculate enemy level based on player's shard count
                int enemyLevel = 1 + (g_score / 10); // Every 10 shards increases enemy level by 1
                if (enemyLevel > MAX_ENEMY_LEVEL) enemyLevel = MAX_ENEMY_LEVEL;
                
                // Randomly choose which edge to spawn from
                int edge = rand() % 4;
                int enemySize = getEnemySize(enemyLevel);
                switch (edge) {
                    case 0: // Top edge
                        g_enemies[i] = {rand() % SCREEN_WIDTH, -enemySize, ENEMY_SPEED, true, currentTime, enemyLevel, 0, 0, 0};
                        break;
                    case 1: // Bottom edge
                        g_enemies[i] = {rand() % SCREEN_WIDTH, SCREEN_HEIGHT, ENEMY_SPEED, true, currentTime, enemyLevel, 0, 0, 0};
                        break;
                    case 2: // Left edge
                        g_enemies[i] = {-enemySize, rand() % SCREEN_HEIGHT, ENEMY_SPEED, true, currentTime, enemyLevel, 0, 0, 0};
                        break;
                    case 3: // Right edge
                        g_enemies[i] = {SCREEN_WIDTH, rand() % SCREEN_HEIGHT, ENEMY_SPEED, true, currentTime, enemyLevel, 0, 0, 0};
                        break;
                }
                g_lastEnemySpawn = currentTime;
                break;
            }
        }
    }

    // Enemy movement and cleanup
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (g_enemies[i].active) {
            int enemySize = getEnemySize(g_enemies[i].level);
            
            // Handle knockback
            if (currentTime < g_enemies[i].knockbackEndTime) {
                g_enemies[i].x += g_enemies[i].knockbackX * 0.1f; // Apply knockback gradually
                g_enemies[i].y += g_enemies[i].knockbackY * 0.1f;
            } else {
                // Normal movement towards player
                float dx = g_player.x - g_enemies[i].x;
                float dy = g_player.y - g_enemies[i].y;
                float distance = sqrt(dx * dx + dy * dy);
                
                if (distance > 0) {
                    // Normalize direction and move towards player
                    dx /= distance;
                    dy /= distance;
                    g_enemies[i].x += dx * g_enemies[i].speed;
                    g_enemies[i].y += dy * g_enemies[i].speed;
                }
            }
            
            // Remove enemies that are too far off screen
            if (g_enemies[i].x < -enemySize * 2 || g_enemies[i].x > SCREEN_WIDTH + enemySize * 2 ||
                g_enemies[i].y < -enemySize * 2 || g_enemies[i].y > SCREEN_HEIGHT + enemySize * 2) {
                g_enemies[i].active = false;
            }
        }
    }

    // Collision detection between attack and enemies
    if (g_attack.active) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (g_enemies[i].active) {
                int enemySize = getEnemySize(g_enemies[i].level);
                SDL_Rect enemyRect = {g_enemies[i].x, g_enemies[i].y, enemySize, enemySize};
                if (SDL_HasIntersection(&g_attack.rect, &enemyRect)) {
                    // Enemy hit by attack - level down and knockback
                    g_enemies[i].level--;
                    
                    // Calculate knockback direction (away from player)
                    float dx = g_enemies[i].x - g_player.x;
                    float dy = g_enemies[i].y - g_player.y;
                    float distance = sqrt(dx * dx + dy * dy);
                    
                    if (distance > 0) {
                        dx /= distance;
                        dy /= distance;
                        float knockbackDist = enemySize * ENEMY_KNOCKBACK_DISTANCE;
                        g_enemies[i].knockbackX = dx * knockbackDist;
                        g_enemies[i].knockbackY = dy * knockbackDist;
                        g_enemies[i].knockbackEndTime = currentTime + 200; // 200ms knockback
                    }
                    
                    // If enemy reaches level 0, drop shard and deactivate
                    if (g_enemies[i].level <= 0) {
                        for (int j = 0; j < MAX_SHARDS; j++) {
                            if (!g_shards[j].active) {
                                int shardValue;
                                SDL_Color shardColor;
                                getShardProperties(g_enemies[i].level + 1, shardValue, shardColor); // Use original level
                                g_shards[j] = {g_enemies[i].x + enemySize/2 - SHARD_SIZE/2, 
                                             g_enemies[i].y + enemySize/2 - SHARD_SIZE/2, 
                                             true, currentTime, shardValue, shardColor};
                                break;
                            }
                        }
                        
                        // 1% chance to drop a magnet
                        if (rand() % 100 < MAGNET_DROP_CHANCE) {
                            for (int k = 0; k < MAX_MAGNETS; k++) {
                                if (!g_magnets[k].active) {
                                    g_magnets[k] = {g_enemies[i].x + enemySize/2 - MAGNET_SIZE/2, 
                                                  g_enemies[i].y + enemySize/2 - MAGNET_SIZE/2, 
                                                  true, currentTime};
                                    break;
                                }
                            }
                        }
                        
                        g_enemies[i].active = false;
                    }
                }
            }
        }
    }

    // Collision detection between player and enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (g_enemies[i].active) {
            int enemySize = getEnemySize(g_enemies[i].level);
            SDL_Rect enemyRect = {g_enemies[i].x, g_enemies[i].y, enemySize, enemySize};
            SDL_Rect playerRect = {g_player.x, g_player.y, PLAYER_SIZE, PLAYER_SIZE};
            if (SDL_HasIntersection(&playerRect, &enemyRect)) {
                // Player hit by enemy - reset player position to center and lose all shards
                g_player.x = SCREEN_WIDTH / 2 - PLAYER_SIZE / 2;
                g_player.y = SCREEN_HEIGHT / 2 - PLAYER_SIZE / 2;
                g_score = 0; // Reset score to 0 when player dies
                // Deactivate the enemy that hit the player
                g_enemies[i].active = false;
            }
        }
    }

    // Shard movement and collection
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (g_shards[i].active) {
            // Move shard towards player only if magnet effect is active
            if (currentTime < g_magnetEffectEndTime) {
                float dx = g_player.x + PLAYER_SIZE/2 - (g_shards[i].x + SHARD_SIZE/2);
                float dy = g_player.y + PLAYER_SIZE/2 - (g_shards[i].y + SHARD_SIZE/2);
                float distance = sqrt(dx * dx + dy * dy);
                
                if (distance > 0) {
                    // Normalize direction and move towards player
                    dx /= distance;
                    dy /= distance;
                    g_shards[i].x += dx * 3.0f; // Shard speed towards player
                    g_shards[i].y += dy * 3.0f;
                }
            }
            
            // Player collection (when shard gets close enough)
            SDL_Rect shardRect = {g_shards[i].x, g_shards[i].y, SHARD_SIZE, SHARD_SIZE};
            SDL_Rect playerRect = {g_player.x, g_player.y, PLAYER_SIZE, PLAYER_SIZE};
            if (SDL_HasIntersection(&playerRect, &shardRect)) {
                g_shards[i].active = false;
                g_score += g_shards[i].value;
            }
        }
    }

    // Magnet cleanup and player collection
    for (int i = 0; i < MAX_MAGNETS; i++) {
        if (g_magnets[i].active) {
            // Remove old magnets
            if (currentTime - g_magnets[i].spawnTime > MAGNET_LIFETIME) {
                g_magnets[i].active = false;
                continue;
            }
            
            // Player collection
            SDL_Rect magnetRect = {g_magnets[i].x, g_magnets[i].y, MAGNET_SIZE, MAGNET_SIZE};
            SDL_Rect playerRect = {g_player.x, g_player.y, PLAYER_SIZE, PLAYER_SIZE};
            if (SDL_HasIntersection(&playerRect, &magnetRect)) {
                g_magnets[i].active = false;
                // Activate magnet effect for 20 seconds
                g_magnetEffectEndTime = currentTime + 20000; // 20 seconds
            }
        }
    }

    // Rendering
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);

    // Render player
    SDL_Rect playerRect = {g_player.x, g_player.y, PLAYER_SIZE, PLAYER_SIZE};
    if (g_playerTexture) {
        SDL_RenderCopy(g_renderer, g_playerTexture, NULL, &playerRect);
    } else {
        SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(g_renderer, &playerRect);
    }

    if (g_attack.active) {
        SDL_SetRenderDrawColor(g_renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(g_renderer, &g_attack.rect);
    }

    // Render enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (g_enemies[i].active) {
            int enemySize = getEnemySize(g_enemies[i].level);
            SDL_Rect enemyRect = {g_enemies[i].x, g_enemies[i].y, enemySize, enemySize};
            
            // Use texture if available, otherwise use colored rectangle
            if (g_enemyTextures[g_enemies[i].level]) {
                SDL_RenderCopy(g_renderer, g_enemyTextures[g_enemies[i].level], NULL, &enemyRect);
            } else {
                // Color enemies based on level (darker red for higher levels)
                int redIntensity = 100 + (g_enemies[i].level * 15);
                if (redIntensity > 255) redIntensity = 255;
                SDL_SetRenderDrawColor(g_renderer, redIntensity, 100, 100, 255);
                SDL_RenderFillRect(g_renderer, &enemyRect);
            }
            
            // Render level number
            SDL_Color white = {255, 255, 255, 255};
            if (g_font) {
                renderText(g_renderer, g_font, std::to_string(g_enemies[i].level), g_enemies[i].x + enemySize/2 - 4, g_enemies[i].y + enemySize/2 - 6, white);
            }
        }
    }

    // Render shards
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (g_shards[i].active) {
            SDL_SetRenderDrawColor(g_renderer, g_shards[i].color.r, g_shards[i].color.g, g_shards[i].color.b, g_shards[i].color.a);
            SDL_Rect shardRect = {g_shards[i].x, g_shards[i].y, SHARD_SIZE, SHARD_SIZE};
            SDL_RenderFillRect(g_renderer, &shardRect);
        }
    }

    // Render magnets
    SDL_SetRenderDrawColor(g_renderer, 0, 255, 255, 255);
    for (int i = 0; i < MAX_MAGNETS; i++) {
        if (g_magnets[i].active) {
            SDL_Rect magnetRect = {g_magnets[i].x, g_magnets[i].y, MAGNET_SIZE, MAGNET_SIZE};
            SDL_RenderFillRect(g_renderer, &magnetRect);
        }
    }

    // Render score
    SDL_Color white = {255, 255, 255, 255};
    if (g_font) {
        renderText(g_renderer, g_font, "Shards: " + std::to_string(g_score), 10, 10, white);
    }

    SDL_RenderPresent(g_renderer);
    
    // Check if we should quit
    if (g_quit) {
        #ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
        #endif
    }
}

int SDL_main(int argc, char* argv[]) {
    // Initialize random seed
    srand(time(NULL));
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        // Don't return here - we can still run without images
    } else {
        std::cout << "SDL_image initialized successfully" << std::endl;
    }

    g_window = SDL_CreateWindow("Simple SDL Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!g_window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) {
        std::cout << "Accelerated renderer failed, trying software renderer..." << std::endl;
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!g_renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 1;
    }

    // Load bitmap font
    g_font = new BitmapFont();
    std::cout << "Attempting to load bitmap font..." << std::endl;
    
    const char* fontPath = "assets/dbyte_1x.png";
    
    if (!g_font->loadFont(g_renderer, fontPath)) {
        std::cerr << "Failed to load bitmap font - text rendering will be disabled" << std::endl;
        delete g_font;
        g_font = nullptr;
    } else {
        std::cout << "Bitmap font loaded successfully!" << std::endl;
    }

    // Load player character image
    const char* playerPath = "assets/char.png";
    
    SDL_Surface* playerSurface = IMG_Load(playerPath);
    if (playerSurface) {
        g_playerTexture = SDL_CreateTextureFromSurface(g_renderer, playerSurface);
        SDL_FreeSurface(playerSurface);
        if (!g_playerTexture) {
            std::cerr << "Failed to create player texture: " << SDL_GetError() << std::endl;
        }
    } else {
        std::cout << "Player image not found, using placeholder rectangle" << std::endl;
    }

    // Load enemy textures for each level
    for (int level = 1; level <= MAX_ENEMY_LEVEL; level++) {
        std::string filename = "assets/enemy" + std::to_string(level) + ".png";
        
        SDL_Surface* enemySurface = IMG_Load(filename.c_str());
        if (enemySurface) {
            g_enemyTextures[level] = SDL_CreateTextureFromSurface(g_renderer, enemySurface);
            SDL_FreeSurface(enemySurface);
            if (!g_enemyTextures[level]) {
                std::cerr << "Failed to create enemy texture for level " << level << ": " << SDL_GetError() << std::endl;
            }
        }
    }

    // Initialize game state
    g_player = {SCREEN_WIDTH / 2 - PLAYER_SIZE / 2, SCREEN_HEIGHT / 2 - PLAYER_SIZE / 2, DOWN};
    g_attack = {false, {0, 0, PLAYER_SIZE, PLAYER_SIZE}, 0};
    
    // Initialize enemies array
    for (int i = 0; i < MAX_ENEMIES; i++) {
        g_enemies[i] = {0, 0, ENEMY_SPEED, false, 0, 1, 0, 0, 0};
    }
    
    // Initialize shards array
    for (int i = 0; i < MAX_SHARDS; i++) {
        g_shards[i] = {0, 0, false, 0, 5, {255, 255, 0, 255}}; // Default yellow shard
    }
    
    // Initialize magnets array
    for (int i = 0; i < MAX_MAGNETS; i++) {
        g_magnets[i] = {0, 0, false, 0};
    }
    
    g_lastEnemySpawn = 0;
    g_score = 0;
    g_magnetEffectEndTime = 0;
    g_quit = false;

    #ifdef __EMSCRIPTEN__
    // Set up the main loop for Emscripten
    emscripten_set_main_loop(gameLoop, 0, 1);
    #else
    // Standard desktop game loop
    while (!g_quit) {
        gameLoop();
        SDL_Delay(16);
    }
    #endif

    // Cleanup
    if (g_playerTexture) {
        SDL_DestroyTexture(g_playerTexture);
    }
    
    // Clean up enemy textures
    for (int level = 1; level <= MAX_ENEMY_LEVEL; level++) {
        if (g_enemyTextures[level]) {
            SDL_DestroyTexture(g_enemyTextures[level]);
        }
    }
    
    if (g_font) {
        delete g_font;
    }
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

// Platform-specific wrapper main function
#ifdef __EMSCRIPTEN__
int main(int argc, char* argv[]) {
    return SDL_main(argc, argv);
}
#endif

#ifdef __APPLE__
int main(int argc, char* argv[]) {
    return SDL_main(argc, argv);
}
#endif

#ifdef __linux__
int main(int argc, char* argv[]) {
    return SDL_main(argc, argv);
}
#endif
