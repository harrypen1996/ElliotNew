/*
 * CanalUx - Collision Manager Implementation
 */

#include "managers/collision_manager.hpp"
#include "entities/player.hpp"
#include "managers/mob_manager.hpp"
#include "managers/projectile_manager.hpp"
#include "world/room.hpp"

namespace CanalUx {

CollisionManager::CollisionManager() {
}

CollisionManager::~CollisionManager() {
}

void CollisionManager::checkCollisions(Player* player, 
                                        MobManager* mobManager,
                                        ProjectileManager* projectileManager,
                                        Room* currentRoom) {
    if (!currentRoom) return;
    
    // Check player vs mobs
    if (player && mobManager) {
        checkPlayerMobCollisions(player, mobManager);
    }
    
    // Check player projectiles vs mobs
    if (projectileManager && mobManager) {
        checkProjectileMobCollisions(projectileManager, mobManager);
    }
    
    // Check enemy projectiles vs player
    if (projectileManager && player) {
        checkProjectilePlayerCollisions(projectileManager, player);
    }
}

void CollisionManager::checkPlayerMobCollisions(Player* player, MobManager* mobManager) {
    if (!player || !mobManager) return;
    
    Tyra::Vec2 playerSize(Constants::PLAYER_SIZE / Constants::TILE_SIZE,
                          Constants::PLAYER_SIZE / Constants::TILE_SIZE);
    
    for (auto& mob : mobManager->getMobs()) {
        if (!mob.active) continue;
        
        Tyra::Vec2 mobSizeInTiles(mob.size.x / Constants::TILE_SIZE,
                                   mob.size.y / Constants::TILE_SIZE);
        
        if (checkAABB(player->position, playerSize, mob.position, mobSizeInTiles)) {
            // Player hit by mob - take damage
            player->takeDamage(1);
        }
    }
}

void CollisionManager::checkProjectileMobCollisions(ProjectileManager* projectileManager, 
                                                     MobManager* mobManager) {
    if (!projectileManager || !mobManager) return;
    
    auto& projectiles = projectileManager->getProjectiles();
    auto& mobs = mobManager->getMobs();
    
    for (auto& projectile : projectiles) {
        if (!projectile.isActive() || !projectile.isFromPlayer()) continue;
        
        Tyra::Vec2 projSize(Constants::PROJECTILE_SIZE / Constants::TILE_SIZE,
                            Constants::PROJECTILE_SIZE / Constants::TILE_SIZE);
        
        for (auto& mob : mobs) {
            if (!mob.active) continue;
            
            Tyra::Vec2 mobSizeInTiles(mob.size.x / Constants::TILE_SIZE,
                                       mob.size.y / Constants::TILE_SIZE);
            
            if (checkAABB(projectile.position, projSize, mob.position, mobSizeInTiles)) {
                // Projectile hit mob
                mob.health -= projectile.getDamage();
                if (mob.health <= 0) {
                    mob.active = false;
                }
                projectile.destroy();
                break;  // Projectile can only hit one mob
            }
        }
    }
}

void CollisionManager::checkProjectilePlayerCollisions(ProjectileManager* projectileManager, 
                                                        Player* player) {
    if (!projectileManager || !player) return;
    
    Tyra::Vec2 playerSize(Constants::PLAYER_SIZE / Constants::TILE_SIZE,
                          Constants::PLAYER_SIZE / Constants::TILE_SIZE);
    
    for (auto& projectile : projectileManager->getProjectiles()) {
        // Only check enemy projectiles
        if (!projectile.isActive() || projectile.isFromPlayer()) continue;
        
        Tyra::Vec2 projSize(Constants::PROJECTILE_SIZE / Constants::TILE_SIZE,
                            Constants::PROJECTILE_SIZE / Constants::TILE_SIZE);
        
        if (checkAABB(projectile.position, projSize, player->position, playerSize)) {
            // Player hit by enemy projectile
            player->takeDamage(1);
            projectile.destroy();
        }
    }
}

bool CollisionManager::checkAABB(const Tyra::Vec2& pos1, const Tyra::Vec2& size1,
                                  const Tyra::Vec2& pos2, const Tyra::Vec2& size2) const {
    return (pos1.x < pos2.x + size2.x &&
            pos1.x + size1.x > pos2.x &&
            pos1.y < pos2.y + size2.y &&
            pos1.y + size1.y > pos2.y);
}

}  // namespace CanalUx