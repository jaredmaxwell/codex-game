#include "game.h"
#include "bitmap_font.h"
#include "player.h"
#include "enemy.h"
#include "item.h"
#include "pet.h"
#include "asset_manager.h"
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
const int GRID_CELL_SIZE = 500; // Size of each grid cell (world size / grid size)
const int MAX_GRID_WIDTH = 32; // Maximum grid width (world width / cell size)
const int MAX_GRID_HEIGHT = 32; // Maximum grid height (world height / cell size)


// Enemy class is now defined in enemy.h



// Spatial partitioning grid
struct GridCell {
    std::vector<int> enemyIndices; // Indices of enemies in this cell
};

// Game state variables
static AssetManager* g_assetManager = nullptr;
static Player g_player;
static Pet g_pet;
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

// Get nearby enemies from spatial grid for collision avoidance
std::vector<int> getNearbyEnemies(int centerX, int centerY, int currentEnemyIndex, int searchRadius = GRID_CELL_SIZE) {
    std::vector<int> nearbyEnemies;
    
    // Calculate grid bounds to search
    int minGridX = (centerX - searchRadius) / GRID_CELL_SIZE;
    int maxGridX = (centerX + searchRadius) / GRID_CELL_SIZE;
    int minGridY = (centerY - searchRadius) / GRID_CELL_SIZE;
    int maxGridY = (centerY + searchRadius) / GRID_CELL_SIZE;
    
    // Clamp to grid bounds
    minGridX = std::max(0, minGridX);
    maxGridX = std::min(g_gridWidth - 1, maxGridX);
    minGridY = std::max(0, minGridY);
    maxGridY = std::min(g_gridHeight - 1, maxGridY);
    
    // Check all grid cells in the search area
    for (int gridY = minGridY; gridY <= maxGridY; gridY++) {
        for (int gridX = minGridX; gridX <= maxGridX; gridX++) {
            int gridIndex = gridY * g_gridWidth + gridX;
            if (gridIndex >= 0 && gridIndex < g_gridWidth * g_gridHeight) {
                // Add all enemies from this grid cell, excluding the current enemy
                for (int enemyIndex : g_spatialGrid[gridIndex].enemyIndices) {
                    if (enemyIndex != currentEnemyIndex) {
                        nearbyEnemies.push_back(enemyIndex);
                    }
                }
            }
        }
    }
    
    return nearbyEnemies;
}


// Helper functions
void renderText(SDL_Renderer* renderer, BitmapFont* font, const std::string& text, int x, int y, SDL_Color color) {
    if (!font) {
        return;
    }
    font->renderText(renderer, text, x, y, color);
}


GameScene::GameScene() {
    // Initialize random seed
    srand(time(NULL));
}

GameScene::~GameScene() {
    // Cleanup will be handled here
    if (g_assetManager) {
        delete g_assetManager;
        g_assetManager = nullptr;
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
    
    // Initialize asset manager
    g_assetManager = new AssetManager();
    if (!g_assetManager->initialize(renderer)) {
        std::cerr << "Failed to initialize AssetManager - some assets may not be available" << std::endl;
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
    
    // Get tilemap from asset manager
    m_tilemap = g_assetManager->getTilemap();
    m_tmxLoader = g_assetManager->getTMXLoader();
    
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
    
    // Initialize pet near player
    g_pet.initialize(g_worldWidth / 2 - Pet::SIZE / 2 + 30, g_worldHeight / 2 - Pet::SIZE / 2 + 30);
    
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
    // Get current time first
    Uint32 currentTime = SDL_GetTicks();
    
    // Handle continuous movement with keyboard state
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    g_player.handleInput(keystate);
    g_player.update();
    
    // Update pet
    g_pet.update(g_player, g_enemies, currentTime);
    
    // Update camera to follow player
    m_camera.update(g_player.getCenterX(), g_player.getCenterY());

    // Enemy spawning
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
    
    // Update all enemies with spatial partitioning
    for (size_t i = 0; i < g_enemies.size(); i++) {
        if (g_enemies[i].isActive()) {
            // Get nearby enemies for this enemy using spatial partitioning
            int centerX = g_enemies[i].getCenterX();
            int centerY = g_enemies[i].getCenterY();
            std::vector<int> nearbyEnemies = getNearbyEnemies(centerX, centerY, i, GRID_CELL_SIZE * 2);
            
            // Spatial partitioning is working - no debug output needed
            
            // Update enemy with spatial partitioning
            g_enemies[i].updateWithSpatialPartitioning(g_player, g_enemies, g_worldWidth, g_worldHeight, 
                                                      currentTime, nearbyEnemies);
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
                            g_enemies[i].getShardProperties(shardValue, shardColor);
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

    // Collision detection between pet projectiles and enemies
    for (const auto& projectile : g_pet.getProjectiles()) {
        if (!projectile.active) continue;
        
        SDL_Rect projectileRect = {projectile.x, projectile.y, Projectile::SIZE, Projectile::SIZE};
        
        for (size_t i = 0; i < g_enemies.size(); i++) {
            if (g_enemies[i].isActive() && g_enemies[i].checkCollision(projectileRect)) {
                // Enemy hit by pet projectile - level down and knockback
                int originalLevel = g_enemies[i].getLevel();
                g_enemies[i].takeDamage();
                
                // Calculate knockback direction (away from pet)
                float dx = g_enemies[i].getX() - g_pet.getX();
                float dy = g_enemies[i].getY() - g_pet.getY();
                float distance = sqrt(dx * dx + dy * dy);
                
                g_enemies[i].applyKnockback(dx, dy, distance, currentTime);
                
                // If enemy reaches level 0, drop shard and deactivate
                if (!g_enemies[i].isActive()) {
                    // Find inactive shard slot
                    for (int j = 0; j < Item::MAX_SHARDS; j++) {
                        if (!g_items[j].isActive()) {
                            int shardValue;
                            SDL_Color shardColor;
                            g_enemies[i].getShardProperties(shardValue, shardColor);
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
                
                // Mark projectile as inactive (it hit something)
                const_cast<Projectile&>(projectile).active = false;
                break; // Projectile can only hit one enemy
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
    if (g_assetManager && g_assetManager->isTilemapLoaded()) {
        g_assetManager->getTMXLoader().renderTilemap(m_renderer, g_assetManager->getTilemap(), m_camera.getOffsetX(), m_camera.getOffsetY(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    // Render player
    SDL_Rect playerRect = g_player.getRect();
    playerRect.x += m_camera.getOffsetX();
    playerRect.y += m_camera.getOffsetY();
    if (g_assetManager && g_assetManager->getPlayerTexture()) {
        SDL_RenderCopy(m_renderer, g_assetManager->getPlayerTexture(), NULL, &playerRect);
    } else {
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(m_renderer, &playerRect);
    }

    // Render pet
    if (g_pet.isActive()) {
        SDL_Texture* petTexture = nullptr;
        if (g_assetManager) {
            petTexture = g_assetManager->getPetTexture();
        }
        g_pet.render(m_renderer, petTexture, m_camera.getOffsetX(), m_camera.getOffsetY());
        
        // Render pet projectiles
        g_pet.renderProjectiles(m_renderer, m_camera.getOffsetX(), m_camera.getOffsetY());
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
            SDL_Texture* enemyTexture = nullptr;
            if (g_assetManager) {
                enemyTexture = g_assetManager->getEnemyTexture(g_enemies[i].getLevel());
            }
            g_enemies[i].render(m_renderer, enemyTexture, 
                               m_camera.getOffsetX(), m_camera.getOffsetY());
            
            // Render level number
            SDL_Color white = {255, 255, 255, 255};
            if (g_assetManager && g_assetManager->getFont()) {
                int enemySize = g_enemies[i].getSize();
                renderText(m_renderer, g_assetManager->getFont(), std::to_string(g_enemies[i].getLevel()), 
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
    if (g_assetManager && g_assetManager->getFont()) {
        renderText(m_renderer, g_assetManager->getFont(), "Shards: " + std::to_string(g_score), 10, 10, white);
        renderText(m_renderer, g_assetManager->getFont(), "Enemies: " + std::to_string(g_enemies.size()), 10, 30, white);
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
    
    // Reset pet position near player
    g_pet.initialize(g_worldWidth / 2 - Pet::SIZE / 2 + 30, g_worldHeight / 2 - Pet::SIZE / 2 + 30);
    
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