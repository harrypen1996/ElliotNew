/*
 * CanalUx - Font Class
 * Renders text using a sprite-based font
 * Based on Tyra engine example by Guido Diego Quispe Robles
 */

#pragma once

#include <tyra>
#include <array>
#include <string>

#define FONT_CHAR_SIZE 96

namespace CanalUx {

class Font {
public:
    Font();
    ~Font();
    
    void load(Tyra::TextureRepository* repository, Tyra::Renderer2D* renderer);
    void free(Tyra::TextureRepository* repository);
    void drawText(const char* text, int x, int y, Tyra::Color color);
    void drawText(const std::string& text, int x, int y, Tyra::Color color);
    void drawTextWithShadow(const std::string& text, int x, int y, Tyra::Color color, Tyra::Color shadowColor, float scale = 1.0f);

private:
    static const int chars[FONT_CHAR_SIZE];
    static const int charWidths[FONT_CHAR_SIZE];

    Tyra::Renderer2D* renderer2D;
    Tyra::Sprite allFont;
    std::array<Tyra::Sprite, FONT_CHAR_SIZE> font;
};

}  // namespace CanalUx