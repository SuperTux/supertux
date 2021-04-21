//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_RAIN_PARTICLE_SYSTEM_HPP
#define HEADER_SUPERTUX_OBJECT_RAIN_PARTICLE_SYSTEM_HPP

#include "math/easing.hpp"
#include "object/particlesystem_interactive.hpp"
#include "scripting/rain.hpp"
#include "video/surface_ptr.hpp"

class RainParticleSystem final :
  public ParticleSystem_Interactive,
  public ExposedObject<RainParticleSystem, scripting::Rain>
{
public:
  RainParticleSystem();
  RainParticleSystem(const ReaderMapping& reader);
  ~RainParticleSystem() override;

  virtual void draw(DrawingContext& context) override;

  void init();
  virtual void update(float dt_sec) override;

  virtual std::string get_class() const override { return "particles-rain"; }
  virtual std::string get_display_name() const override { return _("Rain Particles"); }
  virtual ObjectSettings get_settings() override;

  void fade_speed(float new_speed, float fade_time);
  void fade_angle(float new_angle, float fade_time, easing ease_func);
  void fade_amount(float new_amount, float fade_time);

  virtual const std::string get_icon_path() const override {
    return "images/engine/editor/rain.png";
  }

  // Minimum and maximum multiplier for the amount of particles (intensity)
  static float constexpr const max_amount = 5.0f;
  static float constexpr const min_amount = 0.1f;

  // Minimum value of m_current_amount for the fog to be > 0
  static float constexpr const fog_start_amount = 1.0f;

  // When m_current_amount == max_amount, fog is this value
  static float constexpr const fog_max_value = 0.6f;

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx) override {
    ExposedObject<RainParticleSystem, scripting::Rain>::expose(vm, table_idx);
  }

  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx) override {
    ExposedObject<RainParticleSystem, scripting::Rain>::unexpose(vm, table_idx);
  }

private:
  void set_amount(float amount);
  void set_angle(float angle);

private:
  class RainParticle : public Particle
  {
  public:
    float speed;

    RainParticle() :
      speed()
    {}
  };

  SurfacePtr rainimages[2];

  float m_current_speed;
  float m_target_speed;
  float m_speed_fade_time_remaining;

  float m_begin_angle;
  float m_current_angle;
  float m_target_angle;
  float m_angle_fade_time_remaining;
  float m_angle_fade_time_total;
  easing m_angle_easing;

  float m_current_amount;
  float m_target_amount;
  float m_amount_fade_time_remaining;
  
  float m_current_real_amount;

private:
  RainParticleSystem(const RainParticleSystem&) = delete;
  RainParticleSystem& operator=(const RainParticleSystem&) = delete;
};

#endif

/* EOF */
