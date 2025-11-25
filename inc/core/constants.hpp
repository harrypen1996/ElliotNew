/*
 * CanalUx - Game Constants
 * Central location for all game configuration values
 */

#pragma once

namespace CanalUx {
namespace Constants {

// Display
constexpr int TILE_SIZE = 32;
constexpr float SCREEN_WIDTH = 512.0f;   // 16 tiles
constexpr float SCREEN_HEIGHT = 448.0f;  // 14 tiles

// Screen size in tiles (for reference)
constexpr int SCREEN_TILES_X = 16;
constexpr int SCREEN_TILES_Y = 14;

// Player defaults
constexpr float PLAYER_SPEED = 1.0f;
constexpr float PLAYER_SIZE = 32.0f;
constexpr int PLAYER_MAX_HEALTH = 6;  // 3 hearts = 6 half-hearts
constexpr float PLAYER_SHOOT_COOLDOWN = 5000.0f;
constexpr float PLAYER_SUBMERGE_DURATION = 3000.0f;  // Max time underwater
constexpr float PLAYER_SUBMERGE_COOLDOWN = 5000.0f;  // Cooldown before can submerge again

// Projectile defaults
constexpr float PROJECTILE_SPEED = 1.25f;
constexpr float PROJECTILE_SIZE = 16.0f;

// Physics
constexpr float DRAG_COEFFICIENT = 0.2f;
constexpr float VELOCITY_THRESHOLD = 0.01f;
constexpr float MAX_VELOCITY = 0.5f;

// Room generation - minimum must be screen size (16x14 tiles)
constexpr int ROOM_MIN_WIDTH = 16;   // Must be >= SCREEN_TILES_X
constexpr int ROOM_MAX_WIDTH = 20;
constexpr int ROOM_MIN_HEIGHT = 14;  // Must be >= SCREEN_TILES_Y
constexpr int ROOM_MAX_HEIGHT = 18;
constexpr int LEVEL_GRID_WIDTH = 9;
constexpr int LEVEL_GRID_HEIGHT = 8;
constexpr int MIN_ROOMS_PER_LEVEL = 6;
constexpr int MAX_ROOMS_PER_LEVEL = 12;

// Game progression
constexpr int TOTAL_LEVELS = 3;  // 3 canal sections to escape

// Mob defaults
constexpr float MOB_BASE_SPEED = 0.025f;

// Sprite sheet indices (will be configured based on your assets)
namespace Tiles {
    constexpr int WATER = 45;
    constexpr int WATER_EDGE_TOP = 125;
    constexpr int WATER_EDGE_LEFT = 44;
    constexpr int WATER_EDGE_RIGHT = 46;
    // Add more as needed
}

namespace Sprites {
    constexpr int PLAYER_IDLE = 0;
    constexpr int PLAYER_SUBMERGED = 1;
    // Mob sprites
    constexpr int DUCK = 0;
    constexpr int SWAN = 1;
    constexpr int FISH = 2;
    constexpr int FROG = 3;
    // Add more as needed
}

}  // namespace Constants
}  // namespace CanalUx