/*
 * CanalUx - Room Generator Implementation
 */

#include "world/room_generator.hpp"

namespace CanalUx {

RoomGenerator::RoomGenerator() {
}

RoomGenerator::~RoomGenerator() {
}

std::vector<std::vector<int>> RoomGenerator::generateWater(
    int width, int height,
    bool doorLeft, bool doorRight,
    bool doorTop, bool doorBottom) {
    
    std::vector<std::vector<int>> water(height, std::vector<int>(width, 0));
    
    // Fill the interior with water
    for (int y = 2; y < height - 2; y++) {
        for (int x = 2; x < width - 2; x++) {
            water[y][x] = WATER_FILL;
        }
    }
    
    // Top water edge (row 1)
    water[1][1] = WATER_CORNER_TL;
    water[1][width - 2] = WATER_CORNER_TR;
    for (int x = 2; x < width - 2; x++) {
        water[1][x] = WATER_EDGE_TOP;
    }
    
    // Side water edges
    for (int y = 2; y < height - 2; y++) {
        water[y][1] = WATER_EDGE_LEFT;
        water[y][width - 2] = WATER_EDGE_RIGHT;
    }
    
    // Calculate midpoints for doors
    int midWidth = width / 2;
    int midHeight = height / 2;
    
    // Handle door openings in water layer
    if (doorLeft) {
        for (int y = midHeight - 1; y <= midHeight; y++) {
            water[y][0] = WATER_FILL;
            water[y][1] = WATER_FILL;
        }
        water[midHeight - 2][0] = WATER_DOOR_TRANSITION;
        water[midHeight - 2][1] = WATER_TRANSITION_TL;
        water[midHeight + 1][0] = WATER_FILL;
    }
    
    if (doorRight) {
        for (int y = midHeight - 1; y <= midHeight; y++) {
            water[y][width - 2] = WATER_FILL;
            water[y][width - 1] = WATER_FILL;
        }
        water[midHeight - 2][width - 2] = WATER_TRANSITION_TR;
        water[midHeight - 2][width - 1] = WATER_DOOR_SIDE;
        water[midHeight + 1][width - 1] = WATER_FILL;
    }
    
    if (doorTop) {
        for (int x = midWidth - 1; x <= midWidth; x++) {
            water[0][x] = WATER_FILL;
            water[1][x] = WATER_FILL;
        }
        water[0][midWidth - 2] = WATER_DOOR_LEFT;
        water[1][midWidth - 2] = WATER_TRANSITION_TL;
        water[0][midWidth + 1] = WATER_EDGE_RIGHT;
        water[1][midWidth + 1] = WATER_TRANSITION_TR;
    }
    
    if (doorBottom) {
        for (int x = midWidth - 1; x <= midWidth; x++) {
            water[height - 2][x] = WATER_FILL;
            water[height - 1][x] = WATER_FILL;
        }
        water[height - 2][midWidth - 2] = WATER_DOOR_LEFT;
        water[height - 1][midWidth - 2] = WATER_DOOR_LEFT;
        water[height - 2][midWidth + 1] = WATER_DOOR_RIGHT;
        water[height - 1][midWidth + 1] = WATER_DOOR_RIGHT;
    }
    
    return water;
}

std::vector<std::vector<int>> RoomGenerator::generateLand(
    int width, int height,
    bool doorLeft, bool doorRight,
    bool doorTop, bool doorBottom) {
    
    std::vector<std::vector<int>> land(height, std::vector<int>(width, 0));
    
    // Top and bottom walls
    for (int x = 0; x < width; x++) {
        land[0][x] = LAND_WALL_TOP;
        land[height - 1][x] = LAND_WALL_BOTTOM;
    }
    
    // Left and right walls
    for (int y = 0; y < height; y++) {
        land[y][0] = LAND_WALL_LEFT;
        land[y][width - 1] = LAND_WALL_RIGHT;
    }
    
    // Outer corners
    land[0][0] = LAND_CORNER_TL;
    land[0][width - 1] = LAND_WALL_TOP;
    land[height - 1][0] = LAND_CORNER_BL;
    land[height - 1][width - 1] = LAND_WALL_BOTTOM;
    
    // Inner corners (the grass/water transition)
    land[1][1] = LAND_INNER_TL;
    land[1][width - 2] = LAND_INNER_TR;
    land[height - 2][1] = LAND_INNER_BL;
    land[height - 2][width - 2] = LAND_INNER_BR;
    
    // Inner edges (row/column 1 and width/height - 2)
    for (int x = 2; x < width - 2; x++) {
        land[1][x] = LAND_EDGE_TOP;
        land[height - 2][x] = LAND_EDGE_BOTTOM;
    }
    for (int y = 2; y < height - 2; y++) {
        land[y][1] = LAND_EDGE_LEFT;
        land[y][width - 2] = LAND_EDGE_RIGHT;
    }
    
    // Calculate midpoints for doors
    int midWidth = width / 2;
    int midHeight = height / 2;
    
    // Handle door openings
    if (doorLeft) {
        for (int y = midHeight - 1; y <= midHeight; y++) {
            land[y][0] = 0;
            land[y][1] = 0;
        }
        land[midHeight - 3][0] = LAND_DOOR_TOP_L;
        land[midHeight - 2][0] = LAND_EDGE_TOP;
        land[midHeight - 2][1] = LAND_DOOR_EDGE_TOP_L;
        land[midHeight + 2][0] = LAND_DOOR_BOTTOM_L;
        land[midHeight + 1][0] = LAND_EDGE_BOTTOM;
        land[midHeight + 1][1] = LAND_DOOR_EDGE_BOTTOM_L;
    }
    
    if (doorRight) {
        for (int y = midHeight - 1; y <= midHeight; y++) {
            land[y][width - 2] = 0;
            land[y][width - 1] = 0;
        }
        land[midHeight - 2][width - 2] = LAND_DOOR_EDGE_TOP_R;
        land[midHeight - 3][width - 1] = LAND_WALL_RIGHT;
        land[midHeight - 2][width - 1] = LAND_EDGE_TOP;
        land[midHeight + 1][width - 2] = LAND_DOOR_EDGE_BOTTOM_R;
        land[midHeight + 1][width - 1] = LAND_EDGE_BOTTOM;
        land[midHeight + 2][width - 1] = LAND_WALL_RIGHT;
    }
    
    if (doorTop) {
        for (int x = midWidth - 1; x <= midWidth; x++) {
            land[0][x] = 0;
            land[1][x] = 0;
        }
        land[0][midWidth - 3] = LAND_DOOR_TOP_L;
        land[0][midWidth - 2] = LAND_EDGE_LEFT;
        land[1][midWidth - 2] = LAND_DOOR_EDGE_TOP_L;
        land[0][midWidth + 2] = LAND_DOOR_TOP_R;
        land[0][midWidth + 1] = LAND_EDGE_RIGHT;
        land[1][midWidth + 1] = LAND_DOOR_EDGE_TOP_R;
    }
    
    if (doorBottom) {
        for (int x = midWidth - 1; x <= midWidth; x++) {
            land[height - 2][x] = 0;
            land[height - 1][x] = 0;
        }
        land[height - 2][midWidth - 2] = LAND_DOOR_EDGE_BOTTOM_L;
        land[height - 1][midWidth - 2] = LAND_EDGE_LEFT;
        land[height - 1][midWidth - 3] = LAND_DOOR_BOTTOM_L;
        land[height - 2][midWidth + 1] = LAND_DOOR_EDGE_BOTTOM_R;
        land[height - 1][midWidth + 1] = LAND_EDGE_RIGHT;
        land[height - 1][midWidth + 2] = LAND_DOOR_BOTTOM_R;
    }
    
    return land;
}

std::vector<std::vector<int>> RoomGenerator::generateScenery(
    int width, int height,
    bool doorLeft, bool doorRight,
    bool doorTop, bool doorBottom,
    bool cleared) {
    
    std::vector<std::vector<int>> scenery(height, std::vector<int>(width, 0));
    
    // If room is not cleared, place lock gates blocking doors
    if (!cleared) {
        int midWidth = width / 2;
        int midHeight = height / 2;
        
        if (doorLeft) {
            scenery[midHeight - 1][1] = LOCK_GATE_LEFT;
            scenery[midHeight][1] = LOCK_GATE_RIGHT;
        }
        if (doorRight) {
            scenery[midHeight - 1][width - 2] = LOCK_GATE_LEFT;
            scenery[midHeight][width - 2] = LOCK_GATE_RIGHT;
        }
        if (doorTop) {
            scenery[1][midWidth - 1] = LOCK_GATE_TOP;
            scenery[1][midWidth] = LOCK_GATE_BOTTOM;
        }
        if (doorBottom) {
            scenery[height - 2][midWidth - 1] = LOCK_GATE_TOP;
            scenery[height - 2][midWidth] = LOCK_GATE_BOTTOM;
        }
    }
    
    // TODO: Add random obstacles (shopping carts, logs, etc.)
    
    return scenery;
}

}  // namespace CanalUx