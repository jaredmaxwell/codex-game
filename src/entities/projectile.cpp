#include "projectile.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>

// Static constants
const float PlayerProjectile::BOMB_SPEED = 3.0f;
const float PlayerProjectile::ARROW_SPEED = 8.0f;
const float PlayerProjectile::FIREBALL_SPEED = 6.0f;

PlayerProjectile::PlayerProjectile() 
    : m_type(ProjectileType::BOMB), m_dirX(0), m_dirY(0), m_speed(0), 
      m_spawnTime(0), m_explosionTime(0), m_exploded(false), m_stopped(false), m_explosionRadius(0) {
}

PlayerProjectile::~PlayerProjectile() {
    // No dynamic memory to clean up
}

void PlayerProjectile::initialize(ProjectileType type, int x, int y, float dirX, float dirY) {
    Entity::initialize(x, y);
    m_type = type;
    m_dirX = dirX;
    m_dirY = dirY;
    m_spawnTime = SDL_GetTicks();
    m_exploded = false;
    
    // Set properties based on type
    switch (type) {
        case ProjectileType::BOMB:
            m_speed = BOMB_SPEED;
            m_explosionRadius = BOMB_EXPLOSION_RADIUS;
            m_explosionTime = m_spawnTime + BOMB_TIMER_MS;
            break;
        case ProjectileType::ARROW:
            m_speed = ARROW_SPEED;
            m_explosionRadius = ARROW_EXPLOSION_RADIUS;
            m_explosionTime = 0; // Arrows don't explode on timer
            break;
        case ProjectileType::FIREBALL:
            m_speed = FIREBALL_SPEED;
            m_explosionRadius = FIREBALL_EXPLOSION_RADIUS;
            m_explosionTime = m_spawnTime + 2000; // 2 second timer
            break;
        case ProjectileType::SWORD_SLASH:
            m_speed = 0; // Sword slashes don't move
            m_explosionRadius = 0;
            m_explosionTime = m_spawnTime + 200; // Very short duration
            break;
    }
}

void PlayerProjectile::update() {
    if (!m_active || m_exploded) return;
    
    // Move projectile
    moveProjectile();
    
    // Check for explosion
    checkExplosion();
}

void PlayerProjectile::render(SDL_Renderer* renderer, SDL_Texture* texture, int cameraOffsetX, int cameraOffsetY) const {
    if (!m_active || m_exploded) return;
    
    SDL_Rect projectileRect = getRect();
    projectileRect.x += cameraOffsetX;
    projectileRect.y += cameraOffsetY;
    
    // Set color based on projectile type
    SDL_Color color;
    switch (m_type) {
        case ProjectileType::BOMB:
            // Dark red for bomb
            SDL_SetRenderDrawColor(renderer, 150, 50, 50, 255);
            break;
        case ProjectileType::ARROW:
            // Brown for arrow
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
            break;
        case ProjectileType::FIREBALL:
            // Orange for fireball
            SDL_SetRenderDrawColor(renderer, 255, 140, 0, 255);
            break;
        case ProjectileType::SWORD_SLASH:
            // Silver for sword slash
            SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
            break;
    }
    
    SDL_RenderFillRect(renderer, &projectileRect);
    
    // Draw explosion radius for bombs (as a preview)
    if (m_type == ProjectileType::BOMB && shouldExplode()) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100); // Semi-transparent red
        SDL_Rect explosionRect = {
            projectileRect.x - static_cast<int>(m_explosionRadius / 2),
            projectileRect.y - static_cast<int>(m_explosionRadius / 2),
            static_cast<int>(m_explosionRadius),
            static_cast<int>(m_explosionRadius)
        };
        SDL_RenderDrawRect(renderer, &explosionRect);
    }
}

bool PlayerProjectile::shouldExplode() const {
    if (m_exploded) return false;
    
    Uint32 currentTime = SDL_GetTicks();
    return currentTime >= m_explosionTime;
}

int PlayerProjectile::getSize() const {
    switch (m_type) {
        case ProjectileType::BOMB:
            return BOMB_SIZE;
        case ProjectileType::ARROW:
        case ProjectileType::FIREBALL:
        case ProjectileType::SWORD_SLASH:
        default:
            return PROJECTILE_SIZE;
    }
}

void PlayerProjectile::moveProjectile() {
    if (m_speed > 0 && !m_stopped) {
        // For bombs, gradually slow down to a stop
        if (m_type == ProjectileType::BOMB) {
            Uint32 currentTime = SDL_GetTicks();
            Uint32 elapsed = currentTime - m_spawnTime;
            float timeRatio = static_cast<float>(elapsed) / BOMB_TIMER_MS;
            
            // Slow down over time, reaching 0 speed at explosion time
            float currentSpeed = m_speed * (1.0f - timeRatio);
            if (currentSpeed < 0.1f) currentSpeed = 0.0f;
            
            m_x += m_dirX * currentSpeed;
            m_y += m_dirY * currentSpeed;
        } else {
            // Other projectiles move at constant speed
            m_x += m_dirX * m_speed;
            m_y += m_dirY * m_speed;
        }
    }
}

void PlayerProjectile::checkExplosion() {
    // Don't mark as exploded here - let GameManager handle explosion processing
    // This method is just for checking if explosion should happen
}

void PlayerProjectile::renderTimer(SDL_Renderer* renderer, int cameraOffsetX, int cameraOffsetY) const {
    if (!m_active || m_exploded || m_type != ProjectileType::BOMB) return;
    
    // Get timer text
    std::string timerText = getTimerText();
    if (timerText.empty()) return;
    
    // Calculate position above the projectile
    int timerX = m_x + getSize() / 2 + cameraOffsetX;
    int timerY = m_y - 20 + cameraOffsetY;
    
    // Draw a background rectangle for the timer
    SDL_Rect timerBg = {timerX - 15, timerY - 8, 30, 16};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Semi-transparent black
    SDL_RenderFillRect(renderer, &timerBg);
    
    // Draw timer border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &timerBg);
    
    // For now, we'll draw a simple visual indicator since we don't have direct font access
    // The text rendering will be handled by the GameManager
}

std::string PlayerProjectile::getTimerText() const {
    if (m_type != ProjectileType::BOMB) return "";
    
    Uint32 currentTime = SDL_GetTicks();
    Uint32 remaining = m_explosionTime - currentTime;
    
    if (remaining <= 0) return "0.0";
    
    float seconds = static_cast<float>(remaining) / 1000.0f;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << seconds;
    return oss.str();
}
