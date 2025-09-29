#include "player_select_scene.h"
#include "../rendering/bitmap_font.h"
#include <iostream>

// Game constants (matching other scenes)
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Global font reference
static BitmapFont* g_font = nullptr;

PlayerSelectScene::PlayerSelectScene() {
    // Constructor
}

PlayerSelectScene::~PlayerSelectScene() {
    // Cleanup will be handled by the scene manager
}

bool PlayerSelectScene::initialize(SDL_Renderer* renderer) {
    m_renderer = renderer;
    
    // Load bitmap font if not already loaded
    if (!g_font) {
        g_font = new BitmapFont();
        const char* fontPath = "assets/dbyte_1x.png";
        
        if (!g_font->loadFont(renderer, fontPath)) {
            std::cerr << "Failed to load bitmap font for player select - text rendering will be disabled" << std::endl;
            delete g_font;
            g_font = nullptr;
        } else {
            std::cout << "Player select bitmap font loaded successfully!" << std::endl;
        }
    }
    
    m_close = false;
    m_selectedItem = 0;
    m_selectedClass = CharacterClass::SWORDSMAN;
    
    return true;
}

void PlayerSelectScene::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_QUIT) {
        m_close = true;
    } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                m_close = true;
                break;
            case SDLK_w:
            case SDLK_UP:
                m_selectedItem = (m_selectedItem - 1 + CHARACTER_COUNT) % CHARACTER_COUNT;
                m_selectedClass = m_characters[m_selectedItem].classType;
                break;
            case SDLK_s:
            case SDLK_DOWN:
                m_selectedItem = (m_selectedItem + 1) % CHARACTER_COUNT;
                m_selectedClass = m_characters[m_selectedItem].classType;
                break;
            case SDLK_a:
            case SDLK_LEFT:
                m_selectedItem = (m_selectedItem - 1 + CHARACTER_COUNT) % CHARACTER_COUNT;
                m_selectedClass = m_characters[m_selectedItem].classType;
                break;
            case SDLK_d:
            case SDLK_RIGHT:
                m_selectedItem = (m_selectedItem + 1) % CHARACTER_COUNT;
                m_selectedClass = m_characters[m_selectedItem].classType;
                break;
            case SDLK_j:
            case SDLK_RETURN:
            case SDLK_SPACE:
                // Confirm character selection
                m_close = true;
                break;
        }
    }
}

void PlayerSelectScene::update() {
    // Scene doesn't need continuous updates
}

void PlayerSelectScene::render() {
    // Clear screen with dark background
    SDL_SetRenderDrawColor(m_renderer, 20, 40, 20, 255);
    SDL_RenderClear(m_renderer);
    
    if (g_font) {
        // Title
        SDL_Color titleColor = {255, 255, 255, 255};
        SDL_Color selectedColor = {255, 255, 0, 255};
        SDL_Color normalColor = {200, 200, 200, 255};
        SDL_Color descriptionColor = {150, 200, 150, 255};
        
        // Title
        g_font->renderText(m_renderer, "SELECT YOUR CHARACTER", SCREEN_WIDTH / 2 - 100, 80, titleColor);
        
        // Character selection grid (2x2)
        int startX = SCREEN_WIDTH / 2 - 200;
        int startY = 150;
        int spacingX = 200;
        int spacingY = 120;
        
        for (int i = 0; i < CHARACTER_COUNT; i++) {
            int row = i / 2;
            int col = i % 2;
            int x = startX + col * spacingX;
            int y = startY + row * spacingY;
            
            // Character box background
            SDL_Rect charBox = {x - 20, y - 20, 160, 100};
            SDL_Color boxColor = (i == m_selectedItem) ? 
                SDL_Color{100, 100, 50, 255} : 
                SDL_Color{50, 50, 50, 255};
            
            SDL_SetRenderDrawColor(m_renderer, boxColor.r, boxColor.g, boxColor.b, boxColor.a);
            SDL_RenderFillRect(m_renderer, &charBox);
            
            // Character box border
            SDL_Color borderColor = (i == m_selectedItem) ? 
                SDL_Color{255, 255, 0, 255} : 
                SDL_Color{100, 100, 100, 255};
            
            SDL_SetRenderDrawColor(m_renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
            SDL_RenderDrawRect(m_renderer, &charBox);
            
            // Character name
            SDL_Color nameColor = (i == m_selectedItem) ? selectedColor : normalColor;
            g_font->renderText(m_renderer, m_characters[i].name, x, y, nameColor);
            
            // Character description (wrapped)
            std::string desc = m_characters[i].description;
            // Simple word wrapping for description
            if (desc.length() > 20) {
                // Find a good break point
                size_t breakPoint = desc.find(' ', 15);
                if (breakPoint != std::string::npos) {
                    std::string line1 = desc.substr(0, breakPoint);
                    std::string line2 = desc.substr(breakPoint + 1);
                    g_font->renderText(m_renderer, line1, x, y + 20, descriptionColor);
                    g_font->renderText(m_renderer, line2, x, y + 35, descriptionColor);
                } else {
                    g_font->renderText(m_renderer, desc, x, y + 20, descriptionColor);
                }
            } else {
                g_font->renderText(m_renderer, desc, x, y + 20, descriptionColor);
            }
        }
        
        // Instructions
        SDL_Color instructionColor = {150, 150, 150, 255};
        g_font->renderText(m_renderer, "Use WASD or Arrow Keys to navigate", SCREEN_WIDTH / 2 - 120, 450, instructionColor);
        g_font->renderText(m_renderer, "Press J or ENTER to select character", SCREEN_WIDTH / 2 - 130, 480, instructionColor);
        g_font->renderText(m_renderer, "Press ESC to go back", SCREEN_WIDTH / 2 - 80, 510, instructionColor);
    }
    
    SDL_RenderPresent(m_renderer);
}
