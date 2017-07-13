#include "Keyframe.hpp"

int Keyframe::counter = 0;

Keyframe::Keyframe() : id(counter++) {}
