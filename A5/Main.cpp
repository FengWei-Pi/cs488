#include "A5.hpp"
#include <AL/alut.h>

#include <iostream>
using namespace std;

int main( int argc, char **argv ) {
  std::string title("Assignment 5");
  assert(alutInitWithoutContext(&argc, argv) == AL_TRUE);
  CS488Window::launch(argc, argv, new A5(), 1024, 768, title);

  return 0;
}
