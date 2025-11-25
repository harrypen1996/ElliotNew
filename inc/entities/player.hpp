#pragma once
#include "entities/entity.hpp"

namespace Elliot {

class Player : public Entity {
 public:
  // Explicitly use Tyra::Engine
  Player(Tyra::Engine* t_engine);
  ~Player();

  void update() override;

 private:
  void handleInput();
};

}