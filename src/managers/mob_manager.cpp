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
    
    // Boss rooms get one tough boss
    if (room->getType() == RoomType::BOSS) {
        MobData boss;
        boss.position = Tyra::Vec2(roomWidth / 2.0f, roomHeight / 2.0f - 2.0f);
        boss.velocity = Tyra::Vec2(0, 0);
        boss.size = Tyra::Vec2(64.0f, 64.0f);
        boss.health = 15.0f + levelNumber * 5.0f;
        boss.maxHealth = boss.health;
        boss.speed = 0.025f;
        boss.active = true;
        boss.type = MobType::BOSS;
        boss.state = MobState::IDLE;
        boss.stateTimer = 0;
        boss.actionCooldown = 0;
        boss.facingRight = true;
        boss.submerged = false;
        mobs.push_back(boss);
        TYRA_LOG("MobManager: Spawned boss with ", boss.health, " health");
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
            default:
                updateDuck(mob, currentRoom, player);
                break;
        }
    }
    
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