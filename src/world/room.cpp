/*
 * CanalUx - Room Class Implementation
 */

#include "world/room.hpp"
#include "world/room_generator.hpp"

namespace CanalUx {

Room::Room()
    : width(0),
      height(0),
      openLeft(false),
      openRight(false),
      openTop(false),
      openBottom(false),
      type(RoomType::NORMAL),
      roomExists(false),
      generated(false),
      cleared(false),
      visited(false) {
}

Room::~Room() {
}

void Room::generate(RoomGenerator* generator, int w, int h) {
    width = w;
    height = h;
    
    // Start and shop rooms are pre-cleared (no enemies)
    if (type == RoomType::START || type == RoomType::SHOP) {
        cleared = true;
    }
    
    // Generate tile maps using the generator
    if (generator) {
        waterMap = generator->generateWater(width, height, 
                                            openLeft, openRight, openTop, openBottom);
        landMap = generator->generateLand(width, height,
                                          openLeft, openRight, openTop, openBottom);
        sceneryMap = generator->generateScenery(width, height,
                                                openLeft, openRight, openTop, openBottom,
                                                cleared);
    } else {
        // Fallback: create empty maps
        landMap.resize(height, std::vector<int>(width, 0));
        waterMap.resize(height, std::vector<int>(width, 0));
        sceneryMap.resize(height, std::vector<int>(width, 0));
    }
    
    generated = true;
}

void Room::createDoor(int directionX, int directionY) {
    if (directionX == -1) openLeft = true;
    if (directionX == 1) openRight = true;
    if (directionY == -1) openTop = true;
    if (directionY == 1) openBottom = true;
}

void Room::completeClear() {
    if (cleared) return;
    
    cleared = true;
    
    // Open doors by removing scenery tiles that block them
    int midWidth = width / 2;
    int midHeight = height / 2;
    
    if (openLeft) {
        sceneryMap[midHeight - 1][1] = 0;
        sceneryMap[midHeight][1] = 0;
    }
    if (openRight) {
        sceneryMap[midHeight - 1][width - 2] = 0;
        sceneryMap[midHeight][width - 2] = 0;
    }
    if (openTop) {
        sceneryMap[1][midWidth - 1] = 0;
        sceneryMap[1][midWidth] = 0;
    }
    if (openBottom) {
        sceneryMap[height - 2][midWidth - 1] = 0;
        sceneryMap[height - 2][midWidth] = 0;
    }
}

int Room::getLandTile(int x, int y) const {
    if (y >= 0 && y < height && x >= 0 && x < width) {
        return landMap[y][x];
    }
    return 0;  // Out of bounds = empty
}

int Room::getWaterTile(int x, int y) const {
    if (y >= 0 && y < height && x >= 0 && x < width) {
        return waterMap[y][x];
    }
    return 0;
}

int Room::getSceneryTile(int x, int y) const {
    if (y >= 0 && y < height && x >= 0 && x < width) {
        return sceneryMap[y][x];
    }
    return 0;
}

void Room::setSceneryTile(int x, int y, int tileId) {
    if (y >= 0 && y < height && x >= 0 && x < width) {
        sceneryMap[y][x] = tileId;
    }
}

Tyra::Vec2 Room::getSpawnPoint(int entryDirection) const {
    // entryDirection: 0=left, 1=right, 2=top, 3=bottom
    // Player spawns near the door they entered from
    float spawnX, spawnY;
    
    switch (entryDirection) {
        case 0:  // Entered from left
            spawnX = 2.0f;
            spawnY = height / 2.0f - 0.5f;
            break;
        case 1:  // Entered from right
            spawnX = width - 3.0f;
            spawnY = height / 2.0f - 0.5f;
            break;
        case 2:  // Entered from top
            spawnX = width / 2.0f - 0.5f;
            spawnY = 2.0f;
            break;
        case 3:  // Entered from bottom
            spawnX = width / 2.0f - 0.5f;
            spawnY = height - 3.0f;
            break;
        default:  // Start room - center
            spawnX = width / 2.0f - 0.5f;
            spawnY = height / 2.0f - 0.5f;
            break;
    }
    
    return Tyra::Vec2(spawnX, spawnY);
}

}  // namespace CanalUx