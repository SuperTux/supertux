#include "math/spatial_hashing.hpp"
#include "math/rectf.hpp"
#include "util/log.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
spatial_hashing::spatial_hashing(int c_width, int c_height, int c_gridx, int c_gridy):
  gridx(c_gridx),
  gridy(c_gridy),
  rows(c_width / gridx +1),
  cols(c_height / gridy+1),
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

void spatial_hashing::insert(Rectf aabb, MovingObject* obj) {
  if (obj == NULL)
    return;
  // Check if object si out of bounds
  if (aabb.p1.x < 0 || aabb.p1.y < 0 || aabb.p2.x > width || aabb.p2.y > height)
    return;
  // If object is already inserted, check if coordinates changed
  if (current_stored.count(obj)) {
    // If the AABB's are equal ignore insert, else delete and insert
    // IDEA Use rectangle vs rectangle difference operation for most efficient implementation
    Rectf& stored_aabb = current_stored[obj];
    if (aabb.p1 == stored_aabb.p1 && aabb.p2 == stored_aabb.p2)
      return;
    remove(obj);
  }
  // Get coordinates for top and bottom of AABB
  int startx, starty, endx, endy;
  startx = std::max<int>(0, aabb.p1.x / gridx);
  starty = std::max<int>(0, aabb.p1.y / gridy);
  endx   = std::min<int>(rows, aabb.p2.x / gridx);
  endy   = std::min<int>(cols, aabb.p2.y / gridy);
  //log_debug << startx << " " << starty << " " << endx << " " << endy << " " << grid.size() << " " << grid[0].size() << std::endl;
  for(int xcoord = startx ; xcoord <= endx ; xcoord++)
  {
    for(int ycoord = starty ; ycoord <= endy ; ycoord++)
    {
    //  log_debug << "Inserting into "<< xcoord << " " << ycoord << " " << grid.size() << " " << grid[0].size() << std::endl;
      grid[xcoord][ycoord].insert(obj);
    }
  }
  // Store current coordinates in HashMap
  current_stored[obj] = aabb;
}

void spatial_hashing::search(Rectf r, std::function<void()> collision_ok, std::set< MovingObject* >& fill)
{
  if(r.p1.x < 0 || r.p1.y < 0 || r.p2.x > width || r.p2.y > height)
    return;

  int startx, starty, endx, endy;
  startx = r.p1.x / gridx;
  starty = r.p1.y / gridy;
  endx   = r.p2.x / gridx;
  endy   = r.p2.y / gridy;

  for(int xcoord = startx ; xcoord <= endx ; xcoord++)
  {
    for(int ycoord = starty ; ycoord <= endy ; ycoord++)
    {
      for(const auto& obj : grid[xcoord][ycoord])
      {
        fill.insert(obj);
      }
    }
  }

}

bool spatial_hashing::collides(Rectf r)
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
      sum += grid[xcoord][ycoord].size();
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
      grid[xcoord][ycoord].erase(obj);
    }
  }
  return true;
}

void spatial_hashing::clear()
{
  // Delete every cell
  for(size_t i = 0;i<grid.size();i++)
  {
    for(size_t j = 0; j < grid[i].size(); j++)
      grid[i][j].clear();
    grid[i].clear();
  }
  grid.clear();
}
