#ifndef SUPERTUX_MATH_SPATIAL_HASHING_HPP_INCLUDED
#define SUPERTUX_MATH_SPATIAL_HASHING_HPP_INCLUDED
#include "math/rectf.hpp"
#include "math/broadphase.hpp"
#include <vector>
#include <map>
#include <set>
#include <list>
#include <functional>
class spatial_hasingIterator;
class spatial_hashing : public collision_broadphase {
public:
  friend class spatial_hasingIterator;
  spatial_hashing(int width, int height,int gridx=90, int gridy=90);
  void insert(const Rectf& aabb,MovingObject* obj);
  virtual bool collides(const Rectf& r);
  virtual bool remove(MovingObject* obj);
  virtual void search(const Rectf& r, std::function<void()> collision_ok, std::list< MovingObject* >& fill);
  virtual void clear();
  ~spatial_hashing() = default;
private:
  int gridx;  // Width of grid cells
  int gridy;  // Height of grid objects
  int rows;
  int cols;

  int width;
  int height;
  std::vector< std::vector< std::set<MovingObject*> > > grid;
  std::map< MovingObject*, Rectf > current_stored;

};

class spatial_hasingIterator  {
public:
  spatial_hasingIterator(spatial_hashing* hash, Rectf aabb);
  MovingObject* next();
private:
  spatial_hashing* m_hash;
  int m_x;
  int m_y;
  int m_extend_x;
  int m_extend_y;
  int m_initial_y;
  bool m_valid;
  std::set<MovingObject*>::const_iterator iter;
  std::set<MovingObject*>::const_iterator iterend;
};
#endif
