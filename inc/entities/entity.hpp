/*
 * CanalUx - Base Entity Class
 * Foundation for all game objects with position, velocity, and size
 */

#pragma once

#include <tyra>
#include "core/constants.hpp"

namespace CanalUx {

/**
 * Base class for all entities in the game (player, mobs, projectiles)
 * Provides common data and basic physics helpers
 * All collision is handled by CollisionManager
 */
class Entity {
public:
    Entity();
    Entity(Tyra::Vec2 pos, Tyra::Vec2 sz);
    virtual ~Entity();

    // Physics helpers
    void applyVelocity();
    void applyDrag(float dragCoefficient = Constants::DRAG_COEFFICIENT);
    void clampVelocity(float maxVel = Constants::MAX_VELOCITY);

    // Bounding box helpers
    float getLeft() const { return position.x; }
    float getRight() const { return position.x + size.x / Constants::TILE_SIZE; }
    float getTop() const { return position.y; }
    float getBottom() const { return position.y + size.y / Constants::TILE_SIZE; }
    Tyra::Vec2 getCenter() const;
    float getSizeInTilesX() const { return size.x / Constants::TILE_SIZE; }
    float getSizeInTilesY() const { return size.y / Constants::TILE_SIZE; }

    // State
    bool isActive() const { return active; }
    void setActive(bool value) { active = value; }
    bool isSubmerged() const { return submerged; }
    void setSubmerged(bool value) { submerged = value; }

    // Core data - public for easy access
    Tyra::Vec2 position;
    Tyra::Vec2 velocity;
    Tyra::Vec2 size;
    bool active;
    bool submerged;
};

}  // namespace CanalUx