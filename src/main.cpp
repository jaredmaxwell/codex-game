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

    // Test file system access in Emscripten
    #ifdef __EMSCRIPTEN__
    std::cout << "Emscripten build detected - testing file system access..." << std::endl;
    FILE* testFile = fopen("assets/dbyte_1x.png", "rb");
    if (testFile) {
        std::cout << "File system access successful - assets/dbyte_1x.png is accessible" << std::endl;
        fclose(testFile);
    } else {
        std::cerr << "File system access failed - assets/dbyte_1x.png is not accessible" << std::endl;
    }
    #endif

    // Load bitmap font
    BitmapFont* font = new BitmapFont();
    std::cout << "Attempting to load bitmap font..." << std::endl;
    if (!font->loadFont(ren, "assets/dbyte_1x.png")) {
        std::cerr << "Failed to load bitmap font - text rendering will be disabled" << std::endl;
        std::cerr << "Renderer info: " << (ren ? "valid" : "null") << std::endl;
        delete font;
        font = nullptr;
    } else {
        std::cout << "Bitmap font loaded successfully!" << std::endl;
    }

    // Load player character image
    SDL_Texture* playerTexture = nullptr;
    SDL_Surface* playerSurface = IMG_Load("assets/char.png");
    if (playerSurface) {
        playerTexture = SDL_CreateTextureFromSurface(ren, playerSurface);
        SDL_FreeSurface(playerSurface);
        if (!playerTexture) {
            std::cerr << "Failed to create player texture: " << SDL_GetError() << std::endl;
        }
    } else {
        std::cout << "Player image not found, using placeholder rectangle" << std::endl;
    }

    // Load enemy textures for each level
    SDL_Texture* enemyTextures[MAX_ENEMY_LEVEL + 1] = {nullptr}; // +1 because levels are 1-based
    for (int level = 1; level <= MAX_ENEMY_LEVEL; level++) {
        std::string filename = "assets/enemy" + std::to_string(level) + ".png";
        SDL_Surface* enemySurface = IMG_Load(filename.c_str());
        if (enemySurface) {
            enemyTextures[level] = SDL_CreateTextureFromSurface(ren, enemySurface);
            SDL_FreeSurface(enemySurface);
            if (!enemyTextures[level]) {
                std::cerr << "Failed to create enemy texture for level " << level << ": " << SDL_GetError() << std::endl;
            }
        }
    }

    Player player{SCREEN_WIDTH / 2 - PLAYER_SIZE / 2, SCREEN_HEIGHT / 2 - PLAYER_SIZE / 2, DOWN};
    Attack attack{false, {0, 0, PLAYER_SIZE, PLAYER_SIZE}, 0};
    
    // Initialize enemies array
    Enemy enemies[MAX_ENEMIES];
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i] = {0, 0, ENEMY_SPEED, false, 0, 1, 0, 0, 0};
    }
    
    // Initialize shards array
    Shard shards[MAX_SHARDS];
    for (int i = 0; i < MAX_SHARDS; i++) {
        shards[i] = {0, 0, false, 0, 5, {255, 255, 0, 255}}; // Default yellow shard
    }
    
    // Initialize magnets array
    Magnet magnets[MAX_MAGNETS];
    for (int i = 0; i < MAX_MAGNETS; i++) {
        magnets[i] = {0, 0, false, 0};
    }
    
    Uint32 lastEnemySpawn = 0;
    int score = 0;
    Uint32 magnetEffectEndTime = 0; // When the magnet effect expires

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

        // Handle continuous movement with keyboard state
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        float moveX = 0, moveY = 0;
        
        if (keystate[SDL_SCANCODE_W]) {
            moveY -= 1;
            player.dir = UP;
        }
        if (keystate[SDL_SCANCODE_S]) {
            moveY += 1;
            player.dir = DOWN;
        }
        if (keystate[SDL_SCANCODE_A]) {
            moveX -= 1;
            player.dir = LEFT;
        }
        if (keystate[SDL_SCANCODE_D]) {
            moveX += 1;
            player.dir = RIGHT;
        }
        
        // Normalize diagonal movement
        if (moveX != 0 && moveY != 0) {
            float length = sqrt(moveX * moveX + moveY * moveY);
            moveX /= length;
            moveY /= length;
        }
        
        // Apply movement
        player.x += moveX * PLAYER_SPEED;
        player.y += moveY * PLAYER_SPEED;

        if (attack.active && SDL_GetTicks() - attack.startTime > ATTACK_DURATION) {
            attack.active = false;
        }

        // Enemy spawning
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastEnemySpawn > ENEMY_SPAWN_RATE) {
            // Find an inactive enemy to spawn
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].active) {
                    // Calculate enemy level based on player's shard count
                    int enemyLevel = 1 + (score / 10); // Every 10 shards increases enemy level by 1
                    if (enemyLevel > MAX_ENEMY_LEVEL) enemyLevel = MAX_ENEMY_LEVEL;
                    
                    // Randomly choose which edge to spawn from
                    int edge = rand() % 4;
                    int enemySize = getEnemySize(enemyLevel);
                    switch (edge) {
                        case 0: // Top edge
                            enemies[i] = {rand() % SCREEN_WIDTH, -enemySize, ENEMY_SPEED, true, currentTime, enemyLevel, 0, 0, 0};
                            break;
                        case 1: // Bottom edge
                            enemies[i] = {rand() % SCREEN_WIDTH, SCREEN_HEIGHT, ENEMY_SPEED, true, currentTime, enemyLevel, 0, 0, 0};
                            break;
                        case 2: // Left edge
                            enemies[i] = {-enemySize, rand() % SCREEN_HEIGHT, ENEMY_SPEED, true, currentTime, enemyLevel, 0, 0, 0};
                            break;
                        case 3: // Right edge
                            enemies[i] = {SCREEN_WIDTH, rand() % SCREEN_HEIGHT, ENEMY_SPEED, true, currentTime, enemyLevel, 0, 0, 0};
                            break;
                    }
                    lastEnemySpawn = currentTime;
                    break;
                }
            }
        }

        // Enemy movement and cleanup
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                int enemySize = getEnemySize(enemies[i].level);
                
                // Handle knockback
                if (currentTime < enemies[i].knockbackEndTime) {
                    enemies[i].x += enemies[i].knockbackX * 0.1f; // Apply knockback gradually
                    enemies[i].y += enemies[i].knockbackY * 0.1f;
                } else {
                    // Normal movement towards player
                    float dx = player.x - enemies[i].x;
                    float dy = player.y - enemies[i].y;
                    float distance = sqrt(dx * dx + dy * dy);
                    
                    if (distance > 0) {
                        // Normalize direction and move towards player
                        dx /= distance;
                        dy /= distance;
                        enemies[i].x += dx * enemies[i].speed;
                        enemies[i].y += dy * enemies[i].speed;
                    }
                }
                
                // Remove enemies that are too far off screen
                if (enemies[i].x < -enemySize * 2 || enemies[i].x > SCREEN_WIDTH + enemySize * 2 ||
                    enemies[i].y < -enemySize * 2 || enemies[i].y > SCREEN_HEIGHT + enemySize * 2) {
                    enemies[i].active = false;
                }
            }
        }

        // Collision detection between attack and enemies
        if (attack.active) {
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    int enemySize = getEnemySize(enemies[i].level);
                    SDL_Rect enemyRect = {enemies[i].x, enemies[i].y, enemySize, enemySize};
                    if (SDL_HasIntersection(&attack.rect, &enemyRect)) {
                        // Enemy hit by attack - level down and knockback
                        enemies[i].level--;
                        
                        // Calculate knockback direction (away from player)
                        float dx = enemies[i].x - player.x;
                        float dy = enemies[i].y - player.y;
                        float distance = sqrt(dx * dx + dy * dy);
                        
                        if (distance > 0) {
                            dx /= distance;
                            dy /= distance;
                            float knockbackDist = enemySize * ENEMY_KNOCKBACK_DISTANCE;
                            enemies[i].knockbackX = dx * knockbackDist;
                            enemies[i].knockbackY = dy * knockbackDist;
                            enemies[i].knockbackEndTime = currentTime + 200; // 200ms knockback
                        }
                        
                        // If enemy reaches level 0, drop shard and deactivate
                        if (enemies[i].level <= 0) {
                            for (int j = 0; j < MAX_SHARDS; j++) {
                                if (!shards[j].active) {
                                    int shardValue;
                                    SDL_Color shardColor;
                                    getShardProperties(enemies[i].level + 1, shardValue, shardColor); // Use original level
                                    shards[j] = {enemies[i].x + enemySize/2 - SHARD_SIZE/2, 
                                               enemies[i].y + enemySize/2 - SHARD_SIZE/2, 
                                               true, currentTime, shardValue, shardColor};
                                    break;
                                }
                            }
                            
                            // 1% chance to drop a magnet
                            if (rand() % 100 < MAGNET_DROP_CHANCE) {
                                for (int k = 0; k < MAX_MAGNETS; k++) {
                                    if (!magnets[k].active) {
                                        magnets[k] = {enemies[i].x + enemySize/2 - MAGNET_SIZE/2, 
                                                    enemies[i].y + enemySize/2 - MAGNET_SIZE/2, 
                                                    true, currentTime};
                                        break;
                                    }
                                }
                            }
                            
                            enemies[i].active = false;
                        }
                    }
                }
            }
        }

        // Collision detection between player and enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                int enemySize = getEnemySize(enemies[i].level);
                SDL_Rect enemyRect = {enemies[i].x, enemies[i].y, enemySize, enemySize};
                SDL_Rect playerRect = {player.x, player.y, PLAYER_SIZE, PLAYER_SIZE};
                if (SDL_HasIntersection(&playerRect, &enemyRect)) {
                    // Player hit by enemy - reset player position to center and lose all shards
                    player.x = SCREEN_WIDTH / 2 - PLAYER_SIZE / 2;
                    player.y = SCREEN_HEIGHT / 2 - PLAYER_SIZE / 2;
                    score = 0; // Reset score to 0 when player dies
                    // Deactivate the enemy that hit the player
                    enemies[i].active = false;
                }
            }
        }

        // Shard movement and collection
        for (int i = 0; i < MAX_SHARDS; i++) {
            if (shards[i].active) {
                // Move shard towards player only if magnet effect is active
                if (currentTime < magnetEffectEndTime) {
                    float dx = player.x + PLAYER_SIZE/2 - (shards[i].x + SHARD_SIZE/2);
                    float dy = player.y + PLAYER_SIZE/2 - (shards[i].y + SHARD_SIZE/2);
                    float distance = sqrt(dx * dx + dy * dy);
                    
                    if (distance > 0) {
                        // Normalize direction and move towards player
                        dx /= distance;
                        dy /= distance;
                        shards[i].x += dx * 3.0f; // Shard speed towards player
                        shards[i].y += dy * 3.0f;
                    }
                }
                
                // Player collection (when shard gets close enough)
                SDL_Rect shardRect = {shards[i].x, shards[i].y, SHARD_SIZE, SHARD_SIZE};
                SDL_Rect playerRect = {player.x, player.y, PLAYER_SIZE, PLAYER_SIZE};
                if (SDL_HasIntersection(&playerRect, &shardRect)) {
                    shards[i].active = false;
                    score += shards[i].value;
                }
            }
        }

        // Magnet cleanup and player collection
        for (int i = 0; i < MAX_MAGNETS; i++) {
            if (magnets[i].active) {
                // Remove old magnets
                if (currentTime - magnets[i].spawnTime > MAGNET_LIFETIME) {
                    magnets[i].active = false;
                    continue;
                }
                
                // Player collection
                SDL_Rect magnetRect = {magnets[i].x, magnets[i].y, MAGNET_SIZE, MAGNET_SIZE};
                SDL_Rect playerRect = {player.x, player.y, PLAYER_SIZE, PLAYER_SIZE};
                if (SDL_HasIntersection(&playerRect, &magnetRect)) {
                    magnets[i].active = false;
                    // Activate magnet effect for 20 seconds
                    magnetEffectEndTime = currentTime + 20000; // 20 seconds
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        // Render player
        SDL_Rect playerRect = {player.x, player.y, PLAYER_SIZE, PLAYER_SIZE};
        if (playerTexture) {
            SDL_RenderCopy(ren, playerTexture, NULL, &playerRect);
        } else {
            SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
            SDL_RenderFillRect(ren, &playerRect);
        }

        if (attack.active) {
            SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
            SDL_RenderFillRect(ren, &attack.rect);
        }

        // Render enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                int enemySize = getEnemySize(enemies[i].level);
                SDL_Rect enemyRect = {enemies[i].x, enemies[i].y, enemySize, enemySize};
                
                // Use texture if available, otherwise use colored rectangle
                if (enemyTextures[enemies[i].level]) {
                    SDL_RenderCopy(ren, enemyTextures[enemies[i].level], NULL, &enemyRect);
                } else {
                    // Color enemies based on level (darker red for higher levels)
                    int redIntensity = 100 + (enemies[i].level * 15);
                    if (redIntensity > 255) redIntensity = 255;
                    SDL_SetRenderDrawColor(ren, redIntensity, 100, 100, 255);
                    SDL_RenderFillRect(ren, &enemyRect);
                }
                
                // Render level number
                SDL_Color white = {255, 255, 255, 255};
                if (font) {
                    renderText(ren, font, std::to_string(enemies[i].level), enemies[i].x + enemySize/2 - 4, enemies[i].y + enemySize/2 - 6, white);
                }
            }
        }

        // Render shards
        for (int i = 0; i < MAX_SHARDS; i++) {
            if (shards[i].active) {
                SDL_SetRenderDrawColor(ren, shards[i].color.r, shards[i].color.g, shards[i].color.b, shards[i].color.a);
                SDL_Rect shardRect = {shards[i].x, shards[i].y, SHARD_SIZE, SHARD_SIZE};
                SDL_RenderFillRect(ren, &shardRect);
            }
        }

        // Render magnets
        SDL_SetRenderDrawColor(ren, 0, 255, 255, 255);
        for (int i = 0; i < MAX_MAGNETS; i++) {
            if (magnets[i].active) {
                SDL_Rect magnetRect = {magnets[i].x, magnets[i].y, MAGNET_SIZE, MAGNET_SIZE};
                SDL_RenderFillRect(ren, &magnetRect);
            }
        }

        // Render score
        SDL_Color white = {255, 255, 255, 255};
        if (font) {
            renderText(ren, font, "Shards: " + std::to_string(score), 10, 10, white);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    if (playerTexture) {
        SDL_DestroyTexture(playerTexture);
    }
    
    // Clean up enemy textures
    for (int level = 1; level <= MAX_ENEMY_LEVEL; level++) {
        if (enemyTextures[level]) {
            SDL_DestroyTexture(enemyTextures[level]);
        }
    }
    
    if (font) {
        delete font;
    }
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
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
