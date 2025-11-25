/*
 * CanalUx - A canal-themed roguelike for PlayStation 2
 * Built with Tyra Game Engine
 */

#include <tyra>
#include "core/game.hpp"

int main() {
    Tyra::EngineOptions options;
    Tyra::Engine engine(options);
    CanalUx::Game game(&engine);
    engine.run(&game);
    return 0;
}