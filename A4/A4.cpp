#include <glm/ext.hpp>
#include <string>
#include <algorithm>

#include "A4.hpp"
#include <cmath>
#include "polyroots.hpp"
#include "Primitive.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"
#include "ray.hpp"

#define PRINT_DEBUG 0

void print(std::string s) {
  if (!PRINT_DEBUG) {
    return;
  }
  std::cerr << s;
}

void println(std::string s) {
  if (!PRINT_DEBUG) {
    return;
  }
  print(s);
  std::cerr << std::endl;
}

void print(glm::vec2 p) {
  if (!PRINT_DEBUG) {
    return;
  }
  std::cerr << "[" << p.x << ", " << p.y << "]";
}

void println(glm::vec2 p) {
  if (!PRINT_DEBUG) {
    return;
  }
  print(p);
  std::cerr << std::endl;
}

void print(glm::vec3 p) {
  if (!PRINT_DEBUG) {
    return;
  }
  std::cerr << "[" << p.x << ", " << p.y << ", " << p.z << "]";
}

void println(glm::vec3 p) {
  if (!PRINT_DEBUG) {
    return;
  }
  print(p);
  std::cerr << std::endl;
}

void print(glm::vec4 p) {
  if (!PRINT_DEBUG) {
    return;
  }
  std::cerr << "[" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << "]";
}

void println(glm::vec4 p) {
  if (!PRINT_DEBUG) {
    return;
  }
  print(p);
  std::cerr << std::endl;
}

void A4_Render(
    // What to render
    SceneNode * root,

    // Image to write to, set to a given width and height
    Image & image,

    // Viewing parameters
    const glm::vec3 & lookFrom,
    const glm::vec3 & view,
    const glm::vec3 & up,
    double fovy,

    // Lighting parameters
    const glm::vec3 & ambient,
    const std::list<Light *> & lights
) {

  std::cout << "Calling A4_Render(\n" <<
      "\t" << *root <<
      "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
      "\t" << "eye:  " << glm::to_string(lookFrom) << std::endl <<
      "\t" << "view: " << glm::to_string(view) << std::endl <<
      "\t" << "up:   " << glm::to_string(up) << std::endl <<
      "\t" << "fovy: " << fovy << std::endl <<
      "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
      "\t" << "lights{" << std::endl;

  for(const Light * light : lights) {
    std::cout << "\t\t" <<  *light << std::endl;
  }
  std::cout << "\t}" << std::endl;
  std:: cout <<")" << std::endl;

  const double nx = image.width();
  const double ny = image.height();
  const double d = 1;

  const double windowHeight = 2 * d * glm::tan(glm::radians(fovy/2));
  const double windowWidth = (nx / ny) * windowHeight;

  std::cerr << "nx: " << nx << std::endl;
  std::cerr << "ny: " << ny << std::endl;
  std::cerr << "windowHeight: " << windowHeight << std::endl;
  std::cerr << "windowWidth: " << windowWidth << std::endl;
  std::cerr << "d: " << d << std::endl;

  glm::mat4 T;

  T = glm::translate(T, glm::vec3(-nx / 2, -ny / 2, d));
  /** Why was x scaled by -1 */
  T = glm::scale(glm::vec3(-windowWidth / nx, windowHeight / ny, 1)) * T;

  const glm::vec3 w = glm::normalize(view);
  const glm::vec3 u = glm::normalize(glm::cross(up, w));
  const glm::vec3 v = glm::normalize(glm::cross(w, u));

  std::cerr << "w: ";
  println(w);
  std::cerr << "u: ";
  println(u);
  std::cerr << "v: ";
  println(v);

  glm::mat4 ViewToWorld {
    glm::vec4(u, 0),
    glm::vec4(v, 0),
    glm::vec4(w, 0),
    glm::vec4(lookFrom, 1),
  };

  T = ViewToWorld * T;

  int miss = 0;
  int hit = 0;

  for (uint y = 0; y < ny; y += 1) {
    for (uint x = 0; x < nx; x += 1) {
      glm::vec4 pixel = T * glm::vec4(x, y, 0, 1);
      Ray ray{glm::vec4(lookFrom, 1), pixel};

      for (SceneNode * child : root->children) {
        GeometryNode* node = (GeometryNode*) child;
        try {
          glm::vec4 intersection = node->m_primitive->intersect(ray);
          PhongMaterial *material = (PhongMaterial*) node->m_material;
          glm::vec3 fragColour{ambient};

          for (Light* light : lights) {
            Ray shadow{intersection, glm::vec4(light->position, 1)};

            bool intersectionFound = false;

            for (SceneNode * other : root->children) {
              if (intersectionFound) break;
              if (other != child) {
                GeometryNode* otherNode = (GeometryNode*) other;
                try {
                  otherNode->m_primitive->intersect(shadow);
                  intersectionFound = true;
                } catch (IntersectionNotFound& ex) {}
              }
            }

            if (intersectionFound) continue;

            // Light hits point

            // Direction from fragment to light source.
            glm::vec4 lp = glm::normalize(shadow.to - shadow.from);
            glm::vec3 l = glm::vec3(lp.x, lp.y, lp.z);

            // Direction from fragment to viewer (origin - fragPosition).
            glm::vec4 vp = glm::normalize(ray.from - ray.to);
            glm::vec3 v = glm::vec3(vp.x, vp.y, vp.z);

            NonhierSphere* sphere = (NonhierSphere*)node->m_primitive;

            glm::vec4 normal = glm::normalize(intersection - glm::vec4(sphere->m_pos, 1));
            glm::vec3 fragNormal = glm::vec3(normal.x, normal.y, normal.z);

            float n_dot_l = std::max((float)glm::dot(fragNormal, l), 0.0f);

            glm::vec3 diffuse;
            diffuse = material->m_kd * n_dot_l;

            glm::vec3 specular = glm::vec3(0.0);

            if (n_dot_l > 0.0) {
            // Halfway vector.
              glm::vec3 h = glm::normalize(v + l);
              float n_dot_h = std::max((float)glm::dot(fragNormal, h), 0.0f);

              specular = material->m_ks * std::pow(n_dot_h, material->m_shininess);
            }

            fragColour += light->colour * (diffuse + specular);
          }


          hit += 1;

          image(x, y, 0) = fragColour.r;
          image(x, y, 1) = fragColour.g;
          image(x, y, 2) = fragColour.b;
        } catch (IntersectionNotFound& ex) {
          miss += 1;
        }
      }
    }
  }

  std::cerr << "Misses: " << miss << std::endl;
  std::cerr << "Hits: " << hit << std::endl;
}
