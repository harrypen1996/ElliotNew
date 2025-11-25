/*
 * CanalUx - Main Game Class Implementation
 */

#include "core/game.hpp"

namespace CanalUx {

Game::Game(Tyra::Engine* t_engine)
    : engine(t_engine),
      state(GameState::MENU),
      currentLevelNumber(1),
      screenWidth(Constants::SCREEN_WIDTH),
      screenHeight(Constants::SCREEN_HEIGHT) {
}

Game::~Game() {
    cleanup();
}

void Game::init() {
    TYRA_LOG("CanalUx: Initializing game...");
    
    // Set background color (dark water-ish blue)
    engine->renderer.setClearScreenColor(Tyra::Color(32.0f, 48.0f, 64.0f));

    // Get actual screen dimensions
    auto& screenSettings = engine->renderer.core.getSettings();
    screenWidth = screenSettings.getWidth();
    screenHeight = screenSettings.getHeight();

    TYRA_LOG("Screen size: ", screenWidth, "x", screenHeight);

    // Load all game assets
    loadAssets();

    // For now, jump straight into playing
    // Later we'll add a proper menu
    startNewGame();

    TYRA_LOG("CanalUx: Initialization complete");
}

void Game::loop() {
    handleInput();
    update();
    render();
}

void Game::loadAssets() {
    TYRA_LOG("CanalUx: Loading assets...");
    
    // TODO: Initialize TextureManager and load all spritesheets
    // textureManager = std::make_unique<TextureManager>(engine);
    // textureManager->loadAll();
    
    TYRA_LOG("CanalUx: Assets loaded");
}

void Game::initLevel(int levelNumber) {
    TYRA_LOG("CanalUx: Initializing level ", levelNumber);
    
    currentLevelNumber = levelNumber;
    
    // Create and generate the level
    currentLevel = std::make_unique<Level>(levelNumber);
    currentLevel->generate();
    
    // TODO: Spawn player at start room
    // Room* startRoom = currentLevel->getStartRoom();
    // player->position = startRoom->getSpawnPoint(-1);  // -1 = center spawn
    
    TYRA_LOG("CanalUx: Level ", levelNumber, " ready");
}

void Game::cleanup() {
    TYRA_LOG("CanalUx: Cleaning up...");
    
    // TODO: Free textures and resources
    
    TYRA_LOG("CanalUx: Cleanup complete");
}

void Game::handleInput() {
    // Global input handling (pause, quit, etc.)
    if (engine->pad.getPressed().Start) {
        if (state == GameState::PLAYING) {
            setState(GameState::PAUSED);
        } else if (state == GameState::PAUSED) {
            setState(GameState::PLAYING);
        }
    }

    // Debug: Circle to quit
    if (engine->pad.getPressed().Circle) {
        exit(0);
    }

    // State-specific input is handled by respective systems
    // e.g., player input handled by Player::update()
}

void Game::update() {
    if (state != GameState::PLAYING) {
        return;
    }

    // TODO: Update all game systems
    // player->update(currentLevel->getCurrentRoom(), entityManager.get());
    // entityManager->update(currentLevel->getCurrentRoom());
    // camera->follow(player->getPosition());
    
    // Check for room transitions
    // Check for level completion
    // Check for player death
}

void Game::render() {
    auto& renderer = engine->renderer;
    
    renderer.beginFrame();

    switch (state) {
        case GameState::MENU:
            // TODO: Render menu
            break;
            
        case GameState::PLAYING:
        case GameState::PAUSED:
            // TODO: Render game world
            // Render order:
            // 1. Water layer (background)
            // 2. Land/terrain layer
            // 3. Obstacles (shopping carts, logs)
            // 4. Entities sorted by Y (mobs, projectiles)
            // 5. Player (unless submerged, then before obstacles)
            // 6. Scenery layer (things that go on top)
            // 7. HUD
            
            if (state == GameState::PAUSED) {
                // TODO: Render pause overlay
            }
            break;
            
        case GameState::GAME_OVER:
            // TODO: Render game over screen
            break;
            
        case GameState::VICTORY:
            // TODO: Render victory screen
            break;
    }

    renderer.endFrame();
}

void Game::setState(GameState newState) {
    //TYRA_LOG("CanalUx: State change ", static_cast<int>(state), " -> ", static_cast<int>(newState));
    state = newState;
}

void Game::startNewGame() {
    TYRA_LOG("CanalUx: Starting new game");
    
    // TODO: Reset player stats
    // player = std::make_unique<Player>(&engine->pad);
    // entityManager = std::make_unique<EntityManager>();
    // camera = std::make_unique<Camera>(screenWidth, screenHeight);
    
    initLevel(1);
    setState(GameState::PLAYING);
}

void Game::advanceToNextLevel() {
    if (currentLevelNumber >= Constants::TOTAL_LEVELS) {
        setState(GameState::VICTORY);
        return;
    }
    
    initLevel(currentLevelNumber + 1);
}

void Game::onPlayerDeath() {
    setState(GameState::GAME_OVER);
}

void Game::onLevelComplete() {
    advanceToNextLevel();
}

}  // namespace CanalUx