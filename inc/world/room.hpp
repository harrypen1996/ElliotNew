/*
 * CanalUx - Room Class
 * Represents a single room/section of the canal
 */

#pragma once

#include <tyra>
#include <vector>
#include "core/constants.hpp"

namespace CanalUx {

// Forward declaration
class RoomGenerator;

enum class RoomType {
    NORMAL,
    START,
    END,         // Exit lock to next level
    BOSS,
    SHOP,
    SPECIAL      // Item room
};

/**
 * A single room in the canal level
 * Contains tile maps for water, land, and scenery layers
 */
class Room {
public:
    Room();
    ~Room();

    // Generation
    void generate(RoomGenerator* generator, int width, int height);
    void createDoor(int directionX, int directionY);
    void completeClear();  // Called when all enemies defeated

    // Tile access (const for use in collision detection)
    int getLandTile(int x, int y) const;
    int getWaterTile(int x, int y) const;
    int getSceneryTile(int x, int y) const;
    void setSceneryTile(int x, int y, int tileId);

    // Map access
    const std::vector<std::vector<int>>& getLandMap() const { return landMap; }
    const std::vector<std::vector<int>>& getWaterMap() const { return waterMap; }
    const std::vector<std::vector<int>>& getSceneryMap() const { return sceneryMap; }

    // Properties
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    RoomType getType() const { return type; }
    void setType(RoomType t) { type = t; }
    
    // Door states
    bool hasLeftDoor() const { return openLeft; }
    bool hasRightDoor() const { return openRight; }
    bool hasTopDoor() const { return openTop; }
    bool hasBottomDoor() const { return openBottom; }

    // Room state
    bool exists() const { return roomExists; }
    void markExists() { roomExists = true; }
    bool isGenerated() const { return generated; }
    bool isCleared() const { return cleared; }
    void setCleared(bool value) { cleared = value; }
    bool isVisited() const { return visited; }
    void setVisited(bool value) { visited = value; }

    // Spawn point for player entering this room
    Tyra::Vec2 getSpawnPoint(int entryDirection) const;

private:
    // Tile maps (y, x indexing)
    std::vector<std::vector<int>> landMap;
    std::vector<std::vector<int>> waterMap;
    std::vector<std::vector<int>> sceneryMap;

    // Dimensions
    int width;
    int height;

    // Door states
    bool openLeft;
    bool openRight;
    bool openTop;
    bool openBottom;

    // Room state
    RoomType type;
    bool roomExists;  // Part of level layout
    bool generated;   // Tiles have been generated
    bool cleared;
    bool visited;
};

}  // namespace CanalUx