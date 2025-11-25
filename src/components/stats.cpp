/*
 * CanalUx - Stats Component Implementation
 */

#include "components/stats.hpp"
#include <algorithm>

namespace CanalUx {

Stats::Stats()
    : health(Constants::PLAYER_MAX_HEALTH),
      maxHealth(Constants::PLAYER_MAX_HEALTH),
      baseDamage(1.0f),
      baseSpeed(Constants::PLAYER_SPEED),
      baseShotSpeed(Constants::PROJECTILE_SPEED),
      baseFireRate(Constants::PLAYER_SHOOT_COOLDOWN),
      baseRange(10.0f),
      damageMultiplier(1.0f),
      speedMultiplier(1.0f),
      shotSpeedMultiplier(1.0f),
      fireRateMultiplier(1.0f),
      rangeMultiplier(1.0f) {
}

Stats::Stats(int maxHp, float dmg, float spd)
    : health(maxHp),
      maxHealth(maxHp),
      baseDamage(dmg),
      baseSpeed(spd),
      baseShotSpeed(Constants::PROJECTILE_SPEED),
      baseFireRate(Constants::PLAYER_SHOOT_COOLDOWN),
      baseRange(10.0f),
      damageMultiplier(1.0f),
      speedMultiplier(1.0f),
      shotSpeedMultiplier(1.0f),
      fireRateMultiplier(1.0f),
      rangeMultiplier(1.0f) {
}

void Stats::setHealth(int value) {
    health = std::clamp(value, 0, maxHealth);
}

void Stats::setMaxHealth(int value) {
    maxHealth = std::max(1, value);
    // Clamp current health if needed
    if (health > maxHealth) {
        health = maxHealth;
    }
}

void Stats::damage(int amount) {
    health = std::max(0, health - amount);
}

void Stats::heal(int amount) {
    health = std::min(maxHealth, health + amount);
}

void Stats::resetMultipliers() {
    damageMultiplier = 1.0f;
    speedMultiplier = 1.0f;
    shotSpeedMultiplier = 1.0f;
    fireRateMultiplier = 1.0f;
    rangeMultiplier = 1.0f;
}

}  // namespace CanalUx