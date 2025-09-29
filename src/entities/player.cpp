#include "player.h"
#include <cmath>
#include <algorithm>
#include <iostream>

Player::Player() : m_dir(DOWN), m_alive(true), m_score(0), m_characterClass(CharacterClass::SWORDSMAN) {
    m_attack = {false, {0, 0, PLAYER_SIZE, PLAYER_SIZE}, 0};
}

Player::~Player() {
    // No dynamic memory to clean up
}

void Player::initialize(int startX, int startY) {
    Entity::initialize(startX, startY);
    m_dir = DOWN;
    m_attack = {false, {0, 0, PLAYER_SIZE, PLAYER_SIZE}, 0};
    m_alive = true;
    m_score = 0;
}

void Player::update() {
    // Update attack state
    if (m_attack.active && SDL_GetTicks() - m_attack.startTime > ATTACK_DURATION) {
        m_attack.active = false;
    }
    
    // Update projectiles
    updateProjectiles();
}

void Player::render(SDL_Renderer* renderer, SDL_Texture* texture, int cameraOffsetX, int cameraOffsetY) const {
    SDL_Rect playerRect = getRect();
    playerRect.x += cameraOffsetX;
    playerRect.y += cameraOffsetY;
    
    if (texture) {
        SDL_RenderCopy(renderer, texture, NULL, &playerRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &playerRect);
    }
}

void Player::handleInput(const Uint8* keystate) {
    float moveX = 0, moveY = 0;
    
    if (keystate[SDL_SCANCODE_W]) {
        moveY -= 1;
        m_dir = UP;
    }
    if (keystate[SDL_SCANCODE_S]) {
        moveY += 1;
        m_dir = DOWN;
    }
    if (keystate[SDL_SCANCODE_A]) {
        moveX -= 1;
        m_dir = LEFT;
    }
    if (keystate[SDL_SCANCODE_D]) {
        moveX += 1;
        m_dir = RIGHT;
    }
    
    // Normalize diagonal movement
    if (moveX != 0 && moveY != 0) {
        float length = sqrt(moveX * moveX + moveY * moveY);
        moveX /= length;
        moveY /= length;
    }
    
    // Apply movement
    m_x += moveX * PLAYER_SPEED;
    m_y += moveY * PLAYER_SPEED;
}

void Player::handleAttack() {
    // Handle different attack types based on character class
    switch (m_characterClass) {
        case CharacterClass::BOMBER:
            handleBomberAttack();
            break;
        case CharacterClass::ARCHER:
            handleArcherAttack();
            break;
        case CharacterClass::MAGE:
            handleMageAttack();
            break;
        case CharacterClass::SWORDSMAN:
            handleSwordsmanAttack();
            break;
    }
}

void Player::handleDeath() {
    m_alive = false;
    m_score = 0; // Lose all score on death
}

void Player::respawn(int worldWidth, int worldHeight) {
    // Reset position to world center
    setPosition(worldWidth / 2 - PLAYER_SIZE / 2, worldHeight / 2 - PLAYER_SIZE / 2);
    m_alive = true;
    m_score = 0;
    clearProjectiles();
}

void Player::setCharacterClass(CharacterClass characterClass) {
    m_characterClass = characterClass;
    clearProjectiles(); // Clear any existing projectiles when changing class
}

void Player::updateProjectiles() {
    for (auto& projectile : m_projectiles) {
        projectile.update();
    }
    
    // Remove inactive projectiles
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const PlayerProjectile& p) { return !p.isActive(); }),
        m_projectiles.end()
    );
}

void Player::renderProjectiles(SDL_Renderer* renderer, int cameraOffsetX, int cameraOffsetY) const {
    for (const auto& projectile : m_projectiles) {
        projectile.render(renderer, nullptr, cameraOffsetX, cameraOffsetY);
        projectile.renderTimer(renderer, cameraOffsetX, cameraOffsetY);
    }
}

void Player::clearProjectiles() {
    m_projectiles.clear();
}

void Player::removeExplodedProjectiles() {
    // Remove projectiles that have exploded and been processed
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const PlayerProjectile& p) { return p.isExploded(); }),
        m_projectiles.end()
    );
}

void Player::handleBomberAttack() {
    // Create a bomb projectile
    PlayerProjectile bomb;
    float dirX = 0, dirY = 0;
    
    // Set direction based on player facing direction
    switch (m_dir) {
        case UP: dirY = -1; break;
        case DOWN: dirY = 1; break;
        case LEFT: dirX = -1; break;
        case RIGHT: dirX = 1; break;
    }
    
    // Normalize direction
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0) {
        dirX /= length;
        dirY /= length;
    }
    
    bomb.initialize(ProjectileType::BOMB, m_x + PLAYER_SIZE/2, m_y + PLAYER_SIZE/2, dirX, dirY);
    m_projectiles.push_back(bomb);
    
    std::cout << "Bomber threw a bomb!" << std::endl;
}

void Player::handleArcherAttack() {
    // Create an arrow projectile
    PlayerProjectile arrow;
    float dirX = 0, dirY = 0;
    
    // Set direction based on player facing direction
    switch (m_dir) {
        case UP: dirY = -1; break;
        case DOWN: dirY = 1; break;
        case LEFT: dirX = -1; break;
        case RIGHT: dirX = 1; break;
    }
    
    // Normalize direction
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0) {
        dirX /= length;
        dirY /= length;
    }
    
    arrow.initialize(ProjectileType::ARROW, m_x + PLAYER_SIZE/2, m_y + PLAYER_SIZE/2, dirX, dirY);
    m_projectiles.push_back(arrow);
    
    std::cout << "Archer fired an arrow!" << std::endl;
}

void Player::handleMageAttack() {
    // Create a fireball projectile
    PlayerProjectile fireball;
    float dirX = 0, dirY = 0;
    
    // Set direction based on player facing direction
    switch (m_dir) {
        case UP: dirY = -1; break;
        case DOWN: dirY = 1; break;
        case LEFT: dirX = -1; break;
        case RIGHT: dirX = 1; break;
    }
    
    // Normalize direction
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0) {
        dirX /= length;
        dirY /= length;
    }
    
    fireball.initialize(ProjectileType::FIREBALL, m_x + PLAYER_SIZE/2, m_y + PLAYER_SIZE/2, dirX, dirY);
    m_projectiles.push_back(fireball);
    
    std::cout << "Mage cast a fireball!" << std::endl;
}

void Player::handleSwordsmanAttack() {
    // Traditional melee attack
    m_attack.active = true;
    m_attack.startTime = SDL_GetTicks();
    
    // Set attack position based on player direction
    switch (m_dir) {
        case UP:
            m_attack.rect = {m_x, m_y - PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE};
            break;
        case DOWN:
            m_attack.rect = {m_x, m_y + PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE};
            break;
        case LEFT:
            m_attack.rect = {m_x - PLAYER_SIZE, m_y, PLAYER_SIZE, PLAYER_SIZE};
            break;
        case RIGHT:
            m_attack.rect = {m_x + PLAYER_SIZE, m_y, PLAYER_SIZE, PLAYER_SIZE};
            break;
    }
    
    std::cout << "Swordsman slashed!" << std::endl;
}
