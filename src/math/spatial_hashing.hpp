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
  spatial_hashing(int width, int height,int gridx=64, int gridy=64);
  void insert(const Rectf& aabb,MovingObject* obj);
  virtual bool collides(const Rectf& r);
  virtual bool remove(MovingObject* obj);
  virtual void search(const Rectf& r, std::function<void()> collision_ok, std::list< MovingObject* >& fill);
  virtual void clear();
  /**
   *  Use below functions for a faster update without compromising iterator integrity.
   *  Once the iterator is no longer being used, call bulk_update() to perform the waiting updates.
   *  bulk_update takes care of resetting the bulk list.
   */
  void add_bulk(const Rectf& pos, MovingObject* obj);
  void do_bulk_update();
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

  std::vector< std::pair< Rectf, MovingObject* > > bulk_update;
  int update_id = -1;
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
