#include "math/collision_graph.hpp"
#include <queue>
#include <set>
#include "supertux/collision_hit.hpp"
collision_graph::collision_graph():
graph()
{

}
void collision_graph::register_collision_top(MovingObject* A, MovingObject* B)
{
  collision_graph_edge e;
  e.B = B;
  e.dir = 0;
  graph[A].push_back(e);
}

void collision_graph::register_collision_bot(MovingObject* A, MovingObject* B)
{
  collision_graph_edge e;
  e.B = B;
  e.dir = 1;
  graph[A].push_back(e);
}

void collision_graph::register_collision_lft(MovingObject* A, MovingObject* B)
{
  collision_graph_edge e;
  e.B = B;
  e.dir = 2;
  graph[A].push_back(e);
}

void collision_graph::register_collision_rgt(MovingObject* A, MovingObject* B)
{
  collision_graph_edge e;
  e.B = B;
  e.dir = 3;
  graph[A].push_back(e);
}

void collision_graph::directional_hull(MovingObject* A, int dir, std::vector< MovingObject* >& fill)
{
  std::set<MovingObject*> inqueue;
  std::queue<MovingObject*> queue;
  queue.push(A);
  inqueue.insert(A);

  while(!queue.empty())
  {
    // Get object from queue
    MovingObject* x = queue.front();
    queue.pop();
    // Iterate over edges
    for(const auto& edge : graph[x])
    {
      if(edge.dir == dir)
      {
        if(!inqueue.count(edge.B))
        {
          queue.push(edge.B);
          inqueue.insert(edge.B);
          fill.push_back(edge.B);
        }
      }
    }
  }

}

void collision_graph::register_collision_hit(CollisionHit h, MovingObject* A, MovingObject* B)
{
  if(h.top)
  {
    register_collision_top(A,B);
    register_collision_bot(B,A);
  }
  if(h.bottom)
  {
    register_collision_bot(A,B);
    register_collision_top(B,A);
  }
  if(h.left)
  {
    register_collision_lft(A,B);
    register_collision_rgt(B,A);
  }
  if(h.right)
  {
    register_collision_rgt(A,B);
    register_collision_lft(B,A);
  }
}

void collision_graph::reset()
{
  graph.clear();
}
