/*
 * CanalUx - Entity Renderer Implementation
 */

#include "rendering/entity_renderer.hpp"
#include "core/camera.hpp"
#include "entities/player.hpp"
#include "managers/projectile_manager.hpp"
#include "managers/mob_manager.hpp"

namespace CanalUx {

EntityRenderer::EntityRenderer() 
    : flashCounter(0) {
}

EntityRenderer::~EntityRenderer() {
}

void EntityRenderer::init(Tyra::TextureRepository* textureRepo) {
    // Load player texture
    auto filepath = Tyra::FileUtils::fromCwd("elliot.png");
    auto* playerTexture = textureRepo->add(filepath);
    
    playerSprite.mode = Tyra::SpriteMode::MODE_STRETCH;
    playerSprite.size = Tyra::Vec2(Constants::PLAYER_SIZE, Constants::PLAYER_SIZE);
    playerTexture->addLink(playerSprite.id);
    
    // Load projectile texture (items sheet)
    filepath = Tyra::FileUtils::fromCwd("items_sheet.png");
    auto* projectileTexture = textureRepo->add(filepath);
    
    projectileSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    projectileSprite.size = Tyra::Vec2(Constants::PROJECTILE_SIZE, Constants::PROJECTILE_SIZE);
    projectileTexture->addLink(projectileSprite.id);
    
    // Load mob sprite sheet (128x256, 64x64 tiles)
    filepath = Tyra::FileUtils::fromCwd("mobs_new.png");
    auto* mobTexture = textureRepo->add(filepath);
    
    mobSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    mobSprite.size = Tyra::Vec2(64.0f, 64.0f);
    mobTexture->addLink(mobSprite.id);
    
    // Load submerged sprite (for entities underwater)
    filepath = Tyra::FileUtils::fromCwd("submerged.png");
    auto* submergedTexture = textureRepo->add(filepath);
    
    submergedSprite.mode = Tyra::SpriteMode::MODE_STRETCH;
    submergedSprite.size = Tyra::Vec2(64.0f, 64.0f);
    submergedTexture->addLink(submergedSprite.id);
    
    TYRA_LOG("EntityRenderer: Initialized");
}

void EntityRenderer::cleanup(Tyra::TextureRepository* textureRepo) {
    textureRepo->freeBySprite(playerSprite);
    textureRepo->freeBySprite(projectileSprite);
    textureRepo->freeBySprite(mobSprite);
    textureRepo->freeBySprite(submergedSprite);
}

void EntityRenderer::render(Tyra::Renderer2D* renderer, 
                            const Camera* camera,
                            const Player* player,
                            const ProjectileManager* projectileManager,
                            const MobManager* mobManager) {
    // Render order: projectiles, mobs, then player (player on top)
    renderProjectiles(renderer, camera, projectileManager);
    renderMobs(renderer, camera, mobManager);
    renderPlayer(renderer, camera, player);
}

void EntityRenderer::renderPlayer(Tyra::Renderer2D* renderer, 
                                   const Camera* camera, 
                                   const Player* player) {
    if (!player || !camera) return;
    
    // Skip rendering every other frame when invincible (flash effect)
    if (player->isInvincible()) {
        flashCounter++;
        if (flashCounter % 8 < 4) {
            return;  // Don't render this frame
        }
    } else {
        flashCounter = 0;
    }
    
    Tyra::Vec2 screenPos = camera->worldToScreen(player->position);
    
    // Render submerged sprite if player is underwater
    if (player->isSubmerged()) {
        Tyra::Sprite sprite;
        sprite.id = submergedSprite.id;
        sprite.mode = Tyra::SpriteMode::MODE_STRETCH;
        sprite.size = Tyra::Vec2(64.0f, 64.0f);
        sprite.position = screenPos;
        sprite.scale = Constants::PLAYER_SIZE / 64.0f;
        renderer->render(sprite);
    } else {
        playerSprite.position = screenPos;
        renderer->render(playerSprite);
    }
}

void EntityRenderer::renderProjectiles(Tyra::Renderer2D* renderer, 
                                        const Camera* camera, 
                                        const ProjectileManager* projectileManager) {
    if (!projectileManager || !camera) return;
    
    for (const auto& projectile : projectileManager->getProjectiles()) {
        if (!projectile.isActive()) continue;
        
        Tyra::Vec2 screenPos = camera->worldToScreen(projectile.position);
        
        // Create sprite for this projectile
        Tyra::Sprite sprite;
        sprite.size = Tyra::Vec2(Constants::PROJECTILE_SIZE, Constants::PROJECTILE_SIZE);
        sprite.position = screenPos;
        sprite.id = projectileSprite.id;
        sprite.mode = Tyra::SpriteMode::MODE_REPEAT;
        
        // Use different tile for player vs enemy projectiles
        int tileIndex;
        if (projectile.isFromPlayer()) {
            tileIndex = 98;   // Player projectile
        } else {
            tileIndex = 99;   // Enemy projectile (feather) - adjust as needed
        }
        
        int tilesPerRow = 256 / 16;
        int column = tileIndex % tilesPerRow;
        int row = tileIndex / tilesPerRow;
        
        sprite.offset = Tyra::Vec2(
            static_cast<float>(column * 16),
            static_cast<float>(row * 16)
        );
        
        renderer->render(sprite);
    }
}

void EntityRenderer::renderMobs(Tyra::Renderer2D* renderer, 
                                 const Camera* camera, 
                                 const MobManager* mobManager) {
    if (!mobManager || !camera) return;
    
    // Sprite sheet layout (128x256, 64x64 tiles, VERTICAL):
    // Row 0 (y=0):   Duck 64x64
    // Row 1 (y=64):  Swan 64x64
    // Row 2 (y=128): Frog 64x64
    // Row 3 (y=192): Trolly/Boss 64x64
    const float tileSize = 64.0f;
    
    for (const auto& mob : mobManager->getMobs()) {
        if (!mob.active) continue;
        
        Tyra::Vec2 screenPos = camera->worldToScreen(mob.position);
        
        // Handle submerged state - render submerged sprite instead
        if (mob.submerged) {
            Tyra::Sprite sprite;
            sprite.id = submergedSprite.id;
            sprite.mode = Tyra::SpriteMode::MODE_STRETCH;
            sprite.size = Tyra::Vec2(64.0f, 64.0f);
            sprite.position = screenPos;
            sprite.scale = mob.size.x / 64.0f;
            renderer->render(sprite);
            continue;
        }
        
        // Create sprite for this mob
        Tyra::Sprite sprite;
        sprite.id = mobSprite.id;
        sprite.mode = Tyra::SpriteMode::MODE_REPEAT;
        sprite.position = screenPos;
        
        // Select Y offset based on mob type
        float offsetY = 0.0f;
        
        switch (mob.type) {
            case MobType::DUCK:
                offsetY = 0.0f;    // Row 0
                break;
            case MobType::SWAN:
                offsetY = 64.0f;   // Row 1
                break;
            case MobType::FROG:
                offsetY = 128.0f;  // Row 2
                break;
            case MobType::BOSS:
                offsetY = 192.0f;  // Row 3 (trolly/boss)
                break;
            default:
                offsetY = 0.0f;
                break;
        }
        
        sprite.size = Tyra::Vec2(tileSize, tileSize);
        sprite.offset = Tyra::Vec2(0.0f, offsetY);
        
        // Scale sprite to match mob size (mob.size is in pixels)
        sprite.scale = mob.size.x / tileSize;
        
        renderer->render(sprite);
    }
}

}  // namespace CanalUx