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
#include "scripting/clouds.hpp"
#include "video/surface_ptr.hpp"

class ReaderMapping;

class CloudParticleSystem final :
  public ParticleSystem,
  public ExposedObject<CloudParticleSystem, scripting::Clouds>
{
public:
  CloudParticleSystem();
  CloudParticleSystem(const ReaderMapping& reader);
  ~CloudParticleSystem() override;

  void init();
  virtual void update(float dt_sec) override;

  virtual void draw(DrawingContext& context) override;

  virtual std::string get_class() const override { return "particles-clouds"; }
  virtual std::string get_display_name() const override { return _("Cloud Particles"); }
  virtual ObjectSettings get_settings() override;

  virtual const std::string get_icon_path() const override {
    return "images/engine/editor/clouds.png";
  }

  virtual void backup(Writer& writer) const override;
  virtual void restore(const ReaderMapping& reader) override;

  void fade_speed(float new_speed, float fade_time);
  void fade_amount(int new_amount, float fade_time, float time_between = 0.f);

  // Minimum and maximum multiplier for the amount of clouds
  static int constexpr const max_amount = 500;
  static int constexpr const min_amount = 0;

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx) override {
    ExposedObject<CloudParticleSystem, scripting::Clouds>::expose(vm, table_idx);
  }

  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx) override {
    ExposedObject<CloudParticleSystem, scripting::Clouds>::unexpose(vm, table_idx);
  }

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

  float m_current_amount;
  //float m_target_amount;
  //float m_amount_fade_time_remaining;
  
  int m_current_real_amount;

private:
  CloudParticleSystem(const CloudParticleSystem&) = delete;
  CloudParticleSystem& operator=(const CloudParticleSystem&) = delete;
};

#endif

/* EOF */
