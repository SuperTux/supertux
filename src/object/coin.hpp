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

#include "object/path_object.hpp"
#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"

class Path;
class PathWalker;
class TileMap;

class Coin : public MovingSprite,
             public PathObject
{

friend class HeavyCoin;

public:
  Coin(const Vector& pos);
  Coin(const ReaderMapping& reader);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  void collect();
  virtual void update(float elapsed_time) override;
  virtual void save(Writer& writer) override;
  virtual std::string get_class() const override {
    return "coin";
  }
  virtual std::string get_display_name() const override {
    return _("Coin");
  }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual void move_to(const Vector& pos) override;

private:
  Vector m_offset;
  bool m_from_tilemap;
  bool m_add_path;
  Physic m_physic;
  std::string m_collect_script;
};

class HeavyCoin final : public Coin
{
public:
  HeavyCoin(const Vector& pos, const Vector& init_velocity);
  HeavyCoin(const ReaderMapping& reader);

  virtual void update(float elapsed_time) override;
  virtual void collision_solid(const CollisionHit& hit) override;

  virtual std::string get_class() const override {
    return "heavycoin";
  }
  virtual std::string get_display_name() const override {
    return _("Heavy coin");
  }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

private:
  Physic m_physic;
};

#endif

/* EOF */
