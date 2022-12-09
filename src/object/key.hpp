//  SuperTux
//  Copyright (C) 2022 Daniel Ward <weluvgoatz@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_KEY_HPP
#define HEADER_SUPERTUX_OBJECT_KEY_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"
#include "supertux/timer.hpp"

class Key final : public MovingSprite
{
public:
  Key(const ReaderMapping& mapping);

  virtual void update(float dt_sec) override;

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit_) override;

  static std::string class_name() { return "key"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Key"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;

private:
  enum KeyState {
    NORMAL,
    COLLECT,
    FOLLOW,
    FOUND,
    USE
  };

  std::list<Vector> m_pos_list;
  bool m_collected;
  KeyState m_state;
  Timer m_wait_timer;
  Timer m_unlock_timer;
  Physic m_physic;
  int m_chain_pos;
  Vector m_my_door_pos;
  Color m_color;

private:
  Key(const Key&) = delete;
  Key& operator=(const Key&) = delete;

};

#endif

/* EOF */
