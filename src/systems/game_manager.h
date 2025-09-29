#pragma once
#include <vector>
#include <SDL.h>
#include "../entities/entity.h"
#include "../entities/player.h"
#include "../entities/enemy.h"
#include "../entities/pet.h"
#include "../entities/item.h"

// Forward declarations
class AssetManager;

class GameManager {
public:
    GameManager();
    ~GameManager();
    
    // Initialize game manager
    void initialize(int worldWidth, int worldHeight);
    
    // Update all game entities
    void update(Uint32 currentTime);
    
    // Render all game entities
    void render(SDL_Renderer* renderer, AssetManager* assetManager, int cameraOffsetX, int cameraOffsetY);
    
    // Handle collisions between all entities
    void handleCollisions(Uint32 currentTime);
    
    // Getters
    Player& getPlayer() { return m_player; }
    Pet& getPet() { return m_pet; }
    const std::vector<Enemy>& getEnemies() const { return m_enemies; }
    const std::vector<Item>& getItems() const { return m_items; }
    
    // Game state
    int getScore() const { return m_player.getScore(); }
    
    // Reset game
    void reset();
    
private:
    // Game entities
    Player m_player;
    Pet m_pet;
    std::vector<Enemy> m_enemies;
    std::vector<Item> m_items;
    
    // Game state
    Uint32 m_lastEnemySpawn;
    Uint32 m_magnetEffectEndTime;
    
    // Explosion effects
    struct Explosion {
        int x, y;
        float radius;
        Uint32 startTime;
        Uint32 duration;
        bool active;
    };
    std::vector<Explosion> m_explosions;
    
    // World bounds
    int m_worldWidth;
    int m_worldHeight;
    
    // Helper methods
    void spawnEnemies(Uint32 currentTime);
    void updateEnemies(Uint32 currentTime);
    void updateItems(Uint32 currentTime);
    void cleanupInactiveEntities();
    
    // Collision handling
    void handlePlayerAttackCollisions(Uint32 currentTime);
    void handlePlayerEnemyCollisions();
    void handleProjectileCollisions(Uint32 currentTime);
    void handleExplosionDamage(int explosionX, int explosionY, float explosionRadius);
    
    // Explosion rendering
    void renderExplosions(SDL_Renderer* renderer, int cameraOffsetX, int cameraOffsetY);
    void updateExplosions(Uint32 currentTime);
    
    // Projectile timer rendering
    void renderProjectileTimers(SDL_Renderer* renderer, AssetManager* assetManager, int cameraOffsetX, int cameraOffsetY);
};
