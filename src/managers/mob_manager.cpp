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

MobManager::MobManager() {
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
    
    // Determine number of mobs based on level and room type
    int minMobs = 2 + levelNumber;
    int maxMobs = 4 + levelNumber;
    
    // Boss rooms get one tough boss instead
    if (room->getType() == RoomType::BOSS) {
        MobData boss;
        boss.position = Tyra::Vec2(room->getWidth() / 2.0f, room->getHeight() / 2.0f - 2.0f);
        boss.size = Tyra::Vec2(64.0f, 64.0f);  // Bigger than normal mobs
        boss.health = 10.0f + levelNumber * 5.0f;
        boss.speed = 0.02f;
        boss.active = true;
        boss.type = 1;  // Boss type
        mobs.push_back(boss);
        TYRA_LOG("MobManager: Spawned boss with ", boss.health, " health");
        return;
    }
    
    // Special rooms get fewer mobs
    if (room->getType() == RoomType::SPECIAL) {
        minMobs = 1;
        maxMobs = 2;
    }
    
    int numMobs = minMobs + (rand() % (maxMobs - minMobs + 1));
    
    float roomWidth = static_cast<float>(room->getWidth());
    float roomHeight = static_cast<float>(room->getHeight());
    
    // Spawn mobs away from the center (where player likely enters)
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
        
        mob.size = Tyra::Vec2(32.0f, 32.0f);
        mob.health = 2.0f + levelNumber * 0.5f;
        mob.speed = 0.015f + static_cast<float>(rand()) / RAND_MAX * 0.01f;
        mob.active = true;
        mob.type = 0;  // Normal mob
        
        mobs.push_back(mob);
    }
    
    TYRA_LOG("MobManager: Spawned ", numMobs, " mobs");
}

void MobManager::update(Room* currentRoom, Player* player, ProjectileManager* projectileManager) {
    if (!currentRoom || !player) return;
    
    for (auto& mob : mobs) {
        if (!mob.active) continue;
        
        // Simple chase AI - move towards player
        float dx = player->position.x - mob.position.x;
        float dy = player->position.y - mob.position.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance > 0.1f) {
            // Normalize and apply speed
            float vx = (dx / distance) * mob.speed;
            float vy = (dy / distance) * mob.speed;
            
            // Update position
            float newX = mob.position.x + vx;
            float newY = mob.position.y + vy;
            
            // Simple wall collision - check if new position is valid
            float sizeInTiles = mob.size.x / Constants::TILE_SIZE;
            
            // Check X movement
            bool canMoveX = true;
            int checkX = static_cast<int>(newX + (vx > 0 ? sizeInTiles : 0));
            if (currentRoom->getLandTile(checkX, static_cast<int>(mob.position.y)) != 0 ||
                currentRoom->getLandTile(checkX, static_cast<int>(mob.position.y + sizeInTiles * 0.9f)) != 0) {
                canMoveX = false;
            }
            
            // Check Y movement
            bool canMoveY = true;
            int checkY = static_cast<int>(newY + (vy > 0 ? sizeInTiles : 0));
            if (currentRoom->getLandTile(static_cast<int>(mob.position.x), checkY) != 0 ||
                currentRoom->getLandTile(static_cast<int>(mob.position.x + sizeInTiles * 0.9f), checkY) != 0) {
                canMoveY = false;
            }
            
            if (canMoveX) mob.position.x = newX;
            if (canMoveY) mob.position.y = newY;
        }
    }
    
    // Remove dead mobs
    mobs.erase(
        std::remove_if(mobs.begin(), mobs.end(),
            [](const MobData& m) { return !m.active || m.health <= 0; }),
        mobs.end()
    );
}

void MobManager::clear() {
    mobs.clear();
}

bool MobManager::isRoomCleared() const {
    return mobs.empty();
}

}  // namespace CanalUx