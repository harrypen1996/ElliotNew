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

// Boss room sizes per level
// Level 1 - Pike: Large open water arena for swimming
constexpr int PIKE_ROOM_WIDTH = 24;
constexpr int PIKE_ROOM_HEIGHT = 20;

// Level 2 - Lock Keeper: Wide but short, shrinks horizontally
constexpr int LOCKKEEPER_ROOM_WIDTH = 28;
constexpr int LOCKKEEPER_ROOM_HEIGHT = 14;  // Minimum height, no vertical space
constexpr int LOCKKEEPER_ROOM_MIN_WIDTH = 12;  // Shrinks to this in phase 3

// Level 3 - Nanny: Narrow but tall, gauntlet phases with barges
constexpr int NANNY_ROOM_WIDTH = 16;   // Narrow - min width
constexpr int NANNY_ROOM_HEIGHT = 28;  // Very tall for gauntlet run
constexpr float NANNY_BARGE_SPEED_1 = 0.12f;   // Gauntlet 1 stream speed
constexpr float NANNY_BARGE_SPEED_2 = 0.16f;   // Gauntlet 2 stream speed (faster)
constexpr float NANNY_BARGE_SPAWN_INTERVAL = 12.0f;  // Frames between barge spawns (continuous stream)
constexpr int NANNY_MIN_GAPS_1 = 2;    // Minimum gaps in stream (gauntlet 1 - easier)
constexpr int NANNY_MIN_GAPS_2 = 1;    // Minimum gaps in stream (gauntlet 2 - harder)

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

// ============================================
// DEBUG / CHEAT FLAGS - Set to false for release!
// ============================================
namespace Cheats {
    // God mode - player can't take damage
    constexpr bool GOD_MODE = false;
    
    // Massive damage - one-shot everything
    constexpr bool ONE_HIT_KILLS = false;
    
    // Start at specific level (0 = normal, 1-3 = skip to that level)
    constexpr int START_LEVEL = 3;
    
    // Rooms start cleared (no enemies, doors open)
    constexpr bool ROOMS_PRE_CLEARED = false;
    
    // Skip straight to boss room
    constexpr bool SKIP_TO_BOSS = true;
    
    // Infinite submerge (no cooldown/duration limit)
    constexpr bool INFINITE_SUBMERGE = false;
    
    // Show debug info on screen (positions, health, etc)
    constexpr bool SHOW_DEBUG_INFO = false;
    
    // Fast player speed multiplier
    constexpr float SPEED_MULTIPLIER = 1.5f;  // Set to 2.0f for double speed
}

}  // namespace Constants
}  // namespace CanalUx