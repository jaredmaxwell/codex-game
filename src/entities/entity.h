#pragma once
#include <SDL.h>

class Entity {
public:
    Entity();
    virtual ~Entity();
    
    // Initialize entity with position
    virtual void initialize(int x, int y);
    
    // Update entity state (pure virtual - must be implemented by derived classes)
    virtual void update() = 0;
    
    // Render entity (pure virtual - must be implemented by derived classes)
    virtual void render(SDL_Renderer* renderer, SDL_Texture* texture, int cameraOffsetX, int cameraOffsetY) const = 0;
    
    // Getters
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getCenterX() const { return m_x + getSize() / 2; }
    int getCenterY() const { return m_y + getSize() / 2; }
    bool isActive() const { return m_active; }
    SDL_Rect getRect() const { return {m_x, m_y, getSize(), getSize()}; }
    
    // Setters
    void setPosition(int x, int y) { m_x = x; m_y = y; }
    void setActive(bool active) { m_active = active; }
    
    // Collision detection
    bool checkCollision(const SDL_Rect& otherRect) const;
    bool checkCollision(const Entity& other) const;
    
    // Distance calculation
    float distanceTo(int x, int y) const;
    float distanceTo(const Entity& other) const;
    
    // Virtual getter for size (must be implemented by derived classes)
    virtual int getSize() const = 0;
    
protected:
    // Position and state
    int m_x, m_y;
    bool m_active;
    
    // Helper method for collision detection
    bool rectsOverlap(const SDL_Rect& a, const SDL_Rect& b) const;
};
