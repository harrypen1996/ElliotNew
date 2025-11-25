#include "elliot_game.hpp"
#include "states/play_state.hpp"

namespace Elliot {

// Explicitly use Tyra::Engine
ElliotGame::ElliotGame(Tyra::Engine* t_engine)
    : engine(t_engine), stateManager(STATE_GAME, STATE_EXIT) {}

ElliotGame::~ElliotGame() {}

void ElliotGame::init() {
  // Explicitly use Tyra::Color
  engine->renderer.setClearScreenColor(Tyra::Color(32.0F, 32.0F, 32.0F));
  stateManager.add(new PlayState(engine));
}

void ElliotGame::loop() {
  stateManager.update();

  if (stateManager.finished()) {
      // Handle exit
  }
}

}