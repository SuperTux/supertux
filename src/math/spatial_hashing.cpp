#include "math/spatial_hashing.hpp"
#include "math/rectf.hpp"
#include "util/log.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
spatial_hashing::spatial_hashing(int c_width, int c_height, int c_gridx, int c_gridy):
  gridx(c_gridx),
  gridy(c_gridy),
  rows(c_height / gridy +1),
  cols(c_width / gridx+1),
  width(c_width),
  height(c_height),
  grid(),
  current_stored() {
  // Set up the vector
  grid.resize(rows+10);
  for(int i = 0;i<rows;i++)
    grid[i].resize(cols+10);
  // Initial grid is set up.
}

void spatial_hashing::insert(const Rectf& aabb, MovingObject* obj) {
  if(aabb.p1.x < 0 || aabb.p1.y < 0 || aabb.p2.x > width || aabb.p2.y > height)
    return;
  if (obj == NULL)
    return;
  // Check if object si out of bounds
  // If object is already inserted, check if coordinates changed
  Rectf insertrect =aabb;
  // Get coordinates for top and bottom of AABB
  insertrect.p1 = Vector( std::max<double>(0, aabb.p1.x), std::max<double>(0, aabb.p1.y));
  insertrect.p2 = Vector( std::min<double>(width, aabb.p2.x), std::min<double>(height, aabb.p2.y));
  int startx, starty, endx, endy;
  startx = std::max<int>(0, insertrect.p1.x / gridx);
  starty = std::max<int>(0, insertrect.p1.y / gridy);
  endx   = std::min<int>(cols, insertrect.p2.x / gridx);
  endy   = std::min<int>(rows, insertrect.p2.y / gridy);
  Rectf newrect = Rectf(Vector(startx, starty),Vector(endx, endy));
  if (current_stored.count(obj)) {

    // If the AABB's are equal ignore insert, else delete and insert
    // IDEA Use rectangle vs rectangle difference operation for most efficient implementation
    Rectf& stored_aabb = current_stored[obj];
    if (newrect.p1 == stored_aabb.p1 && newrect.p2 == stored_aabb.p2)
      return;
    remove(obj);
  }

  for (int xcoord = startx ; xcoord <= endx ; xcoord++) {
    for (int ycoord = starty ; ycoord <= endy ; ycoord++) {
      grid[ycoord][xcoord].insert(obj);
    }
  }
  // Store current coordinates in HashMap
  current_stored[obj] = newrect;
}

void spatial_hashing::search(const Rectf& r, std::function<void()> collision_ok, std::list< MovingObject* >& fill)
{
  if(r.p1.x < 0 || r.p1.y < 0 || r.p2.x > width || r.p2.y > height)
    return;
  int startx, starty, endx, endy;
  startx = std::max<int>(0, r.p1.x / gridx);
  starty = std::max<int>(0, r.p1.y / gridy);
  endx   = std::min<int>(cols, r.p2.x / gridx);
  endy   = std::min<int>(rows, r.p2.y / gridy);

  for (int xcoord = startx ; xcoord <= endx ; xcoord++)
  {
    for (int ycoord = starty ; ycoord <= endy ; ycoord++) {
      for (const auto& obj : grid[ycoord][xcoord]) {
        if (obj != NULL)
        {
            fill.push_back(obj);
        }
      }
    }
  }

}

bool spatial_hashing::collides(const Rectf& r)
{
  // Abort and return true as soon as we have encountered more than 1 object.
  // (We use more than 1 because encountering exactly 1 object would mean no possible collisions.)
  int startx, starty, endx, endy;
  startx = r.p1.x / gridx;
  starty = r.p1.y / gridy;
  endx   = r.p2.x / gridx;
  endy   = r.p2.y / gridy;

  int sum = 0;
  for(int xcoord = startx ; xcoord <= endx ; xcoord++)
  {
    for(int ycoord = starty ; ycoord <= endy ; ycoord++)
    {
      sum += grid[ycoord][xcoord].size();
      if(sum > 1)
        return true;
    }
  }
  return false;
}

bool spatial_hashing::remove(MovingObject* obj)
{
  if(!current_stored.count(obj))
    return false;
  const auto& aabb = current_stored[obj];
  int startx, starty, endx, endy;
  startx = aabb.p1.x / gridx;
  starty = aabb.p1.y / gridy;
  endx   = aabb.p2.x / gridx;
  endy   = aabb.p2.y / gridy;
  for(int xcoord = startx ; xcoord <= endx ; xcoord++)
  {
    for(int ycoord = starty ; ycoord <= endy ; ycoord++)
    {
        grid[ycoord][xcoord].erase(obj);
    }
  }
  return true;
}

void spatial_hashing::clear()
{
  grid.clear();
}

void spatial_hashing::add_bulk(const Rectf& r,MovingObject* obj) {
  update_id++;
  if((int)bulk_update.size() >= update_id) {
    bulk_update.push_back(std::make_pair(r, obj));
  } else {
    bulk_update[update_id].first = r;
    bulk_update[update_id].second = obj;
  }
}

void spatial_hashing::do_bulk_update() {
  for(int i = 0; i <= update_id; i++) {
    insert(bulk_update[i].first, bulk_update[i].second);
  }
  update_id = -1;
}

spatial_hasingIterator::spatial_hasingIterator(spatial_hashing* hash, Rectf aabb) :
  m_hash(hash),
  m_x(0),
  m_y(0),
  m_extend_x(0),
  m_extend_y(0),
  m_initial_y(0),
  m_valid(true)
{
  if(aabb.p1.x < 0 || aabb.p1.y < 0 || aabb.p2.x > m_hash->width || aabb.p2.y > m_hash->height)
  {
        m_valid = false;
        return;
  }
  m_x = std::max<int>(0, aabb.p1.x / hash->gridx);
  m_y = std::max<int>(0, aabb.p1.y / hash->gridy);
  m_extend_x   = std::min<int>(hash->cols, aabb.p2.x / hash->gridx);
  m_extend_y   = std::min<int>(hash->rows, aabb.p2.y / hash->gridy);
  m_initial_y = m_y;
  if(m_x > m_extend_x || m_y > m_extend_y) {
    m_valid = false;
    return;
  }
  iter = m_hash->grid[m_y][m_x].begin();
  iterend = m_hash->grid[m_y][m_x].end();
}

MovingObject* spatial_hasingIterator::next() {
  if(!m_valid)
    return NULL;
  while(iter == iterend) {
    m_y++;
    if(m_y > m_extend_y) {
      m_x++;
      m_y = m_initial_y;
    }
    if ((m_x > m_extend_x) ) {
      return NULL;
    }
    iter = m_hash->grid[m_y][m_x].begin();
    iterend = m_hash->grid[m_y][m_x].end();
  }

  MovingObject* next = *iter;
  if(next == NULL)
    return this->next();
  iter++;
  return next;
}
