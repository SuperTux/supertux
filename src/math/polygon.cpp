#include "math/polygon.hpp"
#include <limits>
#include <cassert>
#include <algorithm>
#include "util/log.hpp"

Polygon::Polygon():
  vertices(),
  edges(),
  normals(),
  middle_point(),
  disabled_normals() {
}

void Polygon::add_vertice(const Vector& v) {
  vertices.push_back(v);
  if (vertices.size() > 1) {
    edges.push_back(v-vertices[vertices.size()-2]);
    // Add a normal for this edge
    normals.push_back(edges[edges.size()-1].perp().unit());
  }
}

double vector_gap(const Vector& a, const Vector& b) {
  return std::abs(a.x-b.x)+std::abs(a.y-b.y);
}

void Polygon::process_neighbor(const Polygon& b) {
  // Loop over all edges (and vertices in order to find 'wrong' edges)
  const int margin = 8;
  for (size_t i = 0; i <= vertices.size(); i++) {
    auto& v = vertices[i];
    auto& v2 = vertices[(i+1)%vertices.size()];
    for (size_t j = 0; j <= b.vertices.size(); j++) {
      auto& w = b.vertices[j];
      auto& w2 = b.vertices[(j+1)%b.vertices.size()];
      double d1 = vector_gap(w, v);
      double d2 = vector_gap(w2, v2);
      bool ndis = disabled_normals[i];
      if (d1 <= margin && d2 <= margin)
        disabled_normals[i] = true;
      double d3 = vector_gap(w, v2);
      double d4 = vector_gap(v, w2);
      if (d3 <= margin && d4 <= margin)
        disabled_normals[i] = true;
      if (!ndis && disabled_normals[i]) {
        log_debug << "Disabled normal " << i << " " << normals[i].x << " " << normals[i].y << std::endl;
        log_debug << w.x << " " << w.y << " " << w2.x << " " << w2.y << std::endl
                  << v.x << " " << v.y << " " << v2.x << " " << v2.y << std::endl;
      }
    }
  }
}

void Polygon::handle_collision(Polygon& b, Manifold& m) {
  // First: Check if there is a collision

  // Check if any of this Polygons axes seperates
  double d_inf = std::numeric_limits<double>::infinity();
  double minOverlap = d_inf;
  Vector minAxis;
  bool set_overlap = false;
  for(size_t i = 0; i < edges.size(); i++)
  {
    double overlap;
    if(disabled_normals[i])
      continue;
    if ((overlap = is_seperating_axis(b, normals[i])) == 0.0f)
      return;
    if ((std::abs(overlap) < std::abs(minOverlap) || minOverlap == d_inf)) {
      if (overlap > 0)
        continue;
      bool seen_similiar = false;
      bool at_least_one_disabled  = false;
      for (size_t j = 0; j < b.normals.size(); j++) {
        if (b.normals[j].is_colinear(normals[i])) {
          seen_similiar = true;
          at_least_one_disabled |= b.disabled_normals[j];
        }
      }
      if (seen_similiar && at_least_one_disabled)
        continue;
      set_overlap = true;
      minAxis = normals[i];
      minOverlap = overlap;
    }
  }
  // Check if any of b's axes seperates
  for (size_t i = 0; i < b.edges.size(); i++) {
    double overlap;
    if (b.disabled_normals[i])
      continue;
    if ((overlap = is_seperating_axis(b, b.normals[i])) == 0.0f)
        return;
    if (overlap > 0)
      continue;
    if (std::abs(overlap) < std::abs(minOverlap)) {
      bool seen_similiar = false;
      bool at_least_one_disabled  = false;
      for (size_t j = 0; j < normals.size(); j++) {
        if (normals[j].is_colinear(b.normals[i])) {
          seen_similiar = true;
          at_least_one_disabled |= disabled_normals[j];
        }
      }
      if (seen_similiar && at_least_one_disabled)
        continue;
      set_overlap = true;
      minAxis = b.normals[i];
      minOverlap = overlap;
    }
  }
  log_debug << "Polygonial collision occured" << std::endl;
  log_debug << minOverlap << std::endl;
  // To resolve the collison use overlap as depth
  // and the axis normal as normal
  m.collided = set_overlap;
  if (!set_overlap)
  log_debug  << "NOT SET " << std::endl;
  m.normal = minAxis.unit();
  m.depth = minOverlap;
}

double Polygon::is_seperating_axis(const Polygon& b, const Vector& axis) {
  Vector a_proj = this->project(axis);
  Vector b_proj = b.project(axis);
  double aRight = b_proj.y - a_proj.x;
  double aLeft  = a_proj.y - b_proj.x;
  if ( aLeft < 0 || aRight < 0)
    return 0.0f;
  // Is a seperating axis iff ranges do not overlap
  if (aRight < aLeft)
    return aRight;
  return -aLeft;
}

Vector Polygon::project(const Vector& axis) const {
  assert(vertices.size() > 0);
  double minimum =  vertices[0]*axis.unit(), maximum = vertices[0]*axis.unit();
  for (const auto& vertice : vertices) {
    double proj = vertice*axis.unit();
    minimum = std::min(proj, minimum);
    maximum = std::max(proj, maximum);
  }
  return Vector(minimum, maximum);
}

void Polygon::debug() {
  log_debug << "Poly debug" << std::endl;
  log_debug << "Vertices: " << vertices.size() << std::endl;
  for (const auto& v : vertices)
    log_debug << v.x << " " << v.y << "  , ";
  log_debug << std::endl;
}
void Polygon::setup() {
  edges.push_back(vertices[0]-vertices[vertices.size()-1]);
  normals.push_back(edges[edges.size()-1].perp());
  disabled_normals.resize(normals.size(), false);
  Vector sumv(0, 0);
  for (const auto& v : vertices) {
    sumv += v;
  }
  sumv *= (double)1/vertices.size();
  middle_point = sumv;
  for (size_t i = 0; i < normals.size(); i++) {
    // Check if normal faces inwards
    Vector point = vertices[i];
    if (normals[i]*(middle_point - point) < 0) {
      // Faces inward => invert it
      normals[i] *= -1;
    }
  }
}
