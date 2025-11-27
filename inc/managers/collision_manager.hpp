/*
 * CanalUx - Collision Manager
 * Centralized collision detection for all entities and world
 */

#pragma once

#include <tyra>
#include "core/constants.hpp"
#include "managers/mob_manager.hpp"

namespace CanalUx {

class Player;
class Room;
class ProjectileManager;
class Projectile;

class CollisionManager {
public:
    CollisionManager();
    ~CollisionManager();

    // === Main update - call once per frame ===
    void checkCollisions(Player* player, 
                         MobManager* mobManager,
                         ProjectileManager* projectileManager,
                         Room* currentRoom);

    // === World collision (tiles + obstacles) ===
    
    // Check and resolve player collision with world
    void resolvePlayerWorldCollision(Player* player, Room* room);
    
    // Check and resolve mob collision with world
    void resolveMobWorldCollision(MobManager::MobData& mob, Room* room);
    
    // Check projectile collision with world (destroys projectile if hit)
    void checkProjectileWorldCollision(Projectile& projectile, Room* room, bool isPlayerProjectile);

private:
    // === Entity vs Entity collisions ===
    void checkPlayerMobCollisions(Player* player, MobManager* mobManager);
    void checkProjectileMobCollisions(ProjectileManager* projectileManager, MobManager* mobManager);
    void checkProjectilePlayerCollisions(ProjectileManager* projectileManager, Player* player);
    
    // === World collision helpers ===
    
    // Check if a position collides with solid tiles
    bool checkTileCollision(Room* room, float x, float y, bool isSubmerged = false);
    
    // Check if a position collides with dynamic obstacles
    // Returns true if blocked
    bool checkObstacleCollisionForPlayer(Room* room, float x, float y, float width, float height);
    bool checkObstacleCollisionForEnemy(Room* room, float x, float y, float width, float height);
    bool checkObstacleCollisionForProjectile(Room* room, float x, float y, bool isPlayerProjectile);
    
    // Helper: AABB collision check
    bool checkAABB(const Tyra::Vec2& pos1, const Tyra::Vec2& size1,
                   const Tyra::Vec2& pos2, const Tyra::Vec2& size2) const;
};

}  // namespace CanalUx