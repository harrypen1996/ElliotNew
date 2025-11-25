/*
 * CanalUx - Main Game Class
 * Coordinator that ties together all game systems
 */

#pragma once

#include <tyra>
#include <memory>
#include "core/constants.hpp"
#include "core/camera.hpp"
#include "world/level.hpp"
#include "entities/player.hpp"
#include "managers/projectile_manager.hpp"
#include "managers/mob_manager.hpp"
#include "managers/collision_manager.hpp"
#include "rendering/room_renderer.hpp"
#include "rendering/entity_renderer.hpp"
#include "rendering/hud_renderer.hpp"

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

    // Accessors
    Tyra::Engine* getEngine() { return engine; }
    GameState getState() const { return state; }
    int getCurrentLevelNumber() const { return currentLevelNumber; }

private:
    // Lifecycle
    void initRenderers();
    void cleanupRenderers();
    void initLevel(int levelNumber);

    // Game loop phases
    void handleInput();
    void update();
    void render();

    // Room transition logic
    void checkRoomTransitions();
    void onRoomEnter();

    // State transitions
    void setState(GameState newState);
    void startNewGame();
    void advanceToNextLevel();
    void onPlayerDeath();
    void onBossDefeated();
    void onLevelComplete();

    // Core engine reference
    Tyra::Engine* engine;

    // Game state
    GameState state;
    int currentLevelNumber;

    // Camera
    Camera camera;

    // Level
    std::unique_ptr<Level> currentLevel;

    // Entities
    std::unique_ptr<Player> player;

    // Managers
    ProjectileManager projectileManager;
    MobManager mobManager;
    CollisionManager collisionManager;

    // Renderers
    RoomRenderer roomRenderer;
    EntityRenderer entityRenderer;
    HUDRenderer hudRenderer;
};

}  // namespace CanalUx