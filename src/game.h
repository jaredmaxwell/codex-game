#pragma once
#include <SDL.h>
#include <string>

class GameScene {
public:
    GameScene();
    ~GameScene();
    
    // Initialize the game scene
    bool initialize(SDL_Renderer* renderer);
    
    // Main game loop function
    void update();
    void render();
    
    // Check if game should quit
    bool shouldQuit() const { return m_quit; }
    
    // Handle events
    void handleEvent(const SDL_Event& event);
    
private:
    // Game state
    bool m_quit = false;
    SDL_Renderer* m_renderer = nullptr;
    
    // Game objects and state will be moved here from main.cpp
    // (This will be implemented in game.cpp)
};
