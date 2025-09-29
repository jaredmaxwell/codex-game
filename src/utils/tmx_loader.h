#pragma once
#include <SDL.h>
#include <string>
#include <vector>

struct TilemapData {
    int width;
    int height;
    int tileWidth;
    int tileHeight;
    std::vector<int> tileData;
    SDL_Texture* tilesetTexture;
    int tilesetWidth;
    int tilesetHeight;
    int tilesPerRow;
    
    // Performance optimization data
    std::vector<SDL_Rect> tileRects;  // Pre-calculated source rectangles
    bool tilesPrepared = false;
};

class TMXLoader {
public:
    TMXLoader();
    ~TMXLoader();
    
    // Load a TMX file and return tilemap data
    bool loadTMX(const std::string& filename, SDL_Renderer* renderer, TilemapData& tilemap);
    
    // Render the tilemap with viewport culling
    void renderTilemap(SDL_Renderer* renderer, const TilemapData& tilemap, int offsetX = 0, int offsetY = 0, int viewportX = 0, int viewportY = 0, int viewportW = 800, int viewportH = 600);
    
    // Prepare tiles for rendering (call once after loading)
    void prepareTiles(TilemapData& tilemap);
    
private:
    // Helper functions for parsing
    bool parseCSVData(const std::string& csvData, std::vector<int>& tileData);
    bool loadTilesetTexture(const std::string& imagePath, SDL_Renderer* renderer, SDL_Texture*& texture);
};
