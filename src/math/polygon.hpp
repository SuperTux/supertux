#ifndef SUPERTUX_HEADER_MATH_POLYGON_HPP_
#define SUPERTUX_HEADER_MATH_POLYGON_HPP_
#include <vector>
#include "math/vector.hpp"

struct Manifold {
  Vector normal;
  double depth;

  bool collided = false;

public:
  Manifold():
    normal(),
    depth(),
    collided(){
  }
};

double vector_gap(const Vector& a, const Vector& b);

class Polygon {
 public:
   Polygon();
    /**
     *  Adds a vertice to this polygon.
     *  @param point The new vertice
     */
    void add_vertice(const Vector& point);
    /**
     * Checks for overlapping vertices and marks normals using those as disabled.
     * @param b The neighbouring polygon.
     **/
    void process_neighbor(const Polygon& b);
    /**
     *  Handles a collision between the tile and a moving object.
     *  Returns the axis of resolution.
     *
     *  @param b The polygon to check collisions against
     */
     void handle_collision(const Polygon& b, Manifold& m);
     /**
      *  Checks if the axis represented by the vector
      *  is seperating (i.e the polygons do not overlap if projected onto the axis)
      *
      *  @param b The second polygon
      *  @param axis The vector representing the axis
      */
     double is_seperating_axis(const Polygon& b, const Vector& axis);
     /**
      * Projects a polygon onto an axis.
      * @param axis The axis to project onto.
      * @returns 0 if the axis not seperating, else the overlap length.
      */
     Vector project(const Vector& axis) const;
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
      virtual ~Polygon() {}
 protected:
    // The two commented out fields will be used for rotation later.
    //std::vector< Vector > original_vertices; /** Only used in rotated polygons */
    //double rotation_angle;
    std::vector< Vector > vertices;
    std::vector< Vector > edges;
    std::vector< Vector > normals; /** Edge normals */
    Vector middle_point;
    /** Saves for every normal a flag indicating wether it is enabled. */
    std::vector< bool > disabled_normals;
};
#endif  // SUPERTUX_HEADER_MATH_POLYGON_HPP_
