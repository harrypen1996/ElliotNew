/*
 * CanalUx - Level Class
 * Represents one canal section (floor) containing multiple connected rooms
 */

#pragma once

#include <tyra>
#include <vector>
#include <random>
#include "world/room.hpp"
#include "world/room_generator.hpp"
#include "core/constants.hpp"

namespace CanalUx {

/**
 * A Level represents one "floor" or canal section.
 * Contains a grid of rooms connected by doors (locks).
 * The player must navigate through to find the boss room and exit.
 */
class Level {
public:
    explicit Level(int levelNumber);
    ~Level();

    // Generation
    void generate();
    void printDebugMap() const;

    // Room access
    Room* getRoom(int gridX, int gridY);
    const Room* getRoom(int gridX, int gridY) const;
    Room* getCurrentRoom() { return currentRoom; }
    Room* getStartRoom();
    
    // Navigation
    void setCurrentRoom(int gridX, int gridY);
    bool canMoveToRoom(int gridX, int gridY) const;
    
    // Grid position tracking
    int getCurrentGridX() const { return currentGridX; }
    int getCurrentGridY() const { return currentGridY; }
    int getStartGridX() const { return startGridX; }
    int getStartGridY() const { return startGridY; }

    // Level info
    int getLevelNumber() const { return levelNumber; }
    int getRoomCount() const { return roomCount; }
    
    // Grid access for minimap
    int getGridWidth() const { return GRID_WIDTH; }
    int getGridHeight() const { return GRID_HEIGHT; }

private:
    // Generation phases
    void initializeGrid();
    void generateRoomLayout();
    void assignSpecialRooms();
    void generateRoomTiles();
    
    // Layout helpers
    bool isValidGridPosition(int x, int y) const;
    bool roomExists(int x, int y) const;
    bool canPlaceRoom(int x, int y, int fromDirX, int fromDirY) const;
    int countAdjacentRooms(int x, int y) const;
    float distanceFromStart(int x, int y) const;

    // Grid of rooms
    std::vector<std::vector<Room>> grid;
    
    // Generation state
    std::vector<Tyra::Vec2> roomQueue;  // Rooms to expand from
    RoomGenerator roomGenerator;
    std::mt19937 rng;  // Random number generator

    // Level properties
    int levelNumber;
    int roomCount;
    int targetRoomCount;
    
    // Current position
    Room* currentRoom;
    int currentGridX;
    int currentGridY;
    
    // Start position
    int startGridX;
    int startGridY;

    // Grid dimensions
    static constexpr int GRID_WIDTH = 9;
    static constexpr int GRID_HEIGHT = 8;
};

}  // namespace CanalUx