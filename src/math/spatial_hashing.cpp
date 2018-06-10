#include "math/spatial_hashing.hpp"
#include "math/rectf.hpp"
#include "util/log.hpp"
#include <iostream>
spatial_hashing::spatial_hashing(int width, int height, int gridx, int gridy)
{
  this->gridx = gridx;
  this->gridy = gridy;
  int rows = width / gridx + 1;
  int cols = height / gridy + 1;
  // Set up the vector 
  grid.resize(rows);
  for(int i = 0;i<rows;i++)
    grid[i].resize(cols);
  // Initial grid is set up.
}

void spatial_hashing::insert(Rectf aabb,MovingObject* obj)
{
  if(obj == NULL)
    return;
  // If object is already inserted, check if coordinates changed 
  if(current_stored.count(obj))
  {
    // If the AABB's are equal ignore insert, else delete and insert 
    // IDEA Use rectangle vs rectangle difference operation for most efficient implementation
    Rectf& stored_aabb = current_stored[obj];
    if(aabb.p1 == stored_aabb.p1 && aabb.p2 == stored_aabb.p2)
      return;
    remove(obj);
  }
  // Get coordinates for top and bottom of AABB
  int startx, starty, endx, endy;
  startx = aabb.p1.x / gridx;
  starty = aabb.p1.y / gridy;
  endx   = aabb.p2.x / gridx;
  endy   = aabb.p2.y / gridy;
  log_debug << startx << " " << starty << " " << endx << " " << endy << " " << grid.size() << " " << grid[0].size() << std::endl;
  for(int xcoord = startx ; xcoord <= endx ; xcoord++)
  {
    for(int ycoord = starty ; ycoord <= endy ; ycoord++)
    {
      log_debug << "Inserting into "<< xcoord << " " << ycoord << " " << grid.size() << " " << grid[0].size() << std::endl;
      grid[xcoord][ycoord].insert(obj);
    }
  }
  // Store current coordinates in HashMap
  current_stored[obj] = aabb;
}

void spatial_hashing::search(Rectf r, std::function<void()> collision_ok, std::set< MovingObject* >& fill)
{
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
}

void spatial_hashing::clear()
{
  // Delete every cell
  for(int i = 0;i<grid.size();i++)
  {
    for(int j = 0; j < grid[i].size(); j++)
      grid[i][j].clear();
    grid[i].clear();
  }
  grid.clear();
}