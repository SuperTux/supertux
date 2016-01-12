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

class Path;
class PathWalker;
class SoundSource;

#include "supertux/script_interface.hpp"

class WillOWisp : public BadGuy,
                  public ScriptInterface
{
public:
  WillOWisp(const ReaderMapping& reader);

  void activate();
  void deactivate();

  void active_update(float elapsed_time);
  virtual bool is_flammable() const { return false; }
  virtual bool is_freezable() const { return false; }
  virtual void kill_fall() { vanish(); }

  /**
   * make WillOWisp vanish
   */
  void vanish();

  virtual void goto_node(int node_no);
  virtual void set_state(const std::string& state);
  virtual void start_moving();
  virtual void stop_moving();

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

protected:
  virtual bool collides(GameObject& other, const CollisionHit& hit) const;
  HitResponse collision_player(Player& player, const CollisionHit& hit);

private:
  enum MyState {
    STATE_STOPPED, STATE_IDLE, STATE_TRACKING, STATE_VANISHING, STATE_WARPING,
    STATE_PATHMOVING, STATE_PATHMOVING_TRACK
  };

private:
  MyState mystate;

  std::string target_sector;
  std::string target_spawnpoint;
  std::string hit_script;

  std::unique_ptr<SoundSource> sound_source;

  std::unique_ptr<Path>        path;
  std::unique_ptr<PathWalker>  walker;

  float flyspeed;
  float track_range;
  float vanish_range;
};

#endif

/* EOF */
