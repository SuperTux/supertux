#if 0
#include "physics/collision_handler.hpp"

CollisionHandler::CollisionHandler() {
}

CollisionHandler::add_object(MovingObject* mobj) {
  m_broadphase.insert(mobj);
}

CollisionHandler::delete_object(MovingObject* mobj) {
  m_broadphase.remove(mobj);
}

CollisionHandler::update_solid_tilemaps(std::vector<TileMap&> solid_tms) {
  this->solid_tms = solid_tms;
}

CollisionHandler::handle_collisions(double t_delta) {
  // First step: Integrate the positions of all objects

  // Collect TileMap collisions (=> save for later movement)

  //
}
#endif
