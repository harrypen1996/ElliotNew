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

// Mob types with unique behaviors
enum class MobType {
    DUCK = 0,    // Dumb chaser - just runs at player
    SWAN = 1,    // Shooter - keeps distance and shoots feathers
    FROG = 2,    // Jumper - hops around, submerges between jumps
    FISH = 3,    // Fast swimmer - quick but low health (future)
    BOSS = 10    // Boss version (bigger, more health)
};

// Mob behavior states
enum class MobState {
    IDLE,
    CHASING,
    ATTACKING,
    JUMPING,
    SUBMERGED,
    SURFACING
};

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
        Tyra::Vec2 velocity;
        Tyra::Vec2 size;
        float health;
        float maxHealth;
        float speed;
        MobType type;
        MobState state;
        bool active;
        
        // Behavior timers
        float stateTimer;      // Time in current state
        float actionCooldown;  // Cooldown for attacks/jumps
        
        // Frog specific
        Tyra::Vec2 jumpTarget;
        
        // For rendering
        bool facingRight;
        bool submerged;
    };
    
    const std::vector<MobData>& getMobs() const { return mobs; }
    std::vector<MobData>& getMobs() { return mobs; }

private:
    void updateDuck(MobData& mob, Room* room, Player* player);
    void updateSwan(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager);
    void updateFrog(MobData& mob, Room* room, Player* player);
    void updateBoss(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager);
    
    bool canMoveTo(MobData& mob, float x, float y, Room* room);
    
    std::vector<MobData> mobs;
    float deltaTime;  // Approximate frame time
};

}  // namespace CanalUx