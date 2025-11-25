#pragma once
#include <tyra>
#include <memory>

namespace Elliot {

class Entity {
 public:
  // Explicitly use Tyra::Engine
  Entity(Tyra::Engine* t_engine) : engine(t_engine) {
      position = Tyra::Vec2(0.0f, 0.0f);
      isSubmerged = false;
  }
  virtual ~Entity() {}

  virtual void update() = 0;
  
  virtual void render() {
      if (sprite) {
          sprite->position = position;
          sprite->color.a = isSubmerged ? 64.0F : 128.0F;
          engine->renderer.renderer2D.render(sprite.get());
      }
  }

  // Explicitly use Tyra::Vec2
  Tyra::Vec2 getPosition() const { return position; }

 protected:
  // Explicitly use Tyra::Engine
  Tyra::Engine* engine;
  // Explicitly use Tyra::Sprite
  std::unique_ptr<Tyra::Sprite> sprite;
  // Explicitly use Tyra::Vec2
  Tyra::Vec2 position;
  bool isSubmerged;
};

}