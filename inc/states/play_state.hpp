#pragma once
#include <vector>
#include <memory>
#include "state/state.hpp"
#include "state/global_state_type.hpp"
#include "entities/player.hpp"
#include "world/map.hpp" // <--- Add this

namespace Elliot {

class PlayState : public State<GlobalStateType> {
 public:
  PlayState(Tyra::Engine* t_engine);
  ~PlayState();

  void onStart() override;
  void update() override;
  GlobalStateType onFinish() override;

  const GlobalStateType& getState() const override { return state; }
  const bool& wantFinish() const override { return _wantFinish; }

 private:
  GlobalStateType state;
  bool _wantFinish;
  
  std::unique_ptr<Player> player;
  std::unique_ptr<Map> map; // <--- Add this
};

}