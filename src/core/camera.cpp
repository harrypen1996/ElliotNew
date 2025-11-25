/*
 * CanalUx - Camera Class Implementation
 */

#include "core/camera.hpp"
#include "world/room.hpp"

namespace CanalUx {

Camera::Camera()
    : position(0.0f, 0.0f),
      offsetX(0.0f),
      offsetY(0.0f),
      screenWidth(Constants::SCREEN_WIDTH),
      screenHeight(Constants::SCREEN_HEIGHT),
      halfScreenTilesX(0.0f),
      halfScreenTilesY(0.0f) {
    setScreenSize(screenWidth, screenHeight);
}

Camera::~Camera() {
}

void Camera::setScreenSize(float width, float height) {
    screenWidth = width;
    screenHeight = height;
    halfScreenTilesX = (screenWidth / Constants::TILE_SIZE) / 2.0f;
    halfScreenTilesY = (screenHeight / Constants::TILE_SIZE) / 2.0f;
    recalculateOffset();
}

void Camera::follow(const Tyra::Vec2& targetPosition) {
    // Center camera on target (offset by 0.5 to center on tile)
    position.x = targetPosition.x + 0.5f;
    position.y = targetPosition.y + 0.5f;
    recalculateOffset();
}

void Camera::clampToRoom(const Room* room) {
    if (!room) return;
    
    float roomWidth = static_cast<float>(room->getWidth());
    float roomHeight = static_cast<float>(room->getHeight());
    
    // Clamp camera center so view doesn't go outside room
    if (position.x < halfScreenTilesX) 
        position.x = halfScreenTilesX;
    if (position.y < halfScreenTilesY) 
        position.y = halfScreenTilesY;
    if (position.x > roomWidth - halfScreenTilesX) 
        position.x = roomWidth - halfScreenTilesX;
    if (position.y > roomHeight - halfScreenTilesY) 
        position.y = roomHeight - halfScreenTilesY;
    
    recalculateOffset();
}

void Camera::recalculateOffset() {
    offsetX = position.x - halfScreenTilesX;
    offsetY = position.y - halfScreenTilesY;
    
    // Clamp offsets to non-negative
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;
}

Tyra::Vec2 Camera::worldToScreen(const Tyra::Vec2& worldPos) const {
    return Tyra::Vec2(
        worldToScreenX(worldPos.x),
        worldToScreenY(worldPos.y)
    );
}

float Camera::worldToScreenX(float worldX) const {
    return (worldX - offsetX) * Constants::TILE_SIZE;
}

float Camera::worldToScreenY(float worldY) const {
    return (worldY - offsetY) * Constants::TILE_SIZE;
}

void Camera::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
    recalculateOffset();
}

void Camera::setPosition(const Tyra::Vec2& pos) {
    position = pos;
    recalculateOffset();
}

}  // namespace CanalUx