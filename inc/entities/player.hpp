/*
 * CanalUx - Player Class
 * The main character who fell into the canal
 */

#pragma once

#include <tyra>
#include "entities/entity.hpp"
#include "components/stats.hpp"

namespace CanalUx {

// Forward declarations
class Room;
class ProjectileManager;

class Player : public Entity {
public:
    explicit Player(Tyra::Pad* pad);
    ~Player();

    // Entity interface
    void update(float deltaTime) override;

    // Player-specific update with dependencies
    void update(Room* currentRoom, ProjectileManager* projectileManager);
    
    // Simplified update (no shooting)
    void update(Room* currentRoom);

    // Combat
    void takeDamage(int amount);
    void heal(int amount);
    bool isInvincible() const;

    // Submerge mechanic
    void trySubmerge();
    bool canSubmerge() const;
    bool isSubmerged() const { return submerged; }
    float getSubmergeTimeRemaining() const;
    float getSubmergeCooldownRemaining() const;

    // Stats access
    Stats& getStats() { return stats; }
    const Stats& getStats() const { return stats; }

    // Facing direction (for rendering and shooting)
    enum class Direction { UP, DOWN, LEFT, RIGHT };
    Direction getFacing() const { return facing; }

private:
    void handleMovementInput();
    void handleShootingInput(ProjectileManager* projectileManager);
    void handleSubmergeInput();
    void updateSubmergeState(float deltaTime);
    void updateInvincibility(float deltaTime);
    void checkRoomCollision(Room* currentRoom);

    // Input
    Tyra::Pad* pad;

    // Player stats (health, damage, speed modifiers)
    Stats stats;

    // Timers
    Tyra::Timer shootTimer;
    float submergeTimeRemaining;
    float submergeCooldownRemaining;
    float invincibilityTimeRemaining;

    // State
    Direction facing;
    bool shooting;
    bool submerged;

    // Base values (can be modified by items)
    float baseSpeed;
    float shootCooldown;
};

}  // namespace CanalUx