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

    // Apply velocity and check collisions
    // (checkRoomCollision handles position updates)
    position.x += velocity.x;
    position.y += velocity.y;
    checkRoomCollision(currentRoom);
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

    // Apply velocity and check collisions
    position.x += velocity.x;
    position.y += velocity.y;
    checkRoomCollision(currentRoom);
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
    if (pad->getPressed().Circle) {
        trySubmerge();
    } else if (submerged && !pad->getPressed().Circle) {
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

    // We need to check collision separately for X and Y movement
    // First, undo the position change
    position.x -= velocity.x;
    position.y -= velocity.y;

    // Store the original position
    float origX = position.x;
    float origY = position.y;

    // --- Check X movement first ---
    position.x += velocity.x;
    
    bool collidedX = false;
    
    if (velocity.x < 0) {
        // Moving left - check left edge
        int checkX = static_cast<int>(position.x);
        int checkY1 = static_cast<int>(origY);
        int checkY2 = static_cast<int>(origY + sizeInTilesY * 0.9f);
        
        if (currentRoom->getLandTile(checkX, checkY1) != 0 ||
            currentRoom->getLandTile(checkX, checkY2) != 0) {
            collidedX = true;
        } else if (!submerged && 
                   (currentRoom->getSceneryTile(checkX, checkY1) != 0 ||
                    currentRoom->getSceneryTile(checkX, checkY2) != 0)) {
            collidedX = true;
        }
        
        // Check dynamic obstacles
        if (!collidedX) {
            for (const auto& obs : currentRoom->getObstacles()) {
                if (obs.blocksPlayer) {
                    // Check if new position overlaps obstacle
                    if (position.x < obs.position.x + 1.0f &&
                        position.x + sizeInTilesX > obs.position.x &&
                        origY < obs.position.y + 1.0f &&
                        origY + sizeInTilesY > obs.position.y) {
                        collidedX = true;
                        break;
                    }
                }
            }
        }
        
        if (collidedX) {
            position.x = static_cast<int>(position.x) + 1.0f;
            velocity.x = 0;
        }
    } else if (velocity.x > 0) {
        // Moving right - check right edge
        int checkX = static_cast<int>(position.x + sizeInTilesX);
        int checkY1 = static_cast<int>(origY);
        int checkY2 = static_cast<int>(origY + sizeInTilesY * 0.9f);
        
        if (currentRoom->getLandTile(checkX, checkY1) != 0 ||
            currentRoom->getLandTile(checkX, checkY2) != 0) {
            collidedX = true;
        } else if (!submerged && 
                   (currentRoom->getSceneryTile(checkX, checkY1) != 0 ||
                    currentRoom->getSceneryTile(checkX, checkY2) != 0)) {
            collidedX = true;
        }
        
        // Check dynamic obstacles
        if (!collidedX) {
            for (const auto& obs : currentRoom->getObstacles()) {
                if (obs.blocksPlayer) {
                    if (position.x + sizeInTilesX > obs.position.x &&
                        position.x < obs.position.x + 1.0f &&
                        origY < obs.position.y + 1.0f &&
                        origY + sizeInTilesY > obs.position.y) {
                        collidedX = true;
                        break;
                    }
                }
            }
        }
        
        if (collidedX) {
            position.x = static_cast<int>(position.x + sizeInTilesX) - sizeInTilesX;
            velocity.x = 0;
        }
    }

    // --- Now check Y movement with the resolved X position ---
    position.y += velocity.y;
    
    bool collidedY = false;
    
    if (velocity.y < 0) {
        // Moving up - check top edge
        int checkY = static_cast<int>(position.y);
        int checkX1 = static_cast<int>(position.x);
        int checkX2 = static_cast<int>(position.x + sizeInTilesX * 0.9f);
        
        if (currentRoom->getLandTile(checkX1, checkY) != 0 ||
            currentRoom->getLandTile(checkX2, checkY) != 0) {
            collidedY = true;
        } else if (!submerged && 
                   (currentRoom->getSceneryTile(checkX1, checkY) != 0 ||
                    currentRoom->getSceneryTile(checkX2, checkY) != 0)) {
            collidedY = true;
        }
        
        // Check dynamic obstacles
        if (!collidedY) {
            for (const auto& obs : currentRoom->getObstacles()) {
                if (obs.blocksPlayer) {
                    if (position.x < obs.position.x + 1.0f &&
                        position.x + sizeInTilesX > obs.position.x &&
                        position.y < obs.position.y + 1.0f &&
                        position.y + sizeInTilesY > obs.position.y) {
                        collidedY = true;
                        break;
                    }
                }
            }
        }
        
        if (collidedY) {
            position.y = static_cast<int>(position.y) + 1.0f;
            velocity.y = 0;
        }
    } else if (velocity.y > 0) {
        // Moving down - check bottom edge
        int checkY = static_cast<int>(position.y + sizeInTilesY);
        int checkX1 = static_cast<int>(position.x);
        int checkX2 = static_cast<int>(position.x + sizeInTilesX * 0.9f);
        
        if (currentRoom->getLandTile(checkX1, checkY) != 0 ||
            currentRoom->getLandTile(checkX2, checkY) != 0) {
            collidedY = true;
        } else if (!submerged && 
                   (currentRoom->getSceneryTile(checkX1, checkY) != 0 ||
                    currentRoom->getSceneryTile(checkX2, checkY) != 0)) {
            collidedY = true;
        }
        
        // Check dynamic obstacles
        if (!collidedY) {
            for (const auto& obs : currentRoom->getObstacles()) {
                if (obs.blocksPlayer) {
                    if (position.x < obs.position.x + 1.0f &&
                        position.x + sizeInTilesX > obs.position.x &&
                        position.y + sizeInTilesY > obs.position.y &&
                        position.y < obs.position.y + 1.0f) {
                        collidedY = true;
                        break;
                    }
                }
            }
        }
        
        if (collidedY) {
            position.y = static_cast<int>(position.y + sizeInTilesY) - sizeInTilesY;
            velocity.y = 0;
        }
    }
}

}  // namespace CanalUx