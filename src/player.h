#pragma once
#include <SDL.h>

enum Direction { UP, DOWN, LEFT, RIGHT };

struct Attack {
    bool active;
    SDL_Rect rect;
    Uint32 startTime;
};

class Player {
public:
    Player();
    ~Player();
    
    // Initialize player
    void initialize(int startX, int startY);
    
    // Update player state
    void update();
    
    // Handle input
    void handleInput(const Uint8* keystate);
    void handleAttack();
    
    // Getters
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    Direction getDirection() const { return m_dir; }
    const Attack& getAttack() const { return m_attack; }
    SDL_Rect getRect() const { return {m_x, m_y, PLAYER_SIZE, PLAYER_SIZE}; }
    
    // Setters
    void setPosition(int x, int y) { m_x = x; m_y = y; }
    
    // Constants
    static const int PLAYER_SIZE = 16;
    static const int PLAYER_SPEED = 5;
    static const int ATTACK_DURATION = 200; // milliseconds

private:
    int m_x, m_y;
    Direction m_dir;
    Attack m_attack;
};
