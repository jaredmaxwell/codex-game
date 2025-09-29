#pragma once
#include <SDL.h>
#include <string>
#include "bitmap_font.h"
#include "tmx_loader.h"
#include "enemy.h"

class AssetManager {
public:
    // Constructor and destructor
    AssetManager();
    ~AssetManager();
    
    // Initialize all assets
    bool initialize(SDL_Renderer* renderer);
    
    // Cleanup all assets
    void cleanup();
    
    // Getters for loaded assets
    BitmapFont* getFont() const { return m_font; }
    SDL_Texture* getPlayerTexture() const { return m_playerTexture; }
    SDL_Texture* getEnemyTexture(int level) const;
    TMXLoader& getTMXLoader() { return m_tmxLoader; }
    TilemapData& getTilemap() { return m_tilemap; }
    
    // Asset loading status
    bool isFontLoaded() const { return m_font != nullptr; }
    bool isPlayerTextureLoaded() const { return m_playerTexture != nullptr; }
    bool isEnemyTextureLoaded(int level) const;
    bool isTilemapLoaded() const { return m_tilemapLoaded; }
    
private:
    // Asset storage
    BitmapFont* m_font;
    SDL_Texture* m_playerTexture;
    SDL_Texture* m_enemyTextures[Enemy::MAX_ENEMY_LEVEL + 1];
    TMXLoader m_tmxLoader;
    TilemapData m_tilemap;
    bool m_tilemapLoaded;
    
    // Asset loading methods
    bool loadFont(SDL_Renderer* renderer);
    bool loadPlayerTexture(SDL_Renderer* renderer);
    bool loadEnemyTextures(SDL_Renderer* renderer);
    bool loadTilemap(SDL_Renderer* renderer);
    
    // Helper methods
    void cleanupTextures();
    void cleanupFont();
};
