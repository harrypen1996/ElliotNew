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

void HUDRenderer::init(Tyra::TextureRepository* textureRepo, Tyra::Renderer2D* renderer2D) {
    // Load hearts texture (128x32, contains 3 heart sprites at 32x32 each)
    auto filepath = Tyra::FileUtils::fromCwd("hearts.png");
    auto* heartsTexture = textureRepo->add(filepath);
    
    heartSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    heartSprite.size = Tyra::Vec2(32.0f, 32.0f);
    heartsTexture->addLink(heartSprite.id);
    
    // Load simple white pixel texture for colored rectangles
    filepath = Tyra::FileUtils::fromCwd("pixel.png");
    auto* pixelTexture = textureRepo->add(filepath);
    
    minimapSprite.mode = Tyra::SpriteMode::MODE_STRETCH;
    minimapSprite.size = Tyra::Vec2(8.0f, 8.0f);
    pixelTexture->addLink(minimapSprite.id);
    
    // Load font
    font.load(textureRepo, renderer2D);
    
    TYRA_LOG("HUDRenderer: Initialized");
}

void HUDRenderer::cleanup(Tyra::TextureRepository* textureRepo) {
    textureRepo->freeBySprite(heartSprite);
    textureRepo->freeBySprite(minimapSprite);
    font.free(textureRepo);
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
    float heartSize = 32.0f;  // Display size
    float heartSpacing = 34.0f;
    
    // Hearts texture is 128x32 with 3 hearts at 32px each
    // Full heart at x=0, half heart at x=32, empty heart at x=64
    float fullHeartOffset = 0.0f;
    float halfHeartOffset = 32.0f;
    float emptyHeartOffset = 64.0f;
    
    // Draw hearts (each heart = 2 health points)
    int numHearts = maxHealth / 2;
    
    for (int i = 0; i < numHearts; i++) {
        int heartHealth = currentHealth - (i * 2);
        float offsetX;
        
        if (heartHealth >= 2) {
            offsetX = fullHeartOffset;
        } else if (heartHealth == 1) {
            offsetX = halfHeartOffset;
        } else {
            offsetX = emptyHeartOffset;
        }
        
        Tyra::Sprite heart;
        heart.id = heartSprite.id;
        heart.mode = Tyra::SpriteMode::MODE_REPEAT;
        heart.size = Tyra::Vec2(heartSize, heartSize);
        heart.position = Tyra::Vec2(startX + i * heartSpacing, startY);
        heart.offset = Tyra::Vec2(offsetX, 0.0f);
        
        renderer->render(heart);
    }
}

void HUDRenderer::renderMinimap(Tyra::Renderer2D* renderer, const Level* level) {
    // Minimap position (top-right corner, below level text)
    float mapStartX = screenWidth - 110.0f;
    float mapStartY = 45.0f;
    float roomSize = 10.0f;
    float roomSpacing = 2.0f;
    
    int gridWidth = level->getGridWidth();
    int gridHeight = level->getGridHeight();
    int currentX = level->getCurrentGridX();
    int currentY = level->getCurrentGridY();
    
    // Draw visited rooms as colored rectangles
    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            const Room* room = level->getRoom(x, y);
            if (!room || !room->exists() || !room->isVisited()) {
                continue;
            }
            
            float drawX = mapStartX + x * (roomSize + roomSpacing);
            float drawY = mapStartY + y * (roomSize + roomSpacing);
            
            // Choose color based on room type
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
                        roomColor = Tyra::Color(150, 150, 150);  // Gray
                    } else {
                        roomColor = Tyra::Color(100, 150, 200);  // Blue
                    }
                    break;
            }
            
            // Draw current room with white border
            if (x == currentX && y == currentY) {
                Tyra::Sprite borderSprite;
                borderSprite.id = minimapSprite.id;
                borderSprite.mode = Tyra::SpriteMode::MODE_STRETCH;
                borderSprite.size = Tyra::Vec2(roomSize + 4.0f, roomSize + 4.0f);
                borderSprite.position = Tyra::Vec2(drawX - 2.0f, drawY - 2.0f);
                borderSprite.color = Tyra::Color(255, 255, 255);
                renderer->render(borderSprite);
            }
            
            // Draw room rectangle
            Tyra::Sprite roomSprite;
            roomSprite.id = minimapSprite.id;
            roomSprite.mode = Tyra::SpriteMode::MODE_STRETCH;
            roomSprite.size = Tyra::Vec2(roomSize, roomSize);
            roomSprite.position = Tyra::Vec2(drawX, drawY);
            roomSprite.color = roomColor;
            renderer->render(roomSprite);
        }
    }
}

void HUDRenderer::renderLevelIndicator(Tyra::Renderer2D* renderer, const Level* level) {
    // Position level text above minimap (top-right area)
    int textX = static_cast<int>(screenWidth - 85.0f);
    int textY = 10;
    
    int levelNum = level->getLevelNumber();
    
    // Draw "Level X" text with shadow for visibility
    std::string levelText = "Level " + std::to_string(levelNum);
    font.drawTextWithShadow(levelText, textX, textY, 
                            Tyra::Color(255, 255, 255),   // White text
                            Tyra::Color(0, 0, 0),         // Black shadow
                            2.0f);                        // Scale
}

}  // namespace CanalUx