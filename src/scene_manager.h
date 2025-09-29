#pragma once
#include <SDL.h>
#include "game.h"
#include "menu_scene.h"
#include "settings.h"

enum class SceneType {
    GAME,
    MENU
};

class SceneManager {
public:
    SceneManager();
    ~SceneManager();
    
    // Initialize the scene manager
    bool initialize(SDL_Renderer* renderer, SDL_Window* window);
    
    // Main loop functions
    void update();
    void render();
    
    // Check if application should quit
    bool shouldQuit() const { return m_quit; }
    
    // Handle events
    void handleEvent(const SDL_Event& event);
    
private:
    // Scene management
    SceneType m_currentScene;
    bool m_quit = false;
    SDL_Renderer* m_renderer = nullptr;
    
    // Scene objects
    GameScene* m_gameScene = nullptr;
    MenuScene* m_menuScene = nullptr;
    
    // Settings
    Settings* m_settings = nullptr;
    
    // Scene switching
    void switchToGame();
    void switchToMenu();
    void handleMenuAction();
    
    // Settings management
    void toggleFullscreen();
    
    // SDL window reference for fullscreen toggle
    SDL_Window* m_window = nullptr;
};
