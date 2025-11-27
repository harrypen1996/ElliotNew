/*
 * CanalUx - Projectile Class Implementation
 */

#include "entities/projectile.hpp"
#include "world/room.hpp"
#include <cmath>

namespace CanalUx {

Projectile::Projectile()
    : Entity(Tyra::Vec2(0.0f, 0.0f), Tyra::Vec2(Constants::PROJECTILE_SIZE, Constants::PROJECTILE_SIZE)),
      fromPlayer(true),
      damage(1.0f),
      distanceTraveled(0.0f),
      maxRange(10.0f),
      acceleration(0.0f),
      maxSpeed(1.0f) {
}

Projectile::Projectile(Tyra::Vec2 pos, Tyra::Vec2 vel, float dmg, bool playerOwned)
    : Entity(pos, Tyra::Vec2(Constants::PROJECTILE_SIZE, Constants::PROJECTILE_SIZE)),
      fromPlayer(playerOwned),
      damage(dmg),
      distanceTraveled(0.0f),
      maxRange(10.0f),
      acceleration(0.0f),
      maxSpeed(1.0f) {
    velocity = vel;
}

Projectile::~Projectile() {
}

void Projectile::update(float deltaTime) {
    if (!active) return;

    // Apply acceleration if set
    if (acceleration > 0.0f) {
        float currentSpeed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (currentSpeed > 0.0f && currentSpeed < maxSpeed) {
            float newSpeed = currentSpeed + acceleration;
            if (newSpeed > maxSpeed) newSpeed = maxSpeed;
            float scale = newSpeed / currentSpeed;
            velocity.x *= scale;
            velocity.y *= scale;
        }
    }

    // Track distance traveled
    float dx = velocity.x;
    float dy = velocity.y;
    distanceTraveled += std::sqrt(dx * dx + dy * dy);

    // Check if exceeded max range
    if (distanceTraveled >= maxRange) {
        destroy();
        return;
    }

    // Move projectile
    position.x += velocity.x;
    position.y += velocity.y;
}

void Projectile::update(Room* currentRoom) {
    if (!active) return;

    // Apply acceleration if set
    if (acceleration > 0.0f) {
        float currentSpeed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (currentSpeed > 0.0f && currentSpeed < maxSpeed) {
            float newSpeed = currentSpeed + acceleration;
            if (newSpeed > maxSpeed) newSpeed = maxSpeed;
            float scale = newSpeed / currentSpeed;
            velocity.x *= scale;
            velocity.y *= scale;
        }
    }

    // Track distance traveled
    float dx = velocity.x;
    float dy = velocity.y;
    distanceTraveled += std::sqrt(dx * dx + dy * dy);

    // Check if exceeded max range
    if (distanceTraveled >= maxRange) {
        destroy();
        return;
    }

    // Store old position
    float oldX = position.x;
    float oldY = position.y;

    // Move projectile
    position.x += velocity.x;
    position.y += velocity.y;

    // Check collision with walls
    checkRoomCollision(currentRoom);
}

void Projectile::checkRoomCollision(Room* currentRoom) {
    if (!currentRoom || !active) return;

    float sizeInTiles = size.x / Constants::TILE_SIZE;

    // Check if projectile hit a solid tile
    int tileX = static_cast<int>(position.x);
    int tileY = static_cast<int>(position.y);

    // Check land tiles (walls)
    if (currentRoom->getLandTile(tileX, tileY) != 0 ||
        currentRoom->getLandTile(static_cast<int>(position.x + sizeInTiles * 0.9f), tileY) != 0 ||
        currentRoom->getLandTile(tileX, static_cast<int>(position.y + sizeInTiles * 0.9f)) != 0) {
        destroy();
        return;
    }

    // Check scenery tiles (obstacles like doors) - projectiles collide with these
    if (currentRoom->getSceneryTile(tileX, tileY) != 0 ||
        currentRoom->getSceneryTile(static_cast<int>(position.x + sizeInTiles * 0.9f), tileY) != 0 ||
        currentRoom->getSceneryTile(tileX, static_cast<int>(position.y + sizeInTiles * 0.9f)) != 0) {
        destroy();
        return;
    }
    
    // Note: Room obstacles (trolleys etc) checked separately - some may allow projectiles through
}

}  // namespace CanalUx