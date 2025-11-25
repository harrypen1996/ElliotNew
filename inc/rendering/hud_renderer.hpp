/*
 * CanalUx - HUD Renderer
 * Handles rendering of UI elements (health, minimap, etc.)
 */

#pragma once

#include <tyra>
#include "core/constants.hpp"
#include "components/stats.hpp"
#include "rendering/font.hpp"

namespace CanalUx {

class Player;
class Level;

class HUDRenderer {
public:
    HUDRenderer();
    ~HUDRenderer();

    // Initialize and load HUD textures
    void init(Tyra::TextureRepository* textureRepo, Tyra::Renderer2D* renderer2D);
    
    // Cleanup textures
    void cleanup(Tyra::TextureRepository* textureRepo);

    // Render HUD elements
    void render(Tyra::Renderer2D* renderer, const Player* player, const Level* level);

private:
    void renderHealth(Tyra::Renderer2D* renderer, const Player* player);
    void renderMinimap(Tyra::Renderer2D* renderer, const Level* level);
    void renderLevelIndicator(Tyra::Renderer2D* renderer, const Level* level);

    Tyra::Sprite heartSprite;    // Heart sprites for health
    Tyra::Sprite minimapSprite;  // Minimap room rectangles (uses pixel texture)
    
    Font font;  // Text rendering
    
    float screenWidth;
    float screenHeight;
};

}  // namespace CanalUx