#pragma once
#include <SDL.h>
#include <string>

class MenuScene {
public:
    MenuScene();
    ~MenuScene();
    
    // Initialize the menu scene
    bool initialize(SDL_Renderer* renderer);
    
    // Main menu loop functions
    void update();
    void render();
    
    // Check if menu should close
    bool shouldClose() const { return m_close; }
    
    // Handle events
    void handleEvent(const SDL_Event& event);
    
    // Reset close state
    void reset() { m_close = false; }
    
    // Get selected menu action
    int getSelectedAction() const { return m_selectedItem; }
    
private:
    // Menu state
    bool m_close = false;
    SDL_Renderer* m_renderer = nullptr;
    
    // Menu items
    int m_selectedItem = 0;
    static const int MENU_ITEMS = 4;
    std::string m_menuItems[MENU_ITEMS] = {
        "Resume Game",
        "Toggle Fullscreen",
        "Restart Game", 
        "Quit Game"
    };
};
