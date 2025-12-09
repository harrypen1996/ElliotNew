/*
 * CanalUx - Room Generator
 * Generates tile maps for individual rooms
 */

#pragma once

#include <vector>
#include "core/constants.hpp"

namespace CanalUx {

/**
 * Generates the tile data for rooms.
 * Creates three layers:
 * - Water: The canal water (background)
 * - Land: Walls and terrain (collision)
 * - Scenery: Obstacles, decorations, door blockers
 */
// Forward declaration
class Room;

class RoomGenerator {
public:
    RoomGenerator();
    ~RoomGenerator();

    // Generate tile maps for a room
    // Returns 2D vectors indexed as [y][x]
    std::vector<std::vector<int>> generateWater(int width, int height,
                                                 bool doorLeft, bool doorRight,
                                                 bool doorTop, bool doorBottom);
    
    std::vector<std::vector<int>> generateLand(int width, int height,
                                                bool doorLeft, bool doorRight,
                                                bool doorTop, bool doorBottom);
    
    std::vector<std::vector<int>> generateScenery(int width, int height,
                                                   bool doorLeft, bool doorRight,
                                                   bool doorTop, bool doorBottom,
                                                   bool cleared);
    
    // Generate side doors for Nanny boss room (barge spawn points)
    // Creates openings on left and right walls at various Y positions
    // numDoorsPerSide: how many doors on each wall (doors are 6 tiles tall, need spacing)
    void generateNannySideDoors(Room* room, int numDoorsPerSide);
    
    // Place danger signs next to a door (indicates boss room ahead)
    // direction: 0=left, 1=right, 2=top, 3=bottom
    void placeDangerSigns(Room* room, int direction);

private:
    // Helper to create a single side door opening
    void createSideDoorOpening(Room* room, int doorCenterY, bool isLeftWall);
    // Tile indices for your tileset (all2.png)
    // These map to positions in a 16x16 grid of 32x32 tiles (512x512 texture)
    
    // Water tiles
    static constexpr int WATER_FILL = 45;
    static constexpr int WATER_EDGE_TOP = 125;
    static constexpr int WATER_EDGE_LEFT = 44;
    static constexpr int WATER_EDGE_RIGHT = 46;
    static constexpr int WATER_CORNER_TL = 28;
    static constexpr int WATER_CORNER_TR = 30;
    static constexpr int WATER_TRANSITION_TL = 110;
    static constexpr int WATER_TRANSITION_TR = 108;
    static constexpr int WATER_DOOR_LEFT = 94;
    static constexpr int WATER_DOOR_RIGHT = 92;
    static constexpr int WATER_DOOR_SIDE = 29;
    static constexpr int WATER_DOOR_TRANSITION = 109;
    
    // Land/wall tiles  
    static constexpr int LAND_CORNER_TL = 193;
    static constexpr int LAND_CORNER_TR = 162;
    static constexpr int LAND_CORNER_BL = 225;
    static constexpr int LAND_CORNER_BR = 130;
    static constexpr int LAND_INNER_TL = 101;
    static constexpr int LAND_INNER_TR = 102;
    static constexpr int LAND_INNER_BL = 117;
    static constexpr int LAND_INNER_BR = 118;
    static constexpr int LAND_EDGE_TOP = 38;
    static constexpr int LAND_EDGE_BOTTOM = 6;
    static constexpr int LAND_EDGE_LEFT = 23;
    static constexpr int LAND_EDGE_RIGHT = 21;
    static constexpr int LAND_WALL_TOP = 162;
    static constexpr int LAND_WALL_BOTTOM = 130;
    static constexpr int LAND_WALL_LEFT = 147;
    static constexpr int LAND_WALL_RIGHT = 22;
    
    // Door transition tiles
    static constexpr int LAND_DOOR_TOP_L = 163;
    static constexpr int LAND_DOOR_TOP_R = 161;
    static constexpr int LAND_DOOR_BOTTOM_L = 131;
    static constexpr int LAND_DOOR_BOTTOM_R = 129;
    static constexpr int LAND_DOOR_EDGE_TOP_L = 39;
    static constexpr int LAND_DOOR_EDGE_TOP_R = 37;
    static constexpr int LAND_DOOR_EDGE_BOTTOM_L = 7;
    static constexpr int LAND_DOOR_EDGE_BOTTOM_R = 5;
    
    // Scenery tiles (door blockers - canal lock gates)
    static constexpr int LOCK_GATE_TOP = 63;
    static constexpr int LOCK_GATE_BOTTOM = 64;
    static constexpr int LOCK_GATE_LEFT = 140;
    static constexpr int LOCK_GATE_RIGHT = 156;
    
    // Danger sign for boss room entrances (using lock gate sprite for now)
    static constexpr int DANGER_SIGN = 63;  // Same as LOCK_GATE_TOP
};

}  // namespace CanalUx