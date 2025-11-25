/*
 * CanalUx - Room Renderer Implementation
 */

#include "rendering/room_renderer.hpp"
#include "world/room.hpp"
#include "core/camera.hpp"

namespace CanalUx {

RoomRenderer::RoomRenderer()
    : visibleTilesX(0),
      visibleTilesY(0) {
}

RoomRenderer::~RoomRenderer() {
}

void RoomRenderer::init(Tyra::TextureRepository* textureRepo) {
    // Load terrain tileset
    auto filepath = Tyra::FileUtils::fromCwd("all2.png");
    auto* texture = textureRepo->add(filepath);
    
    // Set up terrain sprite for tile rendering
    terrainSprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    terrainSprite.size = Tyra::Vec2(Constants::TILE_SIZE, Constants::TILE_SIZE);
    texture->addLink(terrainSprite.id);
    
    // Calculate visible tiles based on screen size
    visibleTilesX = static_cast<int>(Constants::SCREEN_WIDTH / Constants::TILE_SIZE) + 2;
    visibleTilesY = static_cast<int>(Constants::SCREEN_HEIGHT / Constants::TILE_SIZE) + 2;
    
    TYRA_LOG("RoomRenderer: Initialized, visible tiles: ", visibleTilesX, "x", visibleTilesY);
}

void RoomRenderer::cleanup(Tyra::TextureRepository* textureRepo) {
    textureRepo->freeBySprite(terrainSprite);
}

void RoomRenderer::render(Tyra::Renderer2D* renderer, const Room* room, const Camera* camera) {
    if (!room || !camera) return;
    
    float offsetX = camera->getOffsetX();
    float offsetY = camera->getOffsetY();
    
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
                renderer->render(getTileSprite(screenX, screenY, waterTile - 1));
            }
            
            // Render land layer (walls/terrain)
            int landTile = room->getLandTile(tileX, tileY);
            if (landTile > 0) {
                renderer->render(getTileSprite(screenX, screenY, landTile - 1));
            }
            
            // Render scenery layer (obstacles, decorations)
            int sceneryTile = room->getSceneryTile(tileX, tileY);
            if (sceneryTile > 0) {
                renderer->render(getTileSprite(screenX, screenY, sceneryTile - 1));
            }
        }
    }
}

Tyra::Sprite RoomRenderer::getTileSprite(int screenX, int screenY, int tileIndex) {
    Tyra::Sprite sprite;
    sprite.size = Tyra::Vec2(Constants::TILE_SIZE, Constants::TILE_SIZE);
    sprite.position = Tyra::Vec2(static_cast<float>(screenX), static_cast<float>(screenY));
    sprite.id = terrainSprite.id;
    sprite.mode = Tyra::SpriteMode::MODE_REPEAT;
    
    // Calculate offset in tileset (16 tiles per row in a 512px wide texture)
    int tilesPerRow = 512 / Constants::TILE_SIZE;
    int column = tileIndex % tilesPerRow;
    int row = tileIndex / tilesPerRow;
    
    sprite.offset = Tyra::Vec2(
        static_cast<float>(column * Constants::TILE_SIZE),
        static_cast<float>(row * Constants::TILE_SIZE)
    );
    
    return sprite;
}

}  // namespace CanalUx