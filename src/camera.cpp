#include "camera.h"
#include <algorithm>

Camera::Camera() 
    : m_screenWidth(0), m_screenHeight(0)
    , m_deadZoneWidth(0), m_deadZoneHeight(0)
    , m_worldX(0), m_worldY(0)
    , m_offsetX(0), m_offsetY(0)
    , m_minX(0), m_minY(0), m_maxX(0), m_maxY(0)
    , m_hasLimits(false) {
}

Camera::~Camera() {
    // No dynamic memory to clean up
}

void Camera::initialize(int screenWidth, int screenHeight, int deadZoneWidth, int deadZoneHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_deadZoneWidth = deadZoneWidth;
    m_deadZoneHeight = deadZoneHeight;
    
    // Center the camera initially
    m_worldX = 0;
    m_worldY = 0;
    m_offsetX = 0;
    m_offsetY = 0;
    
    updateDeadZoneBounds();
}

void Camera::update(int targetX, int targetY) {
    // Calculate target position in screen coordinates
    int targetScreenX = targetX - m_worldX;
    int targetScreenY = targetY - m_worldY;
    
    // Calculate deadzone bounds relative to current camera position
    int deadZoneLeft = m_worldX + (m_screenWidth - m_deadZoneWidth) / 2;
    int deadZoneRight = deadZoneLeft + m_deadZoneWidth;
    int deadZoneTop = m_worldY + (m_screenHeight - m_deadZoneHeight) / 2;
    int deadZoneBottom = deadZoneTop + m_deadZoneHeight;
    
    // Check if target is outside dead zone and adjust camera
    bool cameraMoved = false;
    
    if (targetX < deadZoneLeft) {
        // Target is to the left of dead zone - move camera to keep target at dead zone edge
        m_worldX = targetX - (m_screenWidth - m_deadZoneWidth) / 2;
        cameraMoved = true;
    } else if (targetX > deadZoneRight) {
        // Target is to the right of dead zone - move camera to keep target at dead zone edge
        m_worldX = targetX - (m_screenWidth + m_deadZoneWidth) / 2;
        cameraMoved = true;
    }
    
    if (targetY < deadZoneTop) {
        // Target is above dead zone - move camera to keep target at dead zone edge
        m_worldY = targetY - (m_screenHeight - m_deadZoneHeight) / 2;
        cameraMoved = true;
    } else if (targetY > deadZoneBottom) {
        // Target is below dead zone - move camera to keep target at dead zone edge
        m_worldY = targetY - (m_screenHeight + m_deadZoneHeight) / 2;
        cameraMoved = true;
    }
    
    // Apply world bounds if they exist
    if (m_hasLimits) {
        m_worldX = std::max(m_minX, std::min(m_worldX, m_maxX - m_screenWidth));
        m_worldY = std::max(m_minY, std::min(m_worldY, m_maxY - m_screenHeight));
    }
    
    // Update offset for rendering (reverse X for correct scrolling direction)
    m_offsetX = -m_worldX;
    m_offsetY = -m_worldY;
}

void Camera::setLimits(int minX, int minY, int maxX, int maxY) {
    m_minX = minX;
    m_minY = minY;
    m_maxX = maxX;
    m_maxY = maxY;
    m_hasLimits = true;
}

void Camera::centerOn(int targetX, int targetY) {
    m_worldX = targetX - (m_screenWidth / 2);
    m_worldY = targetY - (m_screenHeight / 2);
    
    // Apply world bounds if they exist
    if (m_hasLimits) {
        m_worldX = std::max(m_minX, std::min(m_worldX, m_maxX - m_screenWidth));
        m_worldY = std::max(m_minY, std::min(m_worldY, m_maxY - m_screenHeight));
    }
    
    m_offsetX = -m_worldX;
    m_offsetY = -m_worldY;
}

void Camera::updateDeadZoneBounds() {
    m_deadZoneLeft = (m_screenWidth - m_deadZoneWidth) / 2;
    m_deadZoneRight = m_deadZoneLeft + m_deadZoneWidth;
    m_deadZoneTop = (m_screenHeight - m_deadZoneHeight) / 2;
    m_deadZoneBottom = m_deadZoneTop + m_deadZoneHeight;
    
}
