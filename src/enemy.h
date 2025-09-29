#pragma once
#include <SDL.h>
#include <vector>

// Forward declarations
class Player;

class Enemy {
public:
    // Constructor
    Enemy();
    
    // Initialize enemy with position and properties
    void initialize(int x, int y, int level, float speed, Uint32 spawnTime);
    
    // Update enemy state (movement, collision avoidance, etc.)
    void update(const Player& player, const std::vector<Enemy>& enemies, 
                int worldWidth, int worldHeight, Uint32 currentTime);
    
    // Update with spatial partitioning for better performance
    void updateWithSpatialPartitioning(const Player& player, const std::vector<Enemy>& enemies,
                                     int worldWidth, int worldHeight, Uint32 currentTime,
                                     const std::vector<int>& nearbyEnemyIndices);
    
    // Render the enemy
    void render(SDL_Renderer* renderer, SDL_Texture* texture, int cameraOffsetX, int cameraOffsetY) const;
    
    // Getters
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getCenterX() const { return m_x + getSize() / 2; }
    int getCenterY() const { return m_y + getSize() / 2; }
    int getSize() const { return BASE_SIZE + (m_level - 1) * 2; }
    int getLevel() const { return m_level; }
    int getOriginalLevel() const { return m_originalLevel; }
    bool isActive() const { return m_active; }
    SDL_Rect getRect() const { return {m_x, m_y, getSize(), getSize()}; }
    
    // Setters
    void setPosition(int x, int y) { m_x = x; m_y = y; }
    void setActive(bool active) { m_active = active; }
    
    // Combat methods
    void takeDamage();
    void applyKnockback(float dx, float dy, float distance, Uint32 currentTime);
    bool isInKnockback(Uint32 currentTime) const { return currentTime < m_knockbackEndTime; }
    
    // Shard properties when enemy is defeated
    void getShardProperties(int& value, SDL_Color& color) const;
    
    // Collision detection
    bool checkCollision(const SDL_Rect& otherRect) const;
    bool checkCollisionWithPlayer(const Player& player) const;
    
    // Static constants
    static constexpr int BASE_SIZE = 12;
    static constexpr float DEFAULT_SPEED = 1.5f;
    static constexpr float KNOCKBACK_DISTANCE_MULTIPLIER = 2.0f;
    static constexpr float SEPARATION_FORCE = 2.0f;
    static constexpr float MIN_DISTANCE = 3.0f;
    static constexpr int KNOCKBACK_DURATION = 200; // milliseconds
    
    // Game-wide enemy constants
    static constexpr int MAX_ENEMIES = 500; // Increased for stress testing
    static constexpr int ENEMY_SPAWN_RATE = 50; // milliseconds between spawns (10x faster for stress testing)
    static constexpr int MAX_ENEMY_LEVEL = 10;
    
    // Helper functions
    static int getEnemySize(int level);
    static int getEnemyCenterX(int enemyX, int enemySize);
    static int getEnemyCenterY(int enemyY, int enemySize);
    
private:
    // Position and movement
    int m_x, m_y;
    float m_speed;
    
    // State
    bool m_active;
    int m_level;
    int m_originalLevel;
    Uint32 m_spawnTime;
    
    // Knockback system
    float m_knockbackX, m_knockbackY;
    Uint32 m_knockbackEndTime;
    
    // Helper methods
    void moveTowardsPlayer(const Player& player);
    void applyCollisionAvoidance(const std::vector<Enemy>& enemies);
    void checkWorldBounds(int worldWidth, int worldHeight);
    
    // Collision avoidance helpers
    void calculateAvoidanceForce(const Enemy& other, float& avoidX, float& avoidY) const;
    void applyCollisionAvoidanceWithSpatialPartitioning(const std::vector<Enemy>& enemies, 
                                                       const std::vector<int>& nearbyEnemyIndices);
};
