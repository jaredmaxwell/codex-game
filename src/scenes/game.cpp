#include "game.h"
#include "../rendering/bitmap_font.h"
#include "../entities/player.h"
#include "../entities/enemy.h"
#include "../entities/item.h"
#include "../entities/pet.h"
#include "../systems/game_manager.h"
#include "../systems/asset_manager.h"
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
static GameManager* g_gameManager = nullptr;

// Spatial partitioning grid
static GridCell g_spatialGrid[MAX_GRID_WIDTH * MAX_GRID_HEIGHT];
static int g_gridWidth = 0;
static int g_gridHeight = 0;

// World bounds for enemy spawning and cleanup
static int g_worldWidth = 0;
static int g_worldHeight = 0;


// Spatial partitioning functions removed - now handled by GameManager


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
    if (g_gameManager) {
        delete g_gameManager;
        g_gameManager = nullptr;
    }
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

    // Initialize game manager
    g_gameManager = new GameManager();
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
    
    // Initialize game manager with world bounds
    g_gameManager->initialize(g_worldWidth, g_worldHeight);
    
    // Center camera on player initially
    m_camera.centerOn(g_gameManager->getPlayer().getCenterX(), g_gameManager->getPlayer().getCenterY());
    
    return true;
}

void GameScene::setCharacterClass(CharacterClass characterClass) {
    m_characterClass = characterClass;
    
    // Set the player's character class in the game manager
    if (g_gameManager) {
        g_gameManager->getPlayer().setCharacterClass(characterClass);
        std::cout << "Player character class set to: " << static_cast<int>(characterClass) << std::endl;
    }
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
                g_gameManager->getPlayer().handleAttack();
                break;
        }
    }
}

void GameScene::update() {
    // Get current time first
    Uint32 currentTime = SDL_GetTicks();
    
    // Handle continuous movement with keyboard state
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    g_gameManager->getPlayer().handleInput(keystate);
    
    // Update all game entities
    g_gameManager->update(currentTime);
    
    // Update camera to follow player
    m_camera.update(g_gameManager->getPlayer().getCenterX(), g_gameManager->getPlayer().getCenterY());
}

void GameScene::render() {
    // Rendering
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
    
    // Render tilemap background with camera offset
    if (g_assetManager && g_assetManager->isTilemapLoaded()) {
        g_assetManager->getTMXLoader().renderTilemap(m_renderer, g_assetManager->getTilemap(), m_camera.getOffsetX(), m_camera.getOffsetY(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    // Render all game entities
    g_gameManager->render(m_renderer, g_assetManager, m_camera.getOffsetX(), m_camera.getOffsetY());

    // Render score and enemy count
    SDL_Color white = {255, 255, 255, 255};
    if (g_assetManager && g_assetManager->getFont()) {
        renderText(m_renderer, g_assetManager->getFont(), "Shards: " + std::to_string(g_gameManager->getScore()), 10, 10, white);
        renderText(m_renderer, g_assetManager->getFont(), "Enemies: " + std::to_string(g_gameManager->getEnemies().size()), 10, 30, white);
    }

    SDL_RenderPresent(m_renderer);
}

void GameScene::restart() {
    // Reset game manager
    g_gameManager->reset();
    m_quit = false;
    
    std::cout << "Game restarted - all state reset" << std::endl;
}

void GameScene::handleWindowResize(int newWidth, int newHeight) {
    // SDL's logical size and integer scaling handle this automatically
    // The renderer will automatically scale the logical size to fit the new window size
    std::cout << "Window resized to " << newWidth << "x" << newHeight 
              << " - scaling handled automatically" << std::endl;
}