/*
 * CanalUx - Projectile Manager
 * Handles all projectiles in the game
 */

#pragma once

#include <vector>
#include <tyra>
#include "entities/projectile.hpp"

namespace CanalUx {

class Room;

class ProjectileManager {
public:
    ProjectileManager();
    ~ProjectileManager();

    // Spawning
    void spawnPlayerProjectile(Tyra::Vec2 position, Tyra::Vec2 velocity, float damage);
    void spawnEnemyProjectile(Tyra::Vec2 position, Tyra::Vec2 velocity, float damage);
    
    // Spawn accelerating projectile (starts slow, speeds up)
    void spawnAcceleratingProjectile(Tyra::Vec2 position, Tyra::Vec2 velocity, float damage, 
                                      float acceleration, float maxSpeed, bool fromPlayer = false);

    // Update all projectiles
    void update(Room* currentRoom);

    // Clear all projectiles (e.g., on room change)
    void clear();

    // Access for collision checking and rendering
    std::vector<Projectile>& getProjectiles() { return projectiles; }
    const std::vector<Projectile>& getProjectiles() const { return projectiles; }

    // Get only player or enemy projectiles
    std::vector<Projectile*> getPlayerProjectiles();
    std::vector<Projectile*> getEnemyProjectiles();

private:
    void removeDestroyedProjectiles();

    std::vector<Projectile> projectiles;
};

}  // namespace CanalUx