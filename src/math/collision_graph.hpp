#ifndef SUPERTUX_MATH_COLLISION_GRAPH_HPP_INCLUDED
#define SUPERTUX_MATH_COLLISION_GRAPH_HPP_INCLUDED
#include <set>
#include <vector>
#include <list>
#include <map>

class MovingObject;
class CollisionHit;
struct collision_graph_edge {
  MovingObject* B;
  int dir; /** Flag for direction: 0: top, 1:bot, 2: left, 3: right (A.hit_dir(b) = true) */
};
class collision_graph {
private:
  std::map<MovingObject*, std::list<collision_graph_edge> > graph;
public:
  collision_graph();
  void register_collision_top(MovingObject* A, MovingObject* B);
  void register_collision_bot(MovingObject* A, MovingObject* B);
  void register_collision_lft(MovingObject* A, MovingObject* B);
  void register_collision_rgt(MovingObject* A, MovingObject* B);
  void register_collision_hit(CollisionHit h, MovingObject* A, MovingObject* B);
  /**
   *  Fills fil with all objects who
   *    (a) collide with A in direction dir
   *    (b) collide with any objects of (a) or (b) in direction dir
   * Transitive reflexive hull of {A} when using equivalence relation A \sim B \iff A.hit_dir = true
   */
  void directional_hull(MovingObject* A, int dir, std::vector< MovingObject* >& fill);
  /** resets after one collision phase */
  void reset();
};
#endif
