#include "world/map.hpp"

namespace Elliot {

Map::Map(Tyra::Engine* t_engine) : engine(t_engine) {
    cols = 20; // Width of the canal room
    rows = 15; // Height of the canal room
}

Map::~Map() {
}

void Map::init() {
    // 1. Load the Texture
    auto& textureRepo = engine->renderer.getTextureRepository();
    auto filepath = Tyra::FileUtils::fromCwd("all2.png"); 
    auto* texture = textureRepo.add(filepath);

    // 2. Setup the Shared Sprite
    tileSprite = std::make_unique<Tyra::Sprite>();
    tileSprite->mode = Tyra::SpriteMode::MODE_REPEAT; // Important for Texture Atlas
    tileSprite->size.set(TILE_SIZE, TILE_SIZE);
    
    if(texture) {
        texture->addLink(tileSprite->id);
    }

    // 3. Generate a Simple Canal (Test Data)
    // We resize the grid to [rows][cols]
    grid.resize(rows, std::vector<int>(cols));

    for(int y = 0; y < rows; y++) {
        for(int x = 0; x < cols; x++) {
            // Simple Logic: Edges are Land, Middle is Water
            if (x < 2 || x >= cols - 2) {
                grid[y][x] = TILE_GRASS;
            } else {
                grid[y][x] = TILE_WATER;
            }
        }
    }
}

void Map::render() {
    // Loop through the grid and draw
    for(int y = 0; y < rows; y++) {
        for(int x = 0; x < cols; x++) {
            int tileId = grid[y][x];
            
            // 1. Position the sprite on screen
            float posX = x * TILE_SIZE;
            float posY = y * TILE_SIZE;
            tileSprite->position.set(posX, posY);

            // 2. Pick the correct image from the spritesheet
            tileSprite->offset = getTextureOffset(tileId);

            // 3. Render
            engine->renderer.renderer2D.render(tileSprite.get());
        }
    }
}

Tyra::Vec2 Map::getTextureOffset(int tileId) {
    // Assuming the texture is 512x512 and tiles are 32x32
    // That means there are 16 columns per row
    int tilesPerRow = 16; 
    
    int col = tileId % tilesPerRow;
    int row = tileId / tilesPerRow;

    return Tyra::Vec2(col * 32.0F, row * 32.0F);
}

bool Map::isWalkable(int x, int y) {
    // Boundary check
    if (x < 0 || y < 0 || x >= cols || y >= rows) return false;

    int id = grid[y][x];
    // Example logic: Only water is walkable if submerged? 
    // For now, let's say everything is walkable
    return true;
}

}