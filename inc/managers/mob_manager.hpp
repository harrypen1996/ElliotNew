/*
 * CanalUx - Mob Manager
 * Handles all enemy mobs in the current room
 */

#pragma once

#include <vector>
#include <tyra>
#include "core/constants.hpp"

namespace CanalUx {

class Room;
class Player;
class ProjectileManager;
class Mob;

class MobManager {
public:
    MobManager();
    ~MobManager();

    // Spawning
    void spawnMobsForRoom(Room* room, int levelNumber);
    
    // Update all mobs
    void update(Room* currentRoom, Player* player, ProjectileManager* projectileManager);

    // Clear all mobs (e.g., on room change)
    void clear();
    
    // Check if room is cleared (all mobs dead)
    bool isRoomCleared() const;
    
    // Get mob count
    int getMobCount() const { return static_cast<int>(mobs.size()); }

    // Access for collision checking and rendering
    struct MobData {
        Tyra::Vec2 position;
        Tyra::Vec2 size;
        float health;
        float speed;
        int type;      // 0 = normal, 1 = boss, etc.
        bool active;
    };
    
    const std::vector<MobData>& getMobs() const { return mobs; }
    std::vector<MobData>& getMobs() { return mobs; }

private:
    std::vector<MobData> mobs;
};

}  // namespace CanalUx