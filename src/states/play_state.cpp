#include "states/play_state.hpp"

namespace Elliot {

PlayState::PlayState(Tyra::Engine* t_engine) : State(t_engine) {
    state = STATE_GAME;
    _wantFinish = false;
}

PlayState::~PlayState() {}

void PlayState::onStart() {
    TYRA_LOG("Starting Game State...");
    
    // 1. Initialize Map
    map = std::make_unique<Map>(engine);
    map->init();

    // 2. Initialize Player
    player = std::make_unique<Player>(engine);
}

void PlayState::update() {
    player->update();

    engine->renderer.beginFrame();
    
    // Render Map FIRST (Background)
    map->render();
    
    // Render Player SECOND (Foreground)
    player->render();
    
    engine->renderer.endFrame();
}

GlobalStateType PlayState::onFinish() {
    return STATE_EXIT;
}

}