/*
 * CanalUx - Entity Renderer Header
 */

#pragma once

#include <tyra>
#include "core/constants.hpp"
#include "managers/mob_manager.hpp"

namespace CanalUx {

// Forward declarations
class Camera;
class Player;
class ProjectileManager;
class Room;

class EntityRenderer {
public:
    EntityRenderer();
    ~EntityRenderer();
    
    void init(Tyra::TextureRepository* textureRepo);
    void cleanup(Tyra::TextureRepository* textureRepo);
    
    void render(Tyra::Renderer2D* renderer, 
                const Camera* camera,
                const Player* player,
                const ProjectileManager* projectileManager,
                const MobManager* mobManager,
                const Room* room);

private:
    void renderPlayer(Tyra::Renderer2D* renderer, 
                      const Camera* camera, 
                      const Player* player);
    
    void renderProjectiles(Tyra::Renderer2D* renderer, 
                           const Camera* camera, 
                           const ProjectileManager* projectileManager);
    
    void renderMobs(Tyra::Renderer2D* renderer, 
                    const Camera* camera, 
                    const MobManager* mobManager);
    
    void renderPikeBoss(Tyra::Renderer2D* renderer,
                        const MobManager::MobData& pike,
                        const Tyra::Vec2& screenPos);
    
    void renderLockKeeperBoss(Tyra::Renderer2D* renderer, 
                               const MobManager::MobData& lk, 
                               const Tyra::Vec2& screenPos,
                               const Room* room);
    
    void renderNannyBoss(Tyra::Renderer2D* renderer, 
                         const MobManager::MobData& nanny, 
                         const Tyra::Vec2& screenPos);
    
    void renderRoomObstacles(Tyra::Renderer2D* renderer, 
                              const Camera* camera, 
                              const Room* room);
    
    // Sprites
    Tyra::Sprite playerSprite;
    Tyra::Sprite projectileSprite;
    Tyra::Sprite mobSprite;
    Tyra::Sprite submergedSprite;
    Tyra::Sprite pikeSprite;
    Tyra::Sprite shadowSprite;
    Tyra::Sprite lockKeeperSprite;
    Tyra::Sprite trolleySprite;
    Tyra::Sprite nannySprite;
    Tyra::Sprite bargeSprite;
    Tyra::Sprite pixelSprite;  // For solid colored rectangles (health bars, etc.)
    
    // Flash effect counter for invincibility
    int flashCounter;
};

}  // namespace CanalUx