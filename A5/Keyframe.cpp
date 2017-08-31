#include "Keyframe.hpp"

int Keyframe::counter = 0;

Keyframe::Keyframe() : id(counter++), sound("") {}

Keyframe::Keyframe(std::string sound) : id(counter++), sound(sound) {}
