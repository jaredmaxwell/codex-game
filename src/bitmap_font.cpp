#include "bitmap_font.h"
#include <iostream>
#include <SDL_image.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

BitmapFont::BitmapFont() : fontTexture(nullptr), charWidth(6), charHeight(8), charsPerRow(16) {
}

BitmapFont::~BitmapFont() {
    if (fontTexture) {
        SDL_DestroyTexture(fontTexture);
    }
}

bool BitmapFont::loadFont(SDL_Renderer* renderer, const char* fontPath) {
    if (!renderer) {
        std::cerr << "BitmapFont::loadFont: renderer is null" << std::endl;
        return false;
    }
    
    if (!fontPath) {
        std::cerr << "BitmapFont::loadFont: fontPath is null" << std::endl;
        return false;
    }
    
    std::cout << "BitmapFont::loadFont: Attempting to load font from: " << fontPath << std::endl;
    
    #ifdef __EMSCRIPTEN__
    std::cout << "Emscripten: Testing IMG_Load function availability..." << std::endl;
    #endif
    
    SDL_Surface* fontSurface = IMG_Load(fontPath);
    if (!fontSurface) {
        std::cerr << "Failed to load font: " << fontPath << " - " << SDL_GetError() << std::endl;
        std::cerr << "IMG_Load error: " << IMG_GetError() << std::endl;
        #ifdef __EMSCRIPTEN__
        std::cerr << "Emscripten: IMG_Load failed - this might be a file system or SDL_image issue" << std::endl;
        #endif
        return false;
    }
    
    std::cout << "BitmapFont::loadFont: Font surface loaded successfully, size: " 
              << fontSurface->w << "x" << fontSurface->h << std::endl;
    
    // Convert to texture with alpha channel for better rendering
    SDL_SetColorKey(fontSurface, SDL_TRUE, SDL_MapRGB(fontSurface->format, 0, 0, 0));
    
    #ifdef __EMSCRIPTEN__
    std::cout << "Emscripten: Creating font texture from surface..." << std::endl;
    #endif
    
    try {
        fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
        SDL_FreeSurface(fontSurface);
    } catch (...) {
        std::cerr << "Exception during font texture creation" << std::endl;
        SDL_FreeSurface(fontSurface);
        return false;
    }
    
    if (!fontTexture) {
        std::cerr << "Failed to create font texture: " << SDL_GetError() << std::endl;
        #ifdef __EMSCRIPTEN__
        std::cerr << "Emscripten: Attempting to create fallback font texture..." << std::endl;
        // Create a simple fallback texture for Emscripten
        SDL_Surface* fallbackSurface = SDL_CreateRGBSurface(0, 96, 8, 32, 0, 0, 0, 0);
        if (fallbackSurface) {
            SDL_FillRect(fallbackSurface, NULL, SDL_MapRGB(fallbackSurface->format, 255, 255, 255));
            fontTexture = SDL_CreateTextureFromSurface(renderer, fallbackSurface);
            SDL_FreeSurface(fallbackSurface);
            if (fontTexture) {
                std::cout << "Emscripten: Fallback font texture created successfully" << std::endl;
                return true;
            }
        }
        #endif
        return false;
    }
    
    std::cout << "BitmapFont::loadFont: Font texture created successfully" << std::endl;
    return true;
}

void BitmapFont::renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    if (!fontTexture) return;
    
    int currentX = x;
    for (char c : text) {
        renderChar(renderer, c, currentX, y, color);
        currentX += charWidth;
    }
}

void BitmapFont::renderNumber(SDL_Renderer* renderer, int number, int x, int y, SDL_Color color) {
    if (!fontTexture) return;
    
    std::string numStr = std::to_string(number);
    renderText(renderer, numStr, x, y, color);
}

void BitmapFont::renderChar(SDL_Renderer* renderer, char c, int x, int y, SDL_Color color) {
    if (!fontTexture) return;
    
    // dbyte font uses full ASCII range (0-255) in a 16x16 grid
    // Each character maps directly to its ASCII value
    int charIndex = static_cast<unsigned char>(c);
    
    // Calculate source rectangle for the character
    int srcX = (charIndex % charsPerRow) * charWidth;
    int srcY = (charIndex / charsPerRow) * charHeight;
    
    SDL_Rect srcRect = {srcX, srcY, charWidth, charHeight};
    SDL_Rect destRect = {x, y, charWidth, charHeight};
    
    // Set color modulation
    SDL_SetTextureColorMod(fontTexture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(fontTexture, color.a);
    
    SDL_RenderCopy(renderer, fontTexture, &srcRect, &destRect);
}
