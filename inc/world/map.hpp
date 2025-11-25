#pragma once
#include <tyra>
#include <vector>
#include <memory>
#include "constants.hpp"

namespace Elliot {

// Simple constants for tile types (matches your old logic logic)
// You can expand this later based on your 'all2.png' layout
const int TILE_WATER = 45; 
const int TILE_GRASS = 23; // Just a guess based on your old code, adjust as needed

class Map {
 public:
  Map(Tyra::Engine* t_engine);
  ~Map();

  void init();
  void render(); // In future, pass a CameraOffset here

  // Checks if a tile at grid coordinates (x, y) is walkable
  bool isWalkable(int x, int y);

 private:
  Tyra::Engine* engine;
  
  // One sprite used to draw all tiles (we just move it around)
  std::unique_ptr<Tyra::Sprite> tileSprite;
  
  // The grid data: 2D vector of Integers (Tile IDs)
  // Outer vector = Rows (Y), Inner vector = Cols (X)
  std::vector<std::vector<int>> grid;

  // Helper to convert Tile ID to Texture Offset
  Tyra::Vec2 getTextureOffset(int tileId);
  
  // Dimensions
  int cols;
  int rows;
};

}