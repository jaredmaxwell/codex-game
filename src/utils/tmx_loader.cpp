#include "tmx_loader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

// Platform-specific SDL_image includes
#if defined(__EMSCRIPTEN__) || defined(_WIN32)
#include <SDL_image.h>
#elif defined(__APPLE__)
#include <SDL_image.h>
#else
#include <SDL2/SDL_image.h>
#endif

TMXLoader::TMXLoader() {
}

TMXLoader::~TMXLoader() {
}

bool TMXLoader::loadTMX(const std::string& filename, SDL_Renderer* renderer, TilemapData& tilemap) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open TMX file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::string csvData;
    bool inDataSection = false;
    std::string imageSource;
    
    // Parse the TMX file
    while (std::getline(file, line)) {
        // Look for map attributes
        if (line.find("<map") != std::string::npos) {
            // Extract width and height
            size_t widthPos = line.find("width=\"");
            size_t heightPos = line.find("height=\"");
            size_t tileWidthPos = line.find("tilewidth=\"");
            size_t tileHeightPos = line.find("tileheight=\"");
            
            if (widthPos != std::string::npos) {
                widthPos += 7; // Skip "width=\""
                size_t endPos = line.find("\"", widthPos);
                tilemap.width = std::stoi(line.substr(widthPos, endPos - widthPos));
            }
            
            if (heightPos != std::string::npos) {
                heightPos += 8; // Skip "height=\""
                size_t endPos = line.find("\"", heightPos);
                tilemap.height = std::stoi(line.substr(heightPos, endPos - heightPos));
            }
            
            if (tileWidthPos != std::string::npos) {
                tileWidthPos += 11; // Skip "tilewidth=\""
                size_t endPos = line.find("\"", tileWidthPos);
                tilemap.tileWidth = std::stoi(line.substr(tileWidthPos, endPos - tileWidthPos));
            }
            
            if (tileHeightPos != std::string::npos) {
                tileHeightPos += 12; // Skip "tileheight=\""
                size_t endPos = line.find("\"", tileHeightPos);
                tilemap.tileHeight = std::stoi(line.substr(tileHeightPos, endPos - tileHeightPos));
            }
        }
        
        // Look for tileset image source
        if (line.find("<image source=") != std::string::npos) {
            size_t startPos = line.find("source=\"");
            if (startPos != std::string::npos) {
                startPos += 8; // Skip "source=\""
                size_t endPos = line.find("\"", startPos);
                imageSource = line.substr(startPos, endPos - startPos);
            }
        }
        
        // Look for data section
        if (line.find("<data encoding=\"csv\">") != std::string::npos) {
            inDataSection = true;
            continue;
        }
        
        if (inDataSection) {
            if (line.find("</data>") != std::string::npos) {
                inDataSection = false;
                break;
            }
            csvData += line;
        }
    }
    
    file.close();
    
    // Parse CSV data
    if (!parseCSVData(csvData, tilemap.tileData)) {
        std::cerr << "Failed to parse CSV data" << std::endl;
        return false;
    }
    
    // Load tileset texture - prepend assets path if not already present
    std::string fullImagePath = imageSource;
    if (imageSource.find("assets/") == std::string::npos) {
        fullImagePath = "assets/" + imageSource;
    }
    
    if (!loadTilesetTexture(fullImagePath, renderer, tilemap.tilesetTexture)) {
        std::cerr << "Failed to load tileset texture: " << fullImagePath << std::endl;
        return false;
    }
    
    // Get tileset dimensions (assuming 256x256 for now, should be parsed from TMX)
    tilemap.tilesetWidth = 256;
    tilemap.tilesetHeight = 256;
    tilemap.tilesPerRow = tilemap.tilesetWidth / tilemap.tileWidth;
    
    std::cout << "TMX loaded successfully: " << tilemap.width << "x" << tilemap.height 
              << " tiles, " << tilemap.tileWidth << "x" << tilemap.tileHeight << " each" << std::endl;
    
    // Prepare tiles for optimized rendering
    prepareTiles(tilemap);
    
    return true;
}

bool TMXLoader::parseCSVData(const std::string& csvData, std::vector<int>& tileData) {
    // Optimized CSV parsing for large files
    tileData.clear();
    tileData.reserve(csvData.length() / 4); // Rough estimate to avoid reallocations
    
    const char* data = csvData.c_str();
    const char* end = data + csvData.length();
    
    while (data < end) {
        // Skip whitespace
        while (data < end && (*data == ' ' || *data == '\t' || *data == '\n' || *data == '\r')) {
            data++;
        }
        
        if (data >= end) break;
        
        // Find next comma or end
        const char* tokenStart = data;
        while (data < end && *data != ',') {
            data++;
        }
        
        if (data > tokenStart) {
            // Parse the token
            std::string token(tokenStart, data - tokenStart);
            try {
                int tileId = std::stoi(token);
                tileData.push_back(tileId);
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse tile ID: " << token << std::endl;
                return false;
            }
        }
        
        // Skip comma
        if (data < end && *data == ',') {
            data++;
        }
    }
    
    return true;
}

bool TMXLoader::loadTilesetTexture(const std::string& imagePath, SDL_Renderer* renderer, SDL_Texture*& texture) {
    SDL_Surface* surface = IMG_Load(imagePath.c_str());
    if (!surface) {
        std::cerr << "Failed to load tileset image: " << imagePath << " - " << IMG_GetError() << std::endl;
        return false;
    }
    
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        std::cerr << "Failed to create tileset texture: " << SDL_GetError() << std::endl;
        return false;
    }
    
    return true;
}

void TMXLoader::prepareTiles(TilemapData& tilemap) {
    if (tilemap.tilesPrepared) {
        return;
    }
    
    // Pre-calculate all source rectangles for tiles
    tilemap.tileRects.clear();
    tilemap.tileRects.reserve(tilemap.tilesPerRow * tilemap.tilesPerRow);
    
    for (int tileId = 0; tileId < tilemap.tilesPerRow * tilemap.tilesPerRow; tileId++) {
        int srcX = (tileId % tilemap.tilesPerRow) * tilemap.tileWidth;
        int srcY = (tileId / tilemap.tilesPerRow) * tilemap.tileHeight;
        tilemap.tileRects.push_back({srcX, srcY, tilemap.tileWidth, tilemap.tileHeight});
    }
    
    tilemap.tilesPrepared = true;
    std::cout << "Prepared " << tilemap.tileRects.size() << " tile rectangles for optimized rendering" << std::endl;
}

void TMXLoader::renderTilemap(SDL_Renderer* renderer, const TilemapData& tilemap, int offsetX, int offsetY, int viewportX, int viewportY, int viewportW, int viewportH) {
    if (!tilemap.tilesetTexture || !tilemap.tilesPrepared) {
        return;
    }
    
    // Calculate visible tile range for viewport culling
    int startX = std::max(0, (viewportX - offsetX) / tilemap.tileWidth);
    int endX = std::min(tilemap.width, (viewportX + viewportW - offsetX + tilemap.tileWidth - 1) / tilemap.tileWidth);
    int startY = std::max(0, (viewportY - offsetY) / tilemap.tileHeight);
    int endY = std::min(tilemap.height, (viewportY + viewportH - offsetY + tilemap.tileHeight - 1) / tilemap.tileHeight);
    
    // Only render visible tiles with basic batching optimization
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int tileIndex = y * tilemap.width + x;
            if (tileIndex >= tilemap.tileData.size()) {
                continue;
            }
            
            int tileId = tilemap.tileData[tileIndex];
            if (tileId == 0) {
                continue; // Skip empty tiles
            }
            
            // Convert 1-based tile ID to 0-based
            tileId--;
            
            // Use pre-calculated source rectangle
            if (tileId < tilemap.tileRects.size()) {
                SDL_Rect srcRect = tilemap.tileRects[tileId];
                SDL_Rect dstRect = {
                    x * tilemap.tileWidth + offsetX,
                    y * tilemap.tileHeight + offsetY,
                    tilemap.tileWidth,
                    tilemap.tileHeight
                };
                
                SDL_RenderCopy(renderer, tilemap.tilesetTexture, &srcRect, &dstRect);
            }
        }
    }
}
