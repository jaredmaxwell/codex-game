#pragma once
#include <SDL.h>
#include <string>
#include "../entities/player.h"

class PlayerSelectScene {
public:
    PlayerSelectScene();
    ~PlayerSelectScene();
    
    // Initialize the player select scene
    bool initialize(SDL_Renderer* renderer);
    
    // Main scene loop functions
    void update();
    void render();
    
    // Check if scene should close
    bool shouldClose() const { return m_close; }
    
    // Handle events
    void handleEvent(const SDL_Event& event);
    
    // Reset close state
    void reset() { m_close = false; }
    
    // Get selected character class
    CharacterClass getSelectedClass() const { return m_selectedClass; }
    
private:
    // Scene state
    bool m_close = false;
    SDL_Renderer* m_renderer = nullptr;
    
    // Character selection
    int m_selectedItem = 0;
    CharacterClass m_selectedClass = CharacterClass::SWORDSMAN;
    static const int CHARACTER_COUNT = 4;
    
    // Character class data
    struct CharacterInfo {
        std::string name;
        std::string description;
        CharacterClass classType;
    };
    
    CharacterInfo m_characters[CHARACTER_COUNT] = {
        {"Swordsman", "Melee fighter with high health and close combat skills", CharacterClass::SWORDSMAN},
        {"Bomber", "Explosive specialist with area damage abilities", CharacterClass::BOMBER},
        {"Archer", "Ranged fighter with precision and speed", CharacterClass::ARCHER},
        {"Mage", "Magic user with powerful spells and mana", CharacterClass::MAGE}
    };
};
