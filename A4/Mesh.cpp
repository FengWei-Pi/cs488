#include <algorithm>
#include <iostream>
#include <fstream>
#include <limits>
#include <cmath>

#include <glm/ext.hpp>

#include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

Mesh::Mesh( const std::string& fname ) :
  m_vertices(),
  m_faces(),
  boundingVolume(NULL)
{
  std::string code;
  double vx, vy, vz;
  size_t s1, s2, s3;

  std::ifstream ifs( ("Assets/" + fname).c_str() );

  if (ifs.fail()) {
    std::cerr << "Unable to open file for reading." << std::endl;
    exit(1);
  }

  while( ifs >> code ) {
    if( code == "v" ) {
      ifs >> vx >> vy >> vz;
      m_vertices.push_back( glm::vec3( vx, vy, vz ) );
    } else if( code == "f" ) {
      ifs >> s1 >> s2 >> s3;
      m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
    }
  }

  float Infinity = std::numeric_limits<float>::infinity();

  float minX = Infinity;
  float minY = Infinity;
  float minZ = Infinity;

  float maxX = -Infinity;
  float maxY = -Infinity;
  float maxZ = -Infinity;

  for (const glm::vec3& v: m_vertices) {
    minX = std::min(minX, v.x);
    minY = std::min(minY, v.y);
    minZ = std::min(minZ, v.z);

    maxX = std::max(maxX, v.x);
    maxY = std::max(maxY, v.y);
    maxZ = std::max(maxZ, v.z);
  }

  const glm::vec3 min{minX, minY, minZ};
  const glm::vec3 max{maxX, maxY, maxZ};

  boundingVolume = new NonhierSphere((max + min)/2, glm::length(max - min)/2);
}

Mesh::~Mesh() {
  delete boundingVolume;
}

glm::vec4 Mesh::getNormal(const glm::vec4& point) {
  int i = -1;
  double Infinity = std::numeric_limits<double>::infinity();
  double minDotProduct = Infinity;
  glm::vec3 minNormal;

  for (Triangle& tri : m_faces) {
    i += 1;
    const glm::vec3 P0{m_vertices.at(tri.v1)};
    const glm::vec3 P1{m_vertices.at(tri.v2)};
    const glm::vec3 P2{m_vertices.at(tri.v3)};

    const glm::vec3 normal{glm::normalize(glm::cross(P1 - P0, P2 - P0))};

    double dot = glm::dot(normal, glm::vec3(point) - P0);

    if (std::fabs(dot) < minDotProduct) {
      minDotProduct = std::fabs(dot);
      minNormal = normal;

      // std::cerr << "Mesh::getNormal i: " << i <<  std::endl;

      // std::cerr << "glm::dot(normal, glm::vec3(point) - P0): "
      //           << glm::dot(normal, glm::vec3(point) - P0)
      //           << std::endl;
    }
  }

  if (minDotProduct != Infinity) {
    return glm::vec4{minNormal, 0};
  }

  throw NormalNotFound{};
}

double Mesh::intersect(const Ray& ray) {
  // Ensure that the ray intersects the bounding volume.
  boundingVolume->intersect(ray);

  double Infinity = std::numeric_limits<double>::infinity();
  double t = Infinity;

  int i = -1;

  for (const Triangle& tri : m_faces) {
    i += 1;
    const glm::vec3 P0{m_vertices.at(tri.v1)};
    const glm::vec3 P1{m_vertices.at(tri.v2)};
    const glm::vec3 P2{m_vertices.at(tri.v3)};

    const glm::vec3 X{P1 - P0};
    const glm::vec3 Y{P2 - P0};
    const glm::vec3 Z{ray.from - ray.to};

    const double D = glm::determinant(glm::mat3{X, Y, Z});

    if (std::fabs(D) < 0.0001) {
      continue;
    }

    const glm::vec3 R = glm::vec3(ray.from) - P0;

    const double Dx = glm::determinant(glm::mat3{R, Y, Z});
    const double Dy = glm::determinant(glm::mat3{X, R, Z});
    const double Dz = glm::determinant(glm::mat3{X, Y, R});

    glm::vec3 S{Dx/D, Dy/D, Dz/D};

    if (
      !(S.x >= 0) ||
      !(S.y >= 0) ||
      !(S.z >= 0) ||
      !((1 - S.x - S.y) >= 0)
    ) {
      continue;
    }

    if (S.z < t) {
      // std::cerr << "Mesh::intersect i: " << i << std::endl;
      // std::cerr << "Ray: " << glm::to_string(ray.from) << ", " << glm::to_string(ray.to) << std::endl;
      // std::cerr << "t: " << S.z << std::endl;
      // std::cerr << "Ray at i: " << glm::to_string(ray.at(S.z)) << std::endl;
      t = S.z;
      std::cerr << "i: " << i << std::endl;
      std::cerr << "P0: " << glm::to_string(P0) << std::endl;
      std::cerr << "P1: " << glm::to_string(P1) << std::endl;
      std::cerr << "P2: " << glm::to_string(P2) << std::endl;
      std::cerr << "ray.at(" << t << "): " << glm::to_string(ray.at(t)) << std::endl;
    }
  }

  if (t == Infinity) {
    throw IntersectionNotFound{};
  }

  return t;
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*

  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
    const MeshVertex& v = mesh.m_verts[idx];
    out << glm::to_string( v.m_position );
  if( mesh.m_have_norm ) {
      out << " / " << glm::to_string( v.m_normal );
  }
  if( mesh.m_have_uv ) {
      out << " / " << glm::to_string( v.m_uv );
  }
  }

*/
  out << "}";
  return out;
}
