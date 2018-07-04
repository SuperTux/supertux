#ifndef SUPERTUX_PHYSICS_COLLISION_ENGINE_HPP_INCLUDED
#define SUPERTUX_PHYSICS_COLLISION_ENGINE_HPP_INCLUDED
/**
 *  The CollisionEngine is responsible for
 *  handling any collisions occuring in the supertux world.
 *
 *  The following collision phases are run: In a first phase
 *  we check for collisions against the tilemap or static objects
 *  and resolve those using impulse based collision resolution.
 *
 */

 class CollisionEngine {
 public:
   CollisionEngine();

   void add_object(MovingObject* mobj);
   void update_solid_tilemaps();
   /** Called when a MovingObject is destructed.
       Removes the object from the broadphase data structure.
    */
   void delete_object(MovingObject* mobj);

   /**
    * Handles the collisions and updates positions.
    */
   void handle_collisions(double t_delta);
private:
  void handle_tilemap_collisions();
  // Member variables
  broadphase m_broadphase;

 };
 #endif
