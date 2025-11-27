/*
 * CanalUx - Base Entity Class Implementation
 */

#include "entities/entity.hpp"
#include <cmath>

namespace CanalUx {

Entity::Entity()
    : position(0.0f, 0.0f),
      velocity(0.0f, 0.0f),
      size(Constants::TILE_SIZE, Constants::TILE_SIZE),
      active(true),
      submerged(false) {
}

Entity::Entity(Tyra::Vec2 pos, Tyra::Vec2 sz)
    : position(pos),
      velocity(0.0f, 0.0f),
      size(sz),
      active(true),
      submerged(false) {
}

Entity::~Entity() {
}

void Entity::applyVelocity() {
    position.x += velocity.x;
    position.y += velocity.y;
}

void Entity::applyDrag(float dragCoefficient) {
    velocity.x += -dragCoefficient * velocity.x;
    if (std::fabs(velocity.x) < Constants::VELOCITY_THRESHOLD) {
        velocity.x = 0.0f;
    }

    velocity.y += -dragCoefficient * velocity.y;
    if (std::fabs(velocity.y) < Constants::VELOCITY_THRESHOLD) {
        velocity.y = 0.0f;
    }
}

void Entity::clampVelocity(float maxVel) {
    if (velocity.x > maxVel) velocity.x = maxVel;
    if (velocity.x < -maxVel) velocity.x = -maxVel;
    if (velocity.y > maxVel) velocity.y = maxVel;
    if (velocity.y < -maxVel) velocity.y = -maxVel;
}

Tyra::Vec2 Entity::getCenter() const {
    return Tyra::Vec2(
        position.x + (size.x / Constants::TILE_SIZE) / 2.0f,
        position.y + (size.y / Constants::TILE_SIZE) / 2.0f
    );
}

}  // namespace CanalUx