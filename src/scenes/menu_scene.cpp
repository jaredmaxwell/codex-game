#include "menu_scene.h"
#include "../rendering/bitmap_font.h"
#include <iostream>

// Game constants (matching game.cpp)
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Global font reference
static BitmapFont* g_font = nullptr;

MenuScene::MenuScene() {
    // Constructor
}

MenuScene::~MenuScene() {
    // Cleanup will be handled by the scene manager
}

bool MenuScene::initialize(SDL_Renderer* renderer) {
    m_renderer = renderer;
    
    // Load bitmap font if not already loaded
    if (!g_font) {
        g_font = new BitmapFont();
        const char* fontPath = "assets/dbyte_1x.png";
        
        if (!g_font->loadFont(renderer, fontPath)) {
            std::cerr << "Failed to load bitmap font for menu - text rendering will be disabled" << std::endl;
            delete g_font;
            g_font = nullptr;
        } else {
            std::cout << "Menu bitmap font loaded successfully!" << std::endl;
        }
    }
    
    m_close = false;
    m_selectedItem = 0;
    
    return true;
}

void MenuScene::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_QUIT) {
        m_close = true;
    } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_F1:
            case SDLK_ESCAPE:
                m_close = true;
                break;
            case SDLK_w:
            case SDLK_UP:
                m_selectedItem = (m_selectedItem - 1 + MENU_ITEMS) % MENU_ITEMS;
                break;
            case SDLK_s:
            case SDLK_DOWN:
                m_selectedItem = (m_selectedItem + 1) % MENU_ITEMS;
                break;
            case SDLK_j:
            case SDLK_RETURN:
            case SDLK_SPACE:
                // Handle menu selection
                switch (m_selectedItem) {
                    case 0: // Resume Game
                        m_close = true;
                        break;
                    case 1: // Toggle Fullscreen
                        // This will be handled by the scene manager
                        m_close = true;
                        break;
                    case 2: // Restart Game
                        // This will be handled by the scene manager
                        m_close = true;
                        break;
                    case 3: // Quit Game
                        m_close = true;
                        break;
                }
                break;
        }
    }
}

void MenuScene::update() {
    // Menu doesn't need continuous updates
}

void MenuScene::render() {
    // Clear screen with dark background
    SDL_SetRenderDrawColor(m_renderer, 20, 20, 40, 255);
    SDL_RenderClear(m_renderer);
    
    // Render menu title
    SDL_Color titleColor = {255, 255, 255, 255};
    SDL_Color selectedColor = {255, 255, 0, 255};
    SDL_Color normalColor = {200, 200, 200, 255};
    
    if (g_font) {
        // Title
        g_font->renderText(m_renderer, "GAME MENU", SCREEN_WIDTH / 2 - 40, 150, titleColor);
        
        // Menu items
        for (int i = 0; i < MENU_ITEMS; i++) {
            SDL_Color color = (i == m_selectedItem) ? selectedColor : normalColor;
            int y = 250 + i * 40;
            g_font->renderText(m_renderer, m_menuItems[i], SCREEN_WIDTH / 2 - 60, y, color);
        }
        
        // Instructions
        SDL_Color instructionColor = {150, 150, 150, 255};
        g_font->renderText(m_renderer, "Use W/S or UP/DOWN to navigate, J to select", SCREEN_WIDTH / 2 - 130, 450, instructionColor);
        g_font->renderText(m_renderer, "Press F1 or ESC to close menu", SCREEN_WIDTH / 2 - 100, 480, instructionColor);
    }
    
    SDL_RenderPresent(m_renderer);
}
