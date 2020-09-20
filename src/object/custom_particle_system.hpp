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

#ifndef HEADER_SUPERTUX_OBJECT_CUSTOM_PARTICLE_SYSTEM_HPP
#define HEADER_SUPERTUX_OBJECT_CUSTOM_PARTICLE_SYSTEM_HPP

#include "math/easing.hpp"
#include "math/vector.hpp"
#include "object/particlesystem_interactive.hpp"
#include "scripting/custom_particles.hpp"
#include "video/surface.hpp"
#include "video/surface_ptr.hpp"

class CustomParticleSystem final :
  public ParticleSystem_Interactive,
  public ExposedObject<CustomParticleSystem, scripting::CustomParticles>
{
public:
  CustomParticleSystem();
  CustomParticleSystem(const ReaderMapping& reader);
  virtual ~CustomParticleSystem();

  virtual void draw(DrawingContext& context) override;

  void init();
  virtual void update(float dt_sec) override;

  virtual std::string get_class() const override { return "particles-custom"; }
  virtual std::string get_display_name() const override { return _("Custom Particles"); }
  virtual ObjectSettings get_settings() override;

  virtual const std::string get_icon_path() const override {
    return "images/engine/editor/sparkle.png";
  }

  //void fade_amount(int new_amount, float fade_time);
protected:
  virtual int collision(Particle* particle, const Vector& movement) override;

private:

  // Local
  void add_particle(float lifetime, float x, float y);
  void spawn_particles(float lifetime);
  
  float texture_sum_odds;
  float time_last_remaining;

  // Scripting

private:
  enum class RotationMode {
    Fixed,
    Facing,
    Wiggling
  };

  enum class FadeMode {
    None,
    Fade,
    Shrink
  };

  enum class CollisionMode {
    Ignore,
    Stick,
    BounceHeavy,
    BounceLight,
    Destroy
  };

  enum class OffscreenMode {
    Never,
    OnlyOnExit,
    Always
  };

  class SpriteProperties final
  {
  public:
    float likeliness;
    Color color;
    SurfacePtr texture;
    Vector scale;
    
    SpriteProperties() :
      likeliness(1.f),
      color(1.f, 1.f, 1.f, 1.f),
      texture(Surface::from_file("images/engine/editor/sparkle.png")),
      scale(1.f, 1.f)
    {
    }
    
    SpriteProperties(SurfacePtr surface) :
      likeliness(1.f),
      color(1.f, 1.f, 1.f, 1.f),
      texture(surface),
      scale(1.f, 1.f)
    {
    }
    
    SpriteProperties(SpriteProperties& sp, float alpha) :
      likeliness(sp.likeliness),
      color(sp.color.red, sp.color.green, sp.color.blue, sp.color.alpha * alpha),
      texture(sp.texture),
      scale(sp.scale)
    {
    }

    inline bool operator==(const SpriteProperties& sp)
    {
      return this->likeliness == sp.likeliness
          && this->color      == sp.color
          && this->texture    == sp.texture
          && this->scale      == sp.scale;
    }
    inline bool operator!=(const SpriteProperties& sp)
    {
      return !operator==(sp);
    }
  };

  SpriteProperties get_random_texture();

  class CustomParticle : public Particle
  {
  public:
    SpriteProperties original_props, props;
    float lifetime, birth_time, death_time,
          total_birth, total_death;
    FadeMode birth_mode, death_mode;
    EasingMode birth_easing, death_easing;
    bool ready_for_deletion;
    float speedX, speedY,
          accX, accY,
          frictionX, frictionY;
    float feather_factor;
    float angle_speed, angle_acc,
          angle_decc;
    RotationMode angle_mode;
    CollisionMode collision_mode;
    OffscreenMode offscreen_mode;
    bool has_been_on_screen;
    bool has_been_in_life_zone;
    bool last_life_zone_required_instakill;

    CustomParticle() :
      original_props(),
      props(),
      lifetime(),
      birth_time(),
      death_time(),
      total_birth(),
      total_death(),
      birth_mode(),
      death_mode(),
      birth_easing(),
      death_easing(),
      ready_for_deletion(false),
      speedX(),
      speedY(),
      accX(),
      accY(),
      frictionX(),
      frictionY(),
      feather_factor(),
      angle_speed(),
      angle_acc(),
      angle_decc(),
      angle_mode(),
      collision_mode(),
      offscreen_mode(),
      has_been_on_screen(),
      has_been_in_life_zone(false),
      last_life_zone_required_instakill(false)
    {}
  };

  std::vector<SpriteProperties> m_textures;
  std::vector<std::unique_ptr<CustomParticle> > custom_particles;

  std::string m_particle_main_texture;
  int m_max_amount;
  float m_delay;
  float m_particle_lifetime;
  float m_particle_lifetime_variation;
  float m_particle_birth_time,
        m_particle_birth_time_variation,
        m_particle_death_time,
        m_particle_death_time_variation;
  FadeMode m_particle_birth_mode,
           m_particle_death_mode;
  EasingMode m_particle_birth_easing,
             m_particle_death_easing;
  float m_particle_speed_x,
        m_particle_speed_y,
        m_particle_speed_variation_x,
        m_particle_speed_variation_y,
        m_particle_acceleration_x,
        m_particle_acceleration_y,
        m_particle_friction_x,
        m_particle_friction_y;
  float m_particle_feather_factor;
  float m_particle_rotation,
        m_particle_rotation_variation,
        m_particle_rotation_speed,
        m_particle_rotation_speed_variation,
        m_particle_rotation_acceleration,
        m_particle_rotation_decceleration;
  RotationMode m_particle_rotation_mode;
  CollisionMode m_particle_collision_mode;
  OffscreenMode m_particle_offscreen_mode;
  bool m_cover_screen;

private:
  CustomParticleSystem(const CustomParticleSystem&) = delete;
  CustomParticleSystem& operator=(const CustomParticleSystem&) = delete;
};

#endif

/* EOF */
