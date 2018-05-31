#include "math/polygon.hpp"
#include "util/log.hpp"

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

  // Check if any of this Polygons axes seperates
  double minOverlap = std::numeric_limits<double>::infinity();
  Vector minAxis;
  for(const auto& axis : edges)
  {
    double overlap;
    if((overlap = Polygon::is_seperating_axis(b, axis)) == 0.0f)
      return;
    if(overlap < minOverlap)
    {
      minAxis = axis;
      minOverlap = overlap;
    }
  }
  // Check if any of b's axes seperates
  for(const auto& axis : b.edges)
  {
    double overlap;
    if((overlap = Polygon::is_seperating_axis(b,(const Vector&) axis.perp())) == 0.0f)
    {
        return;
    }
    if(overlap < minOverlap)
    {
      minAxis = axis;
      minOverlap = overlap;
    }
  }
  log_debug << "Polygonial collision occured" << std::endl;
  // To resolve the collison use overlap as depth
  // and the axis normal as normal

  // TODO Ignore ignormals

}

double Polygon::is_seperating_axis(Polygon& b,const Vector& axis)
{
  Vector a_proj = this->project(axis);
  Vector b_proj = b.project(axis);
  // Is a seperating axis iff ranges do not overlap
  if(std::max(a_proj.x, b_proj.x) > std::min(a_proj.y, b_proj.y))
    return 0.0f;
  return std::min(a_proj.y, b_proj.y)-std::max(a_proj.x, b_proj.x);
}

Vector Polygon::project(Vector axis)
{
  double minimum =  std::numeric_limits<double>::infinity(), maximum = 0;
  for(const auto& vertice : vertices)
  {
    double proj = vertice*axis.unit();
    minimum = std::min(proj, minimum);
    maximum = std::max(proj, maximum);
  }
  return Vector(minimum, maximum);
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
void Polygon::debug()
{
  log_debug << "Poly debug" << std::endl;
  log_debug << "Vertices: " << vertices.size() << std::endl;
  for(const auto& v: vertices)
    log_debug << v.x << " " << v.y << "  , ";
  log_debug << std::endl;
}
void Polygon::setup()
{
  edges.push_back(vertices[vertices.size()-1]-vertices[0]);
  normals.push_back(edges[edges.size()-1].perp());
  normal_enabled.resize( normals.size() );
  for(int i = 0;i < normals.size();i++)
    normal_enabled[i] = true;
}
