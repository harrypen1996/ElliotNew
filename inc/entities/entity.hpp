/*
 * CanalUx - Base Entity Class
 * Foundation for all game objects with position, velocity, and collision
 */

#pragma once

#include <tyra>
#include "core/constants.hpp"

// Forward declaration
namespace CanalUx {
    class Room;
}

namespace CanalUx {

/**
 * Base class for all entities in the game (player, mobs, projectiles, obstacles)
 * Provides common functionality for physics, collision, and rendering
 */
class Entity {
public:
    Entity();
    Entity(Tyra::Vec2 pos, Tyra::Vec2 sz);
    virtual ~Entity();

    // Core update - called every frame
    virtual void update(float deltaTime) = 0;

    // Physics
    void applyVelocity();
    void applyDrag(float dragCoefficient = Constants::DRAG_COEFFICIENT);
    void clampVelocity(float maxVel = Constants::MAX_VELOCITY);

    // Collision detection
    bool checkCollision(const Entity& other) const;
    bool checkTileCollision(const Room* room, float offsetX, float offsetY) const;
    void resolveCollisionX(const Room* room, float oldX);
    void resolveCollisionY(const Room* room, float oldY);

    // Bounding box helpers
    float getLeft() const { return position.x; }
    float getRight() const { return position.x + size.x / Constants::TILE_SIZE; }
    float getTop() const { return position.y; }
    float getBottom() const { return position.y + size.y / Constants::TILE_SIZE; }
    Tyra::Vec2 getCenter() const;

    // State
    bool isActive() const { return active; }
    void setActive(bool value) { active = value; }
    bool isSubmerged() const { return submerged; }
    void setSubmerged(bool value) { submerged = value; }

    // Position and movement
    Tyra::Vec2 position;
    Tyra::Vec2 velocity;
    Tyra::Vec2 size;

protected:
    bool active;      // If false, entity is marked for removal
    bool submerged;   // Underwater (for diving mechanic)

    // Helper to check if a specific tile position is solid
    bool isTileSolid(const Room* room, int tileX, int tileY) const;
};

}  // namespace CanalUx