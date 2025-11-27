/*
 * CanalUx - Main Game Class Implementation
 */

#include "core/game.hpp"

namespace CanalUx {

Game::Game(Tyra::Engine* t_engine)
    : engine(t_engine),
      state(GameState::MENU),
      currentLevelNumber(1) {
}

Game::~Game() {
    cleanupRenderers();
}

void Game::init() {
    TYRA_LOG("CanalUx: Initializing game...");
    
    // Set background color
    engine->renderer.setClearScreenColor(Tyra::Color(32.0f, 48.0f, 64.0f));

    // Initialize camera with screen size
    auto& screenSettings = engine->renderer.core.getSettings();
    camera.setScreenSize(screenSettings.getWidth(), screenSettings.getHeight());

    // Initialize all renderers
    initRenderers();

    // Start the game
    startNewGame();

    TYRA_LOG("CanalUx: Initialization complete");
}

void Game::loop() {
    handleInput();
    update();
    render();
}

void Game::initRenderers() {
    auto& textureRepo = engine->renderer.getTextureRepository();
    
    roomRenderer.init(&textureRepo);
    entityRenderer.init(&textureRepo);
    hudRenderer.init(&textureRepo, &engine->renderer.renderer2D);
    
    TYRA_LOG("CanalUx: Renderers initialized");
}

void Game::cleanupRenderers() {
    auto& textureRepo = engine->renderer.getTextureRepository();
    
    roomRenderer.cleanup(&textureRepo);
    entityRenderer.cleanup(&textureRepo);
    hudRenderer.cleanup(&textureRepo);
    
    TYRA_LOG("CanalUx: Renderers cleaned up");
}

void Game::initLevel(int levelNumber) {
    TYRA_LOG("CanalUx: Initializing level ", levelNumber);
    
    currentLevelNumber = levelNumber;
    
    // Create and generate the level
    currentLevel = std::make_unique<Level>(levelNumber);
    currentLevel->generate();
    
    // Create player
    player = std::make_unique<Player>(&engine->pad);
    
    // Cheat: Skip to boss room
    Room* spawnRoom = nullptr;
    if (Constants::Cheats::SKIP_TO_BOSS) {
        spawnRoom = currentLevel->getBossRoom();
        if (spawnRoom) {
            int bossX, bossY;
            currentLevel->getBossRoomGridPos(bossX, bossY);
            currentLevel->setCurrentRoom(bossX, bossY);
            TYRA_LOG("CHEAT: Skipping to boss room at (", bossX, ", ", bossY, ")");
        }
    }
    
    // Fall back to start room if no boss room or cheat disabled
    if (!spawnRoom) {
        spawnRoom = currentLevel->getStartRoom();
    }
    
    if (spawnRoom) {
        player->position.x = spawnRoom->getWidth() / 2.0f - 0.5f;
        player->position.y = spawnRoom->getHeight() / 2.0f - 0.5f;
        spawnRoom->setVisited(true);
        
        TYRA_LOG("Player spawned at (", player->position.x, ", ", player->position.y, ")");
        
        // Spawn mobs for the room (including boss if boss room)
        if (Constants::Cheats::SKIP_TO_BOSS && spawnRoom->getType() == RoomType::BOSS) {
            mobManager.spawnMobsForRoom(spawnRoom, currentLevelNumber);
        }
    }
    
    // Clear managers for new level
    projectileManager.clear();
    if (!Constants::Cheats::SKIP_TO_BOSS) {
        mobManager.clear();
    }
    
    // Setup camera
    camera.follow(player->position);
    camera.clampToRoom(spawnRoom);
    
    TYRA_LOG("CanalUx: Level ", levelNumber, " ready");
}

void Game::handleInput() {
    // Handle input based on game state
    switch (state) {
        case GameState::PLAYING:
            if (engine->pad.getPressed().Start) {
                setState(GameState::PAUSED);
            }
            // Debug: Circle to reset level
            if (engine->pad.getPressed().Cross) {
                TYRA_LOG("CanalUx: Resetting level...");
                startNewGame();
            }
            break;
            
        case GameState::PAUSED:
            if (engine->pad.getPressed().Start) {
                setState(GameState::PLAYING);
            }
            break;
            
        case GameState::GAME_OVER:
            // Press Cross to restart from level 1
            if (engine->pad.getPressed().Cross) {
                TYRA_LOG("CanalUx: Restarting game after death...");
                startNewGame();
            }
            break;
            
        case GameState::VICTORY:
            // Press Cross to start a new game
            if (engine->pad.getPressed().Cross) {
                TYRA_LOG("CanalUx: Starting new game after victory...");
                startNewGame();
            }
            break;
            
        default:
            break;
    }
}

void Game::update() {
    if (state != GameState::PLAYING) {
        return;
    }

    Room* room = currentLevel->getCurrentRoom();
    if (!room || !player) return;
    
    // Update player
    player->update(room, &projectileManager);
    
    // Update projectiles
    projectileManager.update(room);
    
    // Update mobs
    mobManager.update(room, player.get(), &projectileManager);
    
    // Check collisions
    collisionManager.checkCollisions(player.get(), &mobManager, &projectileManager, room);
    
    // Check if room is cleared
    if (mobManager.isRoomCleared() && !room->isCleared()) {
        room->completeClear();
        TYRA_LOG("Room cleared!");
        
        // Check if this was the boss room
        if (room->getType() == RoomType::BOSS) {
            onBossDefeated();
        }
    }
    
    // Check room transitions
    checkRoomTransitions();
    
    // Update camera
    camera.follow(player->position);
    camera.clampToRoom(currentLevel->getCurrentRoom());
    
    // Check player death
    if (player->getStats().isDead()) {
        onPlayerDeath();
    }
}

void Game::checkRoomTransitions() {
    Room* room = currentLevel->getCurrentRoom();
    if (!room || !player) return;
    
    int gridX = currentLevel->getCurrentGridX();
    int gridY = currentLevel->getCurrentGridY();
    bool roomChanged = false;
    
    // Left exit
    if (player->position.x < 0.0f) {
        Room* nextRoom = currentLevel->getRoom(gridX - 1, gridY);
        if (nextRoom && nextRoom->exists()) {
            currentLevel->setCurrentRoom(gridX - 1, gridY);
            player->position.x = nextRoom->getWidth() - 2.0f;
            player->position.y = nextRoom->getHeight() / 2.0f - 0.5f;
            roomChanged = true;
            TYRA_LOG("Moved to room (", gridX - 1, ", ", gridY, ")");
        } else {
            player->position.x = 0.0f;
        }
    }
    // Right exit
    else if (player->position.x > room->getWidth() - 1.0f) {
        Room* nextRoom = currentLevel->getRoom(gridX + 1, gridY);
        if (nextRoom && nextRoom->exists()) {
            currentLevel->setCurrentRoom(gridX + 1, gridY);
            player->position.x = 1.0f;
            player->position.y = nextRoom->getHeight() / 2.0f - 0.5f;
            roomChanged = true;
            TYRA_LOG("Moved to room (", gridX + 1, ", ", gridY, ")");
        } else {
            player->position.x = room->getWidth() - 1.0f;
        }
    }
    // Top exit
    else if (player->position.y < 0.0f) {
        Room* nextRoom = currentLevel->getRoom(gridX, gridY - 1);
        if (nextRoom && nextRoom->exists()) {
            currentLevel->setCurrentRoom(gridX, gridY - 1);
            player->position.x = nextRoom->getWidth() / 2.0f - 0.5f;
            player->position.y = nextRoom->getHeight() - 2.0f;
            roomChanged = true;
            TYRA_LOG("Moved to room (", gridX, ", ", gridY - 1, ")");
        } else {
            player->position.y = 0.0f;
        }
    }
    // Bottom exit
    else if (player->position.y > room->getHeight() - 1.0f) {
        Room* nextRoom = currentLevel->getRoom(gridX, gridY + 1);
        if (nextRoom && nextRoom->exists()) {
            currentLevel->setCurrentRoom(gridX, gridY + 1);
            player->position.x = nextRoom->getWidth() / 2.0f - 0.5f;
            player->position.y = 1.0f;
            roomChanged = true;
            TYRA_LOG("Moved to room (", gridX, ", ", gridY + 1, ")");
        } else {
            player->position.y = room->getHeight() - 1.0f;
        }
    }
    
    if (roomChanged) {
        onRoomEnter();
    }
}

void Game::onRoomEnter() {
    // Clear projectiles when entering a new room
    projectileManager.clear();
    
    // Mark room as visited and spawn mobs
    Room* room = currentLevel->getCurrentRoom();
    if (room) {
        room->setVisited(true);
        mobManager.spawnMobsForRoom(room, currentLevelNumber);
    }
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
            {
                Room* room = currentLevel->getCurrentRoom();
                
                // Render room tiles
                roomRenderer.render(&renderer.renderer2D, room, &camera);
                
                // Render entities (projectiles, mobs, player)
                entityRenderer.render(&renderer.renderer2D, &camera, 
                                      player.get(), &projectileManager, &mobManager);
                
                // Render HUD
                hudRenderer.render(&renderer.renderer2D, player.get(), currentLevel.get());
                
                if (state == GameState::PAUSED) {
                    // TODO: Render pause overlay
                }
            }
            break;
            
        case GameState::GAME_OVER:
            {
                // Still show the game world behind
                Room* room = currentLevel->getCurrentRoom();
                roomRenderer.render(&renderer.renderer2D, room, &camera);
                entityRenderer.render(&renderer.renderer2D, &camera, 
                                      player.get(), &projectileManager, &mobManager);
                hudRenderer.render(&renderer.renderer2D, player.get(), currentLevel.get());
                
                // TODO: Render "GAME OVER - Press X to restart" text overlay
            }
            break;
            
        case GameState::VICTORY:
            {
                // Still show the game world behind
                Room* room = currentLevel->getCurrentRoom();
                roomRenderer.render(&renderer.renderer2D, room, &camera);
                entityRenderer.render(&renderer.renderer2D, &camera, 
                                      player.get(), &projectileManager, &mobManager);
                hudRenderer.render(&renderer.renderer2D, player.get(), currentLevel.get());
                
                // TODO: Render "VICTORY! - Press X to play again" text overlay
            }
            break;
    }

    renderer.endFrame();
}

void Game::setState(GameState newState) {
    TYRA_LOG("CanalUx: State change ", static_cast<int>(state), " -> ", static_cast<int>(newState));
    state = newState;
}

void Game::startNewGame() {
    TYRA_LOG("CanalUx: Starting new game");
    
    // Cheat: Start at specific level
    int startLevel = 1;
    if (Constants::Cheats::START_LEVEL > 0 && Constants::Cheats::START_LEVEL <= Constants::TOTAL_LEVELS) {
        startLevel = Constants::Cheats::START_LEVEL;
        TYRA_LOG("CHEAT: Starting at level ", startLevel);
    }
    
    initLevel(startLevel);
    setState(GameState::PLAYING);
}

void Game::advanceToNextLevel() {
    if (currentLevelNumber >= Constants::TOTAL_LEVELS) {
        TYRA_LOG("CanalUx: All levels complete! Victory!");
        setState(GameState::VICTORY);
        return;
    }
    
    TYRA_LOG("CanalUx: Advancing to level ", currentLevelNumber + 1);
    initLevel(currentLevelNumber + 1);
}

void Game::onPlayerDeath() {
    TYRA_LOG("CanalUx: Player died! Press X to restart.");
    setState(GameState::GAME_OVER);
}

void Game::onBossDefeated() {
    TYRA_LOG("CanalUx: Boss defeated!");
    advanceToNextLevel();
}

void Game::onLevelComplete() {
    advanceToNextLevel();
}

}  // namespace CanalUx