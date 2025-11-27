/*
 * CanalUx - Entity Renderer Implementation
 */

#include "rendering/entity_renderer.hpp"
#include "core/camera.hpp"
#include "entities/player.hpp"
#include "managers/projectile_manager.hpp"
#include "managers/mob_manager.hpp"
#include "world/room.hpp"
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
    
    // Load Lock Keeper boss sprite (256x256 placeholder)
    filepath = Tyra::FileUtils::fromCwd("lockkeeper_placeholder.png");
    auto* lockKeeperTexture = textureRepo->add(filepath);
    
    lockKeeperSprite.mode = Tyra::SpriteMode::MODE_STRETCH;
    lockKeeperSprite.size = Tyra::Vec2(256.0f, 256.0f);
    lockKeeperTexture->addLink(lockKeeperSprite.id);
    
    // Ring attack uses projectiles now - no ring sprite needed
    
    // Trolley uses the same mobs sprite sheet (row 3, y=192)
    // Just link to the same texture as mobSprite
    trolleySprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    trolleySprite.size = Tyra::Vec2(64.0f, 64.0f);
    trolleySprite.id = mobSprite.id;  // Share texture with mob sprite sheet
    
    TYRA_LOG("EntityRenderer: Initialized");
}

void EntityRenderer::cleanup(Tyra::TextureRepository* textureRepo) {
    textureRepo->freeBySprite(playerSprite);
    textureRepo->freeBySprite(projectileSprite);
    textureRepo->freeBySprite(mobSprite);
    textureRepo->freeBySprite(submergedSprite);
    textureRepo->freeBySprite(pikeSprite);
    textureRepo->freeBySprite(shadowSprite);
    textureRepo->freeBySprite(lockKeeperSprite);
    // Note: trolleySprite shares texture with mobSprite, don't free separately
    // Note: ringSprite no longer used - ring attack uses projectiles
}

void EntityRenderer::render(Tyra::Renderer2D* renderer, 
                            const Camera* camera,
                            const Player* player,
                            const ProjectileManager* projectileManager,
                            const MobManager* mobManager,
                            const Room* room) {
    // Render order: obstacles, projectiles, mobs, then player (player on top)
    if (room) {
        renderRoomObstacles(renderer, camera, room);
    }
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
        
        // Handle Lock Keeper boss specially
        if (mob.type == MobType::BOSS_LOCKKEEPER) {
            renderLockKeeperBoss(renderer, mob, screenPos, nullptr);
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
        case MobState::PIKE_CIRCLING:
        case MobState::PIKE_CHARGING: {
            // MOVEMENT STATES - Show as submerged ripple
            Tyra::Sprite subSprite;
            subSprite.id = submergedSprite.id;
            subSprite.mode = Tyra::SpriteMode::MODE_STRETCH;
            subSprite.size = Tyra::Vec2(64.0f, 64.0f);
            subSprite.position = screenPos;
            // Movement while submerged
            subSprite.position.x += std::sin(pike.stateTimer * 0.1f) * 2.0f;
            subSprite.position.y += std::cos(pike.stateTimer * 0.15f) * 1.5f;
            // Charging is faster movement = bigger ripple
            float rippleScale = (pike.state == MobState::PIKE_CHARGING) ? 1.8f : 1.4f;
            subSprite.scale = rippleScale;
            subSprite.color = Tyra::Color(80, 130, 180, 140);  // Blue tint
            renderer->render(subSprite);
            return;  // Don't render pike body
        }
        
        case MobState::PIKE_EMERGING:
            // Head pointing UP (rotated +90), from row 1 left side
            offsetX = 0.0f;
            offsetY = 128.0f;
            spriteWidth = 128.0f;
            spriteHeight = 128.0f;
            scale = 0.75f;
            // Wiggle side to side while emerging
            wiggleX = std::sin(pike.stateTimer * 0.3f) * 4.0f;
            // Rise up effect
            if (pike.stateTimer < 10) {
                sprite.position.y += 20.0f - pike.stateTimer * 2.0f;  // Rising up
            }
            break;
            
        case MobState::PIKE_TAIL_SWEEP:
            // Tail pointing UP (rotated -90), from row 1 right side
            offsetX = 128.0f;
            offsetY = 128.0f;
            spriteWidth = 128.0f;
            spriteHeight = 128.0f;
            scale = 0.75f;
            // Wiggle back and forth during sweep - more aggressive
            wiggleX = std::sin(pike.stateTimer * 0.6f) * 8.0f;
            break;
            
        case MobState::PIKE_SUBMERGED:
            // Legacy state - show as ripple
            {
                Tyra::Sprite subSprite;
                subSprite.id = submergedSprite.id;
                subSprite.mode = Tyra::SpriteMode::MODE_STRETCH;
                subSprite.size = Tyra::Vec2(64.0f, 64.0f);
                subSprite.position = screenPos;
                subSprite.position.x += std::sin(pike.stateTimer * 0.1f) * 2.0f;
                subSprite.position.y += std::cos(pike.stateTimer * 0.15f) * 1.5f;
                subSprite.scale = 1.5f;
                subSprite.color = Tyra::Color(100, 150, 200, 150);
                renderer->render(subSprite);
            }
            return;
            
        case MobState::PIKE_LEAP: {
            // Full pike sprite is 256x128, rendered at scale 0.6 = ~154x77 on screen
            // Shadow needs to be centered beneath the pike
            
            float pikeScale = 0.6f;
            float pikeWidthOnScreen = 256.0f * pikeScale;  // ~154 pixels
            float pikeHeightOnScreen = 128.0f * pikeScale; // ~77 pixels
            
            // Shadow grows as pike rises, shrinks as it falls
            float leapProgress = pike.stateTimer / 55.0f;  // Matches crash timing
            float shadowScale = 0.5f + 0.45f * std::sin(std::min(leapProgress, 1.0f) * 3.14159f);
            
            // Shadow base size - wider to match pike body
            float shadowBaseWidth = 180.0f;
            float shadowBaseHeight = 50.0f;
            float shadowWidthOnScreen = shadowBaseWidth * shadowScale;
            float shadowHeightOnScreen = shadowBaseHeight * shadowScale;
            
            // Center shadow under the pike
            // Pike position is top-left, so center is at position + (width/2, height/2)
            // Shadow should be centered at same X, but below on Y
            Tyra::Sprite shadow;
            shadow.id = shadowSprite.id;
            shadow.mode = Tyra::SpriteMode::MODE_STRETCH;
            shadow.size = Tyra::Vec2(shadowBaseWidth, shadowBaseHeight);
            
            // Center X: pike center minus half shadow width
            shadow.position.x = screenPos.x + (pikeWidthOnScreen / 2.0f) - (shadowWidthOnScreen / 2.0f);
            // Y: below pike (at water level)
            shadow.position.y = screenPos.y + pikeHeightOnScreen + 10.0f;
            
            shadow.scale = shadowScale;
            // More transparent - blend with water (alpha around 60-90)
            int shadowAlpha = 50 + static_cast<int>(40 * shadowScale);
            shadow.color = Tyra::Color(30, 40, 50, shadowAlpha);
            renderer->render(shadow);
            
            // Full pike in the air
            offsetY = 0.0f;
            spriteWidth = 256.0f;
            spriteHeight = 128.0f;
            
            // Arc motion: rise then fall
            float arcHeight = 0.0f;
            if (pike.stateTimer < 25) {
                // Rising
                arcHeight = (pike.stateTimer / 25.0f) * 50.0f;
            } else if (pike.stateTimer < 55) {
                // In air then falling
                float airProgress = (pike.stateTimer - 25) / 30.0f;
                arcHeight = 50.0f * (1.0f - airProgress * airProgress);
            }
            sprite.position.y -= arcHeight;
            
            scale = pikeScale;
            wiggleX = std::sin(pike.stateTimer * 0.4f) * 2.0f;
            break;
        }
            
        default:
            // Fallback - show full pike
            offsetY = 0.0f;
            spriteWidth = 256.0f;
            spriteHeight = 128.0f;
            scale = 0.5f;
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

void EntityRenderer::renderLockKeeperBoss(Tyra::Renderer2D* renderer, 
                                           const MobManager::MobData& lk, 
                                           const Tyra::Vec2& screenPos,
                                           const Room* room) {
    (void)room;  // Unused for now
    
    // Lock Keeper sprite - placeholder is 256x256
    Tyra::Sprite sprite;
    sprite.id = lockKeeperSprite.id;
    sprite.mode = Tyra::SpriteMode::MODE_STRETCH;
    sprite.position = screenPos;
    sprite.size = Tyra::Vec2(256.0f, 256.0f);
    sprite.scale = 0.5f;  // Scale down to ~128x128 on screen
    
    // Animate based on state
    float wiggleX = 0.0f;
    float scaleModifier = 1.0f;
    
    switch (lk.state) {
        case MobState::LOCKKEEPER_WALKING:
            // Slight bob while walking
            wiggleX = std::sin(lk.stateTimer * 0.15f) * 3.0f;
            break;
            
        case MobState::LOCKKEEPER_WINDUP:
            // Shake while winding up
            wiggleX = std::sin(lk.stateTimer * 0.5f) * (lk.stateTimer / 10.0f);
            // Grow slightly
            scaleModifier = 1.0f + (lk.stateTimer / 45.0f) * 0.1f;
            break;
            
        case MobState::LOCKKEEPER_SLAM:
            // Compressed during slam
            scaleModifier = 0.9f;
            break;
            
        case MobState::LOCKKEEPER_THROW_WINDUP:
            // Lean back
            wiggleX = -5.0f - lk.stateTimer * 0.2f;
            break;
            
        case MobState::LOCKKEEPER_THROWING:
            // Lean forward
            wiggleX = 10.0f;
            break;
            
        case MobState::LOCKKEEPER_STUNNED:
            // Slight wobble
            wiggleX = std::sin(lk.stateTimer * 0.3f) * 2.0f;
            scaleModifier = 0.95f;
            break;
            
        default:
            break;
    }
    
    sprite.position.x += wiggleX;
    sprite.scale *= scaleModifier;
    
    // Flip sprite based on facing direction
    if (!lk.facingRight) {
        sprite.flipHorizontal = true;
    }
    
    renderer->render(sprite);
    
    // Ring attack now uses projectiles instead of sprite - no ring rendering needed
    
    // Render flying trolley during throw
    if (lk.state == MobState::LOCKKEEPER_THROWING) {
        // Trolley flies in an arc from boss to target
        Tyra::Sprite trolley;
        trolley.id = trolleySprite.id;
        trolley.mode = Tyra::SpriteMode::MODE_REPEAT;
        trolley.size = Tyra::Vec2(64.0f, 64.0f);
        trolley.offset = Tyra::Vec2(0.0f, 192.0f);  // Row 3 in mobs sheet
        trolley.scale = 0.75f;
        
        // Interpolate position
        float t = lk.trolleyProgress;
        float startX = screenPos.x + 64.0f;
        float startY = screenPos.y + 32.0f;
        
        // Target position - need to convert from world coords
        // Approximate: target is somewhere in the arena
        float targetX = startX + (lk.trolleyTarget.x - lk.position.x) * Constants::TILE_SIZE;
        float targetY = startY + (lk.trolleyTarget.y - lk.position.y) * Constants::TILE_SIZE;
        
        // Linear interpolation with arc
        trolley.position.x = startX + (targetX - startX) * t;
        float arcHeight = std::sin(t * 3.14159f) * 80.0f;  // Arc up then down
        trolley.position.y = startY + (targetY - startY) * t - arcHeight;
        
        renderer->render(trolley);
        
        // Render shadow below trolley
        Tyra::Sprite shadow;
        shadow.id = shadowSprite.id;
        shadow.mode = Tyra::SpriteMode::MODE_STRETCH;
        shadow.size = Tyra::Vec2(64.0f, 32.0f);
        shadow.position.x = trolley.position.x;
        shadow.position.y = startY + (targetY - startY) * t + 20.0f;  // On ground
        shadow.scale = 0.3f + 0.4f * std::sin(t * 3.14159f);
        shadow.color = Tyra::Color(30, 30, 30, 80);
        
        renderer->render(shadow);
    }
}

void EntityRenderer::renderRoomObstacles(Tyra::Renderer2D* renderer, 
                                          const Camera* camera, 
                                          const Room* room) {
    if (!room || !camera) return;
    
    for (const auto& obstacle : room->getObstacles()) {
        Tyra::Vec2 screenPos = camera->worldToScreen(obstacle.position);
        
        // Render based on obstacle type
        if (obstacle.type == 0) {  // Trolley
            Tyra::Sprite sprite;
            sprite.id = trolleySprite.id;
            sprite.mode = Tyra::SpriteMode::MODE_REPEAT;
            sprite.size = Tyra::Vec2(64.0f, 64.0f);
            sprite.offset = Tyra::Vec2(0.0f, 192.0f);  // Row 3 in mobs sheet
            sprite.position = screenPos;
            sprite.scale = 0.5f; // tile size
            
            renderer->render(sprite);
        }
    }
}

}  // namespace CanalUx