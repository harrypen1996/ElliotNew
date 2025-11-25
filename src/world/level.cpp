/*
 * CanalUx - Level Class Implementation
 */

#include "world/level.hpp"
#include <algorithm>
#include <cmath>

namespace CanalUx {

Level::Level(int lvlNum)
    : levelNumber(lvlNum),
      roomCount(0),
      targetRoomCount(0),
      currentRoom(nullptr),
      currentGridX(0),
      currentGridY(0),
      startGridX(GRID_WIDTH / 2),
      startGridY(GRID_HEIGHT / 2) {
    
    // Static counter ensures each Level instance gets a different seed
    static unsigned int instanceCounter = 0;
    instanceCounter++;
    
    // Seed RNG using multiple sources for better randomness on PS2
    std::random_device rd;
    unsigned int seed = rd();
    
    // Mix in time-based value for additional entropy
    Tyra::Timer timer;
    timer.prime();
    seed ^= static_cast<unsigned int>(timer.getTimeDelta() * 1000);
    seed ^= static_cast<unsigned int>(reinterpret_cast<uintptr_t>(&seed));
    seed ^= (instanceCounter * 2654435761u);  // Multiply by golden ratio prime
    
    rng.seed(seed);
    
    TYRA_LOG("Level RNG seed: ", seed, " (instance ", instanceCounter, ")");
    
    // More rooms as levels progress
    int baseRooms = Constants::MIN_ROOMS_PER_LEVEL;
    int bonusRooms = levelNumber * 2;
    std::uniform_int_distribution<int> roomDist(baseRooms + bonusRooms, 
                                                 baseRooms + bonusRooms + 4);
    targetRoomCount = roomDist(rng);
}

Level::~Level() {
}

void Level::generate() {
    TYRA_LOG("Level ", levelNumber, ": Starting generation (target: ", targetRoomCount, " rooms)");
    
    initializeGrid();
    generateRoomLayout();
    assignSpecialRooms();
    generateRoomTiles();
    
    // Set starting position
    currentGridX = startGridX;
    currentGridY = startGridY;
    currentRoom = getRoom(startGridX, startGridY);
    
    TYRA_LOG("Level ", levelNumber, ": Generation complete (", roomCount, " rooms)");
    printDebugMap();
}

void Level::initializeGrid() {
    grid.clear();
    grid.resize(GRID_HEIGHT, std::vector<Room>(GRID_WIDTH));
    roomQueue.clear();
    roomCount = 0;
}

void Level::generateRoomLayout() {
    // Direction vectors: Left, Right, Up, Down
    const int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    // Place start room in center - just mark it as existing, don't generate tiles yet
    grid[startGridY][startGridX].setType(RoomType::START);
    grid[startGridY][startGridX].markExists();  // New method - just marks room as part of layout
    roomQueue.push_back(Tyra::Vec2(startGridX, startGridY));
    roomCount = 1;
    
    TYRA_LOG("Placed start room at (", startGridX, ", ", startGridY, ")");
    
    // BFS-style expansion
    int iterations = 0;
    while (roomCount < targetRoomCount && !roomQueue.empty() && iterations < 100) {
        iterations++;
        std::vector<Tyra::Vec2> newRooms;
        
        // Shuffle the queue for more organic generation
        std::shuffle(roomQueue.begin(), roomQueue.end(), rng);
        
        for (const auto& roomPos : roomQueue) {
            int x = static_cast<int>(roomPos.x);
            int y = static_cast<int>(roomPos.y);
            
            // Try each direction
            std::vector<int> dirOrder = {0, 1, 2, 3};
            std::shuffle(dirOrder.begin(), dirOrder.end(), rng);
            
            for (int dirIdx : dirOrder) {
                if (roomCount >= targetRoomCount) break;
                
                int newX = x + directions[dirIdx][0];
                int newY = y + directions[dirIdx][1];
                int fromDirX = -directions[dirIdx][0];
                int fromDirY = -directions[dirIdx][1];
                
                // Check if we can place a room here
                if (!canPlaceRoom(newX, newY, fromDirX, fromDirY)) {
                    continue;
                }
                
                // Random chance to skip (creates more interesting layouts)
                std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
                if (chanceDist(rng) < 0.3f) {  // Reduced skip chance
                    continue;
                }
                
                // Create the new room
                Room& newRoom = grid[newY][newX];
                newRoom.setType(RoomType::NORMAL);
                newRoom.markExists();
                
                // Create doors between rooms
                grid[y][x].createDoor(directions[dirIdx][0], directions[dirIdx][1]);
                newRoom.createDoor(fromDirX, fromDirY);
                
                newRooms.push_back(Tyra::Vec2(newX, newY));
                roomCount++;
                
                TYRA_LOG("Placed room at (", newX, ", ", newY, ") from (", x, ", ", y, ")");
            }
        }
        
        // Add new rooms to queue for next iteration
        for (const auto& newRoom : newRooms) {
            roomQueue.push_back(newRoom);
        }
        
        // Remove rooms that have been fully processed (surrounded or no valid expansion)
        roomQueue.erase(
            std::remove_if(roomQueue.begin(), roomQueue.end(), 
                [this](const Tyra::Vec2& pos) {
                    return countAdjacentRooms(static_cast<int>(pos.x), 
                                              static_cast<int>(pos.y)) >= 3;
                }),
            roomQueue.end()
        );
        
        TYRA_LOG("Iteration ", iterations, ": ", roomCount, " rooms, ", roomQueue.size(), " in queue");
    }
    
    // Mark dead-end rooms (only one connection) as potential end rooms
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (roomExists(x, y) && grid[y][x].getType() == RoomType::NORMAL) {
                int connections = countAdjacentRooms(x, y);
                if (connections == 1) {
                    grid[y][x].setType(RoomType::END);
                    TYRA_LOG("Marked end room at (", x, ", ", y, ")");
                }
            }
        }
    }
}

void Level::assignSpecialRooms() {
    std::vector<std::pair<int, int>> endRooms;
    
    // Find all end rooms (dead ends)
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (roomExists(x, y) && grid[y][x].getType() == RoomType::END) {
                endRooms.push_back({x, y});
            }
        }
    }
    
    if (endRooms.empty()) {
        TYRA_LOG("Warning: No end rooms found for special room assignment");
        return;
    }
    
    // Sort by distance from start (furthest first)
    std::sort(endRooms.begin(), endRooms.end(), 
        [this](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return distanceFromStart(a.first, a.second) > 
                   distanceFromStart(b.first, b.second);
        });
    
    // Assign special room types
    int assigned = 0;
    for (const auto& [x, y] : endRooms) {
        Room& room = grid[y][x];
        
        switch (assigned) {
            case 0:
                // Furthest room is the boss room
                room.setType(RoomType::BOSS);
                TYRA_LOG("Boss room at (", x, ", ", y, ") - distance: ", distanceFromStart(x, y));
                break;
            case 1:
                // Second furthest is shop
                room.setType(RoomType::SHOP);
                TYRA_LOG("Shop room at (", x, ", ", y, ")");
                break;
            case 2:
                // Third is special item room
                room.setType(RoomType::SPECIAL);
                TYRA_LOG("Special room at (", x, ", ", y, ")");
                break;
            default:
                // Any additional dead ends stay as END type
                break;
        }
        assigned++;
        
        // Only assign up to 3 special rooms
        if (assigned >= 3) break;
    }
}

void Level::generateRoomTiles() {
    std::uniform_int_distribution<int> widthDist(Constants::ROOM_MIN_WIDTH, 
                                                  Constants::ROOM_MAX_WIDTH);
    std::uniform_int_distribution<int> heightDist(Constants::ROOM_MIN_HEIGHT, 
                                                   Constants::ROOM_MAX_HEIGHT);
    
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (!roomExists(x, y)) continue;
            
            Room& room = grid[y][x];
            
            // Start room is already generated, skip it
            if (room.isGenerated()) continue;
            
            // Determine room size - all rooms must be at least screen size
            int width, height;
            
            switch (room.getType()) {
                case RoomType::START:
                case RoomType::BOSS:
                    // Fixed size for important rooms (screen size)
                    width = Constants::ROOM_MIN_WIDTH;
                    height = Constants::ROOM_MIN_HEIGHT;
                    break;
                case RoomType::SHOP:
                case RoomType::SPECIAL:
                    // Fixed screen size for special rooms
                    width = Constants::ROOM_MIN_WIDTH;
                    height = Constants::ROOM_MIN_HEIGHT;
                    break;
                default:
                    // Random size for normal rooms
                    // Make one dimension standard and vary the other
                    if (rng() % 2 == 0) {
                        width = widthDist(rng);
                        height = Constants::ROOM_MIN_HEIGHT;
                    } else {
                        width = Constants::ROOM_MIN_WIDTH;
                        height = heightDist(rng);
                    }
                    break;
            }
            
            // Ensure even dimensions for door placement
            width = (width / 2) * 2;
            height = (height / 2) * 2;
            
            room.generate(&roomGenerator, width, height);
        }
    }
}

bool Level::isValidGridPosition(int x, int y) const {
    return x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT;
}

bool Level::roomExists(int x, int y) const {
    if (!isValidGridPosition(x, y)) return false;
    return grid[y][x].exists();
}

bool Level::canPlaceRoom(int x, int y, int fromDirX, int fromDirY) const {
    // Must be valid grid position
    if (!isValidGridPosition(x, y)) return false;
    
    // Must not already have a room
    if (grid[y][x].exists()) return false;
    
    // Check adjacent cells (excluding the direction we came from)
    // We don't want rooms to cluster too much
    const int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (const auto& dir : directions) {
        // Skip the direction we came from
        if (dir[0] == fromDirX && dir[1] == fromDirY) continue;
        
        int checkX = x + dir[0];
        int checkY = y + dir[1];
        
        if (roomExists(checkX, checkY)) {
            // Would create a cluster - don't allow
            return false;
        }
    }
    
    return true;
}

int Level::countAdjacentRooms(int x, int y) const {
    int count = 0;
    const int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (const auto& dir : directions) {
        if (roomExists(x + dir[0], y + dir[1])) {
            count++;
        }
    }
    
    return count;
}

float Level::distanceFromStart(int x, int y) const {
    float dx = static_cast<float>(x - startGridX);
    float dy = static_cast<float>(y - startGridY);
    return std::sqrt(dx * dx + dy * dy);
}

Room* Level::getRoom(int gridX, int gridY) {
    if (!isValidGridPosition(gridX, gridY)) return nullptr;
    if (!grid[gridY][gridX].exists()) return nullptr;
    return &grid[gridY][gridX];
}

const Room* Level::getRoom(int gridX, int gridY) const {
    if (!isValidGridPosition(gridX, gridY)) return nullptr;
    if (!grid[gridY][gridX].exists()) return nullptr;
    return &grid[gridY][gridX];
}

Room* Level::getStartRoom() {
    return getRoom(startGridX, startGridY);
}

Room* Level::getBossRoom() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x].isGenerated() && grid[y][x].getType() == RoomType::BOSS) {
                return &grid[y][x];
            }
        }
    }
    return nullptr;
}

void Level::getBossRoomGridPos(int& outX, int& outY) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x].isGenerated() && grid[y][x].getType() == RoomType::BOSS) {
                outX = x;
                outY = y;
                return;
            }
        }
    }
    outX = startGridX;
    outY = startGridY;
}

void Level::setCurrentRoom(int gridX, int gridY) {
    Room* room = getRoom(gridX, gridY);
    if (room) {
        currentGridX = gridX;
        currentGridY = gridY;
        currentRoom = room;
    }
}

bool Level::canMoveToRoom(int gridX, int gridY) const {
    return roomExists(gridX, gridY);
}

void Level::printDebugMap() const {
    TYRA_LOG("=== Level ", levelNumber, " Map ===");
    
    for (int y = 0; y < GRID_HEIGHT; y++) {
        std::string row;
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (!grid[y][x].exists()) {
                row += "[ ]";
            } else {
                switch (grid[y][x].getType()) {
                    case RoomType::START:   row += "[S]"; break;
                    case RoomType::BOSS:    row += "[B]"; break;
                    case RoomType::SHOP:    row += "[$]"; break;
                    case RoomType::SPECIAL: row += "[*]"; break;
                    case RoomType::END:     row += "[E]"; break;
                    default:                row += "[.]"; break;
                }
            }
        }
        TYRA_LOG(row);
    }
    
    TYRA_LOG("===================");
}

}  // namespace CanalUx