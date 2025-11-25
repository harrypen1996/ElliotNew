/*
 * CanalUx - Main Game Class
 * Handles game loop, state management, and coordinates all systems
 */

#pragma once

#include <tyra>
#include <memory>
#include "constants.hpp"
#include "world/level.hpp"

// Forward declarations
namespace CanalUx {
    class Player;
    class Level;
    class EntityManager;
    class TextureManager;
    class Camera;
}

namespace CanalUx {

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

    // Subsystems (will be implemented as we go)
    // std::unique_ptr<TextureManager> textureManager;
    // std::unique_ptr<EntityManager> entityManager;
    // std::unique_ptr<Camera> camera;
    // std::unique_ptr<Player> player;
    
    std::unique_ptr<Level> currentLevel;

    // Temporary - until we implement proper subsystems
    float screenWidth;
    float screenHeight;
};

}  // namespace CanalUx