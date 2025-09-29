#include "game_manager.h"
#include "asset_manager.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    
    // Handle projectile collisions
    handleProjectileCollisions(currentTime);
    
    // Update explosions
    updateExplosions(currentTime);
    
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
    
    // Render player projectiles
    m_player.renderProjectiles(renderer, cameraOffsetX, cameraOffsetY);
    
    // Render projectile timers with font
    renderProjectileTimers(renderer, assetManager, cameraOffsetX, cameraOffsetY);
    
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
    
    // Render explosions
    renderExplosions(renderer, cameraOffsetX, cameraOffsetY);
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

void GameManager::handleProjectileCollisions(Uint32 currentTime) {
    // Check each player projectile for collisions and explosions
    for (auto& projectile : m_player.getProjectiles()) {
        if (!projectile.isActive()) continue;
        
        // Check if projectile should explode or has already exploded
        if (projectile.shouldExplode() || projectile.isExploded()) {
            // Process explosion if it should explode but hasn't been processed yet
            if (projectile.shouldExplode() && !projectile.isExploded()) {
                // Handle explosion damage
                handleExplosionDamage(projectile.getX(), projectile.getY(), projectile.getExplosionRadius());
                std::cout << "Projectile exploded at (" << projectile.getX() << ", " << projectile.getY() << ") with radius " << projectile.getExplosionRadius() << std::endl;
                
                // Create explosion effect
                if (projectile.getType() == ProjectileType::BOMB) {
                    Explosion explosion;
                    explosion.x = projectile.getX();
                    explosion.y = projectile.getY();
                    explosion.radius = projectile.getExplosionRadius();
                    explosion.startTime = SDL_GetTicks();
                    explosion.duration = 1000; // 1 second for better visibility
                    explosion.active = true;
                    m_explosions.push_back(explosion);
                    std::cout << "Created explosion effect at (" << explosion.x << ", " << explosion.y << ") with radius " << explosion.radius << std::endl;
                }
                
                // Mark projectile as exploded after processing
                projectile.setExploded(true);
            }
        }
        
        // Check collision with enemies (for direct hit projectiles like arrows and bombs)
        if (projectile.getType() == ProjectileType::ARROW || projectile.getType() == ProjectileType::BOMB) {
            for (auto& enemy : m_enemies) {
                if (enemy.isActive() && enemy.checkCollision(projectile.getRect())) {
                    if (projectile.getType() == ProjectileType::ARROW) {
                        // Direct hit - damage enemy
                        enemy.takeDamage();
                        
                        // Calculate knockback direction
                        float dx = enemy.getX() - projectile.getX();
                        float dy = enemy.getY() - projectile.getY();
                        float distance = sqrt(dx * dx + dy * dy);
                        enemy.applyKnockback(dx, dy, distance, currentTime);
                        
                        // Handle enemy death and item drops
                        if (!enemy.isActive()) {
                            enemy.handleDeath(m_items, currentTime);
                        }
                        
                        // Remove projectile on hit
                        projectile.setActive(false);
                    } else if (projectile.getType() == ProjectileType::BOMB) {
                        // Bomb hit enemy - stop moving but keep timer running
                        projectile.setStopped(true);
                        std::cout << "Bomb hit enemy and stopped at (" << projectile.getX() << ", " << projectile.getY() << ") - waiting for timer" << std::endl;
                    }
                    break;
                }
            }
        }
    }
    
    // Remove exploded projectiles after processing
    m_player.removeExplodedProjectiles();
}

void GameManager::handleExplosionDamage(int explosionX, int explosionY, float explosionRadius) {
    if (explosionRadius <= 0) return;
    
    int enemiesHit = 0;
    
    // Check each enemy for explosion damage
    for (auto& enemy : m_enemies) {
        if (!enemy.isActive()) continue;
        
        // Calculate distance from explosion center to enemy center
        float dx = enemy.getCenterX() - explosionX;
        float dy = enemy.getCenterY() - explosionY;
        float distance = sqrt(dx * dx + dy * dy);
        
        // If enemy is within explosion radius, damage it
        if (distance <= explosionRadius) {
            enemiesHit++;
            
            // Damage enemy
            enemy.takeDamage();
            
            // Apply knockback away from explosion center
            if (distance > 0) {
                float knockbackX = dx / distance;
                float knockbackY = dy / distance;
                enemy.applyKnockback(knockbackX, knockbackY, distance, SDL_GetTicks());
            }
            
            // Handle enemy death and item drops
            if (!enemy.isActive()) {
                std::cout << "Enemy killed by explosion!" << std::endl;
                enemy.handleDeath(m_items, SDL_GetTicks());
            }
        }
    }
    
}

void GameManager::updateExplosions(Uint32 currentTime) {
    // Update explosion effects
    for (auto& explosion : m_explosions) {
        if (explosion.active && currentTime - explosion.startTime > explosion.duration) {
            explosion.active = false;
        }
    }
    
    // Remove inactive explosions
    m_explosions.erase(
        std::remove_if(m_explosions.begin(), m_explosions.end(),
            [](const Explosion& e) { return !e.active; }),
        m_explosions.end()
    );
}

void GameManager::renderExplosions(SDL_Renderer* renderer, int cameraOffsetX, int cameraOffsetY) {
    for (const auto& explosion : m_explosions) {
        if (!explosion.active) continue;
        
        // Calculate explosion progress (0.0 to 1.0)
        Uint32 currentTime = SDL_GetTicks();
        float progress = static_cast<float>(currentTime - explosion.startTime) / explosion.duration;
        if (progress > 1.0f) progress = 1.0f;
        
        // Calculate current radius (grows from 0 to full radius)
        float currentRadius = explosion.radius * progress;
        
        // Draw explosion circle
        int centerX = explosion.x + cameraOffsetX;
        int centerY = explosion.y + cameraOffsetY;
        
        // Draw a simple filled circle for better visibility
        SDL_SetRenderDrawColor(renderer, 255, 100, 0, 150); // Orange with transparency
        
        // Draw filled circle using multiple concentric circles
        for (int r = 0; r < static_cast<int>(currentRadius); r += 2) {
            int segments = 16;
            for (int j = 0; j < segments; j++) {
                float angle1 = (2.0f * M_PI * j) / segments;
                float angle2 = (2.0f * M_PI * (j + 1)) / segments;
                
                int x1 = centerX + static_cast<int>(r * cos(angle1));
                int y1 = centerY + static_cast<int>(r * sin(angle1));
                int x2 = centerX + static_cast<int>(r * cos(angle2));
                int y2 = centerY + static_cast<int>(r * sin(angle2));
                
                SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            }
        }
        
        // Draw outer circle outline
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow outline
        int segments = 32;
        for (int j = 0; j < segments; j++) {
            float angle1 = (2.0f * M_PI * j) / segments;
            float angle2 = (2.0f * M_PI * (j + 1)) / segments;
            
            int x1 = centerX + static_cast<int>(currentRadius * cos(angle1));
            int y1 = centerY + static_cast<int>(currentRadius * sin(angle1));
            int x2 = centerX + static_cast<int>(currentRadius * cos(angle2));
            int y2 = centerY + static_cast<int>(currentRadius * sin(angle2));
            
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
}

void GameManager::renderProjectileTimers(SDL_Renderer* renderer, AssetManager* assetManager, int cameraOffsetX, int cameraOffsetY) {
    if (!assetManager || !assetManager->getFont()) return;
    
    for (const auto& projectile : m_player.getProjectiles()) {
        if (!projectile.isActive() || projectile.isExploded() || projectile.getType() != ProjectileType::BOMB) continue;
        
        // Get timer text
        std::string timerText = projectile.getTimerText();
        if (timerText.empty()) continue;
        
        // Calculate position above the projectile
        int timerX = projectile.getX() + projectile.getSize() / 2 + cameraOffsetX;
        int timerY = projectile.getY() - 20 + cameraOffsetY;
        
        // Set text color (white)
        SDL_Color textColor = {255, 255, 255, 255};
        
        // Render the timer text using bitmap font
        assetManager->getFont()->renderText(renderer, timerText, timerX - 10, timerY - 5, textColor);
    }
}

