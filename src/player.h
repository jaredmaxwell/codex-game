#pragma once
#include <SDL.h>
#include "entity.h"

enum Direction { UP, DOWN, LEFT, RIGHT };

struct Attack {
    bool active;
    SDL_Rect rect;
    Uint32 startTime;
};

class Player : public Entity {
public:
    Player();
    ~Player();
    
    // Initialize player
    void initialize(int startX, int startY);
    
    // Update player state
    void update() override;
    
    // Render player
    void render(SDL_Renderer* renderer, SDL_Texture* texture, int cameraOffsetX, int cameraOffsetY) const override;
    
    // Handle input
    void handleInput(const Uint8* keystate);
    void handleAttack();
    
    // Player state management
    void handleDeath();
    void respawn(int worldWidth, int worldHeight);
    bool isAlive() const { return m_alive; }
    
    // Score management
    int getScore() const { return m_score; }
    void addScore(int points) { m_score += points; }
    void resetScore() { m_score = 0; }
    
    // Getters
    Direction getDirection() const { return m_dir; }
    const Attack& getAttack() const { return m_attack; }
    
    // Entity interface
    int getSize() const override { return PLAYER_SIZE; }
    
    // Constants
    static const int PLAYER_SIZE = 16;
    static const int PLAYER_SPEED = 5;
    static const int ATTACK_DURATION = 200; // milliseconds

private:
    Direction m_dir;
    Attack m_attack;
    bool m_alive;
    int m_score;
};
