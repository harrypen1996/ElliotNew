#include <tyra>
#include "elliot_game.hpp"

int main() {
  Tyra::EngineOptions options;
  
  // Set to true if you want to see logs on your PC while running on PS2
  options.loadUsbDriver = false; 
  options.writeLogsToFile = false;

  Tyra::Engine engine(options);

  Elliot::ElliotGame game(&engine);
  engine.run(&game);
  return 0;
}