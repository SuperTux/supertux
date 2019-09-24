//  SuperTux
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

#ifndef HEADER_SUPERTUX_BADGUY_GHOUL_HPP
#define HEADER_SUPERTUX_BADGUY_GHOUL_HPP

#include "badguy/badguy.hpp"
#include "object/path_object.hpp"

class Ghoul final : public BadGuy,
                    public PathObject
{
public:
  Ghoul(const ReaderMapping& reader);
  virtual std::string get_class() const override { return "ghoul"; }
  virtual std::string get_display_name() const override { return _("Ghoul"); }
  virtual bool is_freezable() const override;
  virtual bool is_flammable() const override;
  
  virtual void finish_construction() override;

  virtual void activate() override;
  virtual void deactivate() override;
  virtual void active_update(float dt_sec) override;
  
  virtual void goto_node(int node_no);
  virtual void set_state(const std::string& state);
  virtual void start_moving();
  virtual void stop_moving();

  virtual void move_to(const Vector& pos) override;
  
protected:
  virtual bool collision_squished(GameObject& object) override;
  
private:
  enum MyState {
    STATE_STOPPED, STATE_IDLE, STATE_TRACKING, STATE_PATHMOVING, STATE_PATHMOVING_TRACK
  };
  
private:
  MyState m_mystate;
  float m_flyspeed;
  float m_track_range;
  
private:
  Ghoul(const Ghoul&) = delete;
  Ghoul& operator=(const Ghoul&) = delete;
};

#endif

/* EOF */