#include "entities/player.hpp"

namespace Elliot {

// Explicitly use Tyra::Engine
Player::Player(Tyra::Engine* t_engine) : Entity(t_engine) {
    position.set(200.0F, 200.0F);

    auto& textureRepo = engine->renderer.getTextureRepository();
    // Tyra::FileUtils
    auto filepath = Tyra::FileUtils::fromCwd("player.png");
    auto* texture = textureRepo.add(filepath);

    // Tyra::Sprite
    sprite = std::make_unique<Tyra::Sprite>();
    // Tyra::SpriteMode
    sprite->mode = Tyra::SpriteMode::MODE_STRETCH;
    sprite->size.set(32.0F, 32.0F);
    
    if (texture) {
        texture->addLink(sprite->id);
    }
}

Player::~Player() {}

void Player::update() {
    handleInput();
}

void Player::handleInput() {
    const auto& pressed = engine->pad.getPressed();

    if (pressed.DpadLeft)  position.x -= 3.0F;
    if (pressed.DpadRight) position.x += 3.0F;
    if (pressed.DpadUp)    position.y -= 3.0F;
    if (pressed.DpadDown)  position.y += 3.0F;

    if (engine->pad.getClicked().Cross) {
        isSubmerged = !isSubmerged;
    }
}

}