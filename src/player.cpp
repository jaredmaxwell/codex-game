#include "player.h"
#include <cmath>

Player::Player() : m_dir(DOWN), m_alive(true), m_score(0) {
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
}
