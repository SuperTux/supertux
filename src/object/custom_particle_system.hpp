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
#include "object/particle_zone.hpp"
#include "scripting/custom_particles.hpp"
#include "video/surface.hpp"
#include "video/surface_ptr.hpp"

class CustomParticleSystem :
  public ParticleSystem_Interactive,
  public ExposedObject<CustomParticleSystem, scripting::CustomParticles>
{
  friend class ParticleEditor;
  friend class scripting::CustomParticles;
public:
  CustomParticleSystem();
  CustomParticleSystem(const ReaderMapping& reader);
  ~CustomParticleSystem() override;

  virtual void draw(DrawingContext& context) override;

  void reinit_textures();
  virtual void update(float dt_sec) override;

  static std::string class_name() { return "particles-custom"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Custom Particles"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual void save(Writer& writer) override;
  virtual ObjectSettings get_settings() override;

  virtual const std::string get_icon_path() const override {
    return "images/engine/editor/sparkle.png";
  }

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx) override {
    ExposedObject<CustomParticleSystem, scripting::CustomParticles>::expose(vm, table_idx);
  }

  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx) override {
    ExposedObject<CustomParticleSystem, scripting::CustomParticles>::unexpose(vm, table_idx);
  }

  //void fade_amount(int new_amount, float fade_time);
protected:
  virtual int collision(Particle* particle, const Vector& movement) override;
  CollisionHit get_collision(Particle* particle, const Vector& movement);

private:
  struct ease_request
  {
    float* value;
    float begin;
    float end;
    float time_total;
    float time_remain;
    easing func;
  };

  // Local
  void add_particle(float lifetime, float x, float y);
  void spawn_particles(float lifetime);

  std::vector<ParticleZone::ZoneDetails> get_zones();

  float get_abs_x();
  float get_abs_y();

  float texture_sum_odds;
  float time_last_remaining;

public:
  // Scripting
  void clear() { custom_particles.clear(); }
  void ease_value(float* value, float target, float time, easing func);

private:
  std::vector<ease_request> script_easings;

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
    StickForever,
    BounceHeavy,
    BounceLight,
    Destroy,
    FadeOut
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
    Vector hb_scale;
    Vector hb_offset;

    SpriteProperties() :
      likeliness(1.f),
      color(1.f, 1.f, 1.f, 1.f),
      texture(Surface::from_file("images/engine/editor/sparkle.png")),
      scale(1.f, 1.f),
      hb_scale(1.f, 1.f),
      hb_offset(0.f, 0.f)
    {
    }

    SpriteProperties(const SurfacePtr& surface) :
      likeliness(1.f),
      color(1.f, 1.f, 1.f, 1.f),
      texture(surface),
      scale(1.f, 1.f),
      hb_scale(1.f, 1.f),
      hb_offset(0.f, 0.f)
    {
    }

    SpriteProperties(const SpriteProperties& sp, float alpha) :
      likeliness(sp.likeliness),
      color(sp.color.red, sp.color.green, sp.color.blue, sp.color.alpha * alpha),
      texture(sp.texture),
      scale(sp.scale),
      hb_scale(sp.hb_scale),
      hb_offset(sp.hb_offset)
    {
    }

    inline bool operator==(const SpriteProperties& sp)
    {
      return this->likeliness == sp.likeliness
          && this->color      == sp.color
          && this->texture    == sp.texture
          && this->scale      == sp.scale
          && this->hb_scale   == sp.hb_scale
          && this->hb_offset  == sp.hb_offset;
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
    bool stuck;

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
      last_life_zone_required_instakill(false),
      stuck(false)
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

public:
  // TODO: Put all those member variables in some (abstract?) class of which
  //       both CustomParticlesSystem and ParticleProbs will inherit (so that
  //       I don't have to write all the variables 4 times just in the header)

  // For the particle editor
  class ParticleProps final
  {
  public:
    std::vector<SpriteProperties> m_textures;
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

    ParticleProps() :
      m_textures(),
      m_particle_main_texture(),
      m_max_amount(25),
      m_delay(0.1f),
      m_particle_lifetime(5.f),
      m_particle_lifetime_variation(0.f),
      m_particle_birth_time(0.f),
      m_particle_birth_time_variation(0.f),
      m_particle_death_time(0.f),
      m_particle_death_time_variation(0.f),
      m_particle_birth_mode(),
      m_particle_death_mode(),
      m_particle_birth_easing(),
      m_particle_death_easing(),
      m_particle_speed_x(0.f),
      m_particle_speed_y(0.f),
      m_particle_speed_variation_x(0.f),
      m_particle_speed_variation_y(0.f),
      m_particle_acceleration_x(0.f),
      m_particle_acceleration_y(0.f),
      m_particle_friction_x(0.f),
      m_particle_friction_y(0.f),
      m_particle_feather_factor(0.f),
      m_particle_rotation(0.f),
      m_particle_rotation_variation(0.f),
      m_particle_rotation_speed(0.f),
      m_particle_rotation_speed_variation(0.f),
      m_particle_rotation_acceleration(0.f),
      m_particle_rotation_decceleration(0.f),
      m_particle_rotation_mode(),
      m_particle_collision_mode(),
      m_particle_offscreen_mode(),
      m_cover_screen(true)
    {
    }
  };

  std::unique_ptr<ParticleProps> get_props() const
  {
    std::unique_ptr<ParticleProps> props = std::make_unique<ParticleProps>();

    for (auto& texture : m_textures)
      props->m_textures.push_back(texture);
    props->m_particle_main_texture = m_particle_main_texture;
    props->m_max_amount = m_max_amount;
    props->m_delay = m_delay;
    props->m_particle_lifetime = m_particle_lifetime;
    props->m_particle_lifetime_variation = m_particle_lifetime_variation;
    props->m_particle_birth_time = m_particle_birth_time;
    props->m_particle_birth_time_variation = m_particle_birth_time_variation;
    props->m_particle_death_time = m_particle_death_time;
    props->m_particle_death_time_variation = m_particle_death_time_variation;
    props->m_particle_birth_mode = m_particle_birth_mode;
    props->m_particle_death_mode = m_particle_death_mode;
    props->m_particle_birth_easing = m_particle_birth_easing;
    props->m_particle_death_easing = m_particle_death_easing;
    props->m_particle_speed_x = m_particle_speed_x;
    props->m_particle_speed_y = m_particle_speed_y;
    props->m_particle_speed_variation_x = m_particle_speed_variation_x;
    props->m_particle_speed_variation_y = m_particle_speed_variation_y;
    props->m_particle_acceleration_x = m_particle_acceleration_x;
    props->m_particle_acceleration_y = m_particle_acceleration_y;
    props->m_particle_friction_x = m_particle_friction_x;
    props->m_particle_friction_y = m_particle_friction_y;
    props->m_particle_feather_factor = m_particle_feather_factor;
    props->m_particle_rotation = m_particle_rotation;
    props->m_particle_rotation_variation = m_particle_rotation_variation;
    props->m_particle_rotation_speed = m_particle_rotation_speed;
    props->m_particle_rotation_speed_variation = m_particle_rotation_speed_variation;
    props->m_particle_rotation_acceleration = m_particle_rotation_acceleration;
    props->m_particle_rotation_decceleration = m_particle_rotation_decceleration;
    props->m_particle_rotation_mode = m_particle_rotation_mode;
    props->m_particle_collision_mode = m_particle_collision_mode;
    props->m_particle_offscreen_mode = m_particle_offscreen_mode;
    props->m_cover_screen = m_cover_screen;

    return props;
  }

  void set_props(ParticleProps* props)
  {
    m_textures.clear();
    for (auto& texture : props->m_textures)
      m_textures.push_back(texture);
    m_particle_main_texture = props->m_particle_main_texture;
    m_max_amount = props->m_max_amount;
    m_delay = props->m_delay;
    m_particle_lifetime = props->m_particle_lifetime;
    m_particle_lifetime_variation = props->m_particle_lifetime_variation;
    m_particle_birth_time = props->m_particle_birth_time;
    m_particle_birth_time_variation = props->m_particle_birth_time_variation;
    m_particle_death_time = props->m_particle_death_time;
    m_particle_death_time_variation = props->m_particle_death_time_variation;
    m_particle_birth_mode = props->m_particle_birth_mode;
    m_particle_death_mode = props->m_particle_death_mode;
    m_particle_birth_easing = props->m_particle_birth_easing;
    m_particle_death_easing = props->m_particle_death_easing;
    m_particle_speed_x = props->m_particle_speed_x;
    m_particle_speed_y = props->m_particle_speed_y;
    m_particle_speed_variation_x = props->m_particle_speed_variation_x;
    m_particle_speed_variation_y = props->m_particle_speed_variation_y;
    m_particle_acceleration_x = props->m_particle_acceleration_x;
    m_particle_acceleration_y = props->m_particle_acceleration_y;
    m_particle_friction_x = props->m_particle_friction_x;
    m_particle_friction_y = props->m_particle_friction_y;
    m_particle_feather_factor = props->m_particle_feather_factor;
    m_particle_rotation = props->m_particle_rotation;
    m_particle_rotation_variation = props->m_particle_rotation_variation;
    m_particle_rotation_speed = props->m_particle_rotation_speed;
    m_particle_rotation_speed_variation = props->m_particle_rotation_speed_variation;
    m_particle_rotation_acceleration = props->m_particle_rotation_acceleration;
    m_particle_rotation_decceleration = props->m_particle_rotation_decceleration;
    m_particle_rotation_mode = props->m_particle_rotation_mode;
    m_particle_collision_mode = props->m_particle_collision_mode;
    m_particle_offscreen_mode = props->m_particle_offscreen_mode;
    m_cover_screen = props->m_cover_screen;
  }


private:
  CustomParticleSystem(const CustomParticleSystem&) = delete;
  CustomParticleSystem& operator=(const CustomParticleSystem&) = delete;
};

#endif

/* EOF */
