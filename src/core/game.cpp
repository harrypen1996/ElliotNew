/*
 * CanalUx - Main Game Class Implementation
 */

#include "core/game.hpp"

namespace CanalUx {

Game::Game(Tyra::Engine* t_engine)
    : engine(t_engine),
      state(GameState::MENU),
      currentLevelNumber(1),
      cameraX(0.0f),
      cameraY(0.0f),
      screenWidth(Constants::SCREEN_WIDTH),
      screenHeight(Constants::SCREEN_HEIGHT),
      visibleTilesX(0),
      visibleTilesY(0) {
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
    
    // Calculate visible tiles
    visibleTilesX = static_cast<int>(screenWidth / Constants::TILE_SIZE) + 2;
    visibleTilesY = static_cast<int>(screenHeight / Constants::TILE_SIZE) + 2;

    TYRA_LOG("Screen size: ", screenWidth, "x", screenHeight);
    TYRA_LOG("Visible tiles: ", visibleTilesX, "x", visibleTilesY);

    // Load all game assets
    loadAssets();

    // Start the game
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
    
    auto& renderer = engine->renderer;
    auto& textureRepository = renderer.getTextureRepository();
    
    // Load terrain tileset
    auto filepath = Tyra::FileUtils::fromCwd("all2.png");
    auto* texture = textureRepository.add(filepath);
    
    // Set up terrain sprite for tile rendering
    terrainSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    terrainSprite.size = Tyra::Vec2(Constants::TILE_SIZE, Constants::TILE_SIZE);
    texture->addLink(terrainSprite.id);
    
    // Load player texture
    filepath = Tyra::FileUtils::fromCwd("elliot.png");
    auto* playerTexture = textureRepository.add(filepath);
    
    // Set up player sprite
    playerSprite.mode = Tyra::SpriteMode::MODE_STRETCH;
    playerSprite.size = Tyra::Vec2(Constants::PLAYER_SIZE, Constants::PLAYER_SIZE);
    playerTexture->addLink(playerSprite.id);
    
    // Load projectile texture (items sheet)
    filepath = Tyra::FileUtils::fromCwd("items_sheet.png");
    auto* projectileTexture = textureRepository.add(filepath);
    
    // Set up projectile sprite
    projectileSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    projectileSprite.size = Tyra::Vec2(Constants::PROJECTILE_SIZE, Constants::PROJECTILE_SIZE);
    projectileTexture->addLink(projectileSprite.id);
    
    TYRA_LOG("CanalUx: Assets loaded");
}

void Game::initLevel(int levelNumber) {
    TYRA_LOG("CanalUx: Initializing level ", levelNumber);
    
    currentLevelNumber = levelNumber;
    
    // Create and generate the level
    currentLevel = std::make_unique<Level>(levelNumber);
    currentLevel->generate();
    
    // Create player
    player = std::make_unique<Player>(&engine->pad);
    
    // Get the start room and position player in center
    Room* startRoom = currentLevel->getStartRoom();
    if (startRoom) {
        // Position player in center of room
        player->position.x = startRoom->getWidth() / 2.0f - 0.5f;
        player->position.y = startRoom->getHeight() / 2.0f - 0.5f;
        
        // Center camera on player
        cameraX = player->position.x + 0.5f;
        cameraY = player->position.y + 0.5f;
        
        TYRA_LOG("Player spawned at (", player->position.x, ", ", player->position.y, ")");
    }
    
    TYRA_LOG("CanalUx: Level ", levelNumber, " ready");
}

void Game::cleanup() {
    TYRA_LOG("CanalUx: Cleaning up...");
    
    // Free textures
    engine->renderer.getTextureRepository().freeBySprite(terrainSprite);
    engine->renderer.getTextureRepository().freeBySprite(playerSprite);
    engine->renderer.getTextureRepository().freeBySprite(projectileSprite);
    
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

    // Debug: Circle to reset level
    if (engine->pad.getPressed().Circle) {
        TYRA_LOG("CanalUx: Resetting level...");
        initLevel(currentLevelNumber);
    }
}

void Game::update() {
    if (state != GameState::PLAYING) {
        return;
    }

    // Get current room
    Room* room = currentLevel->getCurrentRoom();
    if (!room || !player) return;
    
    // Update player (with projectile manager for shooting)
    player->update(room, &projectileManager);
    
    // Update projectiles
    projectileManager.update(room);
    
    // Check for room transitions
    int currentGridX = currentLevel->getCurrentGridX();
    int currentGridY = currentLevel->getCurrentGridY();
    bool roomChanged = false;
    
    // Left exit
    if (player->position.x < 0.0f) {
        Room* nextRoom = currentLevel->getRoom(currentGridX - 1, currentGridY);
        if (nextRoom && nextRoom->exists()) {
            currentLevel->setCurrentRoom(currentGridX - 1, currentGridY);
            player->position.x = nextRoom->getWidth() - 2.0f;
            player->position.y = nextRoom->getHeight() / 2.0f - 0.5f;
            roomChanged = true;
            TYRA_LOG("Moved to room (", currentGridX - 1, ", ", currentGridY, ")");
        } else {
            player->position.x = 0.0f;
        }
    }
    // Right exit
    else if (player->position.x > room->getWidth() - 1.0f) {
        Room* nextRoom = currentLevel->getRoom(currentGridX + 1, currentGridY);
        if (nextRoom && nextRoom->exists()) {
            currentLevel->setCurrentRoom(currentGridX + 1, currentGridY);
            player->position.x = 1.0f;
            player->position.y = nextRoom->getHeight() / 2.0f - 0.5f;
            roomChanged = true;
            TYRA_LOG("Moved to room (", currentGridX + 1, ", ", currentGridY, ")");
        } else {
            player->position.x = room->getWidth() - 1.0f;
        }
    }
    // Top exit
    else if (player->position.y < 0.0f) {
        Room* nextRoom = currentLevel->getRoom(currentGridX, currentGridY - 1);
        if (nextRoom && nextRoom->exists()) {
            currentLevel->setCurrentRoom(currentGridX, currentGridY - 1);
            player->position.x = nextRoom->getWidth() / 2.0f - 0.5f;
            player->position.y = nextRoom->getHeight() - 2.0f;
            roomChanged = true;
            TYRA_LOG("Moved to room (", currentGridX, ", ", currentGridY - 1, ")");
        } else {
            player->position.y = 0.0f;
        }
    }
    // Bottom exit
    else if (player->position.y > room->getHeight() - 1.0f) {
        Room* nextRoom = currentLevel->getRoom(currentGridX, currentGridY + 1);
        if (nextRoom && nextRoom->exists()) {
            currentLevel->setCurrentRoom(currentGridX, currentGridY + 1);
            player->position.x = nextRoom->getWidth() / 2.0f - 0.5f;
            player->position.y = 1.0f;
            roomChanged = true;
            TYRA_LOG("Moved to room (", currentGridX, ", ", currentGridY + 1, ")");
        } else {
            player->position.y = room->getHeight() - 1.0f;
        }
    }
    
    // Clear projectiles and update room pointer if changed
    if (roomChanged) {
        projectileManager.clear();
        room = currentLevel->getCurrentRoom();
    }
    
    // Camera follows player
    cameraX = player->position.x + 0.5f;
    cameraY = player->position.y + 0.5f;
    
    // Clamp camera to room bounds
    float halfScreenTilesX = (screenWidth / Constants::TILE_SIZE) / 2.0f;
    float halfScreenTilesY = (screenHeight / Constants::TILE_SIZE) / 2.0f;
    
    if (cameraX < halfScreenTilesX) cameraX = halfScreenTilesX;
    if (cameraY < halfScreenTilesY) cameraY = halfScreenTilesY;
    if (cameraX > room->getWidth() - halfScreenTilesX) 
        cameraX = room->getWidth() - halfScreenTilesX;
    if (cameraY > room->getHeight() - halfScreenTilesY) 
        cameraY = room->getHeight() - halfScreenTilesY;
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
            renderRoom();
            renderProjectiles();
            renderPlayer();
            
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

void Game::renderRoom() {
    Room* room = currentLevel->getCurrentRoom();
    if (!room) return;
    
    auto& renderer = engine->renderer;
    
    // Calculate the top-left visible tile based on camera position
    float halfScreenTilesX = (screenWidth / Constants::TILE_SIZE) / 2.0f;
    float halfScreenTilesY = (screenHeight / Constants::TILE_SIZE) / 2.0f;
    
    float offsetX = cameraX - halfScreenTilesX;
    float offsetY = cameraY - halfScreenTilesY;
    
    // Clamp offsets
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;
    if (offsetX > room->getWidth() - visibleTilesX + 2) 
        offsetX = room->getWidth() - visibleTilesX + 2;
    if (offsetY > room->getHeight() - visibleTilesY + 2) 
        offsetY = room->getHeight() - visibleTilesY + 2;
    
    // Sub-tile offset for smooth scrolling
    float tileOffsetX = (offsetX - static_cast<int>(offsetX)) * Constants::TILE_SIZE;
    float tileOffsetY = (offsetY - static_cast<int>(offsetY)) * Constants::TILE_SIZE;
    
    // Render visible tiles
    for (int x = -1; x < visibleTilesX; x++) {
        for (int y = -1; y < visibleTilesY; y++) {
            int tileX = static_cast<int>(offsetX) + x;
            int tileY = static_cast<int>(offsetY) + y;
            
            int screenX = x * Constants::TILE_SIZE - static_cast<int>(tileOffsetX);
            int screenY = y * Constants::TILE_SIZE - static_cast<int>(tileOffsetY);
            
            // Render water layer (background)
            int waterTile = room->getWaterTile(tileX, tileY);
            if (waterTile > 0) {
                renderer.renderer2D.render(getTileSprite(screenX, screenY, waterTile - 1));
            }
            
            // Render land layer (walls/terrain)
            int landTile = room->getLandTile(tileX, tileY);
            if (landTile > 0) {
                renderer.renderer2D.render(getTileSprite(screenX, screenY, landTile - 1));
            }
            
            // Render scenery layer (obstacles, decorations)
            int sceneryTile = room->getSceneryTile(tileX, tileY);
            if (sceneryTile > 0) {
                renderer.renderer2D.render(getTileSprite(screenX, screenY, sceneryTile - 1));
            }
        }
    }
}

void Game::renderPlayer() {
    if (!player) return;
    
    Room* room = currentLevel->getCurrentRoom();
    if (!room) return;
    
    auto& renderer = engine->renderer;
    
    // Calculate camera offset (same as in renderRoom)
    float halfScreenTilesX = (screenWidth / Constants::TILE_SIZE) / 2.0f;
    float halfScreenTilesY = (screenHeight / Constants::TILE_SIZE) / 2.0f;
    
    float offsetX = cameraX - halfScreenTilesX;
    float offsetY = cameraY - halfScreenTilesY;
    
    // Clamp offsets
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;
    if (offsetX > room->getWidth() - halfScreenTilesX * 2) 
        offsetX = room->getWidth() - halfScreenTilesX * 2;
    if (offsetY > room->getHeight() - halfScreenTilesY * 2) 
        offsetY = room->getHeight() - halfScreenTilesY * 2;
    
    // Calculate player screen position
    float screenX = (player->position.x - offsetX) * Constants::TILE_SIZE;
    float screenY = (player->position.y - offsetY) * Constants::TILE_SIZE;
    
    // Update player sprite position
    playerSprite.position = Tyra::Vec2(screenX, screenY);
    
    renderer.renderer2D.render(playerSprite);
}

void Game::renderProjectiles() {
    Room* room = currentLevel->getCurrentRoom();
    if (!room) return;
    
    auto& renderer = engine->renderer;
    
    // Calculate camera offset (same as in renderRoom)
    float halfScreenTilesX = (screenWidth / Constants::TILE_SIZE) / 2.0f;
    float halfScreenTilesY = (screenHeight / Constants::TILE_SIZE) / 2.0f;
    
    float offsetX = cameraX - halfScreenTilesX;
    float offsetY = cameraY - halfScreenTilesY;
    
    // Clamp offsets
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;
    if (offsetX > room->getWidth() - halfScreenTilesX * 2) 
        offsetX = room->getWidth() - halfScreenTilesX * 2;
    if (offsetY > room->getHeight() - halfScreenTilesY * 2) 
        offsetY = room->getHeight() - halfScreenTilesY * 2;
    
    // Render each projectile
    for (const auto& projectile : projectileManager.getProjectiles()) {
        if (!projectile.isActive()) continue;
        
        // Calculate screen position
        float screenX = (projectile.position.x - offsetX) * Constants::TILE_SIZE;
        float screenY = (projectile.position.y - offsetY) * Constants::TILE_SIZE;
        
        // Create sprite for this projectile
        Tyra::Sprite sprite;
        sprite.size = Tyra::Vec2(Constants::PROJECTILE_SIZE, Constants::PROJECTILE_SIZE);
        sprite.position = Tyra::Vec2(screenX, screenY);
        sprite.id = projectileSprite.id;
        sprite.mode = Tyra::SpriteMode::MODE_REPEAT;
        
        // Use a specific tile from the items sheet for projectile
        // The items sheet is 256px wide with 16px tiles = 16 tiles per row
        int tileIndex = 98;  // Adjust this to pick the right projectile sprite
        int tilesPerRow = 256 / 16;
        int column = tileIndex % tilesPerRow;
        int row = tileIndex / tilesPerRow;
        
        sprite.offset = Tyra::Vec2(
            static_cast<float>(column * 16),
            static_cast<float>(row * 16)
        );
        
        renderer.renderer2D.render(sprite);
    }
}

Tyra::Sprite Game::getTileSprite(int screenX, int screenY, int tileIndex) {
    Tyra::Sprite sprite;
    sprite.size = Tyra::Vec2(Constants::TILE_SIZE, Constants::TILE_SIZE);
    sprite.position = Tyra::Vec2(static_cast<float>(screenX), static_cast<float>(screenY));
    sprite.id = terrainSprite.id;
    sprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    
    // Calculate offset in tileset (16 tiles per row in a 512px wide texture)
    int tilesPerRow = 512 / Constants::TILE_SIZE;  // 16 tiles
    int column = tileIndex % tilesPerRow;
    int row = tileIndex / tilesPerRow;
    
    sprite.offset = Tyra::Vec2(
        static_cast<float>(column * Constants::TILE_SIZE),
        static_cast<float>(row * Constants::TILE_SIZE)
    );
    
    return sprite;
}

void Game::renderTile(int screenX, int screenY, int tileIndex) {
    engine->renderer.renderer2D.render(getTileSprite(screenX, screenY, tileIndex));
}

void Game::setState(GameState newState) {
    TYRA_LOG("CanalUx: State change ", static_cast<int>(state), " -> ", static_cast<int>(newState));
    state = newState;
}

void Game::startNewGame() {
    TYRA_LOG("CanalUx: Starting new game");
    
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