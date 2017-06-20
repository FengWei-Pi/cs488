#include <iostream>
#include <fstream>
#include <limits>
#include <cmath>

#include <glm/ext.hpp>

#include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

Mesh::Mesh( const std::string& fname )
  : m_vertices()
  , m_faces()
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
}

glm::vec4 Mesh::getNormal(glm::vec4 point) {
  for (Triangle& tri : m_faces) {
    const glm::vec3 P0{m_vertices.at(tri.v1)};
    const glm::vec3 P1{m_vertices.at(tri.v2)};
    const glm::vec3 P2{m_vertices.at(tri.v3)};

    const glm::vec3 normal{glm::normalize(glm::cross(P1 - P0, P2 - P0))};

    if (std::fabs(glm::dot(normal, glm::vec3(point) - P0)) < 0.01) {
      return glm::vec4{normal, 0};
    }
  }

  throw NormalNotFound{};
}

glm::vec4 Mesh::intersect(Ray ray) {
  double Infinity = std::numeric_limits<double>::infinity();
  double t = Infinity;

  for (const Triangle& tri : m_faces) {
    const glm::vec3 P0{m_vertices.at(tri.v1)};
    const glm::vec3 P1{m_vertices.at(tri.v2)};
    const glm::vec3 P2{m_vertices.at(tri.v3)};

    const glm::vec3 X{P1 - P0};
    const glm::vec3 Y{P2 - P0};
    const glm::vec3 Z{ray.from - ray.to};

    const glm::mat3 M{X, Y, Z};

    const double det = glm::determinant(M);

    if (std::fabs(det) < 0.0001) {
      std::cerr << "Determinant: " << det << std::endl;
      continue;
    }

    glm::vec3 R = glm::vec3(ray.from) - P0;
    glm::vec3 S = glm::inverse(M) * R;

    if (
      !(S.x >= 0) ||
      !(S.y >= 0) ||
      !((1 - S.x - S.y) >= 0)
    ) {
      // Not a triangle
      continue;
    }

    if (S.z < t && S.z > 0) {
      t = S.z;
    }
  }

  if (t == Infinity) {
    throw IntersectionNotFound{};
  }

  return ray.from + ((float)t) * (ray.to - ray.from);
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
