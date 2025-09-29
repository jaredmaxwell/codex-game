#include "enemy.h"
#include "player.h"
#include <cmath>
#include <algorithm>

Enemy::Enemy() 
    : m_x(0), m_y(0), m_speed(DEFAULT_SPEED), m_active(false), m_level(1), 
      m_spawnTime(0), m_knockbackX(0), m_knockbackY(0), m_knockbackEndTime(0) {
}

void Enemy::initialize(int x, int y, int level, float speed, Uint32 spawnTime) {
    m_x = x;
    m_y = y;
    m_level = level;
    m_speed = speed;
    m_spawnTime = spawnTime;
    m_active = true;
    m_knockbackX = 0;
    m_knockbackY = 0;
    m_knockbackEndTime = 0;
}

void Enemy::update(const Player& player, const std::vector<Enemy>& enemies, 
                   int worldWidth, int worldHeight, Uint32 currentTime) {
    if (!m_active) return;
    
    // Handle knockback
    if (isInKnockback(currentTime)) {
        m_x += m_knockbackX * 0.1f; // Apply knockback gradually
        m_y += m_knockbackY * 0.1f;
    } else {
        // Normal movement and collision avoidance
        moveTowardsPlayer(player);
        applyCollisionAvoidance(enemies);
    }
    
    // Check world bounds
    checkWorldBounds(worldWidth, worldHeight);
}

void Enemy::render(SDL_Renderer* renderer, SDL_Texture* texture, int cameraOffsetX, int cameraOffsetY) const {
    if (!m_active) return;
    
    SDL_Rect destRect = {m_x + cameraOffsetX, m_y + cameraOffsetY, getSize(), getSize()};
    
    if (texture) {
        SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    } else {
        // Fallback to colored rectangle
        int redIntensity = 100 + (m_level * 15);
        if (redIntensity > 255) redIntensity = 255;
        SDL_SetRenderDrawColor(renderer, redIntensity, 100, 100, 255);
        SDL_RenderFillRect(renderer, &destRect);
    }
}

void Enemy::takeDamage() {
    m_level--;
    if (m_level <= 0) {
        m_active = false;
    }
}

void Enemy::applyKnockback(float dx, float dy, float distance, Uint32 currentTime) {
    if (distance > 0) {
        dx /= distance;
        dy /= distance;
        float knockbackDist = getSize() * KNOCKBACK_DISTANCE_MULTIPLIER;
        m_knockbackX = dx * knockbackDist;
        m_knockbackY = dy * knockbackDist;
        m_knockbackEndTime = currentTime + KNOCKBACK_DURATION;
    }
}

bool Enemy::checkCollision(const SDL_Rect& otherRect) const {
    if (!m_active) return false;
    SDL_Rect myRect = getRect();
    return SDL_HasIntersection(&myRect, &otherRect);
}

bool Enemy::checkCollisionWithPlayer(const Player& player) const {
    if (!m_active) return false;
    return checkCollision(player.getRect());
}

void Enemy::moveTowardsPlayer(const Player& player) {
    float dx = player.getX() - m_x;
    float dy = player.getY() - m_y;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance > 0) {
        // Normalize direction and move towards player
        dx /= distance;
        dy /= distance;
        m_x += dx * m_speed;
        m_y += dy * m_speed;
    }
}

void Enemy::applyCollisionAvoidance(const std::vector<Enemy>& enemies) {
    float avoidX = 0, avoidY = 0;
    int centerX = getCenterX();
    int centerY = getCenterY();
    
    for (const Enemy& other : enemies) {
        if (!other.m_active || &other == this) continue;
        
        calculateAvoidanceForce(other, avoidX, avoidY);
    }
    
    // Apply avoidance forces
    m_x += avoidX;
    m_y += avoidY;
}

void Enemy::calculateAvoidanceForce(const Enemy& other, float& avoidX, float& avoidY) const {
    int otherSize = other.getSize();
    int otherCenterX = other.getCenterX();
    int otherCenterY = other.getCenterY();
    
    // Calculate distance between centers
    float dx_dist = getCenterX() - otherCenterX;
    float dy_dist = getCenterY() - otherCenterY;
    float dist = sqrt(dx_dist * dx_dist + dy_dist * dy_dist);
    
    // Calculate minimum distance (sum of radii)
    float minDistance = (getSize() + otherSize) / 2.0f + MIN_DISTANCE;
    
    if (dist < minDistance && dist > 0) {
        // Calculate avoidance force
        float avoidanceForce = SEPARATION_FORCE * (minDistance - dist) / minDistance;
        
        // Normalize direction
        dx_dist /= dist;
        dy_dist /= dist;
        
        // Add to avoidance forces
        avoidX += dx_dist * avoidanceForce;
        avoidY += dy_dist * avoidanceForce;
    }
}

void Enemy::checkWorldBounds(int worldWidth, int worldHeight) {
    int enemySize = getSize();
    if (m_x < -enemySize * 2 || m_x > worldWidth + enemySize * 2 ||
        m_y < -enemySize * 2 || m_y > worldHeight + enemySize * 2) {
        m_active = false;
    }
}

// Static helper functions
int Enemy::getEnemySize(int level) {
    return BASE_SIZE + (level - 1) * 2;
}

int Enemy::getEnemyCenterX(int enemyX, int enemySize) {
    return enemyX + enemySize / 2;
}

int Enemy::getEnemyCenterY(int enemyY, int enemySize) {
    return enemyY + enemySize / 2;
}