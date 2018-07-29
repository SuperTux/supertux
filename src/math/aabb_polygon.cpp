#include "math/aabb_polygon.hpp"
#include <algorithm>
#include "util/log.hpp"
// TODO Introduce mode in which add_vertice is not called (=> for tilemap collisions)
AABBPolygon::AABBPolygon(const Rectf& aabb, bool construct_parent):
  p1(aabb.p1),
  p2(aabb.p2) {
  if(construct_parent)
  {
    Polygon::add_vertice(p1);
    Polygon::add_vertice(Vector(p2.x, p1.y));
    Polygon::add_vertice(p2);
    Polygon::add_vertice(Vector(p1.x, p2.y));
    Polygon::setup();
  }
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
  //log_debug << std::max(abs1, abs2) << std::endl;
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
  Polygon::handle_collision(b,m);
  return;
  // TODO(christ2go) Replace with AABB-Polygons own routine
  double overlap_x = 0, overlap_y = 0;
  double dx = p1.x - b.p1.x;
  overlap_x = (b.p2.x-b.p1.x)/2.0 + (p2.x-p1.x)/2.0  - std::abs(dx);
  double dy = p1.y - b.p1.y;
  overlap_y = (b.p2.y-b.p1.y)/2.0 + (p2.y-p1.y)/2.0 - std::abs(dy);

  if(overlap_x > 0.001 && overlap_y > 0.001)
  {
    m.collided = false;
    return;
  }
  log_debug << "Overlap " << overlap_x << " " << overlap_y << std::endl;
  if (overlap_y < overlap_x)
  {
    int sy = dy < 0 ? -1 : 1;
    m.normal = Vector(0, sy);
    m.depth = overlap_y;
  } else {
    int sx = dx < 0 ? 1 : -1;
    m.normal = Vector(sx, 0);
    m.depth = overlap_y;
  }
  m.collided = true;
  return;
  double min_overlap = 100000.0;
  if (dx > 0 && !disabled_normals[3] && !b.disabled_normals[1] && overlap_x < min_overlap)
  {
    min_overlap = overlap_x;
    m.normal = Vector(1,0);
    m.depth = overlap_x;
  }
  if (dx < 0 && !disabled_normals[1] && !b.disabled_normals[3] && overlap_x < min_overlap)
  {
    min_overlap = overlap_x;
    m.normal = Vector(1,0);
    m.depth = overlap_x;
  }
  if (dy > 0 && !disabled_normals[0] && !b.disabled_normals[2] && overlap_y < min_overlap)
  {
    min_overlap = overlap_y;
    m.normal = Vector(0,1);
    m.depth = overlap_y;
  }
  if (dy < 0 && !disabled_normals[2] && !b.disabled_normals[0] && overlap_y < min_overlap)
  {
    m.normal = Vector(0,-1);
    m.depth = overlap_y;
  }
  m.collided = true;

}

void AABBPolygon::reset_ignored_normals() {
  for (int i = 0; i < 4; i++) {
    disabled_normals[i] = false;
  }
}
