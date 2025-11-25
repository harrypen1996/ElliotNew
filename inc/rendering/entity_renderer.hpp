/*
 * CanalUx - Entity Renderer
 * Handles rendering of all entities (player, mobs, projectiles)
 */

#pragma once

#include <tyra>
#include "core/constants.hpp"

namespace CanalUx {

class Player;
class Camera;
class ProjectileManager;
class MobManager;

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
                const MobManager* mobManager);

    // Individual render methods (can be called separately if needed)
    void renderPlayer(Tyra::Renderer2D* renderer, const Camera* camera, const Player* player);
    void renderProjectiles(Tyra::Renderer2D* renderer, const Camera* camera, const ProjectileManager* projectileManager);
    void renderMobs(Tyra::Renderer2D* renderer, const Camera* camera, const MobManager* mobManager);

private:
    Tyra::Sprite playerSprite;
    Tyra::Sprite projectileSprite;
    Tyra::Sprite mobSprite;  // Sprite sheet for all mobs
    Tyra::Sprite submergedSprite;
    
    int flashCounter;  // For invincibility flash effect
};

}  // namespace CanalUx