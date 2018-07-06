#ifndef SUPERTUX_MATH_AABB_POLYGON_INCLUDED
#define SUPERTUX_MATH_AABB_POLYGON_INCLUDED

#include "math/polygon.hpp"
#include "math/rectf.hpp"

class AABBPolygon : public Polygon {
public:
  AABBPolygon(const Rectf& AABB);
  void add_vertice(const Vector& v);
  void setup();
  // Specail override for AABB Polygon
  void process_neighbor(const Rectf& b);
  void process_neighbor(int xoffset, int yoffset);
  void handle_collision(const AABBPolygon& b, Manifold& m);
  ~AABBPolygon(){}
private:
  Vector p1;
  Vector p2;
};
#endif
