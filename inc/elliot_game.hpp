#pragma once
#include <tyra>
#include "state/state_manager.hpp"
#include "state/global_state_type.hpp"

namespace Elliot {

class ElliotGame : public Tyra::Game {
 public:
  // Explicitly use Tyra::Engine
  ElliotGame(Tyra::Engine* engine);
  ~ElliotGame();

  void init() override;
  void loop() override;

 private:
  Tyra::Engine* engine;
  StateManager<GlobalStateType> stateManager;
};

}