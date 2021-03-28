//  SuperTux - "Will-O-Wisp" Badguy
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

#ifndef HEADER_SUPERTUX_BADGUY_WILLOWISP_HPP
#define HEADER_SUPERTUX_BADGUY_WILLOWISP_HPP

#include "badguy/badguy.hpp"
#include "object/path_object.hpp"
#include "squirrel/exposed_object.hpp"
#include "scripting/willowisp.hpp"

class SoundSource;

class WillOWisp final :
  public BadGuy,
  public ExposedObject<WillOWisp, scripting::WillOWisp>,
  public PathObject
{
public:
  WillOWisp(const ReaderMapping& reader);

  virtual void finish_construction() override;
  virtual void after_editor_set() override;

  virtual void activate() override;
  virtual void deactivate() override;

  virtual void active_update(float dt_sec) override;
  virtual bool is_flammable() const override { return false; }
  virtual bool is_freezable() const override { return false; }
  virtual bool is_hurtable() const override { return false; }
  virtual void kill_fall() override { vanish(); }

  virtual void goto_node(int node_no);
  virtual void set_state(const std::string& state);
  virtual void start_moving();
  virtual void stop_moving();

  virtual void stop_looping_sounds() override;
  virtual void play_looping_sounds() override;

  virtual std::string get_class() const override { return "willowisp"; }
  virtual std::string get_display_name() const override { return _("Will o' Wisp"); }

  virtual ObjectSettings get_settings() override;
  virtual void move_to(const Vector& pos) override;

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx) override
  {
    ExposedObject<WillOWisp, scripting::WillOWisp>::expose(vm, table_idx);
  }

  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx) override
  {
    ExposedObject<WillOWisp, scripting::WillOWisp>::unexpose(vm, table_idx);
  }

  /** make WillOWisp vanish */
  void vanish();

  Color get_color() const { return m_color; }

private:
  virtual bool collides(GameObject& other, const CollisionHit& hit) const override;
  virtual HitResponse collision_player(Player& player, const CollisionHit& hit) override;

private:
  enum MyState {
    STATE_STOPPED, STATE_IDLE, STATE_TRACKING, STATE_VANISHING, STATE_WARPING,
    STATE_PATHMOVING, STATE_PATHMOVING_TRACK
  };

private:
  MyState m_mystate;

  std::string m_target_sector;
  std::string m_target_spawnpoint;
  std::string m_hit_script;

  std::unique_ptr<SoundSource> m_sound_source;
  float m_flyspeed;
  float m_track_range;
  float m_vanish_range;

  Color m_color;

private:
  WillOWisp(const WillOWisp&) = delete;
  WillOWisp& operator=(const WillOWisp&) = delete;
};

#endif

/* EOF */
