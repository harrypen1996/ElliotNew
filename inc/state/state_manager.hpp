#pragma once
#include <vector>
#include <algorithm>
#include "state.hpp"

namespace Elliot {

template <typename StateTypeT>
class StateManager {
 public:
  StateManager(const StateTypeT& t_initialState, const StateTypeT& t_exitState) {
    stateInitialized = false;
    currentState = t_initialState;
    exitState = t_exitState;
  }

  ~StateManager() { 
      for (auto* state : states) delete state; 
      states.clear();
  }

  bool finished() const { return currentState == exitState; }

  void add(State<StateTypeT>* state) { states.push_back(state); }

  void update() {
    for (auto& state : states) {
      if (state->getState() == currentState) {
        if (state->wantFinish()) {
          currentState = state->onFinish();
          stateInitialized = false;
          break;
        }
        if (!stateInitialized) {
          state->onStart();
          stateInitialized = true;
        }
        state->update();
        break;
      }
    }
  }

 private:
  bool stateInitialized;
  std::vector<State<StateTypeT>*> states;
  StateTypeT currentState, exitState;
};

}