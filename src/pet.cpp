#include "pet.h"
#include "player.h"
#include "enemy.h"
#include <cmath>
#include <algorithm>

Pet::Pet() : m_x(0), m_y(0), m_active(false), m_lastShotTime(0) {
    m_projectiles.reserve(20); // Reserve space for projectiles
}

Pet::~Pet() {
    // Cleanup handled by vector destructor
}

void Pet::initialize(int startX, int startY) {
    m_x = startX;
    m_y = startY;
    m_active = true;
    m_lastShotTime = 0;
    m_projectiles.clear();
}

void Pet::update(const Player& player, const std::vector<Enemy>& enemies, Uint32 currentTime) {
    if (!m_active) return;
    
    // Follow the player
    followPlayer(player);
    
    // Find and shoot at nearest enemy
    findAndShootNearestEnemy(enemies, currentTime);
    
    // Update projectiles
    updateProjectiles(currentTime);
}

void Pet::followPlayer(const Player& player) {
    int playerCenterX = player.getCenterX();
    int playerCenterY = player.getCenterY();
    int petCenterX = getCenterX();
    int petCenterY = getCenterY();
    
    // Calculate distance to player
    float dx = playerCenterX - petCenterX;
    float dy = playerCenterY - petCenterY;
    float distance = sqrt(dx * dx + dy * dy);
    
    // If pet is too far from player, move towards player
    if (distance > FOLLOW_DISTANCE) {
        // Normalize direction and move towards player
        if (distance > 0) {
            float moveX = (dx / distance) * 5.0f; // Increased pet speed from 3.0f to 5.0f
            float moveY = (dy / distance) * 5.0f;
            
            m_x += static_cast<int>(moveX);
            m_y += static_cast<int>(moveY);
        }
    }
}

void Pet::findAndShootNearestEnemy(const std::vector<Enemy>& enemies, Uint32 currentTime) {
    // Check if we can shoot (cooldown)
    if (currentTime - m_lastShotTime < SHOOT_COOLDOWN) {
        return;
    }
    
    float nearestDistance = DETECTION_RANGE;
    int nearestEnemyIndex = -1;
    
    // Find nearest enemy within detection range
    for (size_t i = 0; i < enemies.size(); i++) {
        if (!enemies[i].isActive()) continue;
        
        float distance = distanceTo(enemies[i].getCenterX(), enemies[i].getCenterY());
        if (distance < nearestDistance) {
            nearestDistance = distance;
            nearestEnemyIndex = static_cast<int>(i);
        }
    }
    
    // Shoot at nearest enemy if found
    if (nearestEnemyIndex >= 0) {
        shootAt(enemies[nearestEnemyIndex].getCenterX(), 
                enemies[nearestEnemyIndex].getCenterY(), 
                currentTime);
    }
}

void Pet::shootAt(int targetX, int targetY, Uint32 currentTime) {
    // Calculate direction to target
    float dx = targetX - getCenterX();
    float dy = targetY - getCenterY();
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance > 0) {
        // Normalize direction
        float velocityX = (dx / distance) * Projectile::SPEED;
        float velocityY = (dy / distance) * Projectile::SPEED;
        
        // Create new projectile
        Projectile projectile;
        projectile.active = true;
        projectile.x = getCenterX() - Projectile::SIZE / 2;
        projectile.y = getCenterY() - Projectile::SIZE / 2;
        projectile.velocityX = velocityX;
        projectile.velocityY = velocityY;
        projectile.spawnTime = currentTime;
        
        m_projectiles.push_back(projectile);
        m_lastShotTime = currentTime;
    }
}

float Pet::distanceTo(int x, int y) const {
    float dx = x - getCenterX();
    float dy = y - getCenterY();
    return sqrt(dx * dx + dy * dy);
}

void Pet::updateProjectiles(Uint32 currentTime) {
    for (auto& projectile : m_projectiles) {
        if (!projectile.active) continue;
        
        // Check lifetime
        if (currentTime - projectile.spawnTime > Projectile::LIFETIME) {
            projectile.active = false;
            continue;
        }
        
        // Update position
        projectile.x += static_cast<int>(projectile.velocityX);
        projectile.y += static_cast<int>(projectile.velocityY);
    }
    
    // Remove inactive projectiles
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const Projectile& p) { return !p.active; }),
        m_projectiles.end()
    );
}

void Pet::render(SDL_Renderer* renderer, SDL_Texture* texture, int cameraOffsetX, int cameraOffsetY) const {
    if (!m_active) return;
    
    SDL_Rect petRect = getRect();
    petRect.x += cameraOffsetX;
    petRect.y += cameraOffsetY;
    
    if (texture) {
        SDL_RenderCopy(renderer, texture, NULL, &petRect);
    } else {
        // Fallback: draw a colored rectangle
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Cyan color for pet
        SDL_RenderFillRect(renderer, &petRect);
    }
}

void Pet::renderProjectiles(SDL_Renderer* renderer, int cameraOffsetX, int cameraOffsetY) const {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow projectiles
    
    for (const auto& projectile : m_projectiles) {
        if (!projectile.active) continue;
        
        SDL_Rect projectileRect = {
            projectile.x + cameraOffsetX,
            projectile.y + cameraOffsetY,
            Projectile::SIZE,
            Projectile::SIZE
        };
        
        SDL_RenderFillRect(renderer, &projectileRect);
    }
}
