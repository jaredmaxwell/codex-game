#include "entity.h"
#include <cmath>

Entity::Entity() : m_x(0), m_y(0), m_active(false) {
}

Entity::~Entity() {
}

void Entity::initialize(int x, int y) {
    m_x = x;
    m_y = y;
    m_active = true;
}

bool Entity::checkCollision(const SDL_Rect& otherRect) const {
    SDL_Rect thisRect = getRect();
    return rectsOverlap(thisRect, otherRect);
}

bool Entity::checkCollision(const Entity& other) const {
    return checkCollision(other.getRect());
}

float Entity::distanceTo(int x, int y) const {
    float dx = x - getCenterX();
    float dy = y - getCenterY();
    return sqrt(dx * dx + dy * dy);
}

float Entity::distanceTo(const Entity& other) const {
    return distanceTo(other.getCenterX(), other.getCenterY());
}

bool Entity::rectsOverlap(const SDL_Rect& a, const SDL_Rect& b) const {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}
