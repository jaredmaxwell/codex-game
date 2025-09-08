#include "player.h"
#include <cmath>

Player::Player() : m_x(0), m_y(0), m_dir(DOWN) {
    m_attack = {false, {0, 0, PLAYER_SIZE, PLAYER_SIZE}, 0};
}

Player::~Player() {
    // No dynamic memory to clean up
}

void Player::initialize(int startX, int startY) {
    m_x = startX;
    m_y = startY;
    m_dir = DOWN;
    m_attack = {false, {0, 0, PLAYER_SIZE, PLAYER_SIZE}, 0};
}

void Player::update() {
    // Update attack state
    if (m_attack.active && SDL_GetTicks() - m_attack.startTime > ATTACK_DURATION) {
        m_attack.active = false;
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
