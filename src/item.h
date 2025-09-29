#pragma once
#include <SDL.h>

enum class ItemType {
    SHARD,
    MAGNET
};

class Item {
public:
    // Constructor
    Item();
    
    // Initialize item with position and properties
    void initialize(int x, int y, ItemType type, Uint32 spawnTime, int value = 0, SDL_Color color = {255, 255, 0, 255});
    
    // Update item state (movement, lifetime, etc.)
    void update(int playerCenterX, int playerCenterY, Uint32 currentTime, bool magnetEffectActive);
    
    // Render the item
    void render(SDL_Renderer* renderer, int cameraOffsetX, int cameraOffsetY) const;
    
    // Getters
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getCenterX() const { return m_x + getSize() / 2; }
    int getCenterY() const { return m_y + getSize() / 2; }
    int getSize() const;
    ItemType getType() const { return m_type; }
    bool isActive() const { return m_active; }
    int getValue() const { return m_value; }
    SDL_Color getColor() const { return m_color; }
    SDL_Rect getRect() const { return {m_x, m_y, getSize(), getSize()}; }
    
    // Setters
    void setPosition(int x, int y) { m_x = x; m_y = y; }
    void setActive(bool active) { m_active = active; }
    
    // Collision detection
    bool checkCollision(const SDL_Rect& otherRect) const;
    bool checkCollisionWithPlayer(const SDL_Rect& playerRect) const;
    
    // Static constants
    static constexpr int SHARD_SIZE = 8;
    static constexpr int MAGNET_SIZE = 12;
    static constexpr int MAX_SHARDS = 50;
    static constexpr int MAX_MAGNETS = 5;
    static constexpr int MAGNET_DROP_CHANCE = 1; // 1 in 100 chance
    
private:
    // Position
    int m_x, m_y;
    
    // State
    bool m_active;
    ItemType m_type;
    Uint32 m_spawnTime;
    
    // Item-specific properties
    int m_value;
    SDL_Color m_color;
    
    // Helper methods
    void moveTowardsPlayer(int playerCenterX, int playerCenterY);
    bool isExpired(Uint32 currentTime) const;
};
