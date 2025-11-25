/*
 * CanalUx - HUD Renderer Implementation
 */

#include "rendering/hud_renderer.hpp"
#include "entities/player.hpp"
#include "world/level.hpp"

namespace CanalUx {

HUDRenderer::HUDRenderer() 
    : screenWidth(Constants::SCREEN_WIDTH),
      screenHeight(Constants::SCREEN_HEIGHT) {
}

HUDRenderer::~HUDRenderer() {
}

void HUDRenderer::init(Tyra::TextureRepository* textureRepo) {
    // Load items sheet for heart sprites and minimap icons
    auto filepath = Tyra::FileUtils::fromCwd("items_sheet.png");
    auto* itemsTexture = textureRepo->add(filepath);
    
    heartSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    heartSprite.size = Tyra::Vec2(16.0f, 16.0f);
    itemsTexture->addLink(heartSprite.id);
    
    // Use same texture for minimap icons
    minimapSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    minimapSprite.size = Tyra::Vec2(8.0f, 8.0f);
    itemsTexture->addLink(minimapSprite.id);
    
    TYRA_LOG("HUDRenderer: Initialized");
}

void HUDRenderer::cleanup(Tyra::TextureRepository* textureRepo) {
    textureRepo->freeBySprite(heartSprite);
    textureRepo->freeBySprite(minimapSprite);
}

void HUDRenderer::render(Tyra::Renderer2D* renderer, const Player* player, const Level* level) {
    // Render health
    if (player) {
        renderHealth(renderer, player);
    }
    
    // Render minimap
    if (level) {
        renderMinimap(renderer, level);
    }
    
    // Render level number
    if (level) {
        renderLevelIndicator(renderer, level);
    }
}

void HUDRenderer::renderHealth(Tyra::Renderer2D* renderer, const Player* player) {
    const Stats& stats = player->getStats();
    int currentHealth = stats.getHealth();
    int maxHealth = stats.getMaxHealth();
    
    // Position hearts in top-left corner
    float startX = 10.0f;
    float startY = 10.0f;
    float heartSpacing = 18.0f;
    
    // Items sheet is 256px wide with 16px tiles = 16 tiles per row
    // Heart tiles - look for heart/orb sprites in your sheet
    // Row 12-13 area seems to have circular icons
    // Adjust these values to match your actual heart sprites
    int fullHeartTile = 196;   // Try different values to find hearts
    int halfHeartTile = 197;   
    int emptyHeartTile = 198;
    
    int tilesPerRow = 256 / 16;
    
    // Draw hearts (each heart = 2 health points)
    int numHearts = maxHealth / 2;
    
    for (int i = 0; i < numHearts; i++) {
        int heartHealth = currentHealth - (i * 2);
        int tileIndex;
        
        if (heartHealth >= 2) {
            tileIndex = fullHeartTile;
        } else if (heartHealth == 1) {
            tileIndex = halfHeartTile;
        } else {
            tileIndex = emptyHeartTile;
        }
        
        int column = tileIndex % tilesPerRow;
        int row = tileIndex / tilesPerRow;
        
        Tyra::Sprite heart;
        heart.id = heartSprite.id;
        heart.mode = Tyra::SpriteMode::MODE_REPEAT;
        heart.size = Tyra::Vec2(16.0f, 16.0f);
        heart.position = Tyra::Vec2(startX + i * heartSpacing, startY);
        heart.offset = Tyra::Vec2(
            static_cast<float>(column * 16),
            static_cast<float>(row * 16)
        );
        
        renderer->render(heart);
    }
}

void HUDRenderer::renderMinimap(Tyra::Renderer2D* renderer, const Level* level) {
    // Minimap position (top-right corner)
    float mapStartX = screenWidth - 100.0f;
    float mapStartY = 10.0f;
    float roomSize = 10.0f;
    float roomSpacing = 2.0f;
    
    int gridWidth = level->getGridWidth();
    int gridHeight = level->getGridHeight();
    int currentX = level->getCurrentGridX();
    int currentY = level->getCurrentGridY();
    
    // Draw visited rooms
    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            const Room* room = level->getRoom(x, y);
            if (!room || !room->exists() || !room->isVisited()) {
                continue;
            }
            
            float drawX = mapStartX + x * (roomSize + roomSpacing);
            float drawY = mapStartY + y * (roomSize + roomSpacing);
            
            // Choose color/tile based on room type
            Tyra::Color roomColor;
            switch (room->getType()) {
                case RoomType::START:
                    roomColor = Tyra::Color(100, 200, 100);  // Green
                    break;
                case RoomType::BOSS:
                    roomColor = Tyra::Color(200, 50, 50);    // Red
                    break;
                case RoomType::SHOP:
                    roomColor = Tyra::Color(200, 200, 50);   // Yellow
                    break;
                case RoomType::SPECIAL:
                    roomColor = Tyra::Color(200, 100, 200);  // Purple
                    break;
                default:
                    if (room->isCleared()) {
                        roomColor = Tyra::Color(150, 150, 150);  // Gray (cleared)
                    } else {
                        roomColor = Tyra::Color(100, 150, 200);  // Blue (uncleared)
                    }
                    break;
            }
            
            // Draw room as colored square
            Tyra::Sprite roomSprite;
            roomSprite.id = minimapSprite.id;
            roomSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
            roomSprite.size = Tyra::Vec2(roomSize, roomSize);
            roomSprite.position = Tyra::Vec2(drawX, drawY);
            roomSprite.color = roomColor;
            // Use a solid tile (adjust index to find a solid colored tile in your sheet)
            roomSprite.offset = Tyra::Vec2(0, 0);
            
            renderer->render(roomSprite);
            
            // Draw current room indicator (white border)
            if (x == currentX && y == currentY) {
                // Draw a smaller white square inside
                Tyra::Sprite currentIndicator;
                currentIndicator.id = minimapSprite.id;
                currentIndicator.mode = Tyra::SpriteMode::MODE_REPEAT;
                currentIndicator.size = Tyra::Vec2(roomSize - 4.0f, roomSize - 4.0f);
                currentIndicator.position = Tyra::Vec2(drawX + 2.0f, drawY + 2.0f);
                currentIndicator.color = Tyra::Color(255, 255, 255);
                currentIndicator.offset = Tyra::Vec2(0, 0);
                
                renderer->render(currentIndicator);
            }
        }
    }
}

void HUDRenderer::renderLevelIndicator(Tyra::Renderer2D* renderer, const Level* level) {
    // Position level indicator below minimap
    float indicatorX = screenWidth - 90.0f;
    float indicatorY = 110.0f;
    
    int levelNum = level->getLevelNumber();
    
    // Draw level number as colored squares (1, 2, or 3 squares)
    for (int i = 0; i < levelNum; i++) {
        Tyra::Sprite levelDot;
        levelDot.id = minimapSprite.id;
        levelDot.mode = Tyra::SpriteMode::MODE_REPEAT;
        levelDot.size = Tyra::Vec2(12.0f, 12.0f);
        levelDot.position = Tyra::Vec2(indicatorX + i * 16.0f, indicatorY);
        levelDot.color = Tyra::Color(255, 200, 100);  // Orange/gold
        levelDot.offset = Tyra::Vec2(0, 0);
        
        renderer->render(levelDot);
    }
    
    // Draw empty dots for remaining levels
    for (int i = levelNum; i < Constants::TOTAL_LEVELS; i++) {
        Tyra::Sprite levelDot;
        levelDot.id = minimapSprite.id;
        levelDot.mode = Tyra::SpriteMode::MODE_REPEAT;
        levelDot.size = Tyra::Vec2(12.0f, 12.0f);
        levelDot.position = Tyra::Vec2(indicatorX + i * 16.0f, indicatorY);
        levelDot.color = Tyra::Color(80, 80, 80);  // Dark gray
        levelDot.offset = Tyra::Vec2(0, 0);
        
        renderer->render(levelDot);
    }
}

}  // namespace CanalUx