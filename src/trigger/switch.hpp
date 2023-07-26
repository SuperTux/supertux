//  SuperTux - Switch Trigger
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_TRIGGER_SWITCH_HPP
#define HEADER_SUPERTUX_TRIGGER_SWITCH_HPP

#include "trigger/trigger_base.hpp"

class Switch final : public SpritedTrigger
{
public:
  Switch(const ReaderMapping& reader);
  ~Switch() override;

  static std::string class_name() { return "switch"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Switch"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;

  GameObjectTypes get_types() const override;
  std::string get_default_sprite_name() const override;

  virtual void update(float dt_sec) override;
  virtual void event(Player& player, EventType type) override;

  virtual void on_flip(float height) override;

private:
  enum Type {
    SIDED_LEFT,
    SIDED_RIGHT,
    WALL
  };

  enum SwitchState {
    OFF,
    TURN_ON,
    ON,
    TURN_OFF
  };

private:
  std::string script;
  std::string off_script;
  SwitchState state;
  bool bistable;

private:
  Switch(const Switch&) = delete;
  Switch& operator=(const Switch&) = delete;
};

#endif

/* EOF */
