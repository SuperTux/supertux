//  SuperTux
//  Copyright (C) 2020 Maxim Bernard <mbernard2@videotron.ca>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>. 

#include "collision/collision_movement_manager.hpp"
#include "object/player.hpp"

void
CollisionGroundMovementManager::register_movement(
  CollisionObject& moving_object,
  CollisionObject& target_object,
  const Vector& movement)
{
  m_movements_per_target[&target_object].register_movement(moving_object, movement);
}

void
CollisionGroundMovementManager::register_movement(
  TileMap& moving_tilemap,
  CollisionObject& target_object,
  const Vector& movement)
{
  m_movements_per_target[&target_object].register_movement(moving_tilemap, movement);
}

void
CollisionGroundMovementManager::apply_all_ground_movement()
{
  do
  {
    // New objects can be added to the map while processing it.
    // Therefore, we make a copy, clear the original, and if
    // new onces get added, the process is repeated.
    auto movements_per_target_initial = m_movements_per_target;
    m_movements_per_target.clear();

    for (auto movements_for_target : movements_per_target_initial)
    {
      CollisionObject& target = *movements_for_target.first;
      TargetMovementData& movements = movements_for_target.second;

      auto& objects_map = movements.get_objects_map();
      auto& tilemaps_map = movements.get_tilemaps_map();

      // Find the lowest "y" position (i.e. the highest point since
      // (0,0) is the top-left corner) and the associated object
      Vector lowest_y_vector;
      bool first_to_do = true;

      for (const auto& movement_for_object : objects_map) {
        if (first_to_do || movement_for_object.second.y < lowest_y_vector.y) {
          lowest_y_vector = movement_for_object.second;
          first_to_do = false;
        }
      }

      for (const auto& movement_for_tilemap : tilemaps_map) {
        if (first_to_do || movement_for_tilemap.second.y < lowest_y_vector.y) {
          lowest_y_vector = movement_for_tilemap.second;
          first_to_do = false;
        }
      }

      if (!first_to_do) {
        // Move the object to the highest possible point.
        target.set_movement(target.get_movement() + lowest_y_vector);
      }
    }
  } while (!m_movements_per_target.empty());
}

void
CollisionGroundMovementManager::TargetMovementData::register_movement(
  CollisionObject& moving_object,
  const Vector& movement)
{
  m_moving_objects[&moving_object] = movement;
}

void
CollisionGroundMovementManager::TargetMovementData::register_movement(
  TileMap& moving_tilemap,
  const Vector& movement)
{
  m_moving_tilemaps[&moving_tilemap] = movement;
}

/* EOF */
