#include "math/polygon.hpp"

void Polygon::add_vertice(Vector v)
{
  vertices.push_back(v);
  if(vertices.size() > 1)
  {
    edges.push_back(v-vertices[vertices.size()-2]);
    // Add a normal for this edge
    normals.push_back(edges[edges.size()-1].perp());
  }

}

void Polygon::process_neighbor(Polygon& b)
{
  // Loop over all edges (and vertices in order to find 'wrong' edges)

}

void Polygon::process_octile_neighbour(int dir, Polygon& b)
{
  // Check if the polygons have an edge at direction in common
}

void Polygon::handle_collision(Polygon& b)
{
  // First: Check if there is a collision
  // Also store axis of least penetration

}

bool Polygon::is_seperating_axis(Polygon& b, Vector& axis)
{

}

void Polygon::disable_normal(const Vector& n)
{
  for(int i = 0;i<normals.size();i++)
  {
    if(!normal_enabled[i])
    {
      // Check if they are colinear
      if(normals[i].is_colinear(n))
      {
        normal_enabled[i] = false;
      }
    }
  }
}

void Polygon::setup()
{
  edges.push_back(vertices[vertices.size()-1]-vertices[0]);
  normals.push_back(edges[edges.size()-1].perp());
  normal_enabled.resize( normals.size() );
  for(int i = 0;i < normals.size();i++)
    normal_enabled[i] = true;
}
