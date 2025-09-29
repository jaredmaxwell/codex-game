#pragma once
#include <SDL.h>

class Camera {
public:
    Camera();
    ~Camera();
    
    // Initialize camera with screen dimensions and dead zone
    void initialize(int screenWidth, int screenHeight, int deadZoneWidth, int deadZoneHeight);
    
    // Update camera position based on target position
    void update(int targetX, int targetY);
    
    // Get camera offset for rendering
    int getOffsetX() const { return m_offsetX; }
    int getOffsetY() const { return m_offsetY; }
    
    // Get camera position in world coordinates
    int getWorldX() const { return m_worldX; }
    int getWorldY() const { return m_worldY; }
    
    // Convert world coordinates to screen coordinates
    int worldToScreenX(int worldX) const { return worldX - m_offsetX; }
    int worldToScreenY(int worldY) const { return worldY - m_offsetY; }
    
    // Convert screen coordinates to world coordinates
    int screenToWorldX(int screenX) const { return screenX + m_offsetX; }
    int screenToWorldY(int screenY) const { return screenY + m_offsetY; }
    
    // Get camera bounds for culling
    SDL_Rect getViewport() const { return {m_worldX, m_worldY, m_screenWidth, m_screenHeight}; }
    
    // Set camera limits (optional - for bounded worlds)
    void setLimits(int minX, int minY, int maxX, int maxY);
    
    // Reset camera to center on target
    void centerOn(int targetX, int targetY);

private:
    int m_screenWidth, m_screenHeight;
    int m_deadZoneWidth, m_deadZoneHeight;
    int m_deadZoneLeft, m_deadZoneRight, m_deadZoneTop, m_deadZoneBottom;
    
    int m_worldX, m_worldY;  // Camera position in world coordinates
    int m_offsetX, m_offsetY;  // Offset for rendering (negative of world position)
    
    // Optional world bounds
    int m_minX, m_minY, m_maxX, m_maxY;
    bool m_hasLimits;
    
    void updateDeadZoneBounds();
};
