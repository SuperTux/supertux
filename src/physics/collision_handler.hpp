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

class CollisionHandler {
 public:
  CollisionHandler();
  /**
   *  Called when a moving object is added to the game.
   *  Inserts the new object into the broadphase data structure.
   */
  void add_object(MovingObject* mobj);
  /**
   *  Use this method to notify about changes in the solid tilemaps.
   *  <i>Could</i> be called before every collision cycle.
   */
  void update_solid_tilemaps();
   /** Called when a MovingObject is destructed.
       Removes the object from the broadphase data structure.
    */
  void delete_object(MovingObject* mobj);

   /**
    * Handles the collisions and updates positions.
    * Advances game time by @param t_delta.
    */
  void handle_collisions(double t_delta);

 private:
  /**
   * Resolves collisions against the tilemap for every object.
   */
  void handle_tilemap_collisions();
  // Member variables
  broadphase m_broadphase; /*< Broadphase data structure */
  std::set< MovingObject* > m_mobjects; /*< set of all moving objects */
};
  #endif
