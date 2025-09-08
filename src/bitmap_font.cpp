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
    
    SDL_Surface* fontSurface = IMG_Load(fontPath);
    if (!fontSurface) {
        std::cerr << "Failed to load font: " << fontPath << " - " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Convert to texture with alpha channel for better rendering
    SDL_SetColorKey(fontSurface, SDL_TRUE, SDL_MapRGB(fontSurface->format, 0, 0, 0));
    fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
    SDL_FreeSurface(fontSurface);
    
    if (!fontTexture) {
        std::cerr << "Failed to create font texture: " << SDL_GetError() << std::endl;
        return false;
    }
    
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
