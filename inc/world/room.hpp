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
 * Obstacle placed during gameplay (e.g., trolley from Lock Keeper)
 */
struct RoomObstacle {
    Tyra::Vec2 position;      // Tile position
    int type;                 // Obstacle type (0 = trolley, etc.)
    bool blocksPlayer;        // Player cannot walk through
    bool blocksEnemies;       // Enemies cannot walk through
    bool blocksPlayerShots;   // Player projectiles collide
    bool blocksEnemyShots;    // Enemy projectiles collide
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
    
    // Dynamic obstacles (runtime)
    void addObstacle(const RoomObstacle& obstacle);
    void clearObstacles();
    const std::vector<RoomObstacle>& getObstacles() const { return obstacles; }
    bool hasObstacleAt(float x, float y) const;
    
    // Arena shrinking (for Lock Keeper boss)
    void shrinkArena(float amount);  // Shrink from edges
    float getArenaMinX() const { return arenaMinX; }
    float getArenaMaxX() const { return arenaMaxX; }
    float getArenaMinY() const { return arenaMinY; }
    float getArenaMaxY() const { return arenaMaxY; }
    void resetArenaBounds();

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
    
    // Dynamic obstacles
    std::vector<RoomObstacle> obstacles;
    
    // Arena bounds (can shrink during boss fights)
    float arenaMinX;
    float arenaMaxX;
    float arenaMinY;
    float arenaMaxY;

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