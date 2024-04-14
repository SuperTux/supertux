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

#ifndef HEADER_SUPERTUX_OBJECT_CLOUD_PARTICLE_SYSTEM_HPP
#define HEADER_SUPERTUX_OBJECT_CLOUD_PARTICLE_SYSTEM_HPP

#include "object/particlesystem.hpp"

#include "video/surface_ptr.hpp"

class ReaderMapping;

/**
 * @scripting
 * @summary A ""CloudParticleSystem"" that was given a name can be controlled by scripts.
 * @instances A ""CloudParticleSystem"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class CloudParticleSystem final : public ParticleSystem
{
public:
  static void register_class(ssq::VM& vm);

public:
  CloudParticleSystem();
  CloudParticleSystem(const ReaderMapping& reader);
  ~CloudParticleSystem() override;

  void init();
  virtual void update(float dt_sec) override;

  virtual void draw(DrawingContext& context) override;

  static std::string class_name() { return "particles-clouds"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "CloudParticleSystem"; }
  static std::string display_name() { return _("Cloud Particles"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual ObjectSettings get_settings() override;

  virtual const std::string get_icon_path() const override {
    return "images/engine/editor/clouds.png";
  }

  /**
   * @scripting
   * @description Smoothly changes the rain speed to the given value in ""time"" seconds.
   * @param float $speed
   * @param float $time
   */
  void fade_speed(float speed, float time);
  /**
   * @scripting
   * @description Smoothly changes the amount of particles to the given value in ""time"" seconds.
   * @param int $amount
   * @param float $time
   * @param float $time_between
   */
  void fade_amount(int amount, float time, float time_between);
  /**
   * @scripting
   * @description Smoothly changes the amount of particles to the given value in ""time"" seconds.
   * @param int $amount
   * @param float $time
   */
  void set_amount(int amount, float time);

  // Minimum and maximum multiplier for the amount of clouds
  static int constexpr const max_amount = 500;
  static int constexpr const min_amount = 0;

private:
  /** Returns the amount that got inserted (In case max_amount got hit) */
  int add_clouds(int amount, float fade_time);

  /** Returns the amount that got removed (In case min_amount got hit) */
  int remove_clouds(int amount, float fade_time);

private:
  class CloudParticle : public Particle
  {
  public:
    float speed;
    float target_alpha;
    float target_time_remaining;

    CloudParticle() :
      speed(),
      target_alpha(),
      target_time_remaining()
    {}
  };

  SurfacePtr cloudimage;

  float m_current_speed;
  float m_target_speed;
  float m_speed_fade_time_remaining;

  int m_current_amount;
  int m_current_real_amount;

private:
  CloudParticleSystem(const CloudParticleSystem&) = delete;
  CloudParticleSystem& operator=(const CloudParticleSystem&) = delete;
};

#endif

/* EOF */
