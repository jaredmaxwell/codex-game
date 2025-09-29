#pragma once
#include <SDL.h>
#include <string>
#include "../utils/tmx_loader.h"
#include "../rendering/camera.h"
#include "../entities/player.h"

class GameScene {
public:
    GameScene();
    ~GameScene();
    
    // Initialize the game scene
    bool initialize(SDL_Renderer* renderer);
    
    // Set character class for the player
    void setCharacterClass(CharacterClass characterClass);
    
    // Main game loop function
    void update();
    void render();
    
    // Check if game should quit
    bool shouldQuit() const { return m_quit; }
    
    // Handle events
    void handleEvent(const SDL_Event& event);
    
    // Restart the game
    void restart();
    
    // Handle window resize events
    void handleWindowResize(int newWidth, int newHeight);
    
private:
    // Game state
    bool m_quit = false;
    SDL_Renderer* m_renderer = nullptr;
    
    // Camera system
    Camera m_camera;
    
    // Tilemap data
    TilemapData m_tilemap;
    TMXLoader m_tmxLoader;
    
    // Game objects and state will be moved here from main.cpp
    // (This will be implemented in game.cpp)
    
    // Character class
    CharacterClass m_characterClass = CharacterClass::SWORDSMAN;
};
