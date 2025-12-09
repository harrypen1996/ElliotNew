/*
 * CanalUx - Room Generator Implementation
 */

#include "world/room_generator.hpp"
#include "world/room.hpp"

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

void RoomGenerator::generateNannySideDoors(Room* room, int numDoorsPerSide) {
    if (!room) return;
    
    int height = room->getHeight();
    
    // Clear any existing side doors
    room->clearSideDoors();
    
    // Calculate the gauntlet zone (where barges can spawn)
    // Door structure is 6 tiles tall:
    //   doorY - 3: top corner frame
    //   doorY - 2: top edge frame
    //   doorY - 1: opening (passable)
    //   doorY:     opening (passable)
    //   doorY + 1: bottom edge frame
    //   doorY + 2: bottom corner frame
    
    // Top buffer: boss at y~2 with 4-tile height, ends around y=6
    // Want significant gap between boss and first door
    // First door's top frame (doorY-3) should have clearance from boss area
    float topBuffer = 16.0f;  // First doorY must be at least this (pushes doors down)
    
    // Bottom buffer: player starts at bottom (y = height-3)
    // Last door's doorY+2 should be < height-4, meaning doorY < height-6
    float bottomBuffer = 6.0f;
    
    float minDoorY = topBuffer;
    float maxDoorY = static_cast<float>(height) - bottomBuffer;
    
    if (numDoorsPerSide <= 0) return;
    
    // Fixed spacing between doors (not distributed across range)
    float doorSpacing = 6.0f;  // 6 tiles between door centers (minimum for frame clearance)
    
    // Calculate total space needed for all doors
    float totalSpaceNeeded = doorSpacing * (numDoorsPerSide - 1);
    
    // Start doors from topBuffer and place them with fixed spacing
    // This clusters them together rather than spreading across the room
    for (int i = 0; i < numDoorsPerSide; i++) {
        float doorY = minDoorY + doorSpacing * i;
        
        // Make sure we don't exceed the bottom boundary
        if (doorY > maxDoorY) {
            break;  // Can't fit more doors
        }
        
        int doorYInt = static_cast<int>(doorY);
        
        // Create left door at this Y
        room->addSideDoor(doorY, true);
        createSideDoorOpening(room, doorYInt, true);
        
        // Create right door at the same Y (mirrored)
        room->addSideDoor(doorY, false);
        createSideDoorOpening(room, doorYInt, false);
    }
}

void RoomGenerator::createSideDoorOpening(Room* room, int doorCenterY, bool isLeftWall) {
    // Create a door opening following the same pattern as the normal left/right doors
    // Door structure (6 tiles tall):
    //   doorCenterY - 3: top frame
    //   doorCenterY - 2: top frame edge
    //   doorCenterY - 1: opening (passable)
    //   doorCenterY:     opening (passable)
    //   doorCenterY + 1: bottom frame edge
    //   doorCenterY + 2: bottom frame
    
    int width = room->getWidth();
    
    if (isLeftWall) {
        // LEFT WALL DOOR (columns 0 and 1)
        // Clear the opening (2 tiles high)
        for (int y = doorCenterY - 1; y <= doorCenterY; y++) {
            room->setLandTile(0, y, 0);
            room->setLandTile(1, y, 0);
        }
        
        // Add water to the opening
        for (int y = doorCenterY - 1; y <= doorCenterY; y++) {
            room->setWaterTile(0, y, WATER_FILL);
            room->setWaterTile(1, y, WATER_FILL);
        }
        
        // Add water transition above opening
        room->setWaterTile(0, doorCenterY - 2, WATER_DOOR_TRANSITION);
        room->setWaterTile(1, doorCenterY - 2, WATER_TRANSITION_TL);
        
        // Add water below opening
        room->setWaterTile(0, doorCenterY + 1, WATER_FILL);
        
        // Add door frame tiles (land layer)
        // Top frame
        room->setLandTile(0, doorCenterY - 3, LAND_DOOR_TOP_L);
        room->setLandTile(0, doorCenterY - 2, LAND_EDGE_TOP);
        room->setLandTile(1, doorCenterY - 2, LAND_DOOR_EDGE_TOP_L);
        
        // Bottom frame
        room->setLandTile(0, doorCenterY + 2, LAND_DOOR_BOTTOM_L);
        room->setLandTile(0, doorCenterY + 1, LAND_EDGE_BOTTOM);
        room->setLandTile(1, doorCenterY + 1, LAND_DOOR_EDGE_BOTTOM_L);
        
    } else {
        // RIGHT WALL DOOR (columns width-2 and width-1)
        int col1 = width - 2;  // Inner column
        int col2 = width - 1;  // Outer wall column
        
        // Clear the opening (2 tiles high)
        for (int y = doorCenterY - 1; y <= doorCenterY; y++) {
            room->setLandTile(col1, y, 0);
            room->setLandTile(col2, y, 0);
        }
        
        // Add water to the opening
        for (int y = doorCenterY - 1; y <= doorCenterY; y++) {
            room->setWaterTile(col1, y, WATER_FILL);
            room->setWaterTile(col2, y, WATER_FILL);
        }
        
        // Add water transition above opening
        room->setWaterTile(col1, doorCenterY - 2, WATER_TRANSITION_TR);
        room->setWaterTile(col2, doorCenterY - 2, WATER_DOOR_SIDE);
        
        // Add water below opening
        room->setWaterTile(col2, doorCenterY + 1, WATER_FILL);
        
        // Add door frame tiles (land layer)
        // Top frame
        room->setLandTile(col2, doorCenterY - 3, LAND_WALL_RIGHT);  // Keep wall corner
        room->setLandTile(col1, doorCenterY - 2, LAND_DOOR_EDGE_TOP_R);
        room->setLandTile(col2, doorCenterY - 2, LAND_EDGE_TOP);
        
        // Bottom frame
        room->setLandTile(col2, doorCenterY + 2, LAND_WALL_RIGHT);  // Keep wall corner
        room->setLandTile(col1, doorCenterY + 1, LAND_DOOR_EDGE_BOTTOM_R);
        room->setLandTile(col2, doorCenterY + 1, LAND_EDGE_BOTTOM);
    }
}

}  // namespace CanalUx