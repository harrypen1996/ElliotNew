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
    BOSS = 10,   // Generic boss (legacy)
    
    // Specific bosses per level
    BOSS_PIKE = 11,        // Level 1 - Giant pike fish
    BOSS_LOCKKEEPER = 12,  // Level 2 - The Lock Keeper
    BOSS_NANNY = 13        // Level 3 - Nanny (grandma)
};

// Mob behavior states
enum class MobState {
    IDLE,
    CHASING,
    ATTACKING,
    JUMPING,
    SUBMERGED,
    SURFACING,
    
    // Pike-specific states
    PIKE_CIRCLING,      // Swimming in circles around player
    PIKE_CHARGING,      // Fast charge attack
    PIKE_TAIL_SWEEP,    // Tail cleave attack
    PIKE_LEAP,          // Leaps out of water, crashes down
    PIKE_SUBMERGED,     // Hidden underwater, ripples visible
    PIKE_EMERGING       // Bursting up from water
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
        
        // Pike boss specific
        float circleAngle;       // Current angle when circling player
        float chargeSpeed;       // Speed during charge attack
        Tyra::Vec2 chargeTarget; // Where pike is charging to
        int attackPattern;       // Which attack in the pattern
        int phase;               // Boss phase (changes behavior at health thresholds)
        float tailSweepAngle;    // Angle for tail sweep attack
        
        // For rendering
        bool facingRight;
        bool submerged;
        float rotation;          // For pike rotation during attacks
    };
    
    const std::vector<MobData>& getMobs() const { return mobs; }
    std::vector<MobData>& getMobs() { return mobs; }

private:
    void updateDuck(MobData& mob, Room* room, Player* player);
    void updateSwan(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager);
    void updateFrog(MobData& mob, Room* room, Player* player);
    void updateBoss(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager);
    
    // Boss-specific updates
    void updatePikeBoss(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager);
    void updateLockKeeperBoss(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager);
    void updateNannyBoss(MobData& mob, Room* room, Player* player, ProjectileManager* projectileManager);
    
    void applyMobRepulsion();
    bool canMoveTo(MobData& mob, float x, float y, Room* room);
    
    std::vector<MobData> mobs;
    float deltaTime;  // Approximate frame time
};

}  // namespace CanalUx