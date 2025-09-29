#include "game_manager.h"
#include "asset_manager.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

GameManager::GameManager() 
    : m_lastEnemySpawn(0), m_magnetEffectEndTime(0), 
      m_worldWidth(0), m_worldHeight(0) {
    m_enemies.reserve(Enemy::MAX_ENEMIES);
    m_items.reserve(Item::MAX_SHARDS + Item::MAX_MAGNETS);
}

GameManager::~GameManager() {
    // Cleanup handled by destructors
}

void GameManager::initialize(int worldWidth, int worldHeight) {
    m_worldWidth = worldWidth;
    m_worldHeight = worldHeight;
    
    // Initialize player at world center
    m_player.initialize(worldWidth / 2 - Player::PLAYER_SIZE / 2, worldHeight / 2 - Player::PLAYER_SIZE / 2);
    
    // Initialize pet near player
    m_pet.initialize(worldWidth / 2 - Pet::SIZE / 2 + 30, worldHeight / 2 - Pet::SIZE / 2 + 30);
    
    // Clear all entities
    m_enemies.clear();
    m_items.clear();
    
    // Reset game state
    m_lastEnemySpawn = 0;
    m_magnetEffectEndTime = 0;
}

void GameManager::update(Uint32 currentTime) {
    // Update player
    m_player.update();
    
    // Update pet
    m_pet.update(m_player, m_enemies, currentTime);
    
    // Handle pet projectile collisions
    m_pet.handleProjectileCollisions(m_enemies, m_items, currentTime);
    
    // Update enemies and items
    updateEnemies(currentTime);
    updateItems(currentTime);
    
    // Spawn new enemies
    spawnEnemies(currentTime);
    
    // Handle all collisions
    handleCollisions(currentTime);
    
    // Cleanup inactive entities
    cleanupInactiveEntities();
}

void GameManager::render(SDL_Renderer* renderer, AssetManager* assetManager, int cameraOffsetX, int cameraOffsetY) {
    // Render player
    SDL_Texture* playerTexture = nullptr;
    if (assetManager) {
        playerTexture = assetManager->getPlayerTexture();
    }
    m_player.render(renderer, playerTexture, cameraOffsetX, cameraOffsetY);
    
    // Render player attack
    if (m_player.getAttack().active) {
        SDL_Rect attackRect = m_player.getAttack().rect;
        attackRect.x += cameraOffsetX;
        attackRect.y += cameraOffsetY;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &attackRect);
    }
    
    // Render pet
    if (m_pet.isActive()) {
        SDL_Texture* petTexture = nullptr;
        if (assetManager) {
            petTexture = assetManager->getPetTexture();
        }
        m_pet.render(renderer, petTexture, cameraOffsetX, cameraOffsetY);
        m_pet.renderProjectiles(renderer, cameraOffsetX, cameraOffsetY);
    }
    
    // Render enemies
    for (const auto& enemy : m_enemies) {
        if (enemy.isActive()) {
            SDL_Texture* enemyTexture = nullptr;
            if (assetManager) {
                enemyTexture = assetManager->getEnemyTexture(enemy.getLevel());
            }
            enemy.render(renderer, enemyTexture, cameraOffsetX, cameraOffsetY, assetManager->getFont());
        }
    }
    
    // Render items
    for (const auto& item : m_items) {
        if (item.isActive()) {
            item.render(renderer, cameraOffsetX, cameraOffsetY);
        }
    }
}

void GameManager::handleCollisions(Uint32 currentTime) {
    handlePlayerAttackCollisions(currentTime);
    handlePlayerEnemyCollisions();
}

void GameManager::reset() {
    // Reset player position
    m_player.initialize(m_worldWidth / 2 - Player::PLAYER_SIZE / 2, m_worldHeight / 2 - Player::PLAYER_SIZE / 2);
    
    // Reset pet position
    m_pet.initialize(m_worldWidth / 2 - Pet::SIZE / 2 + 30, m_worldHeight / 2 - Pet::SIZE / 2 + 30);
    
    // Clear all entities
    m_enemies.clear();
    m_items.clear();
    
    // Reset game state
    m_lastEnemySpawn = 0;
    m_magnetEffectEndTime = 0;
}

void GameManager::spawnEnemies(Uint32 currentTime) {
    if (currentTime - m_lastEnemySpawn > Enemy::ENEMY_SPAWN_RATE && m_enemies.size() < Enemy::MAX_ENEMIES) {
        // Calculate enemy level based on player's score
        int enemyLevel = Enemy::calculateLevel(m_player.getScore());
        
        // Spawn enemy at viewport edges (not world boundaries)
        int spawnX, spawnY;
        int viewportWidth = 800;  // Game viewport width
        int viewportHeight = 600; // Game viewport height
        int spawnDistance = 100;  // Distance outside viewport to spawn
        
        // Choose random edge (0=top, 1=right, 2=bottom, 3=left)
        int edge = rand() % 4;
        
        switch (edge) {
            case 0: // Top edge - spawn above viewport
                spawnX = m_player.getX() + (rand() % viewportWidth) - viewportWidth/2;
                spawnY = m_player.getY() - viewportHeight/2 - spawnDistance;
                break;
            case 1: // Right edge - spawn to the right of viewport
                spawnX = m_player.getX() + viewportWidth/2 + spawnDistance;
                spawnY = m_player.getY() + (rand() % viewportHeight) - viewportHeight/2;
                break;
            case 2: // Bottom edge - spawn below viewport
                spawnX = m_player.getX() + (rand() % viewportWidth) - viewportWidth/2;
                spawnY = m_player.getY() + viewportHeight/2 + spawnDistance;
                break;
            case 3: // Left edge - spawn to the left of viewport
                spawnX = m_player.getX() - viewportWidth/2 - spawnDistance;
                spawnY = m_player.getY() + (rand() % viewportHeight) - viewportHeight/2;
                break;
        }
        
        // Clamp to world boundaries
        if (spawnX < 0) spawnX = 0;
        if (spawnX >= m_worldWidth) spawnX = m_worldWidth - 1;
        if (spawnY < 0) spawnY = 0;
        if (spawnY >= m_worldHeight) spawnY = m_worldHeight - 1;
        
        // Use Enemy factory method
        Enemy newEnemy = Enemy::createEnemy(spawnX, spawnY, enemyLevel, Enemy::DEFAULT_SPEED, currentTime);
        m_enemies.push_back(newEnemy);
        m_lastEnemySpawn = currentTime;
    }
}

void GameManager::updateEnemies(Uint32 currentTime) {
    for (auto& enemy : m_enemies) {
        if (enemy.isActive()) {
            enemy.update(m_player, m_enemies, m_worldWidth, m_worldHeight, currentTime);
        }
    }
}

void GameManager::updateItems(Uint32 currentTime) {
    bool magnetEffectActive = (currentTime < m_magnetEffectEndTime);
    for (auto& item : m_items) {
        if (item.isActive()) {
            item.update(m_player.getCenterX(), m_player.getCenterY(), currentTime, magnetEffectActive);
            
            // Handle item collection
            int playerScore = m_player.getScore();
            Uint32 magnetEffectEndTime = m_magnetEffectEndTime;
            if (item.handleCollection(m_player.getRect(), currentTime, playerScore, magnetEffectEndTime)) {
                m_player.addScore(playerScore - m_player.getScore()); // Add the difference
                m_magnetEffectEndTime = magnetEffectEndTime;
            }
        }
    }
}

void GameManager::cleanupInactiveEntities() {
    // Remove inactive enemies
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const Enemy& enemy) { return !enemy.isActive(); }),
        m_enemies.end()
    );
    
    // Remove inactive items
    m_items.erase(
        std::remove_if(m_items.begin(), m_items.end(),
            [](const Item& item) { return !item.isActive(); }),
        m_items.end()
    );
}

void GameManager::handlePlayerAttackCollisions(Uint32 currentTime) {
    if (!m_player.getAttack().active) return;
    
    for (auto& enemy : m_enemies) {
        if (enemy.isActive() && enemy.checkCollision(m_player.getAttack().rect)) {
            // Enemy hit by attack
            enemy.takeDamage();
            
            // Calculate knockback direction
            float dx = enemy.getX() - m_player.getX();
            float dy = enemy.getY() - m_player.getY();
            float distance = sqrt(dx * dx + dy * dy);
            enemy.applyKnockback(dx, dy, distance, currentTime);
            
            // Handle enemy death and item drops
            if (!enemy.isActive()) {
                enemy.handleDeath(m_items, currentTime);
            }
        }
    }
}


void GameManager::handlePlayerEnemyCollisions() {
    for (auto& enemy : m_enemies) {
        if (enemy.isActive() && enemy.checkCollisionWithPlayer(m_player)) {
            // Player hit by enemy - handle death
            m_player.handleDeath();
            m_player.respawn(m_worldWidth, m_worldHeight);
            enemy.setActive(false);
        }
    }
}

