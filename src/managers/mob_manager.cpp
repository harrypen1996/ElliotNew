/*
 * CanalUx - Mob Manager Implementation
 */

#include "managers/mob_manager.hpp"
#include "world/room.hpp"
#include "entities/player.hpp"
#include "managers/projectile_manager.hpp"
#include <cmath>
#include <cstdlib>

namespace CanalUx {

MobManager::MobManager() : deltaTime(1.0f / 60.0f) {
    mobs.reserve(20);
}

MobManager::~MobManager() {
}

void MobManager::spawnMobsForRoom(Room* room, int levelNumber) {
    clear();
    
    if (!room) return;
    
    // Don't spawn mobs in start room, shop, or already cleared rooms
    if (room->getType() == RoomType::START || 
        room->getType() == RoomType::SHOP ||
        room->isCleared()) {
        return;
    }
    
    float roomWidth = static_cast<float>(room->getWidth());
    float roomHeight = static_cast<float>(room->getHeight());
    
    // Boss rooms get a level-specific boss
    if (room->getType() == RoomType::BOSS) {
        MobData boss;
        boss.position = Tyra::Vec2(roomWidth / 2.0f, roomHeight / 2.0f - 2.0f);
        boss.velocity = Tyra::Vec2(0, 0);
        boss.active = true;
        boss.stateTimer = 0;
        boss.actionCooldown = 60;  // Initial delay before attacking
        boss.facingRight = true;
        boss.submerged = false;
        boss.rotation = 0;
        boss.phase = 1;
        boss.attackPattern = 0;
        boss.circleAngle = 0;
        boss.chargeSpeed = 0;
        boss.tailSweepAngle = 0;
        
        // Spawn different boss based on level
        switch (levelNumber) {
            case 1:
                // PIKE - Giant fish boss
                boss.type = MobType::BOSS_PIKE;
                boss.size = Tyra::Vec2(96.0f, 48.0f);  // Long fish shape
                boss.health = 25.0f;
                boss.maxHealth = boss.health;
                boss.speed = 0.04f;
                boss.state = MobState::PIKE_CIRCLING;
                boss.submerged = true;  // Starts underwater
                TYRA_LOG("MobManager: Spawned PIKE boss");
                break;
                
            case 2:
                // LOCK KEEPER
                boss.type = MobType::BOSS_LOCKKEEPER;
                boss.size = Tyra::Vec2(64.0f, 64.0f);
                boss.health = 30.0f;
                boss.maxHealth = boss.health;
                boss.speed = 0.02f;
                boss.state = MobState::IDLE;
                TYRA_LOG("MobManager: Spawned LOCK KEEPER boss");
                break;
                
            case 3:
                // NANNY
                boss.type = MobType::BOSS_NANNY;
                boss.size = Tyra::Vec2(64.0f, 64.0f);
                boss.health = 35.0f;
                boss.maxHealth = boss.health;
                boss.speed = 0.015f;
                boss.state = MobState::IDLE;
                TYRA_LOG("MobManager: Spawned NANNY boss");
                break;
                
            default:
                // Fallback generic boss
                boss.type = MobType::BOSS;
                boss.size = Tyra::Vec2(64.0f, 64.0f);
                boss.health = 15.0f + levelNumber * 5.0f;
                boss.maxHealth = boss.health;
                boss.speed = 0.025f;
                boss.state = MobState::IDLE;
                TYRA_LOG("MobManager: Spawned generic boss");
                break;
        }
        
        mobs.push_back(boss);
        return;
    }
    
    // Determine number of mobs based on level and room type
    int minMobs = 2 + levelNumber;
    int maxMobs = 4 + levelNumber;
    
    // Special rooms get fewer mobs
    if (room->getType() == RoomType::SPECIAL) {
        minMobs = 1;
        maxMobs = 2;
    }
    
    int numMobs = minMobs + (rand() % (maxMobs - minMobs + 1));
    
    // Spawn a mix of mob types
    for (int i = 0; i < numMobs; i++) {
        MobData mob;
        
        // Random position, but not too close to edges or center
        float minX = 3.0f;
        float maxX = roomWidth - 4.0f;
        float minY = 3.0f;
        float maxY = roomHeight - 4.0f;
        
        // Try to place away from center
        int attempts = 0;
        do {
            mob.position.x = minX + static_cast<float>(rand()) / RAND_MAX * (maxX - minX);
            mob.position.y = minY + static_cast<float>(rand()) / RAND_MAX * (maxY - minY);
            attempts++;
        } while (attempts < 10 && 
                 std::abs(mob.position.x - roomWidth / 2.0f) < 3.0f &&
                 std::abs(mob.position.y - roomHeight / 2.0f) < 3.0f);
        
        mob.velocity = Tyra::Vec2(0, 0);
        mob.size = Tyra::Vec2(32.0f, 32.0f);
        mob.active = true;
        mob.stateTimer = 0;
        mob.actionCooldown = 0;
        mob.facingRight = true;
        mob.submerged = false;
        
        // Randomly choose mob type with weighted distribution
        int typeRoll = rand() % 100;
        if (typeRoll < 50) {
            // 50% chance - Duck (chaser)
            mob.type = MobType::DUCK;
            mob.health = 2.0f + levelNumber * 0.5f;
            mob.speed = 0.025f + static_cast<float>(rand()) / RAND_MAX * 0.01f;
            mob.state = MobState::CHASING;
        } else if (typeRoll < 80) {
            // 30% chance - Frog (jumper)
            mob.type = MobType::FROG;
            mob.health = 3.0f + levelNumber * 0.5f;
            mob.speed = 0.08f;  // Fast jump speed
            mob.state = MobState::IDLE;
            mob.actionCooldown = static_cast<float>(rand() % 60 + 30);  // Random start delay
        } else {
            // 20% chance - Swan (shooter)
            mob.type = MobType::SWAN;
            mob.health = 2.0f + levelNumber * 0.3f;
            mob.speed = 0.012f;  // Slower movement
            mob.state = MobState::IDLE;
            mob.actionCooldown = static_cast<float>(rand() % 60 + 60);  // Shoot delay
        }
        
        mob.maxHealth = mob.health;
        mobs.push_back(mob);
    }
    
    TYRA_LOG("MobManager: Spawned ", numMobs, " mobs");
}

void MobManager::update(Room* currentRoom, Player* player, ProjectileManager* projectileManager) {
    if (!currentRoom || !player) return;
    
    for (auto& mob : mobs) {
        if (!mob.active) continue;
        
        // Update state timer
        mob.stateTimer += 1.0f;
        if (mob.actionCooldown > 0) {
            mob.actionCooldown -= 1.0f;
        }
        
        // Update facing direction based on player position
        mob.facingRight = player->position.x > mob.position.x;
        
        // Update based on mob type
        switch (mob.type) {
            case MobType::DUCK:
                updateDuck(mob, currentRoom, player);
                break;
            case MobType::SWAN:
                updateSwan(mob, currentRoom, player, projectileManager);
                break;
            case MobType::FROG:
                updateFrog(mob, currentRoom, player);
                break;
            case MobType::BOSS:
                updateBoss(mob, currentRoom, player, projectileManager);
                break;
            case MobType::BOSS_PIKE:
                updatePikeBoss(mob, currentRoom, player, projectileManager);
                break;
            case MobType::BOSS_LOCKKEEPER:
                updateLockKeeperBoss(mob, currentRoom, player, projectileManager);
                break;
            case MobType::BOSS_NANNY:
                updateNannyBoss(mob, currentRoom, player, projectileManager);
                break;
            default:
                updateDuck(mob, currentRoom, player);
                break;
        }
    }
    
    // Apply repulsion between mobs so they don't overlap
    applyMobRepulsion();
    
    // Remove dead mobs
    mobs.erase(
        std::remove_if(mobs.begin(), mobs.end(),
            [](const MobData& m) { return !m.active || m.health <= 0; }),
        mobs.end()
    );
}

void MobManager::updateDuck(MobData& mob, Room* room, Player* player) {
    // Duck behavior: Dumbly chase the player
    float dx = player->position.x - mob.position.x;
    float dy = player->position.y - mob.position.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance > 0.5f) {
        // Move towards player
        float vx = (dx / distance) * mob.speed;
        float vy = (dy / distance) * mob.speed;
        
        float newX = mob.position.x + vx;
        float newY = mob.position.y + vy;
        
        if (canMoveTo(mob, newX, mob.position.y, room)) {
            mob.position.x = newX;
        }
        if (canMoveTo(mob, mob.position.x, newY, room)) {
            mob.position.y = newY;
        }
    }
}

void MobManager::updateSwan(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager) {
    // Swan behavior: Keep distance and shoot feathers at player
    float dx = player->position.x - mob.position.x;
    float dy = player->position.y - mob.position.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    float preferredDistance = 5.0f;  // Try to stay this far from player
    
    if (distance < preferredDistance - 1.0f) {
        // Too close - back away
        float vx = -(dx / distance) * mob.speed;
        float vy = -(dy / distance) * mob.speed;
        
        float newX = mob.position.x + vx;
        float newY = mob.position.y + vy;
        
        if (canMoveTo(mob, newX, mob.position.y, room)) {
            mob.position.x = newX;
        }
        if (canMoveTo(mob, mob.position.x, newY, room)) {
            mob.position.y = newY;
        }
    } else if (distance > preferredDistance + 2.0f) {
        // Too far - move closer slowly
        float vx = (dx / distance) * mob.speed * 0.5f;
        float vy = (dy / distance) * mob.speed * 0.5f;
        
        float newX = mob.position.x + vx;
        float newY = mob.position.y + vy;
        
        if (canMoveTo(mob, newX, mob.position.y, room)) {
            mob.position.x = newX;
        }
        if (canMoveTo(mob, mob.position.x, newY, room)) {
            mob.position.y = newY;
        }
    }
    
    // Shoot at player when cooldown is ready
    if (mob.actionCooldown <= 0 && distance < 10.0f) {
        // Create feather projectile
        Tyra::Vec2 projPos = mob.position;
        projPos.x += mob.size.x / Constants::TILE_SIZE / 2.0f;
        projPos.y += mob.size.y / Constants::TILE_SIZE / 2.0f;
        
        // Aim at player
        float projSpeed = 0.06f;
        Tyra::Vec2 projVel;
        projVel.x = (dx / distance) * projSpeed;
        projVel.y = (dy / distance) * projSpeed;
        
        projectileManager->spawnEnemyProjectile(projPos, projVel, 1.0f);
        
        // Reset cooldown (90-150 frames)
        mob.actionCooldown = static_cast<float>(90 + rand() % 60);
    }
}

void MobManager::updateFrog(MobData& mob, Room* room, Player* player) {
    // Frog behavior: Jump around, submerge between jumps
    float dx = player->position.x - mob.position.x;
    float dy = player->position.y - mob.position.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    switch (mob.state) {
        case MobState::IDLE:
            // Wait for cooldown then start jump
            if (mob.actionCooldown <= 0) {
                mob.state = MobState::JUMPING;
                mob.stateTimer = 0;
                
                // Calculate jump target (towards player with some randomness)
                float jumpDist = 2.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
                float angle = std::atan2(dy, dx);
                // Add some randomness to angle
                angle += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.8f;
                
                mob.jumpTarget.x = mob.position.x + std::cos(angle) * jumpDist;
                mob.jumpTarget.y = mob.position.y + std::sin(angle) * jumpDist;
                
                // Clamp to room bounds
                mob.jumpTarget.x = std::max(2.0f, std::min(mob.jumpTarget.x, static_cast<float>(room->getWidth()) - 3.0f));
                mob.jumpTarget.y = std::max(2.0f, std::min(mob.jumpTarget.y, static_cast<float>(room->getHeight()) - 3.0f));
            }
            break;
            
        case MobState::JUMPING:
            // Move towards jump target
            {
                float jdx = mob.jumpTarget.x - mob.position.x;
                float jdy = mob.jumpTarget.y - mob.position.y;
                float jdist = std::sqrt(jdx * jdx + jdy * jdy);
                
                if (jdist > 0.2f && mob.stateTimer < 30) {
                    float vx = (jdx / jdist) * mob.speed;
                    float vy = (jdy / jdist) * mob.speed;
                    
                    mob.position.x += vx;
                    mob.position.y += vy;
                } else {
                    // Land and submerge
                    mob.state = MobState::SUBMERGED;
                    mob.stateTimer = 0;
                    mob.submerged = true;
                }
            }
            break;
            
        case MobState::SUBMERGED:
            // Stay submerged for a bit (invulnerable during this time could be added)
            if (mob.stateTimer > 45) {
                mob.state = MobState::SURFACING;
                mob.stateTimer = 0;
            }
            break;
            
        case MobState::SURFACING:
            // Surface animation
            mob.submerged = false;
            if (mob.stateTimer > 15) {
                mob.state = MobState::IDLE;
                mob.stateTimer = 0;
                mob.actionCooldown = 30 + rand() % 30;  // Wait before next jump
            }
            break;
            
        default:
            mob.state = MobState::IDLE;
            break;
    }
}

void MobManager::updateBoss(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager) {
    // Boss combines behaviors: chases, shoots, and occasionally jumps
    float dx = player->position.x - mob.position.x;
    float dy = player->position.y - mob.position.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    // Chase player
    if (distance > 2.0f) {
        float vx = (dx / distance) * mob.speed;
        float vy = (dy / distance) * mob.speed;
        
        float newX = mob.position.x + vx;
        float newY = mob.position.y + vy;
        
        if (canMoveTo(mob, newX, mob.position.y, room)) {
            mob.position.x = newX;
        }
        if (canMoveTo(mob, mob.position.x, newY, room)) {
            mob.position.y = newY;
        }
    }
    
    // Shoot spread of projectiles periodically
    if (mob.actionCooldown <= 0) {
        // Shoot 3 projectiles in a spread
        for (int i = -1; i <= 1; i++) {
            Tyra::Vec2 projPos = mob.position;
            projPos.x += mob.size.x / Constants::TILE_SIZE / 2.0f;
            projPos.y += mob.size.y / Constants::TILE_SIZE / 2.0f;
            
            float baseAngle = std::atan2(dy, dx);
            float angle = baseAngle + i * 0.3f;  // Spread angle
            
            float projSpeed = 0.05f;
            Tyra::Vec2 projVel;
            projVel.x = std::cos(angle) * projSpeed;
            projVel.y = std::sin(angle) * projSpeed;
            
            projectileManager->spawnEnemyProjectile(projPos, projVel, 1.0f);
        }
        
        mob.actionCooldown = 120 + rand() % 60;  // Longer cooldown for boss
    }
}

void MobManager::updatePikeBoss(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager) {
    /*
     * PIKE BOSS - Level 1
     * 
     * A giant pike fish that lurks beneath the water.
     * 
     * MOVEMENT (always submerged, no damage):
     * - Circling: Swims in circles around player, repositioning
     * - Charging: Fast repositioning underwater
     * 
     * ATTACKS (surfaces to attack):
     * 1. Emerging - Bursts out of water to bite player
     * 2. Tail Sweep - Tail surfaces and sweeps, spawns projectiles
     * 3. Leap - Jumps completely out, crashes down creating splash
     * 
     * PHASES:
     * Phase 1 (100-60% HP): Emerging attack only, slower
     * Phase 2 (60-30% HP): Adds Tail Sweep
     * Phase 3 (<30% HP): Adds Leap attack, faster attack frequency
     */
    
    float dx = player->position.x - mob.position.x;
    float dy = player->position.y - mob.position.y;
    float distToPlayer = std::sqrt(dx * dx + dy * dy);
    
    float roomWidth = static_cast<float>(room->getWidth());
    float roomHeight = static_cast<float>(room->getHeight());
    
    // Pike size in tiles (96x48 pixels = 3x1.5 tiles)
    const float pikeTileWidth = 3.0f;
    const float pikeTileHeight = 1.5f;
    
    // Room bounds for pike (accounting for its size and wall thickness)
    const float minX = 2.5f;
    const float minY = 2.5f;
    const float maxX = roomWidth - 2.5f - pikeTileWidth;
    const float maxY = roomHeight - 2.5f - pikeTileHeight;
    
    // Helper lambda to clamp pike position to room bounds
    auto clampToRoom = [&]() {
        mob.position.x = std::max(minX, std::min(mob.position.x, maxX));
        mob.position.y = std::max(minY, std::min(mob.position.y, maxY));
    };
    
    // Update phase based on health
    float healthPercent = mob.health / mob.maxHealth;
    if (healthPercent <= 0.3f) {
        mob.phase = 3;
    } else if (healthPercent <= 0.6f) {
        mob.phase = 2;
    } else {
        mob.phase = 1;
    }
    
    // Update facing direction
    mob.facingRight = dx > 0;
    
    switch (mob.state) {
        case MobState::PIKE_CIRCLING: {
            // MOVEMENT PHASE - Always submerged, no damage
            // Circles around player, getting into position for attacks
            mob.submerged = true;
            
            // Circle around the player at a set distance
            float circleRadius = 4.0f - mob.phase * 0.5f;  // Gets closer in later phases
            float circleSpeed = 0.015f + (mob.phase - 1) * 0.005f;
            
            mob.circleAngle += circleSpeed;
            if (mob.circleAngle > 6.28f) mob.circleAngle -= 6.28f;
            
            // Target position on circle around player
            float targetX = player->position.x + std::cos(mob.circleAngle) * circleRadius;
            float targetY = player->position.y + std::sin(mob.circleAngle) * circleRadius;
            
            // Clamp target to room bounds
            targetX = std::max(minX, std::min(targetX, maxX));
            targetY = std::max(minY, std::min(targetY, maxY));
            
            // Move towards target position
            float tdx = targetX - mob.position.x;
            float tdy = targetY - mob.position.y;
            float tdist = std::sqrt(tdx * tdx + tdy * tdy);
            
            if (tdist > 0.1f) {
                mob.position.x += (tdx / tdist) * mob.speed;
                mob.position.y += (tdy / tdist) * mob.speed;
            }
            
            // Clamp position after movement
            clampToRoom();
            
            // Update rotation to face movement direction
            mob.rotation = std::atan2(tdy, tdx);
            
            // Decide next action when cooldown is ready
            if (mob.actionCooldown <= 0) {
                int attackRoll = rand() % 100;
                
                // Attack frequency increases with phase
                int attackChance = 50 + mob.phase * 12;  // 62%, 74%, 86%
                
                // Only attack if reasonably close to player (good position)
                bool inGoodPosition = distToPlayer < 5.0f;
                
                if (attackRoll < attackChance && inGoodPosition) {
                    // Time to attack! Choose which attack
                    // All attacks available from phase 1, but weights change
                    int attackChoice = rand() % 100;
                    
                    // Phase 1: 20% leap, 30% tail, 50% emerge
                    // Phase 2: 25% leap, 35% tail, 40% emerge  
                    // Phase 3: 30% leap, 40% tail, 30% emerge
                    int leapChance = 15 + mob.phase * 5;      // 20%, 25%, 30%
                    int tailChance = 25 + mob.phase * 5;      // 30%, 35%, 40%
                    // emerge is the remainder
                    
                    if (attackChoice < leapChance) {
                        // Leap attack
                        mob.state = MobState::PIKE_LEAP;
                        mob.stateTimer = 0;
                        mob.chargeTarget = player->position;
                        mob.submerged = false;
                    } else if (attackChoice < leapChance + tailChance) {
                        // Tail sweep
                        mob.state = MobState::PIKE_TAIL_SWEEP;
                        mob.stateTimer = 0;
                        mob.tailSweepAngle = std::atan2(dy, dx);
                        mob.submerged = false;
                    } else {
                        // Emerging bite attack
                        mob.state = MobState::PIKE_EMERGING;
                        mob.stateTimer = 0;
                        mob.submerged = false;
                    }
                } else if (!inGoodPosition || attackRoll >= attackChance) {
                    // Reposition - move to a position near the player
                    mob.state = MobState::PIKE_CHARGING;
                    mob.stateTimer = 0;
                    // Target a position close to player for next attack
                    float angle = (rand() % 628) / 100.0f;
                    float dist = 1.5f + (rand() % 20) / 10.0f;  // 1.5 to 3.5 tiles from player
                    mob.chargeTarget.x = player->position.x + std::cos(angle) * dist;
                    mob.chargeTarget.y = player->position.y + std::sin(angle) * dist;
                    // Clamp target to room bounds
                    mob.chargeTarget.x = std::max(minX, std::min(mob.chargeTarget.x, maxX));
                    mob.chargeTarget.y = std::max(minY, std::min(mob.chargeTarget.y, maxY));
                    mob.chargeSpeed = 0.08f + mob.phase * 0.02f;
                }
                
                // Cooldown between actions - faster in later phases
                mob.actionCooldown = 60 - mob.phase * 12;  // 48, 36, 24 frames
            }
            break;
        }
        
        case MobState::PIKE_CHARGING: {
            // REPOSITIONING - Fast underwater movement, no damage
            mob.submerged = true;
            
            float cdx = mob.chargeTarget.x - mob.position.x;
            float cdy = mob.chargeTarget.y - mob.position.y;
            float cdist = std::sqrt(cdx * cdx + cdy * cdy);
            
            mob.rotation = std::atan2(cdy, cdx);
            
            if (cdist > 0.5f && mob.stateTimer < 45) {
                // Still moving to position
                mob.position.x += (cdx / cdist) * mob.chargeSpeed;
                mob.position.y += (cdy / cdist) * mob.chargeSpeed;
                
                // Clamp to room bounds
                clampToRoom();
            } else {
                // Reached position, return to circling
                mob.state = MobState::PIKE_CIRCLING;
                mob.stateTimer = 0;
            }
            break;
        }
        
        case MobState::PIKE_EMERGING: {
            // ATTACK - Burst straight up out of water at current position
            mob.submerged = false;
            
            if (mob.stateTimer < 10) {
                // Rising up - brief telegraph, no horizontal movement
            } else if (mob.stateTimer < 35) {
                // Head is up and snapping - player needs to have moved away
                // Very minimal drift towards player (just tracking, not chasing)
                if (distToPlayer < 1.5f && distToPlayer > 0.1f) {
                    // Only slight adjustment if player is very close
                    mob.position.x += (dx / distToPlayer) * 0.01f;
                    mob.position.y += (dy / distToPlayer) * 0.01f;
                    clampToRoom();
                }
                mob.rotation = std::atan2(dy, dx);
            } else if (mob.stateTimer < 55) {
                // Recovery - sinking back down
            } else {
                // Go back underwater and circle
                mob.state = MobState::PIKE_CIRCLING;
                mob.stateTimer = 0;
                mob.submerged = true;
            }
            break;
        }
        
        case MobState::PIKE_TAIL_SWEEP: {
            // ATTACK - Tail surfaces and sweeps, spawns projectiles
            mob.submerged = false;
            
            if (mob.stateTimer < 15) {
                // Wind up - tail rising
            } else if (mob.stateTimer == 15) {
                // Release projectiles in an arc
                int numProjectiles = 4 + mob.phase;  // 5, 6, 7 projectiles
                float arcSpread = 1.0f + mob.phase * 0.15f;  // Wider arc in later phases
                
                for (int i = 0; i < numProjectiles; i++) {
                    float angle = mob.tailSweepAngle - arcSpread + (arcSpread * 2.0f * i / (numProjectiles - 1));
                    
                    Tyra::Vec2 projPos = mob.position;
                    projPos.x += 0.5f;
                    projPos.y += 0.5f;
                    
                    Tyra::Vec2 projVel;
                    projVel.x = std::cos(angle) * (0.04f + mob.phase * 0.01f);
                    projVel.y = std::sin(angle) * (0.04f + mob.phase * 0.01f);
                    
                    projectileManager->spawnEnemyProjectile(projPos, projVel, 1.0f);
                }
            } else if (mob.stateTimer > 45) {
                // Return to circling underwater
                mob.state = MobState::PIKE_CIRCLING;
                mob.stateTimer = 0;
                mob.submerged = true;
            }
            break;
        }
        
        case MobState::PIKE_LEAP: {
            // ATTACK - Jump completely out, crash down with splash
            mob.submerged = false;
            
            if (mob.stateTimer < 25) {
                // Rising up
            } else if (mob.stateTimer < 55) {
                // In the air - arc towards player
                float progress = (mob.stateTimer - 25) / 30.0f;
                
                // Move towards where player was when leap started
                float ldx = mob.chargeTarget.x - mob.position.x;
                float ldy = mob.chargeTarget.y - mob.position.y;
                float ldist = std::sqrt(ldx * ldx + ldy * ldy);
                
                if (ldist > 0.2f) {
                    mob.position.x += (ldx / ldist) * 0.1f;
                    mob.position.y += (ldy / ldist) * 0.1f;
                    clampToRoom();
                }
            } else if (mob.stateTimer == 55) {
                // Crash down - spawn splash projectiles in all directions
                int numSplash = 8 + mob.phase * 2;  // 10, 12, 14 projectiles
                for (int i = 0; i < numSplash; i++) {
                    float angle = (6.28f / numSplash) * i;
                    
                    Tyra::Vec2 projPos = mob.position;
                    projPos.x += 0.5f;
                    projPos.y += 0.5f;
                    
                    Tyra::Vec2 projVel;
                    projVel.x = std::cos(angle) * (0.03f + mob.phase * 0.01f);
                    projVel.y = std::sin(angle) * (0.03f + mob.phase * 0.01f);
                    
                    projectileManager->spawnEnemyProjectile(projPos, projVel, 1.0f);
                }
            } else if (mob.stateTimer > 85) {
                // Recovery complete, go underwater
                mob.state = MobState::PIKE_CIRCLING;
                mob.stateTimer = 0;
                mob.submerged = true;
            }
            break;
        }
        
        // Legacy states - redirect to circling
        case MobState::PIKE_SUBMERGED:
            mob.state = MobState::PIKE_CIRCLING;
            mob.submerged = true;
            break;
            
        default:
            mob.state = MobState::PIKE_CIRCLING;
            mob.submerged = true;
            break;
    }
}

void MobManager::updateLockKeeperBoss(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager) {
    /*
     * LOCK KEEPER BOSS - Level 2
     * 
     * A canal lock keeper who walks along the TOP of the arena.
     * Player is in the water below.
     * 
     * ATTACKS:
     * 1. Slam - Raises arms, slams down creating expanding shockwave ring
     *    - Player MUST submerge to dodge (instant kill if not submerged when ring hits)
     * 2. Trolley Throw - Throws shopping trolley that lands as permanent obstacle
     * 
     * MECHANICS:
     * - Lock Keeper walks left/right along top edge
     * - Arena shrinks over time (walls close in)
     * - Trolleys reduce playable area
     * 
     * PHASES:
     * Phase 1 (100-60% HP): Slow, mostly slams
     * Phase 2 (60-30% HP): Faster, adds trolley throws
     * Phase 3 (<30% HP): Very fast, frequent attacks, arena shrinks faster
     */
    
    float roomWidth = static_cast<float>(room->getWidth());
    float roomHeight = static_cast<float>(room->getHeight());
    
    // Lock Keeper stays at top of room (y = 1)
    const float bossY = 1.5f;
    mob.position.y = bossY;
    
    // Player distance (horizontal only since boss is at top)
    float dx = player->position.x - mob.position.x;
    float distX = std::abs(dx);
    
    // Update phase based on health
    float healthPercent = mob.health / mob.maxHealth;
    if (healthPercent <= 0.3f) {
        mob.phase = 3;
    } else if (healthPercent <= 0.6f) {
        mob.phase = 2;
    } else {
        mob.phase = 1;
    }
    
    // Facing direction
    mob.facingRight = dx > 0;
    
    // Speed increases with phase
    float walkSpeed = 0.03f + (mob.phase - 1) * 0.015f;
    
    mob.stateTimer++;
    if (mob.actionCooldown > 0) mob.actionCooldown--;
    
    switch (mob.state) {
        case MobState::LOCKKEEPER_WALKING: {
            // Walk along top edge, tracking player
            if (dx > 1.0f) {
                mob.position.x += walkSpeed;
            } else if (dx < -1.0f) {
                mob.position.x -= walkSpeed;
            }
            
            // Clamp to room bounds
            mob.position.x = std::max(3.0f, std::min(mob.position.x, roomWidth - 4.0f));
            
            // Decide on attack
            if (mob.actionCooldown <= 0 && distX < 10.0f) {
                int attackRoll = rand() % 100;
                
                // Phase 1: 50% slam, 30% shot, 20% cooldown
                // Phase 2: 40% slam, 30% shot, 25% trolley, 5% cooldown
                // Phase 3: 35% slam, 35% shot, 28% trolley, 2% cooldown
                int slamChance = 50 - (mob.phase - 1) * 7;      // 50, 43, 35
                int shotChance = 30 + (mob.phase - 1) * 2;      // 30, 32, 35
                int trolleyChance = (mob.phase >= 2) ? 25 + (mob.phase - 2) * 3 : 0;  // 0, 25, 28
                
                if (attackRoll < slamChance) {
                    mob.state = MobState::LOCKKEEPER_WINDUP;
                    mob.stateTimer = 0;
                    // Slam expands from bottom of boss sprite
                    mob.slamPosition.x = mob.position.x + 2.0f;
                    mob.slamPosition.y = mob.position.y + 4.0f;
                } else if (attackRoll < slamChance + shotChance) {
                    // Warning shot - aim at player, fires accelerating projectiles
                    mob.state = MobState::LOCKKEEPER_SHOT;
                    mob.stateTimer = 0;
                    // Calculate direction to player
                    float aimDx = player->position.x - (mob.position.x + 2.0f);
                    float aimDy = player->position.y - (mob.position.y + 4.0f);
                    float aimLen = std::sqrt(aimDx * aimDx + aimDy * aimDy);
                    if (aimLen > 0.0f) {
                        mob.shotDirection.x = aimDx / aimLen;
                        mob.shotDirection.y = aimDy / aimLen;
                    } else {
                        mob.shotDirection.x = 0.0f;
                        mob.shotDirection.y = 1.0f;
                    }
                    // Shot starts at bottom center of boss
                    mob.shotPosition.x = mob.position.x + 2.0f;
                    mob.shotPosition.y = mob.position.y + 4.0f;
                } else if (attackRoll < slamChance + shotChance + trolleyChance && mob.trolleysThrown < 6) {
                    mob.state = MobState::LOCKKEEPER_THROW_WINDUP;
                    mob.stateTimer = 0;
                    // Target somewhere in the arena
                    mob.trolleyTarget.x = room->getArenaMinX() + 2.0f + 
                        static_cast<float>(rand() % static_cast<int>(room->getArenaMaxX() - room->getArenaMinX() - 4.0f));
                    mob.trolleyTarget.y = room->getArenaMinY() + 2.0f +
                        static_cast<float>(rand() % static_cast<int>(room->getArenaMaxY() - room->getArenaMinY() - 4.0f));
                } else {
                    // Brief cooldown - shorter now
                    mob.actionCooldown = 15;
                }
            }
            break;
        }
        
        case MobState::LOCKKEEPER_WINDUP: {
            // Arms raising - telegraph for player to prepare
            if (mob.stateTimer >= 45) {  // ~0.75 seconds warning
                mob.state = MobState::LOCKKEEPER_SLAM;
                mob.stateTimer = 0;
                mob.ringRadius = 0.0f;
            }
            break;
        }
        
        case MobState::LOCKKEEPER_SLAM: {
            // Expanding ring of projectiles
            float ringSpeed = 0.12f + mob.phase * 0.02f;  // Speed in tiles per frame
            mob.ringRadius += ringSpeed;
            
            // Spawn projectiles in a ring pattern every few frames
            if (static_cast<int>(mob.stateTimer) % 3 == 0 && mob.ringRadius > 0.5f) {
                // Number of projectiles in the ring increases as it expands
                int numProjectiles = 16 + static_cast<int>(mob.ringRadius * 2);
                if (numProjectiles > 48) numProjectiles = 48;  // Cap it
                
                for (int i = 0; i < numProjectiles; i++) {
                    float angle = (6.28318f / numProjectiles) * i;
                    
                    // Position projectile on the ring
                    Tyra::Vec2 projPos;
                    projPos.x = mob.slamPosition.x + std::cos(angle) * mob.ringRadius;
                    projPos.y = mob.slamPosition.y + std::sin(angle) * mob.ringRadius;
                    
                    // Projectiles move outward with the ring
                    Tyra::Vec2 projVel;
                    projVel.x = std::cos(angle) * ringSpeed;
                    projVel.y = std::sin(angle) * ringSpeed;
                    
                    // Spawn as enemy projectile with 999 damage (instant kill)
                    // Player must submerge to avoid
                    projectileManager->spawnEnemyProjectile(projPos, projVel, 999.0f);
                }
            }
            
            // Ring dissipates after reaching edges
            float maxRadius = std::max(roomWidth, roomHeight);
            if (mob.ringRadius > maxRadius) {
                mob.state = MobState::LOCKKEEPER_STUNNED;
                mob.stateTimer = 0;
                mob.ringRadius = 0.0f;
            }
            break;
        }
        
        case MobState::LOCKKEEPER_THROW_WINDUP: {
            // Winding up to throw trolley
            if (mob.stateTimer >= 30) {
                mob.state = MobState::LOCKKEEPER_THROWING;
                mob.stateTimer = 0;
                mob.trolleyProgress = 0.0f;
            }
            break;
        }
        
        case MobState::LOCKKEEPER_THROWING: {
            // Trolley flying through air
            mob.trolleyProgress += 0.025f;  // Takes ~40 frames to land
            
            if (mob.trolleyProgress >= 1.0f) {
                // Trolley lands - add obstacle to room
                RoomObstacle trolley;
                trolley.position = mob.trolleyTarget;
                trolley.type = 0;  // Trolley type
                trolley.blocksPlayer = true;      // Player can't walk through
                trolley.blocksEnemies = false;    // Enemies can walk through
                trolley.blocksPlayerShots = false;// Player shots pass through
                trolley.blocksEnemyShots = false; // Enemy shots pass through
                room->addObstacle(trolley);
                
                mob.trolleysThrown++;
                
                // Shrink arena slightly each trolley (phase 2+)
                if (mob.phase >= 2) {
                    room->shrinkArena(0.25f);
                }
                
                mob.state = MobState::LOCKKEEPER_STUNNED;
                mob.stateTimer = 0;
            }
            break;
        }
        
        case MobState::LOCKKEEPER_STUNNED: {
            // Recovery after attack - shorter now
            int recoveryTime = 45 - (mob.phase - 1) * 10;  // 45, 35, 25 frames
            if (mob.stateTimer >= recoveryTime) {
                mob.state = MobState::LOCKKEEPER_WALKING;
                mob.stateTimer = 0;
                mob.actionCooldown = 20 - mob.phase * 4;  // 16, 12, 8 frames
            }
            break;
        }
        
        case MobState::LOCKKEEPER_SHOT: {
            // Fire accelerating projectiles at player
            // Projectiles start slow and speed up, giving player time to react
            int spawnRate = 6 - mob.phase;  // Spawn every 5/4/3 frames based on phase
            if (spawnRate < 3) spawnRate = 3;
            
            if (static_cast<int>(mob.stateTimer) % spawnRate == 0) {
                // Spawn accelerating projectile
                Tyra::Vec2 projPos = mob.shotPosition;
                Tyra::Vec2 projVel;
                float initialSpeed = 0.02f;  // Start very slow
                projVel.x = mob.shotDirection.x * initialSpeed;
                projVel.y = mob.shotDirection.y * initialSpeed;
                
                // Acceleration and max speed increase with phase
                float accel = 0.004f + mob.phase * 0.001f;  // 0.005, 0.006, 0.007
                float maxSpd = 0.25f + mob.phase * 0.05f;   // 0.30, 0.35, 0.40
                
                projectileManager->spawnAcceleratingProjectile(projPos, projVel, 999.0f, accel, maxSpd, false);
            }
            
            // Fire for a duration then stop
            int fireTime = 50 + mob.phase * 15;  // 65, 80, 95 frames
            if (mob.stateTimer >= fireTime) {
                mob.state = MobState::LOCKKEEPER_STUNNED;
                mob.stateTimer = 0;
            }
            break;
        }
        
        default:
            mob.state = MobState::LOCKKEEPER_WALKING;
            mob.stateTimer = 0;
            break;
    }
    
    // Phase 3: Arena shrinks over time
    if (mob.phase >= 3 && static_cast<int>(mob.stateTimer) % 300 == 0 && mob.stateTimer > 0) {  // Every ~5 seconds
        room->shrinkArena(0.1f);
    }
}

void MobManager::updateNannyBoss(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager) {
    // TODO: Level 3 boss - Nanny
    // For now, use generic boss behavior
    updateBoss(mob, room, player, projectileManager);
}

void MobManager::applyMobRepulsion() {
    const float repulsionStrength = 0.02f;  // How strongly mobs push apart
    const float minDistance = 1.2f;          // Distance at which repulsion starts (in tiles)
    
    for (size_t i = 0; i < mobs.size(); i++) {
        if (!mobs[i].active || mobs[i].submerged) continue;
        
        for (size_t j = i + 1; j < mobs.size(); j++) {
            if (!mobs[j].active || mobs[j].submerged) continue;
            
            // Calculate distance between mob centers
            float dx = mobs[j].position.x - mobs[i].position.x;
            float dy = mobs[j].position.y - mobs[i].position.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            // Calculate combined radius (in tiles)
            float radiusI = (mobs[i].size.x / Constants::TILE_SIZE) * 0.5f;
            float radiusJ = (mobs[j].size.x / Constants::TILE_SIZE) * 0.5f;
            float combinedRadius = radiusI + radiusJ;
            
            // Apply repulsion if too close
            if (distance < combinedRadius * minDistance && distance > 0.01f) {
                // Normalize direction
                float nx = dx / distance;
                float ny = dy / distance;
                
                // Calculate overlap amount (stronger repulsion when closer)
                float overlap = (combinedRadius * minDistance) - distance;
                float force = overlap * repulsionStrength;
                
                // Apply force to both mobs (push apart)
                mobs[i].position.x -= nx * force;
                mobs[i].position.y -= ny * force;
                mobs[j].position.x += nx * force;
                mobs[j].position.y += ny * force;
            }
        }
    }
}

bool MobManager::canMoveTo(MobData& mob, float x, float y, Room* room) {
    float sizeInTiles = mob.size.x / Constants::TILE_SIZE;
    
    // Check corners
    int x1 = static_cast<int>(x);
    int y1 = static_cast<int>(y);
    int x2 = static_cast<int>(x + sizeInTiles * 0.9f);
    int y2 = static_cast<int>(y + sizeInTiles * 0.9f);
    
    // Check land tiles (walls)
    if (room->getLandTile(x1, y1) != 0 || room->getLandTile(x2, y1) != 0 ||
        room->getLandTile(x1, y2) != 0 || room->getLandTile(x2, y2) != 0) {
        return false;
    }
    
    return true;
}

void MobManager::clear() {
    mobs.clear();
}

bool MobManager::isRoomCleared() const {
    return mobs.empty();
}

}  // namespace CanalUx