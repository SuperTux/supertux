#include "math/aabb_polygon.hpp"
#include <algorithm>
#include "util/log.hpp"
// TODO Introduce mode in which add_vertice is not called (=> for tilemap collisions)
AABBPolygon::AABBPolygon(const Rectf& aabb):
  p1(aabb.p1),
  p2(aabb.p2) {
  Polygon::add_vertice(p1);
  Polygon::add_vertice(Vector(p2.x, p1.y));
  Polygon::add_vertice(p2);
  Polygon::add_vertice(Vector(p1.x, p2.y));
  Polygon::setup();
}

void AABBPolygon::setup() {
  Polygon::setup();
}

inline bool edge_equal(const Vector& p1,
                       const Vector& p2,
                       const Vector& w1,
                       const Vector& w2) {
  int abs1 = vector_gap(p1, w1);
  int abs2 = vector_gap(p2, w2);
  log_debug << std::max(abs1, abs2) << std::endl;
  return std::max(abs1, abs2) <= 4;
}

void AABBPolygon::process_neighbor(const Rectf& r) {
  Vector neigh_top_r = Vector(r.p2.x, r.p1.y);
  Vector neigh_bot_l = Vector(r.p1.x, r.p2.y);
  // Also do this for this AABB
  Vector top_r = Vector(p2.x, p1.y);
  Vector bot_l = Vector(p1.x, p2.y);
  int idx = -1;
  // Check all edges if they are equal
  if (edge_equal(p1, top_r, neigh_bot_l, r.p2)) {
    idx = 0;
  }
  if (edge_equal(top_r, p2, r.p1, neigh_bot_l)) {
    idx = 1;
  }
  if (edge_equal(bot_l, p2, r.p1, neigh_top_r)) {
    idx = 2;
  }
  if (edge_equal(p1, bot_l, neigh_top_r, r.p2)) {
    idx = 3;
  }
  if (idx != -1)
    disabled_normals[idx] = true;
}
/** Used for processing tile neighbours. */
void AABBPolygon::process_neighbor(int xoffset, int yoffset) {
  if (std::abs(xoffset)+std::abs(yoffset) != 1) {
    return;
  }
  int idx = -1;
  if (xoffset == 0 && yoffset == -1) {
    idx = 0;
  }
  if (xoffset == 0 && yoffset == 1) {
    idx = 2;
  }
  if (xoffset == -1 && yoffset == 0) {
    idx = 3;
  }
  if (xoffset == 1 && yoffset == 0) {
    idx = 1;
  }
  assert(idx != -1);
  disabled_normals[idx] = true;
}

void AABBPolygon::handle_collision(const AABBPolygon& b, Manifold& m) {
  // TODO(christ2go) Replace with AABB-Polygons own routine
//  double overlap_x = 0, overlap_y = 0;
  Polygon::handle_collision(b, m);
}
