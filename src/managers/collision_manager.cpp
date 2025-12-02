/*
 * CanalUx - Collision Manager Implementation
 * Centralized collision detection for all entities and world
 */

#include "managers/collision_manager.hpp"
#include "entities/player.hpp"
#include "entities/projectile.hpp"
#include "managers/projectile_manager.hpp"
#include "world/room.hpp"
#include <cmath>

namespace CanalUx {

CollisionManager::CollisionManager() {
}

CollisionManager::~CollisionManager() {
}

// =============================================================================
// Main collision check - called once per frame
// =============================================================================

void CollisionManager::checkCollisions(Player* player, 
                                        MobManager* mobManager,
                                        ProjectileManager* projectileManager,
                                        Room* currentRoom) {
    if (!currentRoom) return;
    
    // === World collisions ===
    
    // Player vs world (tiles + obstacles)
    if (player) {
        resolvePlayerWorldCollision(player, currentRoom);
    }
    
    // Mobs vs world
    if (mobManager) {
        for (auto& mob : mobManager->getMobs()) {
            if (mob.active) {
                resolveMobWorldCollision(mob, currentRoom);
            }
        }
    }
    
    // Projectiles vs world
    if (projectileManager) {
        for (auto& projectile : projectileManager->getProjectiles()) {
            if (projectile.isActive()) {
                checkProjectileWorldCollision(projectile, currentRoom, projectile.isFromPlayer());
            }
        }
    }
    
    // === Entity vs Entity collisions ===
    
    // Player vs mobs (contact damage)
    if (player && mobManager) {
        checkPlayerMobCollisions(player, mobManager);
    }
    
    // Player projectiles vs mobs
    if (projectileManager && mobManager) {
        checkProjectileMobCollisions(projectileManager, mobManager);
    }
    
    // Enemy projectiles vs player
    if (projectileManager && player) {
        checkProjectilePlayerCollisions(projectileManager, player);
    }
}

// =============================================================================
// World collision resolution
// =============================================================================

void CollisionManager::resolvePlayerWorldCollision(Player* player, Room* room) {
    if (!player || !room) return;
    
    float sizeInTilesX = player->size.x / Constants::TILE_SIZE;
    float sizeInTilesY = player->size.y / Constants::TILE_SIZE;
    bool isSubmerged = player->isSubmerged();
    Tyra::Vec2 playerSize(sizeInTilesX, sizeInTilesY);
    
    // Undo position change to test collision properly
    float newX = player->position.x;
    float newY = player->position.y;
    player->position.x -= player->velocity.x;
    player->position.y -= player->velocity.y;
    float origX = player->position.x;
    float origY = player->position.y;
    
    // --- Check X movement ---
    player->position.x = newX;
    bool collidedX = false;
    
    if (player->velocity.x < 0) {
        // Moving left - check tiles
        int checkX = static_cast<int>(player->position.x);
        if (checkTileCollision(room, checkX, origY, isSubmerged) ||
            checkTileCollision(room, checkX, origY + sizeInTilesY * 0.9f, isSubmerged)) {
            collidedX = true;
            player->position.x = checkX + 1.0f;
            player->velocity.x = 0;
        }
        
        // Check obstacles separately
        if (!collidedX) {
            Tyra::Vec2 testPos(player->position.x, origY);
            for (const auto& obs : room->getObstacles()) {
                if (obs.blocksPlayer && checkAABB(testPos, playerSize, obs.position, obs.size)) {
                    player->position.x = obs.position.x + obs.size.x;
                    player->velocity.x = 0;
                    collidedX = true;
                    break;
                }
            }
        }
    } else if (player->velocity.x > 0) {
        // Moving right - check tiles
        int checkX = static_cast<int>(player->position.x + sizeInTilesX);
        if (checkTileCollision(room, checkX, origY, isSubmerged) ||
            checkTileCollision(room, checkX, origY + sizeInTilesY * 0.9f, isSubmerged)) {
            collidedX = true;
            player->position.x = checkX - sizeInTilesX;
            player->velocity.x = 0;
        }
        
        // Check obstacles separately
        if (!collidedX) {
            Tyra::Vec2 testPos(player->position.x, origY);
            for (const auto& obs : room->getObstacles()) {
                if (obs.blocksPlayer && checkAABB(testPos, playerSize, obs.position, obs.size)) {
                    player->position.x = obs.position.x - sizeInTilesX;
                    player->velocity.x = 0;
                    collidedX = true;
                    break;
                }
            }
        }
    }
    
    // --- Check Y movement ---
    player->position.y = newY;
    bool collidedY = false;
    
    if (player->velocity.y < 0) {
        // Moving up - check tiles
        int checkY = static_cast<int>(player->position.y);
        if (checkTileCollision(room, player->position.x, checkY, isSubmerged) ||
            checkTileCollision(room, player->position.x + sizeInTilesX * 0.9f, checkY, isSubmerged)) {
            collidedY = true;
            player->position.y = checkY + 1.0f;
            player->velocity.y = 0;
        }
        
        // Check obstacles separately
        if (!collidedY) {
            for (const auto& obs : room->getObstacles()) {
                if (obs.blocksPlayer && checkAABB(player->position, playerSize, obs.position, obs.size)) {
                    player->position.y = obs.position.y + obs.size.y;
                    player->velocity.y = 0;
                    collidedY = true;
                    break;
                }
            }
        }
    } else if (player->velocity.y > 0) {
        // Moving down - check tiles
        int checkY = static_cast<int>(player->position.y + sizeInTilesY);
        if (checkTileCollision(room, player->position.x, checkY, isSubmerged) ||
            checkTileCollision(room, player->position.x + sizeInTilesX * 0.9f, checkY, isSubmerged)) {
            collidedY = true;
            player->position.y = checkY - sizeInTilesY;
            player->velocity.y = 0;
        }
        
        // Check obstacles separately
        if (!collidedY) {
            for (const auto& obs : room->getObstacles()) {
                if (obs.blocksPlayer && checkAABB(player->position, playerSize, obs.position, obs.size)) {
                    player->position.y = obs.position.y - sizeInTilesY;
                    player->velocity.y = 0;
                    collidedY = true;
                    break;
                }
            }
        }
    }
}

void CollisionManager::resolveMobWorldCollision(MobManager::MobData& mob, Room* room) {
    if (!room) return;
    
    float sizeInTilesX = mob.size.x / Constants::TILE_SIZE;
    float sizeInTilesY = mob.size.y / Constants::TILE_SIZE;
    Tyra::Vec2 mobSize(sizeInTilesX, sizeInTilesY);
    
    // Store position before velocity was applied
    float oldX = mob.position.x - mob.velocity.x;
    float oldY = mob.position.y - mob.velocity.y;
    
    // --- Check X collision ---
    bool collidedX = false;
    
    if (mob.velocity.x < 0) {
        // Moving left - check tiles
        int checkX = static_cast<int>(mob.position.x);
        if (checkTileCollision(room, checkX, oldY, mob.submerged) ||
            checkTileCollision(room, checkX, oldY + sizeInTilesY * 0.9f, mob.submerged)) {
            mob.position.x = checkX + 1.0f;
            mob.velocity.x = 0;
            collidedX = true;
        }
        
        // Check obstacles separately
        if (!collidedX) {
            Tyra::Vec2 testPos(mob.position.x, oldY);
            for (const auto& obs : room->getObstacles()) {
                if (obs.blocksEnemies && checkAABB(testPos, mobSize, obs.position, obs.size)) {
                    mob.position.x = obs.position.x + obs.size.x;
                    mob.velocity.x = 0;
                    collidedX = true;
                    break;
                }
            }
        }
    } else if (mob.velocity.x > 0) {
        // Moving right - check tiles
        int checkX = static_cast<int>(mob.position.x + sizeInTilesX);
        if (checkTileCollision(room, checkX, oldY, mob.submerged) ||
            checkTileCollision(room, checkX, oldY + sizeInTilesY * 0.9f, mob.submerged)) {
            mob.position.x = checkX - sizeInTilesX;
            mob.velocity.x = 0;
            collidedX = true;
        }
        
        // Check obstacles separately
        if (!collidedX) {
            Tyra::Vec2 testPos(mob.position.x, oldY);
            for (const auto& obs : room->getObstacles()) {
                if (obs.blocksEnemies && checkAABB(testPos, mobSize, obs.position, obs.size)) {
                    mob.position.x = obs.position.x - sizeInTilesX;
                    mob.velocity.x = 0;
                    collidedX = true;
                    break;
                }
            }
        }
    }
    
    // --- Check Y collision ---
    bool collidedY = false;
    
    if (mob.velocity.y < 0) {
        // Moving up - check tiles
        int checkY = static_cast<int>(mob.position.y);
        if (checkTileCollision(room, mob.position.x, checkY, mob.submerged) ||
            checkTileCollision(room, mob.position.x + sizeInTilesX * 0.9f, checkY, mob.submerged)) {
            mob.position.y = checkY + 1.0f;
            mob.velocity.y = 0;
            collidedY = true;
        }
        
        // Check obstacles separately
        if (!collidedY) {
            for (const auto& obs : room->getObstacles()) {
                if (obs.blocksEnemies && checkAABB(mob.position, mobSize, obs.position, obs.size)) {
                    mob.position.y = obs.position.y + obs.size.y;
                    mob.velocity.y = 0;
                    collidedY = true;
                    break;
                }
            }
        }
    } else if (mob.velocity.y > 0) {
        // Moving down - check tiles
        int checkY = static_cast<int>(mob.position.y + sizeInTilesY);
        if (checkTileCollision(room, mob.position.x, checkY, mob.submerged) ||
            checkTileCollision(room, mob.position.x + sizeInTilesX * 0.9f, checkY, mob.submerged)) {
            mob.position.y = checkY - sizeInTilesY;
            mob.velocity.y = 0;
            collidedY = true;
        }
        
        // Check obstacles separately
        if (!collidedY) {
            for (const auto& obs : room->getObstacles()) {
                if (obs.blocksEnemies && checkAABB(mob.position, mobSize, obs.position, obs.size)) {
                    mob.position.y = obs.position.y - sizeInTilesY;
                    mob.velocity.y = 0;
                    collidedY = true;
                    break;
                }
            }
        }
    }
}

void CollisionManager::checkProjectileWorldCollision(Projectile& projectile, Room* room, bool isPlayerProjectile) {
    if (!room || !projectile.isActive()) return;
    
    float sizeInTiles = projectile.size.x / Constants::TILE_SIZE;
    int tileX = static_cast<int>(projectile.position.x);
    int tileY = static_cast<int>(projectile.position.y);
    
    // Check tile collision (walls always block)
    if (room->getLandTile(tileX, tileY) != 0 ||
        room->getLandTile(static_cast<int>(projectile.position.x + sizeInTiles * 0.9f), tileY) != 0 ||
        room->getLandTile(tileX, static_cast<int>(projectile.position.y + sizeInTiles * 0.9f)) != 0) {
        projectile.destroy();
        return;
    }
    
    // Check scenery (doors etc) - always blocks projectiles
    if (room->getSceneryTile(tileX, tileY) != 0 ||
        room->getSceneryTile(static_cast<int>(projectile.position.x + sizeInTiles * 0.9f), tileY) != 0 ||
        room->getSceneryTile(tileX, static_cast<int>(projectile.position.y + sizeInTiles * 0.9f)) != 0) {
        projectile.destroy();
        return;
    }
    
    // Check dynamic obstacles
    if (checkObstacleCollisionForProjectile(room, projectile.position.x, projectile.position.y, isPlayerProjectile)) {
        projectile.destroy();
        return;
    }
}

// =============================================================================
// Entity vs Entity collisions
// =============================================================================

void CollisionManager::checkPlayerMobCollisions(Player* player, MobManager* mobManager) {
    if (!player || !mobManager) return;
    
    // Player is immune while submerged or invincible
    if (player->isSubmerged() || player->isInvincible()) return;
    
    Tyra::Vec2 playerSize(Constants::PLAYER_SIZE / Constants::TILE_SIZE,
                          Constants::PLAYER_SIZE / Constants::TILE_SIZE);
    
    for (auto& mob : mobManager->getMobs()) {
        if (!mob.active) continue;
        
        // Submerged mobs can't hurt player
        if (mob.submerged) continue;
        
        Tyra::Vec2 mobSizeInTiles(mob.size.x / Constants::TILE_SIZE,
                                   mob.size.y / Constants::TILE_SIZE);
        
        if (checkAABB(player->position, playerSize, mob.position, mobSizeInTiles)) {
            player->takeDamage(1);
            return;  // Only take damage once per frame
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
            
            // Submerged mobs can't be hit
            if (mob.submerged) continue;
            
            Tyra::Vec2 mobSizeInTiles(mob.size.x / Constants::TILE_SIZE,
                                       mob.size.y / Constants::TILE_SIZE);
            
            if (checkAABB(projectile.position, projSize, mob.position, mobSizeInTiles)) {
                float damage = projectile.getDamage();
                
                // Cheat: One-hit kills
                if (Constants::Cheats::ONE_HIT_KILLS) {
                    damage = 9999.0f;
                }
                
                mob.health -= damage;
                if (mob.health <= 0) {
                    mob.active = false;
                }
                projectile.destroy();
                break;
            }
        }
    }
}

void CollisionManager::checkProjectilePlayerCollisions(ProjectileManager* projectileManager, 
                                                        Player* player) {
    if (!projectileManager || !player) return;
    
    // Player is immune while invincible
    if (player->isInvincible()) return;
    
    Tyra::Vec2 playerSize(Constants::PLAYER_SIZE / Constants::TILE_SIZE,
                          Constants::PLAYER_SIZE / Constants::TILE_SIZE);
    
    for (auto& projectile : projectileManager->getProjectiles()) {
        if (!projectile.isActive() || projectile.isFromPlayer()) continue;
        
        // Skip if player is submerged and this projectile doesn't hit submerged
        if (player->isSubmerged() && !projectile.getHitsSubmerged()) continue;
        
        // Use projectile's actual size (important for barges which are larger)
        Tyra::Vec2 projSize(projectile.size.x / Constants::TILE_SIZE,
                            projectile.size.y / Constants::TILE_SIZE);
        
        if (checkAABB(projectile.position, projSize, player->position, playerSize)) {
            int damage = static_cast<int>(projectile.getDamage());
            if (damage < 1) damage = 1;
            player->takeDamage(damage);
            projectile.destroy();
            return;  // Only one hit per frame
        }
    }
}

// =============================================================================
// Collision helpers
// =============================================================================

bool CollisionManager::checkTileCollision(Room* room, float x, float y, bool isSubmerged) {
    int tileX = static_cast<int>(x);
    int tileY = static_cast<int>(y);
    
    // Land tiles (walls) always block
    if (room->getLandTile(tileX, tileY) != 0) {
        return true;
    }
    
    // Scenery tiles (doors) block unless submerged
    //if (!isSubmerged && room->getSceneryTile(tileX, tileY) != 0) {
    //    return true;
    //}

    if (room->getSceneryTile(tileX, tileY) != 0) {
        return true;
    }
    
    return false;
}

bool CollisionManager::checkObstacleCollisionForPlayer(Room* room, float x, float y, float width, float height) {
    Tyra::Vec2 pos(x, y);
    Tyra::Vec2 size(width, height);
    for (const auto& obs : room->getObstacles()) {
        if (obs.blocksPlayer && checkAABB(pos, size, obs.position, obs.size)) {
            return true;
        }
    }
    return false;
}

bool CollisionManager::checkObstacleCollisionForEnemy(Room* room, float x, float y, float width, float height) {
    Tyra::Vec2 pos(x, y);
    Tyra::Vec2 size(width, height);
    for (const auto& obs : room->getObstacles()) {
        if (obs.blocksEnemies && checkAABB(pos, size, obs.position, obs.size)) {
            return true;
        }
    }
    return false;
}

bool CollisionManager::checkObstacleCollisionForProjectile(Room* room, float x, float y, bool isPlayerProjectile) {
    // Point check - use tiny size for projectile center
    Tyra::Vec2 pos(x, y);
    Tyra::Vec2 size(0.1f, 0.1f);
    for (const auto& obs : room->getObstacles()) {
        bool blocks = isPlayerProjectile ? obs.blocksPlayerShots : obs.blocksEnemyShots;
        if (blocks && checkAABB(pos, size, obs.position, obs.size)) {
            return true;
        }
    }
    return false;
}

bool CollisionManager::checkAABB(const Tyra::Vec2& pos1, const Tyra::Vec2& size1,
                                  const Tyra::Vec2& pos2, const Tyra::Vec2& size2) const {
    return (pos1.x < pos2.x + size2.x &&
            pos1.x + size1.x > pos2.x &&
            pos1.y < pos2.y + size2.y &&
            pos1.y + size1.y > pos2.y);
}

}  // namespace CanalUx