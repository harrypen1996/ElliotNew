/*
 * CanalUx - Player Class Implementation
 */

#include "entities/player.hpp"
#include "world/room.hpp"
#include "managers/projectile_manager.hpp"
#include <cmath>

namespace CanalUx {

Player::Player(Tyra::Pad* t_pad)
    : Entity(Tyra::Vec2(0.0f, 0.0f), Tyra::Vec2(Constants::PLAYER_SIZE, Constants::PLAYER_SIZE)),
      pad(t_pad),
      stats(),
      submergeTimeRemaining(0.0f),
      submergeCooldownRemaining(0.0f),
      invincibilityTimeRemaining(0.0f),
      facing(Direction::DOWN),
      shooting(false),
      baseSpeed(Constants::PLAYER_SPEED),
      shootCooldown(Constants::PLAYER_SHOOT_COOLDOWN) {
    shootTimer.prime();
}

Player::~Player() {
}

void Player::update(float deltaTime) {
    // Basic update without room/projectile context
    // Typically you'd call the full update() with dependencies
    updateSubmergeState(deltaTime);
    updateInvincibility(deltaTime);
    applyDrag();
}

void Player::update(Room* currentRoom, ProjectileManager* projectileManager) {
    // Handle all input
    handleMovementInput();
    handleSubmergeInput();
    handleShootingInput(projectileManager);

    // Update timers (using a rough delta - Tyra's timer gives us milliseconds)
    float deltaTime = 16.67f;  // Approximate 60fps frame time
    updateSubmergeState(deltaTime);
    updateInvincibility(deltaTime);

    // Apply drag to velocity
    applyDrag();

    // Clamp velocity
    clampVelocity();

    // Apply velocity - collision manager will resolve world collisions
    position.x += velocity.x;
    position.y += velocity.y;
    // Note: World collision resolution handled by CollisionManager::resolvePlayerWorldCollision()
}

void Player::update(Room* currentRoom) {
    // Simplified update without shooting
    handleMovementInput();
    handleSubmergeInput();

    // Update timers
    float deltaTime = 16.67f;
    updateSubmergeState(deltaTime);
    updateInvincibility(deltaTime);

    // Apply drag and clamp
    applyDrag();
    clampVelocity();

    // Apply velocity - collision manager will resolve world collisions
    position.x += velocity.x;
    position.y += velocity.y;
    // Note: World collision resolution handled by CollisionManager::resolvePlayerWorldCollision()
}

void Player::handleMovementInput() {
    const auto& leftJoy = pad->getLeftJoyPad();
    float speed = stats.getSpeed() * Constants::Cheats::SPEED_MULTIPLIER;

    // Vertical movement
    if (leftJoy.v <= 100) {
        velocity.y = -0.1f * speed;
        facing = Direction::UP;
    } else if (leftJoy.v >= 200) {
        velocity.y = 0.1f * speed;
        facing = Direction::DOWN;
    }

    // Horizontal movement
    if (leftJoy.h <= 100) {
        velocity.x = -0.1f * speed;
        facing = Direction::LEFT;
    } else if (leftJoy.h >= 200) {
        velocity.x = 0.1f * speed;
        facing = Direction::RIGHT;
    }
}

void Player::handleShootingInput(ProjectileManager* projectileManager) {
    if (!projectileManager) return;

    const auto& rightJoy = pad->getRightJoyPad();
    
    // Convert stick values (0-255) to -1.0 to 1.0 range
    // Center is 128, so subtract and divide
    float stickX = (static_cast<float>(rightJoy.h) - 128.0f) / 128.0f;
    float stickY = (static_cast<float>(rightJoy.v) - 128.0f) / 128.0f;
    
    // Calculate magnitude of stick deflection
    float magnitude = std::sqrt(stickX * stickX + stickY * stickY);
    
    // Deadzone threshold - only shoot if stick is pushed far enough
    const float deadzone = 0.4f;
    
    if (magnitude > deadzone && shootTimer.getTimeDelta() > stats.getFireRate()) {
        // Normalize the direction
        float dirX = stickX / magnitude;
        float dirY = stickY / magnitude;
        
        // Apply shot speed
        float speed = 0.1f * stats.getShotSpeed();
        
        projectileManager->spawnPlayerProjectile(
            position,
            Tyra::Vec2(dirX * speed, dirY * speed),
            stats.getDamage()
        );
        shootTimer.prime();
    }
}

void Player::handleSubmergeInput() {
    // R2 trigger to submerge
    if (pad->getPressed().R2) {
        trySubmerge();
    } else if (submerged && !pad->getPressed().R2) {
        // Released button - start surfacing
        // For now, instant surface. Could add a delay.
        submerged = false;
    }
}

void Player::trySubmerge() {
    if (canSubmerge()) {
        submerged = true;
        submergeTimeRemaining = Constants::PLAYER_SUBMERGE_DURATION;
    }
}

bool Player::canSubmerge() const {
    return !submerged && submergeCooldownRemaining <= 0.0f;
}

float Player::getSubmergeTimeRemaining() const {
    return submergeTimeRemaining;
}

float Player::getSubmergeCooldownRemaining() const {
    return submergeCooldownRemaining;
}

void Player::updateSubmergeState(float deltaTime) {
    if (submerged) {
        submergeTimeRemaining -= deltaTime;
        if (submergeTimeRemaining <= 0.0f) {
            // Forced to surface
            submerged = false;
            submergeCooldownRemaining = Constants::PLAYER_SUBMERGE_COOLDOWN;
        }
    } else if (submergeCooldownRemaining > 0.0f) {
        submergeCooldownRemaining -= deltaTime;
    }
}

void Player::updateInvincibility(float deltaTime) {
    if (invincibilityTimeRemaining > 0.0f) {
        invincibilityTimeRemaining -= deltaTime;
    }
}

void Player::takeDamage(int amount) {
    // Cheat: God mode
    if (Constants::Cheats::GOD_MODE) {
        return;
    }
    
    // Can't take damage while invincible
    // Note: Submerge protection is handled by CollisionManager
    // - Mob collisions are blocked when submerged
    // - Normal projectiles are blocked when submerged
    // - Projectiles with hitsSubmerged flag (barges) CAN damage submerged players
    if (isInvincible()) {
        return;
    }

    stats.damage(amount);
    invincibilityTimeRemaining = 1000.0f;  // 1 second of invincibility
}

void Player::heal(int amount) {
    stats.heal(amount);
}

bool Player::isInvincible() const {
    return invincibilityTimeRemaining > 0.0f;
}

// Note: World collision is now handled by CollisionManager::resolvePlayerWorldCollision()
// The old checkRoomCollision method has been removed.

}  // namespace CanalUx