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

#ifndef HEADER_SUPERTUX_OBJECT_LEAVES_PARTICLE_SYSTEM_HPP
#define HEADER_SUPERTUX_OBJECT_LEAVES_PARTICLE_SYSTEM_HPP

#include "object/particlesystem.hpp"
#include "supertux/timer.hpp"

class ReaderMapping;

class LeavesParticleSystem : public ParticleSystem
{
public:
  LeavesParticleSystem();
  LeavesParticleSystem(const ReaderMapping& reader);
  virtual ~LeavesParticleSystem();

  void init();
  virtual void update(float elapsed_time);

  std::string type() const
  { return "LeavesParticleSystem"; }
  std::string get_class() const {
    return "particles-leaves";
  }
  std::string get_display_name() const {
    return _("Leaves particles");
  }

  virtual const std::string get_icon_path() const {
    return "images/engine/editor/leaves.png";
  }

private:
  class LeavesParticle : public Particle
  {
  public:
    float speed;
    float wobble;
    float anchorx;
    float drift_speed;

    // Turning speed
    float spin_speed;

    // for inertia
    unsigned int leaf_size;

    LeavesParticle() :
      speed(),
      wobble(),
      anchorx(),
      drift_speed(),
      spin_speed(),
      leaf_size()
    {}
  };

  // Wind is simulated in discrete "gusts"

  // Gust state
  enum State {
    ATTACKING,
    DECAYING,
    SUSTAINING,
    RELEASING,
    RESTING,
    MAX_STATE
  };
  State state;


  // Gust state delay timer
  Timer timer;

  // Peak magnitude of gust is gust_onset * randf(5)
  float gust_onset,
  // Current blowing velocity of gust
        gust_current_velocity;

  SurfacePtr leavesimages[6];

private:
  LeavesParticleSystem(const LeavesParticleSystem&);
  LeavesParticleSystem& operator=(const LeavesParticleSystem&);
};

#endif

/* EOF */
