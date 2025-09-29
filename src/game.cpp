#include "game.h"
#include "bitmap_font.h"
#include "player.h"
#include "enemy.h"
#include "item.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>

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

// Spatial partitioning constants
const int GRID_CELL_SIZE = 64; // Size of each grid cell
const int MAX_GRID_WIDTH = 32; // Maximum grid width (world width / cell size)
const int MAX_GRID_HEIGHT = 32; // Maximum grid height (world height / cell size)


// Enemy class is now defined in enemy.h



// Spatial partitioning grid
struct GridCell {
    std::vector<int> enemyIndices; // Indices of enemies in this cell
};

// Game state variables
static BitmapFont* g_font = nullptr;
static SDL_Texture* g_playerTexture = nullptr;
static SDL_Texture* g_enemyTextures[Enemy::MAX_ENEMY_LEVEL + 1] = {nullptr};
static Player g_player;
static std::vector<Enemy> g_enemies;
static Item g_items[Item::MAX_SHARDS + Item::MAX_MAGNETS];
static Uint32 g_lastEnemySpawn = 0;
static int g_score = 0;
static Uint32 g_magnetEffectEndTime = 0;

// Spatial partitioning grid
static GridCell g_spatialGrid[MAX_GRID_WIDTH * MAX_GRID_HEIGHT];
static int g_gridWidth = 0;
static int g_gridHeight = 0;

// World bounds for enemy spawning and cleanup
static int g_worldWidth = 0;
static int g_worldHeight = 0;


// Spatial partitioning helper functions
void clearSpatialGrid() {
    for (int i = 0; i < g_gridWidth * g_gridHeight; i++) {
        g_spatialGrid[i].enemyIndices.clear();
    }
}

int getGridIndex(int x, int y) {
    int gridX = x / GRID_CELL_SIZE;
    int gridY = y / GRID_CELL_SIZE;
    
    // Clamp to grid bounds
    if (gridX < 0) gridX = 0;
    if (gridX >= g_gridWidth) gridX = g_gridWidth - 1;
    if (gridY < 0) gridY = 0;
    if (gridY >= g_gridHeight) gridY = g_gridHeight - 1;
    
    return gridY * g_gridWidth + gridX;
}

void addEnemyToGrid(int enemyIndex) {
    if (enemyIndex >= g_enemies.size() || !g_enemies[enemyIndex].isActive()) return;
    
    int centerX = g_enemies[enemyIndex].getCenterX();
    int centerY = g_enemies[enemyIndex].getCenterY();
    
    int gridIndex = getGridIndex(centerX, centerY);
    g_spatialGrid[gridIndex].enemyIndices.push_back(enemyIndex);
}

void updateSpatialGrid() {
    clearSpatialGrid();
    for (size_t i = 0; i < g_enemies.size(); i++) {
        if (g_enemies[i].isActive()) {
            addEnemyToGrid(i);
        }
    }
}


// Helper functions
void renderText(SDL_Renderer* renderer, BitmapFont* font, const std::string& text, int x, int y, SDL_Color color) {
    if (!font) {
        return;
    }
    font->renderText(renderer, text, x, y, color);
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
    
    for (int level = 1; level <= Enemy::MAX_ENEMY_LEVEL; level++) {
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
    
    // Set up scaling for fullscreen
    // Set logical size for consistent rendering regardless of window size
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Enable integer scaling for pixel-perfect rendering (great for pixel art)
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
    
    std::cout << "Renderer scaling configured: Logical size " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;
    
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
    for (int level = 1; level <= Enemy::MAX_ENEMY_LEVEL; level++) {
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

    // Initialize game state - player will be positioned after world bounds are set
    
    // Initialize enemies vector (reserve space for efficiency)
    g_enemies.clear();
    g_enemies.reserve(Enemy::MAX_ENEMIES);
    
    // Initialize items array
    for (int i = 0; i < Item::MAX_SHARDS + Item::MAX_MAGNETS; i++) {
        g_items[i] = Item(); // Default inactive item
    }
    
    g_lastEnemySpawn = 0;
    g_score = 0;
    g_magnetEffectEndTime = 0;
    m_quit = false;
    
    // Load tilemap (large 1000x1000 map with optimizations)
    if (!m_tmxLoader.loadTMX("assets/game_level.tmx", renderer, m_tilemap)) {
        std::cerr << "Failed to load tilemap - game will continue without background" << std::endl;
    } else {
        std::cout << "Tilemap loaded successfully!" << std::endl;
    }
    
    // Initialize camera with dead zone (200x150 pixel dead zone in center)
    m_camera.initialize(SCREEN_WIDTH, SCREEN_HEIGHT, 200, 150);
    
    // Set camera limits and world bounds based on tilemap size (if available)
    if (m_tilemap.width > 0 && m_tilemap.height > 0) {
        g_worldWidth = m_tilemap.width * m_tilemap.tileWidth;
        g_worldHeight = m_tilemap.height * m_tilemap.tileHeight;
        m_camera.setLimits(0, 0, g_worldWidth, g_worldHeight);
    } else {
        // Fallback to screen size if no tilemap
        g_worldWidth = SCREEN_WIDTH;
        g_worldHeight = SCREEN_HEIGHT;
    }
    
    // Initialize spatial partitioning grid
    g_gridWidth = (g_worldWidth + GRID_CELL_SIZE - 1) / GRID_CELL_SIZE;
    g_gridHeight = (g_worldHeight + GRID_CELL_SIZE - 1) / GRID_CELL_SIZE;
    
    // Clamp grid dimensions to maximum
    if (g_gridWidth > MAX_GRID_WIDTH) g_gridWidth = MAX_GRID_WIDTH;
    if (g_gridHeight > MAX_GRID_HEIGHT) g_gridHeight = MAX_GRID_HEIGHT;
    
    std::cout << "Spatial grid initialized: " << g_gridWidth << "x" << g_gridHeight << " cells" << std::endl;
    
    // Initialize player at world center
    g_player.initialize(g_worldWidth / 2 - Player::PLAYER_SIZE / 2, g_worldHeight / 2 - Player::PLAYER_SIZE / 2);
    
    // Center camera on player initially
    m_camera.centerOn(g_player.getCenterX(), g_player.getCenterY());
    
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
    
    // Update camera to follow player
    m_camera.update(g_player.getCenterX(), g_player.getCenterY());

    // Enemy spawning
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - g_lastEnemySpawn > Enemy::ENEMY_SPAWN_RATE && g_enemies.size() < Enemy::MAX_ENEMIES) {
        // Calculate enemy level based on player's shard count
        int enemyLevel = 1 + (g_score / 10); // Every 10 shards increases enemy level by 1
        if (enemyLevel > Enemy::MAX_ENEMY_LEVEL) enemyLevel = Enemy::MAX_ENEMY_LEVEL;
        
        // Get current camera viewport bounds
        SDL_Rect viewport = m_camera.getViewport();
        int viewportLeft = viewport.x;
        int viewportRight = viewport.x + viewport.w;
        int viewportTop = viewport.y;
        int viewportBottom = viewport.y + viewport.h;
        
        // Randomly choose which edge to spawn from
        int edge = rand() % 4;
        int enemySize = Enemy::getEnemySize(enemyLevel);
        int spawnX, spawnY;
        
        switch (edge) {
            case 0: // Top edge
                spawnX = viewportLeft + rand() % (viewportRight - viewportLeft);
                spawnY = viewportTop - enemySize;
                break;
            case 1: // Bottom edge
                spawnX = viewportLeft + rand() % (viewportRight - viewportLeft);
                spawnY = viewportBottom;
                break;
            case 2: // Left edge
                spawnX = viewportLeft - enemySize;
                spawnY = viewportTop + rand() % (viewportBottom - viewportTop);
                break;
            case 3: // Right edge
                spawnX = viewportRight;
                spawnY = viewportTop + rand() % (viewportBottom - viewportTop);
                break;
            default:
                spawnX = spawnY = 0;
        }
        
        // Create new enemy
        Enemy newEnemy;
        newEnemy.initialize(spawnX, spawnY, enemyLevel, Enemy::DEFAULT_SPEED, currentTime);
        g_enemies.push_back(newEnemy);
        g_lastEnemySpawn = currentTime;
    }

    // Update spatial partitioning grid first
    updateSpatialGrid();
    
    // Update all enemies
    for (size_t i = 0; i < g_enemies.size(); i++) {
        if (g_enemies[i].isActive()) {
            g_enemies[i].update(g_player, g_enemies, g_worldWidth, g_worldHeight, currentTime);
        }
    }
    
    // Remove inactive enemies (cleanup)
    g_enemies.erase(
        std::remove_if(g_enemies.begin(), g_enemies.end(),
            [](const Enemy& enemy) { return !enemy.isActive(); }),
        g_enemies.end()
    );

    // Collision detection between attack and enemies
    if (g_player.getAttack().active) {
        for (size_t i = 0; i < g_enemies.size(); i++) {
            if (g_enemies[i].isActive() && g_enemies[i].checkCollision(g_player.getAttack().rect)) {
                // Enemy hit by attack - level down and knockback
                int originalLevel = g_enemies[i].getLevel();
                g_enemies[i].takeDamage();
                
                // Calculate knockback direction (away from player)
                float dx = g_enemies[i].getX() - g_player.getX();
                float dy = g_enemies[i].getY() - g_player.getY();
                float distance = sqrt(dx * dx + dy * dy);
                
                g_enemies[i].applyKnockback(dx, dy, distance, currentTime);
                
                // If enemy reaches level 0, drop shard and deactivate
                if (!g_enemies[i].isActive()) {
                    // Find inactive shard slot
                    for (int j = 0; j < Item::MAX_SHARDS; j++) {
                        if (!g_items[j].isActive()) {
                            int shardValue;
                            SDL_Color shardColor;
                            getShardProperties(originalLevel, shardValue, shardColor); // Use original level
                            g_items[j].initialize(g_enemies[i].getCenterX() - Item::SHARD_SIZE/2, 
                                                 g_enemies[i].getCenterY() - Item::SHARD_SIZE/2, 
                                                 ItemType::SHARD, currentTime, shardValue, shardColor);
                            break;
                        }
                    }
                    
                    // 1% chance to drop a magnet
                    if (rand() % 100 < Item::MAGNET_DROP_CHANCE) {
                        for (int k = Item::MAX_SHARDS; k < Item::MAX_SHARDS + Item::MAX_MAGNETS; k++) {
                            if (!g_items[k].isActive()) {
                                g_items[k].initialize(g_enemies[i].getCenterX() - Item::MAGNET_SIZE/2, 
                                                    g_enemies[i].getCenterY() - Item::MAGNET_SIZE/2, 
                                                    ItemType::MAGNET, currentTime);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // Collision detection between player and enemies
    for (size_t i = 0; i < g_enemies.size(); i++) {
        if (g_enemies[i].isActive() && g_enemies[i].checkCollisionWithPlayer(g_player)) {
            // Player hit by enemy - reset player position to world center and lose all shards
            g_player.setPosition(g_worldWidth / 2 - Player::PLAYER_SIZE / 2, g_worldHeight / 2 - Player::PLAYER_SIZE / 2);
            g_score = 0; // Reset score to 0 when player dies
            // Deactivate the enemy that hit the player
            g_enemies[i].setActive(false);
        }
    }

    // Item update and collection
    bool magnetEffectActive = (currentTime < g_magnetEffectEndTime);
    for (int i = 0; i < Item::MAX_SHARDS + Item::MAX_MAGNETS; i++) {
        if (g_items[i].isActive()) {
            // Update item (movement, lifetime, etc.)
            g_items[i].update(g_player.getCenterX(), g_player.getCenterY(), currentTime, magnetEffectActive);
            
            // Player collection
            if (g_items[i].checkCollisionWithPlayer(g_player.getRect())) {
                if (g_items[i].getType() == ItemType::SHARD) {
                    g_score += g_items[i].getValue();
                } else if (g_items[i].getType() == ItemType::MAGNET) {
                    // Activate magnet effect for 20 seconds
                    g_magnetEffectEndTime = currentTime + 20000; // 20 seconds
                }
                g_items[i].setActive(false);
            }
        }
    }
}

void GameScene::render() {
    // Rendering
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
    
    // Render tilemap background with camera offset
    m_tmxLoader.renderTilemap(m_renderer, m_tilemap, m_camera.getOffsetX(), m_camera.getOffsetY(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Render player
    SDL_Rect playerRect = g_player.getRect();
    playerRect.x += m_camera.getOffsetX();
    playerRect.y += m_camera.getOffsetY();
    if (g_playerTexture) {
        SDL_RenderCopy(m_renderer, g_playerTexture, NULL, &playerRect);
    } else {
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(m_renderer, &playerRect);
    }

    if (g_player.getAttack().active) {
        SDL_Rect attackRect = g_player.getAttack().rect;
        attackRect.x += m_camera.getOffsetX();
        attackRect.y += m_camera.getOffsetY();
        SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(m_renderer, &attackRect);
    }

    // Render enemies
    for (size_t i = 0; i < g_enemies.size(); i++) {
        if (g_enemies[i].isActive()) {
            g_enemies[i].render(m_renderer, g_enemyTextures[g_enemies[i].getLevel()], 
                               m_camera.getOffsetX(), m_camera.getOffsetY());
            
            // Render level number
            SDL_Color white = {255, 255, 255, 255};
            if (g_font) {
                int enemySize = g_enemies[i].getSize();
                renderText(m_renderer, g_font, std::to_string(g_enemies[i].getLevel()), 
                         g_enemies[i].getX() + m_camera.getOffsetX() + enemySize/2 - 4, 
                         g_enemies[i].getY() + m_camera.getOffsetY() + enemySize/2 - 6, white);
            }
        }
    }

    // Render items
    for (int i = 0; i < Item::MAX_SHARDS + Item::MAX_MAGNETS; i++) {
        if (g_items[i].isActive()) {
            g_items[i].render(m_renderer, m_camera.getOffsetX(), m_camera.getOffsetY());
        }
    }

    // Render score and enemy count
    SDL_Color white = {255, 255, 255, 255};
    if (g_font) {
        renderText(m_renderer, g_font, "Shards: " + std::to_string(g_score), 10, 10, white);
        renderText(m_renderer, g_font, "Enemies: " + std::to_string(g_enemies.size()), 10, 30, white);
    }

    SDL_RenderPresent(m_renderer);
}

void GameScene::restart() {
    // Reset all game state variables
    g_score = 0;
    g_lastEnemySpawn = 0;
    g_magnetEffectEndTime = 0;
    m_quit = false;
    
    // Reset player position to world center
    g_player.initialize(g_worldWidth / 2 - Player::PLAYER_SIZE / 2, g_worldHeight / 2 - Player::PLAYER_SIZE / 2);
    
    // Clear all enemies
    g_enemies.clear();
    
    // Deactivate all items
    for (int i = 0; i < Item::MAX_SHARDS + Item::MAX_MAGNETS; i++) {
        g_items[i].setActive(false);
    }
    
    std::cout << "Game restarted - all state reset" << std::endl;
}

void GameScene::handleWindowResize(int newWidth, int newHeight) {
    // SDL's logical size and integer scaling handle this automatically
    // The renderer will automatically scale the logical size to fit the new window size
    std::cout << "Window resized to " << newWidth << "x" << newHeight 
              << " - scaling handled automatically" << std::endl;
}