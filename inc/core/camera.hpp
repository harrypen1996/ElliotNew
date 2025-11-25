/*
 * CanalUx - Camera Class
 * Handles view position and screen coordinate calculations
 */

#pragma once

#include <tyra>
#include "constants.hpp"

namespace CanalUx {

class Room;

class Camera {
public:
    Camera();
    ~Camera();

    // Update camera to follow a target position
    void follow(const Tyra::Vec2& targetPosition);
    
    // Clamp camera to room bounds
    void clampToRoom(const Room* room);
    
    // Set screen dimensions (call once on init)
    void setScreenSize(float width, float height);
    
    // Convert world position to screen position
    Tyra::Vec2 worldToScreen(const Tyra::Vec2& worldPos) const;
    float worldToScreenX(float worldX) const;
    float worldToScreenY(float worldY) const;
    
    // Get the world-space offset (top-left corner of view)
    float getOffsetX() const { return offsetX; }
    float getOffsetY() const { return offsetY; }
    
    // Get camera center position (in tiles)
    float getX() const { return position.x; }
    float getY() const { return position.y; }
    
    // Direct position control
    void setPosition(float x, float y);
    void setPosition(const Tyra::Vec2& pos);

private:
    void recalculateOffset();

    Tyra::Vec2 position;  // Camera center in tile coordinates
    
    float offsetX;        // World offset for rendering (top-left)
    float offsetY;
    
    float screenWidth;
    float screenHeight;
    float halfScreenTilesX;
    float halfScreenTilesY;
};

}  // namespace CanalUx