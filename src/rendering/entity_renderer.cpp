/*
 * CanalUx - Entity Renderer Implementation
 */

#include "rendering/entity_renderer.hpp"
#include "core/camera.hpp"
#include "entities/player.hpp"
#include "managers/projectile_manager.hpp"
#include "managers/mob_manager.hpp"
#include <cmath>

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
    
    // Load pike boss sprite sheet (256x256)
    // Row 0 (y=0-127): Full pike 256x128
    // Row 1 (y=128-255): Head up 128x128 (left) | Tail up 128x128 (right)
    filepath = Tyra::FileUtils::fromCwd("pike_boss_sheet.png");
    auto* pikeTexture = textureRepo->add(filepath);
    
    pikeSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    pikeSprite.size = Tyra::Vec2(256.0f, 256.0f);
    pikeTexture->addLink(pikeSprite.id);
    
    // Load shadow sprite (128x64) for leap attacks
    filepath = Tyra::FileUtils::fromCwd("shadow.png");
    auto* shadowTexture = textureRepo->add(filepath);
    
    shadowSprite.mode = Tyra::SpriteMode::MODE_STRETCH;
    shadowSprite.size = Tyra::Vec2(128.0f, 64.0f);
    shadowTexture->addLink(shadowSprite.id);
    
    TYRA_LOG("EntityRenderer: Initialized");
}

void EntityRenderer::cleanup(Tyra::TextureRepository* textureRepo) {
    textureRepo->freeBySprite(playerSprite);
    textureRepo->freeBySprite(projectileSprite);
    textureRepo->freeBySprite(mobSprite);
    textureRepo->freeBySprite(submergedSprite);
    textureRepo->freeBySprite(pikeSprite);
    textureRepo->freeBySprite(shadowSprite);
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
        
        // Handle Pike boss specially
        if (mob.type == MobType::BOSS_PIKE) {
            renderPikeBoss(renderer, mob, screenPos);
            continue;
        }
        
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

void EntityRenderer::renderPikeBoss(Tyra::Renderer2D* renderer,
                                     const MobManager::MobData& pike,
                                     const Tyra::Vec2& screenPos) {
    Tyra::Sprite sprite;
    sprite.id = pikeSprite.id;
    sprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    sprite.position = screenPos;
    
    // Pike sprite sheet is 256x256:
    // Row 0 (y=0-127): Full pike 256x128
    // Row 1 (y=128-255): Head rotated up 128x128 (left) | Tail rotated up 128x128 (right)
    
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float spriteWidth = 256.0f;
    float spriteHeight = 128.0f;
    float scale = 0.5f;
    
    // Wiggle effect based on timer (oscillates position slightly)
    float wiggleX = 0.0f;
    float wiggleY = 0.0f;
    
    switch (pike.state) {
        case MobState::PIKE_EMERGING:
            // Head pointing UP (rotated +90), from row 1 left side
            offsetX = 0.0f;
            offsetY = 128.0f;
            spriteWidth = 128.0f;
            spriteHeight = 128.0f;
            scale = 0.75f;
            // Wiggle side to side while emerging
            wiggleX = std::sin(pike.stateTimer * 0.3f) * 3.0f;
            break;
            
        case MobState::PIKE_TAIL_SWEEP:
            // Tail pointing UP (rotated -90), from row 1 right side
            offsetX = 128.0f;
            offsetY = 128.0f;
            spriteWidth = 128.0f;
            spriteHeight = 128.0f;
            scale = 0.75f;
            // Wiggle back and forth during sweep
            wiggleX = std::sin(pike.stateTimer * 0.5f) * 5.0f;
            break;
            
        case MobState::PIKE_SUBMERGED:
            // When submerged, show ripple effect (use submerged sprite)
            {
                Tyra::Sprite subSprite;
                subSprite.id = submergedSprite.id;
                subSprite.mode = Tyra::SpriteMode::MODE_STRETCH;
                subSprite.size = Tyra::Vec2(64.0f, 64.0f);
                subSprite.position = screenPos;
                // Subtle movement while submerged
                subSprite.position.x += std::sin(pike.stateTimer * 0.1f) * 2.0f;
                subSprite.position.y += std::cos(pike.stateTimer * 0.15f) * 1.5f;
                subSprite.scale = 1.5f;
                subSprite.color = Tyra::Color(100, 150, 200, 150);  // Blue tint
                renderer->render(subSprite);
            }
            return;  // Don't render pike sprite when fully submerged
            
        case MobState::PIKE_LEAP:
            // Show shadow on the water where pike will land
            {
                Tyra::Sprite shadow;
                shadow.id = shadowSprite.id;
                shadow.mode = Tyra::SpriteMode::MODE_STRETCH;
                shadow.size = Tyra::Vec2(128.0f, 64.0f);
                shadow.position = screenPos;
                shadow.position.y += 40.0f;  // Shadow below pike
                // Shadow grows as pike rises, shrinks as it falls
                float leapProgress = pike.stateTimer / 60.0f;  // 0 to 1 over 60 frames
                float shadowScale = 0.5f + 0.3f * std::sin(leapProgress * 3.14159f);
                shadow.scale = shadowScale;
                shadow.color = Tyra::Color(50, 50, 50, 120);  // Dark, semi-transparent
                renderer->render(shadow);
            }
            // Full pike in the air
            offsetY = 0.0f;
            spriteWidth = 256.0f;
            spriteHeight = 128.0f;
            // Arc motion: rise then fall
            {
                float leapProgress = pike.stateTimer / 60.0f;
                float arcHeight = std::sin(leapProgress * 3.14159f) * 40.0f;
                sprite.position.y -= arcHeight;
            }
            scale = 0.6f;
            // Slight wiggle in the air
            wiggleX = std::sin(pike.stateTimer * 0.4f) * 2.0f;
            break;
            
        case MobState::PIKE_CHARGING:
            // Full pike, slightly larger and wiggling aggressively
            offsetY = 0.0f;
            spriteWidth = 256.0f;
            spriteHeight = 128.0f;
            scale = 0.55f;
            wiggleY = std::sin(pike.stateTimer * 0.8f) * 2.0f;
            break;
            
        case MobState::PIKE_CIRCLING:
        default:
            // Full pike, normal size with gentle swimming motion
            offsetY = 0.0f;
            spriteWidth = 256.0f;
            spriteHeight = 128.0f;
            scale = 0.5f;
            // Gentle up/down swimming motion
            wiggleY = std::sin(pike.stateTimer * 0.15f) * 1.5f;
            break;
    }
    
    // Apply wiggle
    sprite.position.x += wiggleX;
    sprite.position.y += wiggleY;
    
    sprite.offset = Tyra::Vec2(offsetX, offsetY);
    sprite.size = Tyra::Vec2(spriteWidth, spriteHeight);
    sprite.scale = scale;
    
    renderer->render(sprite);
}

}  // namespace CanalUx