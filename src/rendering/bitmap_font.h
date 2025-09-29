#pragma once
#include <SDL.h>
#include <string>

class BitmapFont {
public:
    BitmapFont();
    ~BitmapFont();
    
    bool loadFont(SDL_Renderer* renderer, const char* fontPath);
    void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color = {255, 255, 255, 255});
    void renderNumber(SDL_Renderer* renderer, int number, int x, int y, SDL_Color color = {255, 255, 255, 255});
    
    int getCharWidth() const { return charWidth; }
    int getCharHeight() const { return charHeight; }
    
private:
    SDL_Texture* fontTexture;
    int charWidth;
    int charHeight;
    int charsPerRow;
    
    void renderChar(SDL_Renderer* renderer, char c, int x, int y, SDL_Color color);
};
