#pragma once

#include <functional>
#include <map>
#include <iostream>

template <typename T>
class StateManager {
  T currentState;
  std::map<T, std::function<void(T)> > transitions;
public:
  StateManager(T start);
  void addState(T state, std::function<void(T)> onEntry);
  void transition(T state);

  T getCurrentState();
};

/**
 * StateManater<T> Definitions
 */

#include <cassert>

template <typename T>
StateManager<T>::StateManager(T start) : currentState(start) {}

template <typename T>
void StateManager<T>::addState(T state, std::function<void(T)> onEntry) {
  transitions.insert(std::pair<T, std::function<void(T)>>(state, onEntry));
}

template <typename T>
void StateManager<T>::transition(T nextState) {
  assert(transitions.find(nextState) != transitions.end());

  if (currentState == nextState) {
    return;
  }

  T oldState = currentState;
  currentState = nextState;

  // Invoke the onEntry function
  transitions.at(currentState)(oldState);
}

template <typename T>
T StateManager<T>::getCurrentState() {
  return currentState;
}
