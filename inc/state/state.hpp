#pragma once
#include <tyra>

namespace Elliot {

template <typename StateTypeT>
class State {
 public:
  // Explicitly use Tyra::Engine
  State(Tyra::Engine* t_engine) : engine(t_engine) {}
  virtual ~State() {}

  virtual const StateTypeT& getState() const = 0;
  virtual const bool& wantFinish() const = 0;
  virtual void onStart() = 0;
  virtual void update() = 0;
  virtual StateTypeT onFinish() = 0;

 protected:
  // Explicitly use Tyra::Engine
  Tyra::Engine* engine;
};

}