#pragma once
#include <SDL.h>
#include <string>
#include "entity.h"

enum class ProjectileType {
    BOMB,
    ARROW,
    FIREBALL,
    SWORD_SLASH
};

class PlayerProjectile : public Entity {
public:
    PlayerProjectile();
    ~PlayerProjectile();
    
    // Initialize projectile with type, position, and direction
    void initialize(ProjectileType type, int x, int y, float dirX, float dirY);
    
    // Update projectile state
    void update() override;
    
    // Render projectile
    void render(SDL_Renderer* renderer, SDL_Texture* texture, int cameraOffsetX, int cameraOffsetY) const override;
    
    // Getters
    ProjectileType getType() const { return m_type; }
    bool isExploded() const { return m_exploded; }
    void setExploded(bool exploded) { m_exploded = exploded; }
    bool isStopped() const { return m_stopped; }
    void setStopped(bool stopped) { m_stopped = stopped; }
    Uint32 getExplosionTime() const { return m_explosionTime; }
    float getExplosionRadius() const { return m_explosionRadius; }
    bool shouldExplode() const;
    
    // Entity interface
    int getSize() const override;
    
    // Timer display (public for rendering)
    void renderTimer(SDL_Renderer* renderer, int cameraOffsetX, int cameraOffsetY) const;
    std::string getTimerText() const;
    
    // Constants
    static const int PROJECTILE_SIZE = 8;
    static const int BOMB_SIZE = 12;
    static const float BOMB_SPEED;
    static const float ARROW_SPEED;
    static const float FIREBALL_SPEED;
    static const int BOMB_TIMER_MS = 3000; // 3 seconds
    static constexpr float BOMB_EXPLOSION_RADIUS = 100.0f; // 100px radius
    static constexpr float ARROW_EXPLOSION_RADIUS = 0.0f; // No explosion
    static constexpr float FIREBALL_EXPLOSION_RADIUS = 50.0f; // 50px radius
    
private:
    ProjectileType m_type;
    float m_dirX, m_dirY;
    float m_speed;
    Uint32 m_spawnTime;
    Uint32 m_explosionTime;
    bool m_exploded;
    bool m_stopped;
    float m_explosionRadius;
    
    // Movement and collision
    void moveProjectile();
    void checkExplosion();
};
