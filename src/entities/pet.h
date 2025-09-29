#pragma once
#include <SDL.h>
#include <vector>
#include "entity.h"

// Forward declarations
class Player;
class Enemy;
class Item;

struct Projectile {
    bool active;
    int x, y;
    float velocityX, velocityY;
    Uint32 spawnTime;
    static const int SIZE = 4;
    static const int SPEED = 8;
    static const int LIFETIME = 2000; // 2 seconds
};

class Pet : public Entity {
public:
    Pet();
    ~Pet();
    
    // Initialize pet
    void initialize(int startX, int startY);
    
    // Update pet state
    void update() override;
    void update(const Player& player, const std::vector<Enemy>& enemies, Uint32 currentTime);
    
    // Render the pet
    void render(SDL_Renderer* renderer, SDL_Texture* texture, int cameraOffsetX, int cameraOffsetY) const override;
    
    // Entity interface
    int getSize() const override { return SIZE; }
    
    // Projectile management
    const std::vector<Projectile>& getProjectiles() const { return m_projectiles; }
    void updateProjectiles(Uint32 currentTime);
    void renderProjectiles(SDL_Renderer* renderer, int cameraOffsetX, int cameraOffsetY) const;
    
    // Collision handling
    void handleProjectileCollisions(std::vector<Enemy>& enemies, std::vector<Item>& items, Uint32 currentTime);
    
    // Constants
    static const int SIZE = 12;
    static const int FOLLOW_DISTANCE = 40;
    static const int SHOOT_COOLDOWN = 1000; // 1 second between shots
    static const int DETECTION_RANGE = 150; // Range to detect enemies
    
private:
    
    // Shooting
    std::vector<Projectile> m_projectiles;
    Uint32 m_lastShotTime;
    
    // Helper methods
    void followPlayer(const Player& player);
    void findAndShootNearestEnemy(const std::vector<Enemy>& enemies, Uint32 currentTime);
    void shootAt(int targetX, int targetY, Uint32 currentTime);
    float distanceTo(int x, int y) const;
};
