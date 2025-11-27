/*
 * CanalUx - Entity Renderer
 * Handles rendering of all entities (player, mobs, projectiles)
 */

#pragma once

#include <tyra>
#include "core/constants.hpp"
#include "managers/mob_manager.hpp"  // Need full definition for MobData

namespace CanalUx {

class Player;
class Camera;
class ProjectileManager;
class Room;

class EntityRenderer {
public:
    EntityRenderer();
    ~EntityRenderer();

    // Initialize with texture repository
    void init(Tyra::TextureRepository* textureRepo);
    
    // Clean up textures
    void cleanup(Tyra::TextureRepository* textureRepo);

    // Render all entities
    void render(Tyra::Renderer2D* renderer, 
                const Camera* camera,
                const Player* player,
                const ProjectileManager* projectileManager,
                const MobManager* mobManager,
                const Room* room = nullptr);

    // Individual render methods (can be called separately if needed)
    void renderPlayer(Tyra::Renderer2D* renderer, const Camera* camera, const Player* player);
    void renderProjectiles(Tyra::Renderer2D* renderer, const Camera* camera, const ProjectileManager* projectileManager);
    void renderMobs(Tyra::Renderer2D* renderer, const Camera* camera, const MobManager* mobManager);
    void renderPikeBoss(Tyra::Renderer2D* renderer, const MobManager::MobData& pike, const Tyra::Vec2& screenPos);
    void renderLockKeeperBoss(Tyra::Renderer2D* renderer, const MobManager::MobData& lk, const Tyra::Vec2& screenPos, const Room* room);
    void renderRoomObstacles(Tyra::Renderer2D* renderer, const Camera* camera, const Room* room);

private:
    Tyra::Sprite playerSprite;
    Tyra::Sprite projectileSprite;
    Tyra::Sprite mobSprite;  // Sprite sheet for all mobs
    Tyra::Sprite submergedSprite;  // Sprite for submerged entities
    Tyra::Sprite pikeSprite;  // Pike boss sprite (256x128)
    Tyra::Sprite shadowSprite;  // Shadow for leaping entities (128x64)
    Tyra::Sprite lockKeeperSprite;  // Lock Keeper boss sprite
    Tyra::Sprite trolleySprite;  // Trolley obstacle/projectile
    
    int flashCounter;  // For invincibility flash effect
};

}  // namespace CanalUx