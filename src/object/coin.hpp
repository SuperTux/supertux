//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_COIN_HPP
#define HEADER_SUPERTUX_OBJECT_COIN_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"

class Path;
class PathWalker;
class TileMap;

class Coin : public MovingSprite
{
public:
  Coin(const Vector& pos);
  Coin(const Vector& pos, TileMap* tilemap);
  Coin(const ReaderMapping& reader);

  HitResponse collision(GameObject& other, const CollisionHit& hit);

  void collect();
  virtual void update(float elapsed_time);
  virtual void save(Writer& writer);
  std::string get_class() const {
    return "coin";
  }
  std::string get_display_name() const {
    return _("Coin");
  }

  ObjectSettings get_settings();
  void after_editor_set();

  virtual void move_to(const Vector& pos);

  Path* get_path() const {
    return path.get();
  }

private:
  std::shared_ptr<Path> path;
  std::shared_ptr<PathWalker> walker;
  Vector offset;
  bool from_tilemap;
  bool add_path;
  Physic physic;
};

class HeavyCoin : public Coin
{
public:
  HeavyCoin(const Vector& pos, const Vector& init_velocity);
  HeavyCoin(const ReaderMapping& reader);

  virtual void update(float elapsed_time);
  virtual void collision_solid(const CollisionHit& hit);

  virtual std::string get_class() const {
    return "heavycoin";
  }

  ObjectSettings get_settings();
  void after_editor_set();

private:
  Physic physic;
};

#endif

/* EOF */
