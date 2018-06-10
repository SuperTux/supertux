#ifndef SUPERTUX_MATH_SPATIAL_HASHING_HPP_INCLUDED 
#define SUPERTUX_MATH_SPATIAL_HASHING_HPP_INCLUDED
#include "math/rectf.hpp"
#include "math/broadphase.hpp"
#include <vector>
#include <map>
#include <set>
#include <list>
#include <functional>

class spatial_hashing : public collision_broadphase {
public:
  spatial_hashing(int width, int height,int gridx=128, int gridy=128);
  void insert(Rectf aabb,MovingObject* obj);
  virtual bool collides(Rectf r);
  virtual bool remove(MovingObject* obj);
  virtual void search(Rectf r, std::function<void()> collision_ok, std::set< MovingObject* >& fill);
  virtual void clear();
  ~spatial_hashing() = default;
private:
  int gridx;  // Width of grid cells 
  int gridy;  // Height of grid objects
  std::vector< std::vector< std::set<MovingObject*> > > grid;  
  std::map< MovingObject*, Rectf > current_stored;
};
#endif