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
    
    // Load GUI sheet (bottom-right quadrant) for minimap icons
    filepath = Tyra::FileUtils::fromCwd("gui_br.png");
    auto* guiTexture = textureRepo->add(filepath);
    
    minimapSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    minimapSprite.size = Tyra::Vec2(48.0f, 48.0f);
    guiTexture->addLink(minimapSprite.id);
    
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
    
    // GUI sprite sheet offsets (512x512 gui_br.png - bottom-right quadrant)
    // Icons are roughly 48x48, positioned in columns around x=144 and x=192
    // Row positions: star ~240, green square ~288, blue orb ~336, heart ~384, purple ~432
    
    Tyra::Vec2 greenSquareOffset(192.0f, 288.0f);   // Green square
    Tyra::Vec2 redHeartOffset(144.0f, 384.0f);      // Red heart
    Tyra::Vec2 yellowStarOffset(144.0f, 240.0f);    // Yellow star
    Tyra::Vec2 blueOrbOffset(144.0f, 336.0f);       // Blue orb
    Tyra::Vec2 goldCoinOffset(192.0f, 384.0f);      // Gold $ coin
    Tyra::Vec2 graySquareOffset(96.0f, 48.0f);      // Gray button (top area)
    
    // Draw visited rooms
    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            const Room* room = level->getRoom(x, y);
            if (!room || !room->exists() || !room->isVisited()) {
                continue;
            }
            
            float drawX = mapStartX + x * (roomSize + roomSpacing);
            float drawY = mapStartY + y * (roomSize + roomSpacing);
            
            // Choose sprite offset based on room type
            Tyra::Vec2 spriteOffset = greenSquareOffset;
            
            switch (room->getType()) {
                case RoomType::START:
                    spriteOffset = blueOrbOffset;
                    break;
                case RoomType::BOSS:
                    spriteOffset = redHeartOffset;
                    break;
                case RoomType::SHOP:
                    spriteOffset = goldCoinOffset;
                    break;
                case RoomType::SPECIAL:
                    spriteOffset = yellowStarOffset;
                    break;
                default:
                    if (room->isCleared()) {
                        spriteOffset = graySquareOffset;
                    } else {
                        spriteOffset = greenSquareOffset;
                    }
                    break;
            }
            
            // Draw current room indicator (brighter border) first if this is current room
            if (x == currentX && y == currentY) {
                Tyra::Sprite borderSprite;
                borderSprite.id = minimapSprite.id;
                borderSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
                borderSprite.size = Tyra::Vec2(roomSize + 4.0f, roomSize + 4.0f);
                borderSprite.position = Tyra::Vec2(drawX - 2.0f, drawY - 2.0f);
                borderSprite.offset = spriteOffset;
                borderSprite.color = Tyra::Color(255, 255, 255);
                renderer->render(borderSprite);
            }
            
            // Draw room icon
            Tyra::Sprite roomSprite;
            roomSprite.id = minimapSprite.id;
            roomSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
            roomSprite.size = Tyra::Vec2(roomSize, roomSize);
            roomSprite.position = Tyra::Vec2(drawX, drawY);
            roomSprite.offset = spriteOffset;
            
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