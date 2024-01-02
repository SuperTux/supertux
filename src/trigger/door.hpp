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

#ifndef HEADER_SUPERTUX_TRIGGER_DOOR_HPP
#define HEADER_SUPERTUX_TRIGGER_DOOR_HPP

#include "trigger/trigger_base.hpp"

#include "supertux/timer.hpp"

class Door final : public SpritedTrigger
{
public:
  Door(const ReaderMapping& reader);

  static std::string class_name() { return "door"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Door"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual void event(Player& player, EventType type) override;

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  virtual void on_flip(float height) override;

  bool is_locked() const { return m_locked; }
  void unlock();

  Color get_lock_color() const { return lock_color; }

private:
  enum DoorState {
    CLOSED,
    OPENING,
    OPEN,
    CLOSING,
    LOCKED,
    UNLOCKING
  };

private:
  DoorState state; /**< current state of the door */
  std::string target_sector; /**< target sector to teleport to */
  std::string target_spawnpoint; /**< target spawnpoint to teleport to */
  std::string script;
  SpritePtr lock_sprite;
  Timer stay_open_timer; /**< time until door will close again */
  Timer unlocking_timer;
  Timer lock_warn_timer;
  bool m_locked;
  Color lock_color;

private:
  Door(const Door&) = delete;
  Door& operator=(const Door&) = delete;
};

#endif

/* EOF */
