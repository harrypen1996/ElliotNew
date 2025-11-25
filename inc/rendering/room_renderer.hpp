/*
 * CanalUx - Room Renderer
 * Handles rendering of room tiles (water, land, scenery layers)
 */

#pragma once

#include <tyra>
#include "core/constants.hpp"

namespace CanalUx {

class Room;
class Camera;

class RoomRenderer {
public:
    RoomRenderer();
    ~RoomRenderer();

    // Initialize with texture repository
    void init(Tyra::TextureRepository* textureRepo);
    
    // Clean up textures
    void cleanup(Tyra::TextureRepository* textureRepo);

    // Render the room
    void render(Tyra::Renderer2D* renderer, const Room* room, const Camera* camera);

private:
    Tyra::Sprite getTileSprite(int screenX, int screenY, int tileIndex);

    Tyra::Sprite terrainSprite;
    
    int visibleTilesX;
    int visibleTilesY;
};

}  // namespace CanalUx