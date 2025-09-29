#pragma once
#include <SDL.h>
#include <vector>

// Forward declarations
class Player;
class Enemy;

struct Projectile {
    bool active;
    int x, y;
    float velocityX, velocityY;
    Uint32 spawnTime;
    static const int SIZE = 4;
    static const int SPEED = 8;
    static const int LIFETIME = 2000; // 2 seconds
};

class Pet {
public:
    Pet();
    ~Pet();
    
    // Initialize pet
    void initialize(int startX, int startY);
    
    // Update pet state
    void update(const Player& player, const std::vector<Enemy>& enemies, Uint32 currentTime);
    
    // Render the pet
    void render(SDL_Renderer* renderer, SDL_Texture* texture, int cameraOffsetX, int cameraOffsetY) const;
    
    // Getters
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getCenterX() const { return m_x + SIZE / 2; }
    int getCenterY() const { return m_y + SIZE / 2; }
    SDL_Rect getRect() const { return {m_x, m_y, SIZE, SIZE}; }
    bool isActive() const { return m_active; }
    
    // Setters
    void setPosition(int x, int y) { m_x = x; m_y = y; }
    void setActive(bool active) { m_active = active; }
    
    // Projectile management
    const std::vector<Projectile>& getProjectiles() const { return m_projectiles; }
    void updateProjectiles(Uint32 currentTime);
    void renderProjectiles(SDL_Renderer* renderer, int cameraOffsetX, int cameraOffsetY) const;
    
    // Constants
    static const int SIZE = 12;
    static const int FOLLOW_DISTANCE = 40;
    static const int SHOOT_COOLDOWN = 1000; // 1 second between shots
    static const int DETECTION_RANGE = 150; // Range to detect enemies
    
private:
    // Position and state
    int m_x, m_y;
    bool m_active;
    
    // Shooting
    std::vector<Projectile> m_projectiles;
    Uint32 m_lastShotTime;
    
    // Helper methods
    void followPlayer(const Player& player);
    void findAndShootNearestEnemy(const std::vector<Enemy>& enemies, Uint32 currentTime);
    void shootAt(int targetX, int targetY, Uint32 currentTime);
    float distanceTo(int x, int y) const;
};
