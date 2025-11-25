/*
 * CanalUx - Entity Renderer Implementation
 */

#include "rendering/entity_renderer.hpp"
#include "core/camera.hpp"
#include "entities/player.hpp"
#include "managers/projectile_manager.hpp"
#include "managers/mob_manager.hpp"

namespace CanalUx {

EntityRenderer::EntityRenderer() {
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
    
    // Load mob texture
    filepath = Tyra::FileUtils::fromCwd("mobs.png");
    auto* mobTexture = textureRepo->add(filepath);
    
    mobSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    mobSprite.size = Tyra::Vec2(Constants::TILE_SIZE, Constants::TILE_SIZE);
    mobTexture->addLink(mobSprite.id);
    
    TYRA_LOG("EntityRenderer: Initialized");
}

void EntityRenderer::cleanup(Tyra::TextureRepository* textureRepo) {
    textureRepo->freeBySprite(playerSprite);
    textureRepo->freeBySprite(projectileSprite);
    textureRepo->freeBySprite(mobSprite);
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
    
    Tyra::Vec2 screenPos = camera->worldToScreen(player->position);
    
    playerSprite.position = screenPos;
    renderer->render(playerSprite);
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
        
        // Use a specific tile from the items sheet for projectile
        // The items sheet is 256px wide with 16px tiles = 16 tiles per row
        int tileIndex = 98;  // Adjust this to pick the right projectile sprite
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
    
    for (const auto& mob : mobManager->getMobs()) {
        if (!mob.active) continue;
        
        Tyra::Vec2 screenPos = camera->worldToScreen(mob.position);
        
        // Create sprite for this mob
        Tyra::Sprite sprite;
        sprite.size = mob.size;
        sprite.position = screenPos;
        sprite.id = mobSprite.id;
        sprite.mode = Tyra::SpriteMode::MODE_REPEAT;
        
        // Select tile based on mob type
        // The mobs sheet is 256px wide with 32px tiles = 8 tiles per row
        int tileIndex = 0;
        switch (mob.type) {
            case 0: tileIndex = 0; break;   // Normal mob
            case 1: tileIndex = 8; break;   // Boss (different row)
            default: tileIndex = 0; break;
        }
        
        int tilesPerRow = 256 / 32;
        int column = tileIndex % tilesPerRow;
        int row = tileIndex / tilesPerRow;
        
        sprite.offset = Tyra::Vec2(
            static_cast<float>(column * 32),
            static_cast<float>(row * 32)
        );
        
        // Scale sprite if mob size differs from tile size
        if (mob.size.x != 32.0f) {
            sprite.scale = mob.size.x / 32.0f;
        }
        
        renderer->render(sprite);
    }
}

}  // namespace CanalUx