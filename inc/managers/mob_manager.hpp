/*
 * CanalUx - Mob Manager
 * Handles all enemy mobs in the current room
 */

#pragma once

#include <vector>
#include <tyra>
#include "core/constants.hpp"
#include "entities/entity.hpp"

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
    PIKE_EMERGING,      // Bursting up from water
    
    // Lock Keeper-specific states
    LOCKKEEPER_WALKING,     // Walking along top edge
    LOCKKEEPER_WINDUP,      // Raising arms for slam
    LOCKKEEPER_SLAM,        // Slamming down - ring expanding
    LOCKKEEPER_THROW_WINDUP,// Winding up to throw trolley
    LOCKKEEPER_THROWING,    // Throwing trolley
    LOCKKEEPER_STUNNED,     // Brief recovery after attack
    LOCKKEEPER_SHOT,        // Firing accelerating warning shots
    
    // Nanny-specific states
    NANNY_IDLE,             // Waiting at top of room
    NANNY_ATTACKING,        // Normal attack phase
    NANNY_GAUNTLET_START,   // Starting gauntlet - teleport player
    NANNY_GAUNTLET_ACTIVE,  // Gauntlet in progress - spawning barges
    NANNY_GAUNTLET_END,     // Player reached top, gauntlet complete
    NANNY_STUNNED           // Vulnerable after gauntlet
};

class MobManager {
public:
    MobManager();
    ~MobManager();

    // Spawning
    void spawnMobsForRoom(Room* room, int levelNumber);
    
    // Update all mobs (AI sets velocity, CollisionManager resolves collisions)
    void update(Room* currentRoom, Player* player, ProjectileManager* projectileManager);

    // Clear all mobs (e.g., on room change)
    void clear();
    
    // Check if room is cleared (all mobs dead)
    bool isRoomCleared() const;
    
    // Get mob count
    int getMobCount() const { return static_cast<int>(mobs.size()); }

    // MobData extends Entity with mob-specific data
    struct MobData : public Entity {
        float health;
        float maxHealth;
        float speed;
        MobType type;
        MobState state;
        
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
        
        // Lock Keeper boss specific
        float ringRadius;        // Current radius of shockwave ring
        float ringThickness;     // Width of the ring
        Tyra::Vec2 slamPosition; // Center of slam attack
        Tyra::Vec2 trolleyTarget;// Where trolley will land
        float trolleyProgress;   // 0-1 flight progress
        int trolleysThrown;      // Count of trolleys thrown this fight
        Tyra::Vec2 shotDirection;// Direction of warning shot
        Tyra::Vec2 shotPosition; // Current position of warning shot projectile
        float shotSpeed;         // Current speed of shot (accelerates)
        
        // Nanny boss specific
        int gauntletNumber;      // Which gauntlet (1 or 2)
        float bargeSpawnTimer;   // Timer for spawning barges
        float gauntletStartY;    // Y position player must reach to end gauntlet
        bool gauntlet1Complete;  // Tracks if first gauntlet done
        bool gauntlet2Complete;  // Tracks if second gauntlet done
        
        // For rendering
        bool facingRight;
        float rotation;          // For pike rotation during attacks
        
        // Constructor to initialize Entity base and mob-specific defaults
        MobData() : Entity(), health(0), maxHealth(0), speed(0), 
                    type(MobType::DUCK), state(MobState::IDLE),
                    stateTimer(0), actionCooldown(0),
                    circleAngle(0), chargeSpeed(0), attackPattern(0), phase(1),
                    tailSweepAngle(0), ringRadius(0), ringThickness(0.5f),
                    trolleyProgress(0), trolleysThrown(0), shotSpeed(0),
                    gauntletNumber(0), bargeSpawnTimer(0), gauntletStartY(0),
                    gauntlet1Complete(false), gauntlet2Complete(false),
                    facingRight(true), rotation(0) {}
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
    
    std::vector<MobData> mobs;
    float deltaTime;  // Approximate frame time
};

}  // namespace CanalUx