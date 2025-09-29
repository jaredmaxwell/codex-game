#include "asset_manager.h"
#include "enemy.h"
#include <iostream>

// Platform-specific SDL_image includes
#if defined(__EMSCRIPTEN__) || defined(_WIN32)
#include <SDL_image.h>
#elif defined(__APPLE__)
#include <SDL_image.h>
#else
#include <SDL2/SDL_image.h>
#endif

AssetManager::AssetManager() 
    : m_font(nullptr), m_playerTexture(nullptr), m_tilemapLoaded(false) {
    // Initialize enemy texture array
    for (int i = 0; i <= Enemy::MAX_ENEMY_LEVEL; i++) {
        m_enemyTextures[i] = nullptr;
    }
}

AssetManager::~AssetManager() {
    cleanup();
}

bool AssetManager::initialize(SDL_Renderer* renderer) {
    if (!renderer) {
        std::cerr << "AssetManager: Invalid renderer provided" << std::endl;
        return false;
    }
    
    std::cout << "Initializing AssetManager..." << std::endl;
    
    // Load all assets
    bool success = true;
    
    if (!loadFont(renderer)) {
        std::cerr << "AssetManager: Failed to load font" << std::endl;
        success = false;
    }
    
    if (!loadPlayerTexture(renderer)) {
        std::cerr << "AssetManager: Failed to load player texture" << std::endl;
        success = false;
    }
    
    if (!loadEnemyTextures(renderer)) {
        std::cerr << "AssetManager: Failed to load enemy textures" << std::endl;
        success = false;
    }
    
    if (!loadTilemap(renderer)) {
        std::cerr << "AssetManager: Failed to load tilemap" << std::endl;
        success = false;
    }
    
    if (success) {
        std::cout << "AssetManager: All assets loaded successfully!" << std::endl;
    } else {
        std::cerr << "AssetManager: Some assets failed to load" << std::endl;
    }
    
    return success;
}

void AssetManager::cleanup() {
    cleanupFont();
    cleanupTextures();
}


SDL_Texture* AssetManager::getEnemyTexture(int level) const {
    if (level < 1 || level > Enemy::MAX_ENEMY_LEVEL) {
        return nullptr;
    }
    return m_enemyTextures[level];
}

bool AssetManager::isEnemyTextureLoaded(int level) const {
    if (level < 1 || level > Enemy::MAX_ENEMY_LEVEL) {
        return false;
    }
    return m_enemyTextures[level] != nullptr;
}

bool AssetManager::loadFont(SDL_Renderer* renderer) {
    m_font = new BitmapFont();
    std::cout << "AssetManager: Loading bitmap font..." << std::endl;
    
    const char* fontPath = "assets/dbyte_1x.png";
    
    if (!m_font->loadFont(renderer, fontPath)) {
        std::cerr << "AssetManager: Failed to load bitmap font - text rendering will be disabled" << std::endl;
        delete m_font;
        m_font = nullptr;
        return false;
    }
    
    std::cout << "AssetManager: Bitmap font loaded successfully!" << std::endl;
    return true;
}

bool AssetManager::loadPlayerTexture(SDL_Renderer* renderer) {
    const char* playerPath = "assets/char.png";
    std::cout << "AssetManager: Loading player texture..." << std::endl;
    
    SDL_Surface* playerSurface = IMG_Load(playerPath);
    if (playerSurface) {
        m_playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
        SDL_FreeSurface(playerSurface);
        if (!m_playerTexture) {
            std::cerr << "AssetManager: Failed to create player texture: " << SDL_GetError() << std::endl;
            return false;
        }
        std::cout << "AssetManager: Player texture loaded successfully!" << std::endl;
        return true;
    } else {
        std::cout << "AssetManager: Player image not found, will use placeholder rectangle" << std::endl;
        return false;
    }
}

bool AssetManager::loadEnemyTextures(SDL_Renderer* renderer) {
    std::cout << "AssetManager: Loading enemy textures..." << std::endl;
    bool allLoaded = true;
    
    for (int level = 1; level <= Enemy::MAX_ENEMY_LEVEL; level++) {
        std::string filename = "assets/enemy" + std::to_string(level) + ".png";
        
        SDL_Surface* enemySurface = IMG_Load(filename.c_str());
        if (enemySurface) {
            m_enemyTextures[level] = SDL_CreateTextureFromSurface(renderer, enemySurface);
            SDL_FreeSurface(enemySurface);
            if (!m_enemyTextures[level]) {
                std::cerr << "AssetManager: Failed to create enemy texture for level " << level << ": " << SDL_GetError() << std::endl;
                allLoaded = false;
            }
        } else {
            std::cout << "AssetManager: Enemy texture for level " << level << " not found" << std::endl;
            allLoaded = false;
        }
    }
    
    if (allLoaded) {
        std::cout << "AssetManager: All enemy textures loaded successfully!" << std::endl;
    } else {
        std::cout << "AssetManager: Some enemy textures failed to load" << std::endl;
    }
    
    return allLoaded;
}

bool AssetManager::loadTilemap(SDL_Renderer* renderer) {
    std::cout << "AssetManager: Loading tilemap..." << std::endl;
    
    if (!m_tmxLoader.loadTMX("assets/game_level.tmx", renderer, m_tilemap)) {
        std::cerr << "AssetManager: Failed to load tilemap - game will continue without background" << std::endl;
        m_tilemapLoaded = false;
        return false;
    } else {
        std::cout << "AssetManager: Tilemap loaded successfully!" << std::endl;
        m_tilemapLoaded = true;
        return true;
    }
}

void AssetManager::cleanupTextures() {
    if (m_playerTexture) {
        SDL_DestroyTexture(m_playerTexture);
        m_playerTexture = nullptr;
    }
    
    for (int level = 1; level <= Enemy::MAX_ENEMY_LEVEL; level++) {
        if (m_enemyTextures[level]) {
            SDL_DestroyTexture(m_enemyTextures[level]);
            m_enemyTextures[level] = nullptr;
        }
    }
}

void AssetManager::cleanupFont() {
    if (m_font) {
        delete m_font;
        m_font = nullptr;
    }
}
