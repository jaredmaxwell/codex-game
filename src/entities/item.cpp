#include "item.h"
#include <cmath>
#include <algorithm>

Item::Item() 
    : m_x(0), m_y(0), m_active(false), m_type(ItemType::SHARD), 
      m_spawnTime(0), m_value(0), m_color({255, 255, 0, 255}) {
}

void Item::initialize(int x, int y, ItemType type, Uint32 spawnTime, int value, SDL_Color color) {
    m_x = x;
    m_y = y;
    m_type = type;
    m_spawnTime = spawnTime;
    m_value = value;
    m_color = color;
    m_active = true;
}

void Item::update(int playerCenterX, int playerCenterY, Uint32 currentTime, bool magnetEffectActive) {
    if (!m_active) return;
    
    // Move towards player if magnet effect is active (only for shards)
    if (m_type == ItemType::SHARD && magnetEffectActive) {
        moveTowardsPlayer(playerCenterX, playerCenterY);
    }
}

void Item::render(SDL_Renderer* renderer, int cameraOffsetX, int cameraOffsetY) const {
    if (!m_active) return;
    
    SDL_Rect destRect = {m_x + cameraOffsetX, m_y + cameraOffsetY, getSize(), getSize()};
    
    if (m_type == ItemType::SHARD) {
        // Render shard with its color
        SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
    } else {
        // Render magnet with cyan color
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    }
    
    SDL_RenderFillRect(renderer, &destRect);
}

int Item::getSize() const {
    return (m_type == ItemType::SHARD) ? SHARD_SIZE : MAGNET_SIZE;
}

bool Item::checkCollision(const SDL_Rect& otherRect) const {
    if (!m_active) return false;
    SDL_Rect myRect = getRect();
    return SDL_HasIntersection(&myRect, &otherRect);
}

bool Item::checkCollisionWithPlayer(const SDL_Rect& playerRect) const {
    if (!m_active) return false;
    return checkCollision(playerRect);
}

void Item::moveTowardsPlayer(int playerCenterX, int playerCenterY) {
    float dx = playerCenterX - getCenterX();
    float dy = playerCenterY - getCenterY();
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance > 0) {
        // Normalize direction and move towards player
        dx /= distance;
        dy /= distance;
        m_x += dx * 3.0f; // Shard speed towards player
        m_y += dy * 3.0f;
    }
}

bool Item::handleCollection(const SDL_Rect& playerRect, Uint32 currentTime, int& playerScore, Uint32& magnetEffectEndTime) {
    if (!isActive() || !checkCollisionWithPlayer(playerRect)) {
        return false;
    }
    
    if (m_type == ItemType::SHARD) {
        playerScore += m_value;
    } else if (m_type == ItemType::MAGNET) {
        magnetEffectEndTime = currentTime + 20000; // 20 seconds
    }
    
    setActive(false);
    return true;
}

