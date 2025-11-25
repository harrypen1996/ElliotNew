/*
 * CanalUx - Base Entity Class Implementation
 */

#include "entities/entity.hpp"
#include "world/room.hpp"
#include <cmath>

namespace CanalUx {

Entity::Entity()
    : position(0.0f, 0.0f),
      velocity(0.0f, 0.0f),
      size(Constants::TILE_SIZE, Constants::TILE_SIZE),
      active(true),
      submerged(false) {
}

Entity::Entity(Tyra::Vec2 pos, Tyra::Vec2 sz)
    : position(pos),
      velocity(0.0f, 0.0f),
      size(sz),
      active(true),
      submerged(false) {
}

Entity::~Entity() {
}

void Entity::applyVelocity() {
    position.x += velocity.x;
    position.y += velocity.y;
}

void Entity::applyDrag(float dragCoefficient) {
    velocity.x += -dragCoefficient * velocity.x;
    if (std::fabs(velocity.x) < Constants::VELOCITY_THRESHOLD) {
        velocity.x = 0.0f;
    }

    velocity.y += -dragCoefficient * velocity.y;
    if (std::fabs(velocity.y) < Constants::VELOCITY_THRESHOLD) {
        velocity.y = 0.0f;
    }
}

void Entity::clampVelocity(float maxVel) {
    if (velocity.x > maxVel) velocity.x = maxVel;
    if (velocity.x < -maxVel) velocity.x = -maxVel;
    if (velocity.y > maxVel) velocity.y = maxVel;
    if (velocity.y < -maxVel) velocity.y = -maxVel;
}

Tyra::Vec2 Entity::getCenter() const {
    return Tyra::Vec2(
        position.x + (size.x / Constants::TILE_SIZE) / 2.0f,
        position.y + (size.y / Constants::TILE_SIZE) / 2.0f
    );
}

bool Entity::checkCollision(const Entity& other) const {
    // AABB collision detection
    float buffer = 0.1f;
    
    float sizeInTilesX = size.x / Constants::TILE_SIZE;
    float sizeInTilesY = size.y / Constants::TILE_SIZE;
    float otherSizeInTilesX = other.size.x / Constants::TILE_SIZE;
    float otherSizeInTilesY = other.size.y / Constants::TILE_SIZE;

    return (position.x < other.position.x + otherSizeInTilesX + buffer &&
            position.x + sizeInTilesX + buffer > other.position.x &&
            position.y < other.position.y + otherSizeInTilesY + buffer &&
            position.y + sizeInTilesY + buffer > other.position.y);
}

bool Entity::isTileSolid(const Room* room, int tileX, int tileY) const {
    if (!room) return true;  // Treat null room as solid
    
    // Check land layer (walls, terrain)
    if (room->getLandTile(tileX, tileY) != 0) {
        return true;
    }
    
    // Check scenery layer (obstacles) - but only if not submerged
    // When submerged, player/entities can pass through floating obstacles
    if (!submerged && room->getSceneryTile(tileX, tileY) != 0) {
        return true;
    }
    
    return false;
}

bool Entity::checkTileCollision(const Room* room, float offsetX, float offsetY) const {
    float sizeInTilesX = size.x / Constants::TILE_SIZE;
    float sizeInTilesY = size.y / Constants::TILE_SIZE;

    // Check corners of the bounding box
    int checkX = static_cast<int>(position.x + offsetX);
    int checkY = static_cast<int>(position.y + offsetY);

    // Check all four corners
    if (isTileSolid(room, checkX, checkY)) return true;
    if (isTileSolid(room, checkX + static_cast<int>(sizeInTilesX * 0.9f), checkY)) return true;
    if (isTileSolid(room, checkX, checkY + static_cast<int>(sizeInTilesY * 0.9f))) return true;
    if (isTileSolid(room, checkX + static_cast<int>(sizeInTilesX * 0.9f), 
                    checkY + static_cast<int>(sizeInTilesY * 0.9f))) return true;

    return false;
}

void Entity::resolveCollisionX(const Room* room, float oldX) {
    float sizeInTilesX = size.x / Constants::TILE_SIZE;
    float sizeInTilesY = size.y / Constants::TILE_SIZE;

    if (velocity.x <= 0) {
        // Moving left - check left edge
        if (isTileSolid(room, static_cast<int>(position.x), static_cast<int>(oldX)) ||
            isTileSolid(room, static_cast<int>(position.x), 
                        static_cast<int>(oldX + sizeInTilesY * 0.9f))) {
            position.x = static_cast<int>(position.x) + 1;
            velocity.x = 0;
        }
    } else {
        // Moving right - check right edge
        if (isTileSolid(room, static_cast<int>(position.x + sizeInTilesX), 
                        static_cast<int>(oldX)) ||
            isTileSolid(room, static_cast<int>(position.x + sizeInTilesX), 
                        static_cast<int>(oldX + sizeInTilesY * 0.9f))) {
            position.x = static_cast<int>(position.x);
            velocity.x = 0;
        }
    }
}

void Entity::resolveCollisionY(const Room* room, float oldY) {
    float sizeInTilesX = size.x / Constants::TILE_SIZE;
    float sizeInTilesY = size.y / Constants::TILE_SIZE;

    if (velocity.y <= 0) {
        // Moving up - check top edge
        if (isTileSolid(room, static_cast<int>(position.x), static_cast<int>(position.y)) ||
            isTileSolid(room, static_cast<int>(position.x + sizeInTilesX * 0.9f), 
                        static_cast<int>(position.y))) {
            position.y = static_cast<int>(position.y) + 1;
            velocity.y = 0;
        }
    } else {
        // Moving down - check bottom edge
        if (isTileSolid(room, static_cast<int>(position.x), 
                        static_cast<int>(position.y + sizeInTilesY)) ||
            isTileSolid(room, static_cast<int>(position.x + sizeInTilesX * 0.9f), 
                        static_cast<int>(position.y + sizeInTilesY))) {
            position.y = static_cast<int>(position.y);
            velocity.y = 0;
        }
    }
}

}  // namespace CanalUx