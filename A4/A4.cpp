#include <glm/ext.hpp>
#include <string>
#include <algorithm>
#include <cmath>
#include <functional>
#include <queue>
#include <tuple>
#include <limits>

#include "A4.hpp"
#include "polyroots.hpp"
#include "Primitive.hpp"
#include "PhongMaterial.hpp"
#include "ray.hpp"
#include "common.hpp"
#include "Mesh.hpp"

#include "SceneNode.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

void visit(SceneNode* root, std::function<void(const glm::mat4, const glm::mat4, GeometryNode&)> fn) {
  typedef std::tuple<const glm::mat4, const glm::mat4, SceneNode*> Node;

  std::queue<Node> fringe;
  fringe.push(Node{root->trans, root->invtrans, root});

  while (!fringe.empty()) {
    const glm::mat4 M = std::get<0>(fringe.front());
    const glm::mat4 invM = std::get<1>(fringe.front());
    SceneNode* parent = std::get<2>(fringe.front());

    fringe.pop();

    for(SceneNode* child : parent->children) {
      glm::mat4 T = M * child->trans;
      glm::mat4 invT = child->invtrans * invM;

      switch (child->m_nodeType) {
        case NodeType::GeometryNode: {
          GeometryNode* geometryNode = static_cast<GeometryNode*>(child);
          fn(T, invT, *geometryNode);
          break;
        }
        case NodeType::SceneNode: {
          break;
        }
        case NodeType::JointNode: {
          JointNode * jointNode = static_cast<JointNode*>(child);
          float xRotation = glm::radians(
            glm::clamp(
              jointNode->m_joint_x.init,
              jointNode->m_joint_x.min,
              jointNode->m_joint_x.max
            )
          );
          float yRotation = glm::radians(
            glm::clamp(
              jointNode->m_joint_y.init,
              jointNode->m_joint_y.min,
              jointNode->m_joint_y.max
            )
          );

          T = T * glm::rotate(glm::mat4(), yRotation, glm::vec3(0, 1, 0));
          T = T * glm::rotate(glm::mat4(), xRotation, glm::vec3(1, 0, 0));

          invT = glm::rotate(glm::mat4(), -yRotation, glm::vec3(0, 1, 0)) * invT;
          invT = glm::rotate(glm::mat4(), -xRotation, glm::vec3(1, 0, 0)) * invT;
          break;
        }
      }

      fringe.push(Node{T, invT, child});
    }
  }
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
  T = glm::scale(glm::vec3(-windowWidth / nx, -windowHeight / ny, 1)) * T;

  const glm::vec3 w = glm::normalize(view);
  const glm::vec3 u = glm::normalize(glm::cross(up, w));
  const glm::vec3 v = glm::normalize(glm::cross(w, u));

  glm::mat4 ViewToWorld {
    glm::vec4(u, 0),
    glm::vec4(v, 0),
    glm::vec4(w, 0),
    glm::vec4(lookFrom, 1),
  };

  T = ViewToWorld * T;

  int miss = 0;
  int hit = 0;

  double Infinity = std::numeric_limits<double>::infinity();

  uint ssFactor = 3;

  // for (unsigned int y = 241; y < 242; y += 1) {
  //   for (unsigned int x = 217; x < 218; x += 1) {
  for (double y = 0; y < ny; y += 1) {
    for (double x = 0; x < nx; x += 1) {
      printf("\r[%3d%%]", (int) ((y * ny + x) * 100 / (1.0 * nx * ny)));

      // if (!(y == 231 && x == 140) && !(y == 231 && x == 141)) continue;

      // std::cerr << "x: " << x << std::endl;
      // std::cerr << "y: " << y << std::endl;

      glm::vec3 fragColourSuperSample{0, 0, 0};

      int subpixelsSkipped = 0;

      for (double a = 0; a < ssFactor; a += 1) {
        for (double b = 0; b < ssFactor; b += 1) {
          glm::vec4 pixel = T * glm::vec4(x + a / ssFactor, y + b / ssFactor, 0, 1);
          Ray ray{glm::vec4(lookFrom, 1), pixel};

          double t = Infinity;
          glm::mat4 toWorld;
          glm::mat4 toModel;
          GeometryNode* n;

          visit(
            root,
            [ray, &toModel, &toWorld, &t, &n](
              const glm::mat4 modelToWorld,
              const glm::mat4 worldToModel,
              GeometryNode& node
            ) -> void {
              try {
                Ray mr{worldToModel * ray.from, worldToModel * ray.to};
                double intersection = node.m_primitive->intersect(mr);
                if (intersection < t) {
                  t = intersection;
                  toWorld = modelToWorld;
                  toModel = worldToModel;
                  n = &node;
                }
              } catch (IntersectionNotFound& ex) {}
            }
          );

          if (t == Infinity) {
            miss += 1;
            subpixelsSkipped += 1;
            continue;
          }

          GeometryNode& node = *n;
          Ray modelRay{toModel * glm::vec4(lookFrom, 1), toModel * pixel};
          glm::vec4 modelIntersection = modelRay.at(t);
          glm::vec4 worldIntersection = toWorld * modelIntersection;

          // std::cerr << "modelRay: " << glm::to_string(modelRay.from) << ", " << glm::to_string(modelRay.to) << std::endl;
          // std::cerr << "t: " << t << std::endl;
          // std::cerr << "worldIntersection: " << glm::to_string(worldIntersection) << std::endl;
          // std::cerr << "modelIntersection: " << glm::to_string(modelIntersection) << std::endl;
          // std::cerr << "Calculated worldIntersection, calculating normal: " << x << ", " << y <<  std::endl;
          // std::cerr << "m_nodeId: " << node.m_nodeId << std::endl;

          PhongMaterial *material = (PhongMaterial*) node.m_material;
          glm::vec3 modelNormal = glm::vec3(node.m_primitive->getNormal(modelIntersection)); // In model space

          fragColourSuperSample += ambient;

          // std::cerr << "modelNormal:" <<  glm::to_string(modelNormal) << std::endl;

          for (Light* light : lights) {
            Ray worldShadowRay{toWorld * modelIntersection, glm::vec4(light->position, 1)}; // In world coordinates

            bool intersectionFound = false;

            visit(
              root,
              [&intersectionFound, worldIntersection, worldShadowRay, node, light] (
                const glm::mat4 otherToWorld,
                const glm::mat4 otherToModel,
                GeometryNode& other
              ) -> void {
                if (intersectionFound) return;
                Ray otherModelShadowRay{otherToModel * worldShadowRay.from, otherToModel * worldShadowRay.to};
                try {
                  double otherT = other.m_primitive->intersect(otherModelShadowRay);
                  glm::vec4 otherModelIntersection = otherModelShadowRay.at(otherT);
                  glm::vec4 otherWorldIntersection = otherToWorld * otherModelIntersection;


                  // std::cerr << "otherWorldIntersection: " << glm::to_string(otherWorldIntersection) << std::endl;
                  // std::cerr << "other m_nodeId: " << other.m_nodeId << std::endl;
                  // std::cerr << "distance: " << std::fabs(glm::length(worldIntersection - otherWorldIntersection)) << std::endl;

                  if (std::fabs(glm::length(worldIntersection - otherWorldIntersection)) < 0.01) {
                    // Error, you've intersected the same point
                  } else {
                    intersectionFound = true;
                  }
                } catch (IntersectionNotFound& ex) {
                  // std::cerr << "Didn't hit light source: " << *light << std::endl;
                }
              }
            );

            // std::cerr << "IntersectionFound? " << (intersectionFound ? "true" : "false") << std::endl;

            if (intersectionFound) {
              continue;
            }

            // Light hits point

            Ray modelShadowRay{modelIntersection, toModel * glm::vec4(light->position, 1)};

            // Direction from fragment to light source.
            glm::vec3 l = glm::vec3(glm::normalize(modelShadowRay.to - modelShadowRay.from));

            // std::cerr << "l: " << glm::to_string(l) << std::endl;

            float n_dot_l = std::max((float)glm::dot(modelNormal, l), 0.0f);

            // std::cerr << "n_dot_l: " << n_dot_l << std::endl;

            glm::vec3 diffuse;
            diffuse = material->m_kd * n_dot_l;

            glm::vec3 specular = glm::vec3(0.0);

            if (n_dot_l > 0.0) {
              // Direction from fragment to viewer (origin - fragPosition).
              glm::vec3 v = glm::vec3(glm::normalize(modelRay.from - modelRay.to));

              // std::cerr << "v: " << glm::to_string(v) << std::endl;

              glm::vec3 h = glm::normalize(v + l);

              // std::cerr << "h: " << glm::to_string(h) << std::endl;
              float n_dot_h = std::max((float)glm::dot(modelNormal, h), 0.0f);

              // std::cerr << "n_dot_h: " << n_dot_h << std::endl;

              specular = material->m_ks * std::pow(n_dot_h, material->m_shininess);
            }

            double distanceFromLightSource = glm::distance(modelShadowRay.to, modelShadowRay.from);
            double attenuation = (
              light->falloff[0] +
              light->falloff[1] * distanceFromLightSource +
              light->falloff[2] * distanceFromLightSource * distanceFromLightSource
            );

            fragColourSuperSample += light->colour * (diffuse + specular) / attenuation;
            hit += 1;
          }
        }
      }


      for (int i = 0; i < subpixelsSkipped; i += 1) {
        fragColourSuperSample += glm::vec3{0, 0, ((double)y)/ny};
      }

      fragColourSuperSample = fragColourSuperSample / (ssFactor * ssFactor);

      image(x, y, 0) = fragColourSuperSample.r;
      image(x, y, 1) = fragColourSuperSample.g;
      image(x, y, 2) = fragColourSuperSample.b;
    }
  }

  printf("\n");

  std::cerr << "Misses: " << miss << std::endl;
  std::cerr << "Hits: " << hit << std::endl;
}

