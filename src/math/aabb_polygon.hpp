#ifndef SUPERTUX_MATH_AABB_POLYGON_INCLUDED
#define SUPERTUX_MATH_AABB_POLYGON_INCLUDED

#include "math/polygon.hpp"
#include "math/rectf.hpp"

class AABBPolygon : public Polygon {
public:
  AABBPolygon(const Rectf& AABB, bool construct_parent = true);
  void add_vertice(const Vector& v);
  void setup();
  // Specail override for AABB Polygon
  void process_neighbor(const Rectf& b);
  void process_neighbor(int xoffset, int yoffset);
  void handle_collision(const AABBPolygon& b, Manifold& m);
  void reset_ignored_normals();
  ~AABBPolygon(){}
  Vector p1;
  Vector p2;
  /** use an array to make stuff even faster */
  bool normal_enabled[4] = {true, true, true, true};
  bool constructed_parent;
  const Rectf& m_aabb;
};
#endif
