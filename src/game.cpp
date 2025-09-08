#include "game.h"
#include "bitmap_font.h"
#include "player.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Platform-specific SDL_image includes
#if defined(__EMSCRIPTEN__) || defined(_WIN32)
#include <SDL_image.h>
#elif defined(__APPLE__)
#include <SDL_image.h>
#else
#include <SDL2/SDL_image.h>
#endif

// Game constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
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

// Game state variables
static BitmapFont* g_font = nullptr;
static SDL_Texture* g_playerTexture = nullptr;
static SDL_Texture* g_enemyTextures[MAX_ENEMY_LEVEL + 1] = {nullptr};
static Player g_player;
static Enemy g_enemies[MAX_ENEMIES];
static Shard g_shards[MAX_SHARDS];
static Magnet g_magnets[MAX_MAGNETS];
static Uint32 g_lastEnemySpawn = 0;
static int g_score = 0;
static Uint32 g_magnetEffectEndTime = 0;

// Helper functions
void renderText(SDL_Renderer* renderer, BitmapFont* font, const std::string& text, int x, int y, SDL_Color color) {
    if (!font) {
        return;
    }
    font->renderText(renderer, text, x, y, color);
}

int getEnemySize(int level) {
    return ENEMY_BASE_SIZE + (level - 1) * 2;
}

void getShardProperties(int enemyLevel, int& value, SDL_Color& color) {
    if (enemyLevel >= 8) {
        value = 25;
        color = {128, 0, 128, 255}; // Purple
    } else if (enemyLevel >= 6) {
        value = 20;
        color = {0, 0, 255, 255}; // Blue
    } else if (enemyLevel >= 4) {
        value = 15;
        color = {0, 255, 0, 255}; // Green
    } else if (enemyLevel >= 2) {
        value = 10;
        color = {255, 165, 0, 255}; // Orange
    } else {
        value = 5;
        color = {255, 255, 0, 255}; // Yellow
    }
}

GameScene::GameScene() {
    // Initialize random seed
    srand(time(NULL));
}

GameScene::~GameScene() {
    // Cleanup will be handled here
    if (g_playerTexture) {
        SDL_DestroyTexture(g_playerTexture);
    }
    
    for (int level = 1; level <= MAX_ENEMY_LEVEL; level++) {
        if (g_enemyTextures[level]) {
            SDL_DestroyTexture(g_enemyTextures[level]);
        }
    }
    
    if (g_font) {
        delete g_font;
    }
}

bool GameScene::initialize(SDL_Renderer* renderer) {
    m_renderer = renderer;
    
    // Load bitmap font
    g_font = new BitmapFont();
    std::cout << "Attempting to load bitmap font..." << std::endl;
    
    const char* fontPath = "assets/dbyte_1x.png";
    
    if (!g_font->loadFont(renderer, fontPath)) {
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
        g_playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
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
            g_enemyTextures[level] = SDL_CreateTextureFromSurface(renderer, enemySurface);
            SDL_FreeSurface(enemySurface);
            if (!g_enemyTextures[level]) {
                std::cerr << "Failed to create enemy texture for level " << level << ": " << SDL_GetError() << std::endl;
            }
        }
    }

    // Initialize game state
    g_player.initialize(SCREEN_WIDTH / 2 - Player::PLAYER_SIZE / 2, SCREEN_HEIGHT / 2 - Player::PLAYER_SIZE / 2);
    
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
    m_quit = false;
    
    return true;
}

void GameScene::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_QUIT) {
        m_quit = true;
    } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                m_quit = true;
                break;
            case SDLK_j:
                g_player.handleAttack();
                break;
        }
    }
}

void GameScene::update() {
    // Handle continuous movement with keyboard state
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    g_player.handleInput(keystate);
    g_player.update();

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
                float dx = g_player.getX() - g_enemies[i].x;
                float dy = g_player.getY() - g_enemies[i].y;
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
    if (g_player.getAttack().active) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (g_enemies[i].active) {
                int enemySize = getEnemySize(g_enemies[i].level);
                SDL_Rect enemyRect = {g_enemies[i].x, g_enemies[i].y, enemySize, enemySize};
                if (SDL_HasIntersection(&g_player.getAttack().rect, &enemyRect)) {
                    // Enemy hit by attack - level down and knockback
                    g_enemies[i].level--;
                    
                    // Calculate knockback direction (away from player)
                    float dx = g_enemies[i].x - g_player.getX();
                    float dy = g_enemies[i].y - g_player.getY();
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
            SDL_Rect playerRect = g_player.getRect();
            if (SDL_HasIntersection(&playerRect, &enemyRect)) {
                // Player hit by enemy - reset player position to center and lose all shards
                g_player.setPosition(SCREEN_WIDTH / 2 - Player::PLAYER_SIZE / 2, SCREEN_HEIGHT / 2 - Player::PLAYER_SIZE / 2);
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
                float dx = g_player.getX() + Player::PLAYER_SIZE/2 - (g_shards[i].x + SHARD_SIZE/2);
                float dy = g_player.getY() + Player::PLAYER_SIZE/2 - (g_shards[i].y + SHARD_SIZE/2);
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
            SDL_Rect playerRect = g_player.getRect();
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
            SDL_Rect playerRect = g_player.getRect();
            if (SDL_HasIntersection(&playerRect, &magnetRect)) {
                g_magnets[i].active = false;
                // Activate magnet effect for 20 seconds
                g_magnetEffectEndTime = currentTime + 20000; // 20 seconds
            }
        }
    }
}

void GameScene::render() {
    // Rendering
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    // Render player
    SDL_Rect playerRect = g_player.getRect();
    if (g_playerTexture) {
        SDL_RenderCopy(m_renderer, g_playerTexture, NULL, &playerRect);
    } else {
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(m_renderer, &playerRect);
    }

    if (g_player.getAttack().active) {
        SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(m_renderer, &g_player.getAttack().rect);
    }

    // Render enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (g_enemies[i].active) {
            int enemySize = getEnemySize(g_enemies[i].level);
            SDL_Rect enemyRect = {g_enemies[i].x, g_enemies[i].y, enemySize, enemySize};
            
            // Use texture if available, otherwise use colored rectangle
            if (g_enemyTextures[g_enemies[i].level]) {
                SDL_RenderCopy(m_renderer, g_enemyTextures[g_enemies[i].level], NULL, &enemyRect);
            } else {
                // Color enemies based on level (darker red for higher levels)
                int redIntensity = 100 + (g_enemies[i].level * 15);
                if (redIntensity > 255) redIntensity = 255;
                SDL_SetRenderDrawColor(m_renderer, redIntensity, 100, 100, 255);
                SDL_RenderFillRect(m_renderer, &enemyRect);
            }
            
            // Render level number
            SDL_Color white = {255, 255, 255, 255};
            if (g_font) {
                renderText(m_renderer, g_font, std::to_string(g_enemies[i].level), g_enemies[i].x + enemySize/2 - 4, g_enemies[i].y + enemySize/2 - 6, white);
            }
        }
    }

    // Render shards
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (g_shards[i].active) {
            SDL_SetRenderDrawColor(m_renderer, g_shards[i].color.r, g_shards[i].color.g, g_shards[i].color.b, g_shards[i].color.a);
            SDL_Rect shardRect = {g_shards[i].x, g_shards[i].y, SHARD_SIZE, SHARD_SIZE};
            SDL_RenderFillRect(m_renderer, &shardRect);
        }
    }

    // Render magnets
    SDL_SetRenderDrawColor(m_renderer, 0, 255, 255, 255);
    for (int i = 0; i < MAX_MAGNETS; i++) {
        if (g_magnets[i].active) {
            SDL_Rect magnetRect = {g_magnets[i].x, g_magnets[i].y, MAGNET_SIZE, MAGNET_SIZE};
            SDL_RenderFillRect(m_renderer, &magnetRect);
        }
    }

    // Render score
    SDL_Color white = {255, 255, 255, 255};
    if (g_font) {
        renderText(m_renderer, g_font, "Shards: " + std::to_string(g_score), 10, 10, white);
    }

    SDL_RenderPresent(m_renderer);
}
