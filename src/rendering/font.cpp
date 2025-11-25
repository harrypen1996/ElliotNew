/*
 * CanalUx - Font Class Implementation
 * Based on Tyra engine example by Guido Diego Quispe Robles
 */

#include "rendering/font.hpp"

namespace CanalUx {

const int Font::chars[FONT_CHAR_SIZE]{
    ' ', '!', '"', ' ', '$', '%', ' ', '{', '(', ')', ' ', '+', ',', '-',
    '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';',
    '<', '=', '>', '?', ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
    'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', ' ', ' ', ' ', ' ', ' ', ' ', 'a', 'b', 'c', 'd', 'e',
    'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '}', ']', '~', ' '};

const int Font::charWidths[FONT_CHAR_SIZE]{
    0, 1, 3, 0, 5, 9, 0, 9, 3, 3, 0, 5, 2, 2, 1, 4, 4, 2, 4, 4, 5, 4, 4, 4,
    4, 4, 1, 2, 4, 5, 4, 4, 0, 6, 5, 5, 5, 4, 4, 5, 5, 1, 4, 5, 4, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 6, 7, 5, 5, 4, 0, 0, 0, 0, 0, 0, 5, 4, 4, 4, 4, 3, 4,
    4, 1, 2, 4, 1, 7, 4, 4, 4, 4, 3, 4, 3, 4, 5, 7, 4, 4, 4, 2, 5, 2, 6, 0,
};

Font::Font() : renderer2D(nullptr) {
}

Font::~Font() {
}

void Font::load(Tyra::TextureRepository* repository, Tyra::Renderer2D* renderer) {
    renderer2D = renderer;

    float height = 16.0f;
    float width = 16.0f;

    allFont.mode = Tyra::SpriteMode::MODE_REPEAT;
    allFont.size = Tyra::Vec2(256, 128);  // 16 chars wide x 8 rows = 96 chars

    auto filepath = Tyra::FileUtils::fromCwd("earthboundFont.png");
    auto* texture = repository->add(filepath);
    texture->addLink(allFont.id);

    int column = 0;
    int row = 0;

    for (int i = 0; i < FONT_CHAR_SIZE; i++) {
        font[i].id = allFont.id;
        font[i].mode = Tyra::SpriteMode::MODE_REPEAT;
        font[i].size = Tyra::Vec2(width, height);
        font[i].offset = Tyra::Vec2(width * column, height * row);
        column++;

        if (column == 16) {
            row++;
            column = 0;
        }
    }
    
    TYRA_LOG("Font: Loaded");
}

void Font::free(Tyra::TextureRepository* repository) {
    repository->freeBySprite(allFont);
}

void Font::drawText(const char* text, int x, int y, Tyra::Color color) {
    drawText(std::string(text), x, y, color);
}

void Font::drawText(const std::string& text, int x, int y, Tyra::Color color) {
    if (!renderer2D) return;
    
    int sizeText = text.size();
    int offsetY = 0;
    int offsetX = 0;

    for (int i = 0; i < sizeText; i++) {
        int fontPos = text[i];
        Tyra::Sprite fontSpr = font[0];

        for (int j = 0; j < FONT_CHAR_SIZE; j++) {
            if (fontPos == chars[j]) {
                fontPos = j;
                fontSpr = font[j];
                fontSpr.color = color;
                fontSpr.position = Tyra::Vec2(static_cast<float>(x + offsetX), 
                                               static_cast<float>(y + offsetY));
                break;
            }
        }

        if (text[i] == '\n') {
            offsetY += 18;
            offsetX = 0;
        } else {
            if ((text[i] != ' ') && (text[i] != '\t')) {
                renderer2D->render(fontSpr);
                offsetX += charWidths[fontPos] + 2;
            } else {
                offsetX += 6;  // Space width
            }
        }
    }
}

void Font::drawTextWithShadow(const std::string& text, int x, int y, Tyra::Color color, Tyra::Color shadowColor, float scale) {
    if (!renderer2D) return;
    
    int shadowOffset = static_cast<int>(scale);
    if (shadowOffset < 1) shadowOffset = 1;
    
    int sizeText = text.size();
    
    // Draw shadow first
    int offsetY = 0;
    int offsetX = 0;
    
    for (int i = 0; i < sizeText; i++) {
        int charIndex = 0;
        Tyra::Sprite fontSpr = font[0];

        for (int j = 0; j < FONT_CHAR_SIZE; j++) {
            if (text[i] == chars[j]) {
                charIndex = j;
                fontSpr = font[j];
                fontSpr.color = shadowColor;
                fontSpr.scale = scale;  // Use scale property
                fontSpr.position = Tyra::Vec2(
                    static_cast<float>(x + offsetX + shadowOffset), 
                    static_cast<float>(y + offsetY + shadowOffset));
                break;
            }
        }

        if (text[i] == '\n') {
            offsetY += static_cast<int>(18 * scale);
            offsetX = 0;
        } else {
            if ((text[i] != ' ') && (text[i] != '\t')) {
                renderer2D->render(fontSpr);
                offsetX += static_cast<int>((charWidths[charIndex] + 2) * scale);
            } else {
                offsetX += static_cast<int>(6 * scale);
            }
        }
    }
    
    // Draw main text on top
    offsetY = 0;
    offsetX = 0;
    
    for (int i = 0; i < sizeText; i++) {
        int charIndex = 0;
        Tyra::Sprite fontSpr = font[0];

        for (int j = 0; j < FONT_CHAR_SIZE; j++) {
            if (text[i] == chars[j]) {
                charIndex = j;
                fontSpr = font[j];
                fontSpr.color = color;
                fontSpr.scale = scale;  // Use scale property
                fontSpr.position = Tyra::Vec2(
                    static_cast<float>(x + offsetX), 
                    static_cast<float>(y + offsetY));
                break;
            }
        }

        if (text[i] == '\n') {
            offsetY += static_cast<int>(18 * scale);
            offsetX = 0;
        } else {
            if ((text[i] != ' ') && (text[i] != '\t')) {
                renderer2D->render(fontSpr);
                offsetX += static_cast<int>((charWidths[charIndex] + 2) * scale);
            } else {
                offsetX += static_cast<int>(6 * scale);
            }
        }
    }
}

}  // namespace CanalUx