/*
 * CanalUx - Stats Component
 * Handles health, damage, and stat modifiers for entities
 */

#pragma once

#include "core/constants.hpp"

namespace CanalUx {

/**
 * Stats component that can be attached to any entity
 * Handles health, damage output, and various modifiers
 */
class Stats {
public:
    Stats();
    Stats(int maxHp, float dmg, float spd);
    ~Stats() = default;

    // Health
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    void setHealth(int value);
    void setMaxHealth(int value);
    void damage(int amount);
    void heal(int amount);
    bool isDead() const { return health <= 0; }

    // Damage output
    float getDamage() const { return baseDamage * damageMultiplier; }
    void setBaseDamage(float value) { baseDamage = value; }
    void setDamageMultiplier(float value) { damageMultiplier = value; }
    void addDamageMultiplier(float value) { damageMultiplier += value; }

    // Speed
    float getSpeed() const { return baseSpeed * speedMultiplier; }
    void setBaseSpeed(float value) { baseSpeed = value; }
    void setSpeedMultiplier(float value) { speedMultiplier = value; }
    void addSpeedMultiplier(float value) { speedMultiplier += value; }

    // Shot speed (projectile velocity)
    float getShotSpeed() const { return baseShotSpeed * shotSpeedMultiplier; }
    void setBaseShotSpeed(float value) { baseShotSpeed = value; }
    void setShotSpeedMultiplier(float value) { shotSpeedMultiplier = value; }

    // Fire rate (lower = faster)
    float getFireRate() const { return baseFireRate * fireRateMultiplier; }
    void setBaseFireRate(float value) { baseFireRate = value; }
    void setFireRateMultiplier(float value) { fireRateMultiplier = value; }

    // Range (how far projectiles travel)
    float getRange() const { return baseRange * rangeMultiplier; }
    void setBaseRange(float value) { baseRange = value; }
    void setRangeMultiplier(float value) { rangeMultiplier = value; }

    // Reset modifiers (e.g., when entering new level or on death)
    void resetMultipliers();

private:
    // Health
    int health;
    int maxHealth;

    // Base stats
    float baseDamage;
    float baseSpeed;
    float baseShotSpeed;
    float baseFireRate;
    float baseRange;

    // Multipliers (from items, buffs, etc.)
    float damageMultiplier;
    float speedMultiplier;
    float shotSpeedMultiplier;
    float fireRateMultiplier;
    float rangeMultiplier;
};

}  // namespace CanalUx