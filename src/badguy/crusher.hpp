//  Crusher - A block to stand on, which can drop down to crush the player
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_CRUSHER_HPP
#define HEADER_SUPERTUX_OBJECT_CRUSHER_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"
#include "supertux/timer.hpp"

class Player;

/** This class is the base class for crushers that tux can stand on */
class Crusher final : public MovingSprite
{
public:
  enum CrusherState
  {
    IDLE,
    CRUSHING,
    DELAY,
    RECOVERING
  };

private:
  enum CrusherSize
  {
    NORMAL,
    LARGE
  };

  enum Type
  {
    ICE,
    ROCK,
    CORRUPTED
  };

public:
  Crusher(const ReaderMapping& reader);

  virtual HitResponse collision(MovingObject& other, const CollisionHit& hit) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  virtual void after_editor_set() override;
  void after_sprite_set();
  inline bool is_sideways() const { return m_sideways; }

  static std::string class_name() { return "crusher"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Crusher"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual GameObjectClasses get_class_types() const override { return MovingSprite::get_class_types().add(typeid(Crusher)); }
  virtual void on_type_change(int old_type);

  virtual ObjectSettings get_settings() override;
  GameObjectTypes get_types() const override;
  std::string get_default_sprite_name() const override;

  inline Physic& get_physic() { return m_physic; }
  inline bool is_big() const { return m_ic_size == LARGE; }
  inline CrusherState get_state() const { return m_state; }

private:
  bool should_crush();

  void crush();
  void recover();

private:
  CrusherState m_state;
  Timer m_state_timer;
  CrusherSize m_ic_size;
  Type m_ic_type;
  Vector m_start_position;
  Physic m_physic;
  bool m_sideways;

private:
  Crusher(const Crusher&) = delete;
  Crusher& operator=(const Crusher&) = delete;
};

#endif

/* EOF */
