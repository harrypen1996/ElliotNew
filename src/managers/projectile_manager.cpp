/*
 * CanalUx - Projectile Manager Implementation
 */

#include "managers/projectile_manager.hpp"
#include "world/room.hpp"

namespace CanalUx {

ProjectileManager::ProjectileManager() {
    // Reserve some space to avoid frequent reallocations
    projectiles.reserve(50);
}

ProjectileManager::~ProjectileManager() {
}

void ProjectileManager::spawnPlayerProjectile(Tyra::Vec2 position, Tyra::Vec2 velocity, float damage) {
    projectiles.emplace_back(position, velocity, damage, true);
}

void ProjectileManager::spawnEnemyProjectile(Tyra::Vec2 position, Tyra::Vec2 velocity, float damage) {
    projectiles.emplace_back(position, velocity, damage, false);
}

void ProjectileManager::spawnAcceleratingProjectile(Tyra::Vec2 position, Tyra::Vec2 velocity, float damage,
                                                     float acceleration, float maxSpeed, bool fromPlayer) {
    projectiles.emplace_back(position, velocity, damage, fromPlayer);
    projectiles.back().setAcceleration(acceleration);
    projectiles.back().setMaxSpeed(maxSpeed);
    projectiles.back().setMaxRange(25.0f);  // Longer range for accelerating projectiles
}

void ProjectileManager::spawnBarge(Tyra::Vec2 position, Tyra::Vec2 velocity, float damage) {
    projectiles.emplace_back(position, velocity, damage, false);
    projectiles.back().size = Tyra::Vec2(96.0f, 32.0f);  // 3 tiles wide, 1 tile tall
    projectiles.back().setHitsSubmerged(true);   // Barges hit submerged players!
    projectiles.back().setIgnoresWalls(true);    // Barges pass through walls!
    projectiles.back().setProjectileType(ProjectileType::BARGE);
    projectiles.back().setMaxRange(50.0f);       // Long range to cross the room
}

void ProjectileManager::addProjectile(const Projectile& projectile) {
    projectiles.push_back(projectile);
}

void ProjectileManager::update(Room* currentRoom) {
    for (auto& projectile : projectiles) {
        projectile.update(currentRoom);
    }
    removeDestroyedProjectiles();
}

void ProjectileManager::clear() {
    projectiles.clear();
}

std::vector<Projectile*> ProjectileManager::getPlayerProjectiles() {
    std::vector<Projectile*> result;
    for (auto& p : projectiles) {
        if (p.isFromPlayer() && p.isActive()) {
            result.push_back(&p);
        }
    }
    return result;
}

std::vector<Projectile*> ProjectileManager::getEnemyProjectiles() {
    std::vector<Projectile*> result;
    for (auto& p : projectiles) {
        if (!p.isFromPlayer() && p.isActive()) {
            result.push_back(&p);
        }
    }
    return result;
}

void ProjectileManager::removeDestroyedProjectiles() {
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const Projectile& p) { return p.isDestroyed(); }),
        projectiles.end()
    );
}

}  // namespace CanalUx