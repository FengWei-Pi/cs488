#pragma once
#include <cmath>
#include <limits>

inline bool isEqual(double x, double y) {
  const double epsilon = std::numeric_limits<double>::min();
  return std::abs(x - y) <= epsilon * std::abs(x);
}

#define PRINT_DEBUG 1

class debug {
public:
  static void print(double s) {
    if (!PRINT_DEBUG) {
      return;
    }

    std::cerr << s;
  }

  static void println(double s) {
    print(s);
    std::cerr << std::endl;
  }

  static void print(std::string s) {
    if (!PRINT_DEBUG) {
      return;
    }
    std::cerr << s;
  }

  static void println(std::string s) {
    if (!PRINT_DEBUG) {
      return;
    }
    print(s);
    std::cerr << std::endl;
  }

  static void print(glm::vec2 p) {
    if (!PRINT_DEBUG) {
      return;
    }
    std::cerr << "[" << p.x << ", " << p.y << "]";
  }

  static void println(glm::vec2 p) {
    if (!PRINT_DEBUG) {
      return;
    }
    print(p);
    std::cerr << std::endl;
  }

  static void print(glm::vec3 p) {
    if (!PRINT_DEBUG) {
      return;
    }
    std::cerr << "[" << p.x << ", " << p.y << ", " << p.z << "]";
  }

  static void println(glm::vec3 p) {
    if (!PRINT_DEBUG) {
      return;
    }
    print(p);
    std::cerr << std::endl;
  }

  static void print(glm::vec4 p) {
    if (!PRINT_DEBUG) {
      return;
    }
    std::cerr << "[" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << "]";
  }

  static void println(glm::vec4 p) {
    if (!PRINT_DEBUG) {
      return;
    }
    print(p);
    std::cerr << std::endl;
  }
};
