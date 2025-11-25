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
     * A giant pike fish that swims around the arena.
     * 
     * ATTACKS:
     * 1. Circling - Swims in circles around player, occasionally lunging
     * 2. Charge - Fast dash across the room
     * 3. Tail Sweep - Spins and sweeps tail in an arc, spawns projectiles
     * 4. Leap - Jumps out of water, crashes down creating splash damage
     * 5. Submerge - Hides underwater, only ripples visible, then emerges to attack
     * 
     * PHASES:
     * Phase 1 (100-60% HP): Circling + occasional Charge
     * Phase 2 (60-30% HP): Adds Tail Sweep, faster attacks
     * Phase 3 (<30% HP): Adds Leap attack, very aggressive
     */
    
    float dx = player->position.x - mob.position.x;
    float dy = player->position.y - mob.position.y;
    float distToPlayer = std::sqrt(dx * dx + dy * dy);
    
    float roomWidth = static_cast<float>(room->getWidth());
    float roomHeight = static_cast<float>(room->getHeight());
    
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
            // Circle around the player at a set distance
            float circleRadius = 4.0f;
            float circleSpeed = 0.02f + (mob.phase - 1) * 0.008f;  // Faster in later phases
            
            mob.circleAngle += circleSpeed;
            if (mob.circleAngle > 6.28f) mob.circleAngle -= 6.28f;
            
            // Target position on circle around player
            float targetX = player->position.x + std::cos(mob.circleAngle) * circleRadius;
            float targetY = player->position.y + std::sin(mob.circleAngle) * circleRadius;
            
            // Clamp to room bounds
            targetX = std::max(2.0f, std::min(targetX, roomWidth - 3.0f));
            targetY = std::max(2.0f, std::min(targetY, roomHeight - 3.0f));
            
            // Move towards target position
            float tdx = targetX - mob.position.x;
            float tdy = targetY - mob.position.y;
            float tdist = std::sqrt(tdx * tdx + tdy * tdy);
            
            if (tdist > 0.1f) {
                mob.position.x += (tdx / tdist) * mob.speed;
                mob.position.y += (tdy / tdist) * mob.speed;
            }
            
            // Update rotation to face movement direction
            mob.rotation = std::atan2(tdy, tdx);
            
            // Decide next attack when cooldown is ready
            if (mob.actionCooldown <= 0) {
                int attackRoll = rand() % 100;
                
                if (mob.phase >= 3 && attackRoll < 20) {
                    // Phase 3: Leap attack
                    mob.state = MobState::PIKE_LEAP;
                    mob.stateTimer = 0;
                    mob.chargeTarget = player->position;
                } else if (mob.phase >= 2 && attackRoll < 45) {
                    // Phase 2+: Tail sweep
                    mob.state = MobState::PIKE_TAIL_SWEEP;
                    mob.stateTimer = 0;
                    mob.tailSweepAngle = std::atan2(dy, dx);
                } else if (attackRoll < 70) {
                    // Charge attack
                    mob.state = MobState::PIKE_CHARGING;
                    mob.stateTimer = 0;
                    mob.chargeTarget = player->position;
                    mob.chargeSpeed = 0.12f + mob.phase * 0.03f;
                } else {
                    // Submerge and ambush
                    mob.state = MobState::PIKE_SUBMERGED;
                    mob.stateTimer = 0;
                    mob.submerged = true;
                }
                
                mob.actionCooldown = 90 - mob.phase * 15;  // Faster attacks in later phases
            }
            break;
        }
        
        case MobState::PIKE_CHARGING: {
            // Fast charge towards target position
            float cdx = mob.chargeTarget.x - mob.position.x;
            float cdy = mob.chargeTarget.y - mob.position.y;
            float cdist = std::sqrt(cdx * cdx + cdy * cdy);
            
            mob.rotation = std::atan2(cdy, cdx);
            
            if (cdist > 0.5f && mob.stateTimer < 60) {
                // Still charging
                mob.position.x += (cdx / cdist) * mob.chargeSpeed;
                mob.position.y += (cdy / cdist) * mob.chargeSpeed;
                
                // Clamp to room
                mob.position.x = std::max(1.0f, std::min(mob.position.x, roomWidth - 2.0f));
                mob.position.y = std::max(1.0f, std::min(mob.position.y, roomHeight - 2.0f));
            } else {
                // Charge complete, brief pause then return to circling
                if (mob.stateTimer > 80) {
                    mob.state = MobState::PIKE_CIRCLING;
                    mob.stateTimer = 0;
                }
            }
            break;
        }
        
        case MobState::PIKE_TAIL_SWEEP: {
            // Spin and create arc of projectiles
            if (mob.stateTimer < 20) {
                // Wind up - slight movement back
                mob.rotation += 0.1f;
            } else if (mob.stateTimer == 20) {
                // Release projectiles in an arc
                int numProjectiles = 5 + mob.phase;
                float arcSpread = 1.2f;  // About 70 degrees each side
                
                for (int i = 0; i < numProjectiles; i++) {
                    float angle = mob.tailSweepAngle - arcSpread + (arcSpread * 2.0f * i / (numProjectiles - 1));
                    
                    Tyra::Vec2 projPos = mob.position;
                    projPos.x += 1.0f;  // Offset from center
                    projPos.y += 0.5f;
                    
                    Tyra::Vec2 projVel;
                    projVel.x = std::cos(angle) * 0.05f;
                    projVel.y = std::sin(angle) * 0.05f;
                    
                    projectileManager->spawnEnemyProjectile(projPos, projVel, 1.0f);
                }
            } else if (mob.stateTimer > 50) {
                // Return to circling
                mob.state = MobState::PIKE_CIRCLING;
                mob.stateTimer = 0;
            }
            break;
        }
        
        case MobState::PIKE_LEAP: {
            // Jump out of water and crash down
            if (mob.stateTimer < 30) {
                // Rising up (could add visual scaling here)
                mob.submerged = false;
            } else if (mob.stateTimer < 60) {
                // In the air - move towards player
                float ldx = mob.chargeTarget.x - mob.position.x;
                float ldy = mob.chargeTarget.y - mob.position.y;
                float ldist = std::sqrt(ldx * ldx + ldy * ldy);
                
                if (ldist > 0.1f) {
                    mob.position.x += (ldx / ldist) * 0.08f;
                    mob.position.y += (ldy / ldist) * 0.08f;
                }
            } else if (mob.stateTimer == 60) {
                // Crash down - spawn splash projectiles in all directions
                int numSplash = 8;
                for (int i = 0; i < numSplash; i++) {
                    float angle = (6.28f / numSplash) * i;
                    
                    Tyra::Vec2 projPos = mob.position;
                    projPos.x += 1.0f;
                    projPos.y += 0.5f;
                    
                    Tyra::Vec2 projVel;
                    projVel.x = std::cos(angle) * 0.04f;
                    projVel.y = std::sin(angle) * 0.04f;
                    
                    projectileManager->spawnEnemyProjectile(projPos, projVel, 1.0f);
                }
            } else if (mob.stateTimer > 90) {
                // Recovery complete
                mob.state = MobState::PIKE_CIRCLING;
                mob.stateTimer = 0;
            }
            break;
        }
        
        case MobState::PIKE_SUBMERGED: {
            // Hidden underwater - slowly move towards player
            mob.submerged = true;
            
            // Slow drift towards player
            if (distToPlayer > 2.0f) {
                mob.position.x += (dx / distToPlayer) * 0.015f;
                mob.position.y += (dy / distToPlayer) * 0.015f;
            }
            
            // After some time, burst out
            if (mob.stateTimer > 90 + rand() % 30) {
                mob.state = MobState::PIKE_EMERGING;
                mob.stateTimer = 0;
            }
            break;
        }
        
        case MobState::PIKE_EMERGING: {
            // Burst out of water - quick attack
            mob.submerged = false;
            
            if (mob.stateTimer < 5) {
                // Brief pause
            } else if (mob.stateTimer < 25) {
                // Lunge at player
                if (distToPlayer > 0.5f) {
                    mob.position.x += (dx / distToPlayer) * 0.1f;
                    mob.position.y += (dy / distToPlayer) * 0.1f;
                }
                mob.rotation = std::atan2(dy, dx);
            } else if (mob.stateTimer > 50) {
                // Return to circling
                mob.state = MobState::PIKE_CIRCLING;
                mob.stateTimer = 0;
            }
            break;
        }
        
        default:
            mob.state = MobState::PIKE_CIRCLING;
            break;
    }
}

void MobManager::updateLockKeeperBoss(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager) {
    // TODO: Level 2 boss - Lock Keeper
    // For now, use generic boss behavior
    updateBoss(mob, room, player, projectileManager);
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