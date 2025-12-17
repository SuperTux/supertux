//  SuperTux - "Will-O-Wisp" Badguy
//  Copyright (C) 2007 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_TREEWILLOWISP_HPP
#define HEADER_SUPERTUX_BADGUY_TREEWILLOWISP_HPP

#include "badguy/badguy.hpp"
#include "supertux/timer.hpp"

class GhostTree;
class SoundSource;

class TreeWillOWisp final : public BadGuy
{
public:
  TreeWillOWisp(GhostTree* tree, const Vector& pos, float radius, float speed);
  ~TreeWillOWisp() override;
  virtual GameObjectClasses get_class_types() const override { return BadGuy::get_class_types().add(typeid(TreeWillOWisp)); }

  virtual void activate() override;
  virtual void active_update(float dt_sec) override;

  virtual bool is_flammable() const override { return false; }
  virtual bool is_freezable() const override { return false; }
  virtual void kill_fall() override {}

  virtual void draw(DrawingContext& context) override;

  virtual void stop_looping_sounds() override;
  virtual void play_looping_sounds() override;

  void vanish();
  void start_sucking(const Vector& suck_target, float duration);
  void fly_away(const Vector& from);

  void set_color(const Color& color);
  inline Color get_color() const { return m_color; }

  inline bool was_sucked() const { return m_state == STATE_SUCKED; }

private:
  enum MyState {
    STATE_DEFAULT,
    STATE_VANISHING,
    STATE_IDLE,
    STATE_SUCKED,
    STATE_FLY_AWAY
  };

private:
  MyState m_state;

  Color m_color;
  float m_angle;
  float m_radius;
  float m_speed;

  std::unique_ptr<SoundSource> m_sound;
  GhostTree* m_tree;

  Vector m_suck_start;
  Vector m_suck_target;
  Timer m_suck_timer;

  Vector m_fly_dir;

private:
  TreeWillOWisp(const TreeWillOWisp&) = delete;
  TreeWillOWisp& operator=(const TreeWillOWisp&) = delete;
};

#endif

/* EOF */
