/*
 * CanalUx - Collision Manager
 * Handles all collision detection between entities
 */

#pragma once

#include <tyra>
#include "core/constants.hpp"

namespace CanalUx {

class Player;
class Room;
class ProjectileManager;
class MobManager;

class CollisionManager {
public:
    CollisionManager();
    ~CollisionManager();

    // Check all collisions for the current frame
    void checkCollisions(Player* player, 
                         MobManager* mobManager,
                         ProjectileManager* projectileManager,
                         Room* currentRoom);

private:
    // Specific collision checks
    void checkPlayerMobCollisions(Player* player, MobManager* mobManager);
    void checkProjectileMobCollisions(ProjectileManager* projectileManager, MobManager* mobManager);
    void checkProjectilePlayerCollisions(ProjectileManager* projectileManager, Player* player);
    
    // Helper: AABB collision check
    bool checkAABB(const Tyra::Vec2& pos1, const Tyra::Vec2& size1,
                   const Tyra::Vec2& pos2, const Tyra::Vec2& size2) const;
};

}  // namespace CanalUx