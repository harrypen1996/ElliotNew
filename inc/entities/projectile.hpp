/*
 * CanalUx - Projectile Class
 * Represents a single projectile (player tears or enemy shots)
 */

#pragma once

#include "entities/entity.hpp"

namespace CanalUx {

class Room;  // Forward declaration

// Projectile types for rendering different sprites
enum class ProjectileType {
    DEFAULT = 0,
    BARGE = 1,      // Nanny boss barges - wide, hits submerged
    FEATHER = 2,    // Swan feathers
    RING = 3        // Lock Keeper ring shockwave pieces
};

class Projectile : public Entity {
public:
    Projectile();
    Projectile(Tyra::Vec2 pos, Tyra::Vec2 vel, float dmg, bool fromPlayer);
    ~Projectile();

    // Update methods
    void update(float deltaTime);

    // Update with room for collision
    void update(Room* currentRoom);

    // Properties
    bool isFromPlayer() const { return fromPlayer; }
    float getDamage() const { return damage; }
    float getDistanceTraveled() const { return distanceTraveled; }
    float getMaxRange() const { return maxRange; }
    void setMaxRange(float range) { maxRange = range; }
    
    // Acceleration - projectile speeds up over time
    void setAcceleration(float accel) { acceleration = accel; }
    float getAcceleration() const { return acceleration; }
    void setMaxSpeed(float speed) { maxSpeed = speed; }
    
    // Submerged hitting - barges hit even when player is submerged
    void setHitsSubmerged(bool hits) { hitsSubmerged = hits; }
    bool getHitsSubmerged() const { return hitsSubmerged; }
    
    // Projectile type for rendering
    void setProjectileType(ProjectileType t) { projectileType = t; }
    ProjectileType getProjectileType() const { return projectileType; }

    // Destruction
    void destroy() { active = false; }
    bool isDestroyed() const { return !active; }

private:
    // Note: World collision now handled by CollisionManager

    bool fromPlayer;  // true = player shot, false = enemy shot
    float damage;
    float distanceTraveled;
    float maxRange;
    float acceleration;  // Speed increase per frame (0 = no acceleration)
    float maxSpeed;      // Maximum speed cap
    bool hitsSubmerged;  // If true, hits player even when submerged
    ProjectileType projectileType;
};

}  // namespace CanalUx