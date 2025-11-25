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

    // Store old position for collision resolution
    float oldX = position.x;
    float oldY = position.y;

    // Apply velocity
    position.x += velocity.x;
    position.y += velocity.y;

    // Check and resolve collisions
    checkRoomCollision(currentRoom);
}

void Player::handleMovementInput() {
    const auto& leftJoy = pad->getLeftJoyPad();
    float speed = stats.getSpeed();

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
    
    float projectileVelX = 0.0f;
    float projectileVelY = 0.0f;
    bool shouldShoot = false;

    // Determine shooting direction from right stick
    if (rightJoy.v <= 100) {
        projectileVelY = -0.1f;
        shouldShoot = true;
    } else if (rightJoy.v >= 200) {
        projectileVelY = 0.1f;
        shouldShoot = true;
    }

    if (rightJoy.h <= 100) {
        projectileVelX = -0.1f;
        shouldShoot = true;
    } else if (rightJoy.h >= 200) {
        projectileVelX = 0.1f;
        shouldShoot = true;
    }

    // Fire if stick is pushed and cooldown has elapsed
    if (shouldShoot && shootTimer.getTimeDelta() > stats.getFireRate()) {
        projectileManager->spawnPlayerProjectile(
            position,
            Tyra::Vec2(projectileVelX * stats.getShotSpeed(), 
                       projectileVelY * stats.getShotSpeed()),
            stats.getDamage()
        );
        shootTimer.prime();
    }
}

void Player::handleSubmergeInput() {
    if (pad->getPressed().Cross) {
        trySubmerge();
    } else if (submerged && !pad->getPressed().Cross) {
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
    if (isInvincible() || submerged) {
        return;  // Can't take damage while invincible or underwater
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

void Player::checkRoomCollision(Room* currentRoom) {
    if (!currentRoom) return;

    float sizeInTilesX = size.x / Constants::TILE_SIZE;
    float sizeInTilesY = size.y / Constants::TILE_SIZE;

    // Horizontal collision
    if (velocity.x <= 0) {
        // Moving left
        if (currentRoom->getLandTile(static_cast<int>(position.x), 
                                      static_cast<int>(position.y)) != 0 ||
            currentRoom->getLandTile(static_cast<int>(position.x), 
                                      static_cast<int>(position.y + sizeInTilesY * 0.9f)) != 0) {
            position.x = static_cast<int>(position.x) + 1;
            velocity.x = 0;
        } else if (!submerged && 
                   (currentRoom->getSceneryTile(static_cast<int>(position.x), 
                                                 static_cast<int>(position.y)) != 0 ||
                    currentRoom->getSceneryTile(static_cast<int>(position.x), 
                                                 static_cast<int>(position.y + sizeInTilesY * 0.9f)) != 0)) {
            position.x = static_cast<int>(position.x) + 1;
            velocity.x = 0;
        }
    } else {
        // Moving right
        if (currentRoom->getLandTile(static_cast<int>(position.x + sizeInTilesX), 
                                      static_cast<int>(position.y)) != 0 ||
            currentRoom->getLandTile(static_cast<int>(position.x + sizeInTilesX), 
                                      static_cast<int>(position.y + sizeInTilesY * 0.9f)) != 0) {
            position.x = static_cast<int>(position.x);
            velocity.x = 0;
        } else if (!submerged && 
                   (currentRoom->getSceneryTile(static_cast<int>(position.x + sizeInTilesX), 
                                                 static_cast<int>(position.y)) != 0 ||
                    currentRoom->getSceneryTile(static_cast<int>(position.x + sizeInTilesX), 
                                                 static_cast<int>(position.y + sizeInTilesY * 0.9f)) != 0)) {
            position.x = static_cast<int>(position.x);
            velocity.x = 0;
        }
    }

    // Vertical collision
    if (velocity.y <= 0) {
        // Moving up
        if (currentRoom->getLandTile(static_cast<int>(position.x), 
                                      static_cast<int>(position.y)) != 0 ||
            currentRoom->getLandTile(static_cast<int>(position.x + sizeInTilesX * 0.9f), 
                                      static_cast<int>(position.y)) != 0) {
            position.y = static_cast<int>(position.y) + 1;
            velocity.y = 0;
        } else if (!submerged && 
                   (currentRoom->getSceneryTile(static_cast<int>(position.x), 
                                                 static_cast<int>(position.y)) != 0 ||
                    currentRoom->getSceneryTile(static_cast<int>(position.x + sizeInTilesX * 0.9f), 
                                                 static_cast<int>(position.y)) != 0)) {
            position.y = static_cast<int>(position.y) + 1;
            velocity.y = 0;
        }
    } else {
        // Moving down
        if (currentRoom->getLandTile(static_cast<int>(position.x), 
                                      static_cast<int>(position.y + sizeInTilesY)) != 0 ||
            currentRoom->getLandTile(static_cast<int>(position.x + sizeInTilesX * 0.9f), 
                                      static_cast<int>(position.y + sizeInTilesY)) != 0) {
            position.y = static_cast<int>(position.y);
            velocity.y = 0;
        } else if (!submerged && 
                   (currentRoom->getSceneryTile(static_cast<int>(position.x), 
                                                 static_cast<int>(position.y + sizeInTilesY)) != 0 ||
                    currentRoom->getSceneryTile(static_cast<int>(position.x + sizeInTilesX * 0.9f), 
                                                 static_cast<int>(position.y + sizeInTilesY)) != 0)) {
            position.y = static_cast<int>(position.y);
            velocity.y = 0;
        }
    }
}

}  // namespace CanalUx