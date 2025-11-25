/*
 * CanalUx - Main Game Class
 * Handles game loop, state management, and coordinates all systems
 */

#pragma once

#include <tyra>
#include <memory>
#include "constants.hpp"
#include "world/level.hpp"
#include "entities/player.hpp"
#include "managers/projectile_manager.hpp"

namespace CanalUx {

// Forward declarations
class EntityManager;
class TextureManager;
class Camera;

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    VICTORY
};

class Game : public Tyra::Game {
public:
    explicit Game(Tyra::Engine* engine);
    ~Game();

    void init() override;
    void loop() override;

    // Accessors for subsystems
    Tyra::Engine* getEngine() { return engine; }
    GameState getState() const { return state; }
    int getCurrentLevel() const { return currentLevelNumber; }

private:
    // Lifecycle
    void loadAssets();
    void initLevel(int levelNumber);
    void cleanup();

    // Game loop phases
    void handleInput();
    void update();
    void render();

    // Rendering helpers
    void renderRoom();
    void renderPlayer();
    void renderProjectiles();
    void renderTile(int screenX, int screenY, int tileIndex);
    Tyra::Sprite getTileSprite(int screenX, int screenY, int tileIndex);

    // State transitions
    void setState(GameState newState);
    void startNewGame();
    void advanceToNextLevel();
    void onPlayerDeath();
    void onLevelComplete();

    // Core engine reference
    Tyra::Engine* engine;

    // Game state
    GameState state;
    int currentLevelNumber;

    // Level
    std::unique_ptr<Level> currentLevel;

    // Player
    std::unique_ptr<Player> player;

    // Managers
    ProjectileManager projectileManager;

    // Textures
    Tyra::Sprite terrainSprite;      // Base sprite for terrain tileset
    Tyra::Sprite playerSprite;       // Player sprite
    Tyra::Sprite projectileSprite;   // Projectile sprite
    
    // Camera position (in tiles)
    float cameraX;
    float cameraY;

    // Screen info
    float screenWidth;
    float screenHeight;
    int visibleTilesX;
    int visibleTilesY;
};

}  // namespace CanalUx