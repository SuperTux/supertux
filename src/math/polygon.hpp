#ifndef SUPERTUX_HEADER_MATH_POLYGON
#define SUPERTUX_HEADER_MATH_POLYGON
#include "math/vector.hpp"
#include <vector>
struct Manifold {
  Vector normal;
  double depth;
  
  bool collided = false;
};
class Polygon {
  public:
    /**
     *  Adds a vertice to this polygon.
     *  @param point The new vertice
     */
    void add_vertice( Vector point );
    /**
     * Checks for overlapping vertices and marks normals using those as disabled.
     * @param b The neighbouring polygon.
     **/
    void process_neighbor(Polygon& b);
    /**
     *  Checks octile adjacent neighbours for normals which have to be deactivated.
     **/
    void process_octile_neighbour(int dir, Polygon& b);
    /**
     *  Handles a collision between the tile and a moving object.
     *  Returns the axis of resolution.
     *
     *  @param b The polygon to check collisions against
     */
     void handle_collision(Polygon& b, Manifold& m);
     /**
      * Flags every normal colinear with n to be ignored.
      * @param n The normal to flag as ignored.
      */
     void disable_normal(const Vector& n);
     /**
      *  Checks if the axis represented by the vector
      *  is seperating (i.e the polygons do not overlap if projected onto the axis)
      *
      *  @param b The second polygon
      *  @param axis The vector representing the axis
      */
     double is_seperating_axis(Polygon& b,const Vector& axis);
     /**
      * Projects a polygon onto an axis.
      * @param axis The axis to project onto.
      * @returns 0 if the axis not seperating, else the overlap length.
      */
     Vector project(Vector axis);
     /**
     * TODO
      * Rotates the polygon angle degrees.
      * @returns A new polygon with vertices rotated.
      * Original vertices will be saved in original_vertices so that rotating
      * a rotated polygon becomes more (numerically) stable.
      */
     //Polygon rotate(double angle);
     /**
      * Inits the normal_enabled vector.
      * Call after all edges were added.
      */
      void setup();

      void debug();
  private:
    std::vector< Vector > original_vertices; /** Only used in rotated polygons */
    double rotation_angle;
    std::vector< Vector > vertices;
    std::vector< Vector > edges;
    std::vector< Vector > normals; /** Edge normals */
    Vector middle_point;
    std::vector< bool > disabled_normals; /** Saves for every normal a flag indicating wether it is enabled. */
};
#endif
