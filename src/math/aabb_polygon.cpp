#include "math/aabb_polygon.hpp"
#include <algorithm>
#include "util/log.hpp"
// TODO Introduce mode in which add_vertice is not called (=> for tilemap collisions)
AABBPolygon::AABBPolygon(const Rectf& aabb, bool construct_parent):
  p1(aabb.p1),
  p2(aabb.p2),
  constructed_parent(construct_parent),
  m_aabb(aabb) {
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
    normal_enabled[idx] = false;
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
  normal_enabled[idx] = false;
}

void AABBPolygon::handle_collision(const AABBPolygon& b, Manifold& m) {
  // TODO(christ2go) Replace with AABB-Polygons own routine
  //Polygon::handle_collision(b, m);
  float itop = b.m_aabb.get_bottom() - m_aabb.get_top();
  float ibottom = m_aabb.get_bottom() - b.m_aabb.get_top();
  float ileft = b.m_aabb.get_right() - m_aabb.get_left();
  float iright = m_aabb.get_right() - b.m_aabb.get_left();

  float vert_penetration = std::min(itop, ibottom);
  float horiz_penetration = std::min(ileft, iright);
  if(vert_penetration < 0 || horiz_penetration < 0)
    return;
  m.collided = true;
  m.normal.x = m.normal.y = 0;
  double min_overlap = 100000.0;

  if (ileft < iright && normal_enabled[3] && b.normal_enabled[1] && horiz_penetration < min_overlap) {
    min_overlap = m.depth = horiz_penetration;
    m.normal = Vector(1, 0);
  }
  if (ileft > iright && normal_enabled[1] && b.normal_enabled[3] && horiz_penetration < min_overlap) {
    min_overlap = m.depth = horiz_penetration;
    m.normal = Vector(-1, 0);
  }
  if (itop < ibottom && normal_enabled[0] && b.normal_enabled[2] && vert_penetration < min_overlap) {
    m.normal = Vector(0, 1);
    min_overlap = m.depth = vert_penetration;
  }
  if (itop > ibottom && normal_enabled[2] && b.normal_enabled[0] && vert_penetration < min_overlap) {
    m.normal = Vector(0,-1);
    min_overlap = m.depth = vert_penetration;
  }
  m.collided = true;
}

void AABBPolygon::reset_ignored_normals() {
  for (int i = 0; i < 4; i++) {
    disabled_normals[i] = false;
  }
}
