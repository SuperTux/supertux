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

#include "object/particlesystem_interactive.hpp"

#include "math/easing.hpp"
#include "math/vector.hpp"
#include "object/particle_zone.hpp"
#include "video/surface.hpp"
#include "video/surface_ptr.hpp"

class CustomParticleSystem : public ParticleSystem_Interactive
{
  friend class ParticleEditor;

public:
  static void register_class(ssq::VM& vm);

public:
  CustomParticleSystem();
  CustomParticleSystem(const ReaderMapping& reader);
  ~CustomParticleSystem() override;

  virtual void draw(DrawingContext& context) override;

  void reinit_textures();
  virtual void update(float dt_sec) override;

  static std::string class_name() { return "particles-custom"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "CustomParticleSystem"; }
  static std::string display_name() { return _("Custom Particles"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual void save(Writer& writer) override;
  virtual ObjectSettings get_settings() override;

  virtual const std::string get_icon_path() const override {
    return "images/engine/editor/sparkle.png";
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

  std::vector<ParticleZone::ZoneDetails> get_zones() const;

  float get_abs_x() const;
  float get_abs_y() const;

  float texture_sum_odds;
  float time_last_remaining;

public:
  // Scripting
  void ease_value(float* value, float target, float time, easing func);

  /** 
   * Instantly removes all particles of that type on the screen.
   */
  void clear();

  /**
   * Spawns particles regardless of whether or not the particles are enabled.
   * @param int $amount
   * @param bool $instantly If ""true"", disregard the delay settings.
   */
  void spawn_particles(int amount, bool instantly);

  /**
   * @deprecated Use the ""max_amount"" property instead!
   */
  int get_max_amount() const;
  /**
   * @deprecated Use the ""max_amount"" property instead!
   * @param int $amount
   */
  void set_max_amount(int amount);

  std::string get_birth_mode() const;
  /**
   * @param string $mode
   */
  void set_birth_mode(const std::string& mode);

  std::string get_death_mode() const;
  /**
   * @param string $mode
   */
  void set_death_mode(const std::string& mode);

  std::string get_rotation_mode() const;
  /**
   * @param string $mode
   */
  void set_rotation_mode(const std::string& mode);

  std::string get_collision_mode() const;
  /**
   * @param string $mode
   */
  void set_collision_mode(const std::string& mode);

  std::string get_offscreen_mode() const;
  /**
   * @param string $mode
   */
  void set_offscreen_mode(const std::string& mode);

  /**
   * @deprecated Use the ""cover_screen"" property instead!
   */
  bool get_cover_screen() const;
  /**
   * @deprecated Use the ""cover_screen"" property instead!
   * @param bool $cover
   */
  void set_cover_screen(bool cover);

  /**
   * @deprecated Use the ""delay"" property instead!
   */
  float get_delay() const;
  /**
   * @deprecated Use the ""delay"" property instead!
   * @param float $delay
   */
  void set_delay(float delay);
  /**
   * @param float $delay
   * @param float $time
   */
  void fade_delay(float delay, float time);
  /**
   * @param float $delay
   * @param float $time
   * @param string $easing
   */
  void ease_delay(float delay, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_lifetime"" property instead!
   */
  float get_lifetime() const;
  /**
   * @deprecated Use the ""particle_lifetime"" property instead!
   * @param float $lifetime
   */
  void set_lifetime(float lifetime);
  /**
   * @param float $lifetime
   * @param float $time
   */
  void fade_lifetime(float lifetime, float time);
  /**
   * @param float $lifetime
   * @param float $time
   * @param string $easing
   */
  void ease_lifetime(float lifetime, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_lifetime_variation"" property instead!
   */
  float get_lifetime_variation() const;
  /**
   * @deprecated Use the ""particle_lifetime_variation"" property instead!
   * @param float $lifetime_variation
   */
  void set_lifetime_variation(float lifetime_variation);
  /**
   * @param float $lifetime_variation
   * @param float $time
   */
  void fade_lifetime_variation(float lifetime_variation, float time);
  /**
   * @param float $lifetime_variation
   * @param float $time
   * @param string $easing
   */
  void ease_lifetime_variation(float lifetime_variation, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_birth_time"" property instead!
   */
  float get_birth_time() const;
  /**
   * @deprecated Use the ""particle_birth_time"" property instead!
   * @param float $birth_time
   */
  void set_birth_time(float birth_time);
  /**
   * @param float $birth_time
   * @param float $time
   */
  void fade_birth_time(float birth_time, float time);
  /**
   * @param float $birth_time
   * @param float $time
   * @param string $easing
   */
  void ease_birth_time(float birth_time, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_birth_time_variation"" property instead!
   */
  float get_birth_time_variation() const;
  /**
   * @deprecated Use the ""particle_birth_time_variation"" property instead!
   * @param float $birth_time_variation
   */
  void set_birth_time_variation(float birth_time_variation);
  /**
   * @param float $birth_time_variation
   * @param float $time
   */
  void fade_birth_time_variation(float birth_time_variation, float time);
  /**
   * @param float $birth_time_variation
   * @param float $time
   * @param string $easing
   */
  void ease_birth_time_variation(float birth_time_variation, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_death_time"" property instead!
   */
  float get_death_time() const;
  /**
   * @deprecated Use the ""particle_death_time"" property instead!
   * @param float $death_time
   */
  void set_death_time(float death_time);
  /**
   * @param float $death_time
   * @param float $time
   */
  void fade_death_time(float death_time, float time);
  /**
   * @param float $death_time
   * @param float $time
   * @param string $easing
   */
  void ease_death_time(float death_time, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_death_time_variation"" property instead!
   */
  float get_death_time_variation() const;
  /**
   * @deprecated Use the ""particle_death_time_variation"" property instead!
   * @param float $death_time_variation
   */
  void set_death_time_variation(float death_time_variation);
  /**
   * @param float $death_time_variation
   * @param float $time
   */
  void fade_death_time_variation(float death_time_variation, float time);
  /**
   * @param float $death_time_variation
   * @param float $time
   * @param string $easing
   */
  void ease_death_time_variation(float death_time_variation, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_speed_x"" property instead!
   */
  float get_speed_x() const;
  /**
   * @deprecated Use the ""particle_speed_x"" property instead!
   * @param float $speed_x
   */
  void set_speed_x(float speed_x);
  /**
   * @param float $speed_x
   * @param float $time
   */
  void fade_speed_x(float speed_x, float time);
  /**
   * @param float $speed_x
   * @param float $time
   * @param string $easing
   */
  void ease_speed_x(float speed_x, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_speed_y"" property instead!
   */
  float get_speed_y() const;
  /**
   * @deprecated Use the ""particle_speed_y"" property instead!
   * @param float $speed_y
   */
  void set_speed_y(float speed_y);
  /**
   * @param float $speed_y
   * @param float $time
   */
  void fade_speed_y(float speed_y, float time);
  /**
   * @param float $speed_y
   * @param float $time
   * @param string $easing
   */
  void ease_speed_y(float speed_y, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_speed_variation_x"" property instead!
   */
  float get_speed_variation_x() const;
  /**
   * @deprecated Use the ""particle_speed_variation_x"" property instead!
   * @param float $speed_variation_x
   */
  void set_speed_variation_x(float speed_variation_x);
  /**
   * @param float $speed_variation_x
   * @param float $time
   */
  void fade_speed_variation_x(float speed_variation_x, float time);
  /**
   * @param float $speed_variation_x
   * @param float $time
   * @param string $easing
   */
  void ease_speed_variation_x(float speed_variation_x, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_speed_variation_y"" property instead!
   */
  float get_speed_variation_y() const;
  /**
   * @deprecated Use the ""particle_speed_variation_y"" property instead!
   * @param float $speed_variation_y
   */
  void set_speed_variation_y(float speed_variation_y);
  /**
   * @param float $speed_variation_y
   * @param float $time
   */
  void fade_speed_variation_y(float speed_variation_y, float time);
  /**
   * @param float $speed_variation_y
   * @param float $time
   * @param string $easing
   */
  void ease_speed_variation_y(float speed_variation_y, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_acceleration_x"" property instead!
   */
  float get_acceleration_x() const;
  /**
   * @deprecated Use the ""particle_acceleration_x"" property instead!
   * @param float $acceleration_x
   */
  void set_acceleration_x(float acceleration_x);
  /**
   * @param float $acceleration_x
   * @param float $time
   */
  void fade_acceleration_x(float acceleration_x, float time);
  /**
   * @param float $acceleration_x
   * @param float $time
   * @param string $easing
   */
  void ease_acceleration_x(float acceleration_x, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_acceleration_y"" property instead!
   */
  float get_acceleration_y() const;
  /**
   * @deprecated Use the ""particle_acceleration_y"" property instead!
   * @param float $acceleration_y
   */
  void set_acceleration_y(float acceleration_y);
  /**
   * @param float $acceleration_y
   * @param float $time
   */
  void fade_acceleration_y(float acceleration_y, float time);
  /**
   * @param float $acceleration_y
   * @param float $time
   * @param string $easing
   */
  void ease_acceleration_y(float acceleration_y, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_friction_x"" property instead!
   */
  float get_friction_x() const;
  /**
   * @deprecated Use the ""particle_friction_x"" property instead!
   * @param float $friction_x
   */
  void set_friction_x(float friction_x);
  /**
   * @param float $friction_x
   * @param float $time
   */
  void fade_friction_x(float friction_x, float time);
  /**
   * @param float $friction_x
   * @param float $time
   * @param string $easing
   */
  void ease_friction_x(float friction_x, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_friction_y"" property instead!
   */
  float get_friction_y() const;
  /**
   * @deprecated Use the ""particle_friction_y"" property instead!
   * @param float $friction_y
   */
  void set_friction_y(float friction_y);
  /**
   * @param float $friction_y
   * @param float $time
   */
  void fade_friction_y(float friction_y, float time);
  /**
   * @param float $friction_y
   * @param float $time
   * @param string $easing
   */
  void ease_friction_y(float friction_y, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_feather_factor"" property instead!
   */
  float get_feather_factor() const;
  /**
   * @deprecated Use the ""particle_feather_factor"" property instead!
   * @param float $feather_factor
   */
  void set_feather_factor(float feather_factor);
  /**
   * @param float $feather_factor
   * @param float $time
   */
  void fade_feather_factor(float feather_factor, float time);
  /**
   * @param float $feather_factor
   * @param float $time
   * @param string $easing
   */
  void ease_feather_factor(float feather_factor, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_rotation"" property instead!
   */
  float get_rotation() const;
  /**
   * @deprecated Use the ""particle_rotation"" property instead!
   * @param float $rotation
   */
  void set_rotation(float rotation);
  /**
   * @param float $rotation
   * @param float $time
   */
  void fade_rotation(float rotation, float time);
  /**
   * @param float $rotation
   * @param float $time
   * @param string $easing
   */
  void ease_rotation(float rotation, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_rotation_variation"" property instead!
   */
  float get_rotation_variation() const;
  /**
   * @deprecated Use the ""particle_rotation_variation"" property instead!
   * @param float $rotation_variation
   */
  void set_rotation_variation(float rotation_variation);
  /**
   * @param float $rotation_variation
   * @param float $time
   */
  void fade_rotation_variation(float rotation_variation, float time);
  /**
   * @param float $rotation_variation
   * @param float $time
   * @param string $easing
   */
  void ease_rotation_variation(float rotation_variation, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_rotation_speed"" property instead!
   */
  float get_rotation_speed() const;
  /**
   * @deprecated Use the ""particle_rotation_speed"" property instead!
   * @param float $rotation_speed
   */
  void set_rotation_speed(float rotation_speed);
  /**
   * @param float $rotation_speed
   * @param float $time
   */
  void fade_rotation_speed(float rotation_speed, float time);
  /**
   * @param float $rotation_speed
   * @param float $time
   * @param string $easing
   */
  void ease_rotation_speed(float rotation_speed, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_rotation_speed_variation"" property instead!
   */
  float get_rotation_speed_variation() const;
  /**
   * @deprecated Use the ""particle_rotation_speed_variation"" property instead!
   * @param float $rotation_speed_variation
   */
  void set_rotation_speed_variation(float rotation_speed_variation);
  /**
   * @param float $rotation_speed_variation
   * @param float $time
   */
  void fade_rotation_speed_variation(float rotation_speed_variation, float time);
  /**
   * @param float $rotation_speed_variation
   * @param float $time
   * @param string $easing
   */
  void ease_rotation_speed_variation(float rotation_speed_variation, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_rotation_acceleration"" property instead!
   */
  float get_rotation_acceleration() const;
  /**
   * @deprecated Use the ""particle_rotation_acceleration"" property instead!
   * @param float $rotation_acceleration
   */
  void set_rotation_acceleration(float rotation_acceleration);
  /**
   * @param float $rotation_acceleration
   * @param float $time
   */
  void fade_rotation_acceleration(float rotation_acceleration, float time);
  /**
   * @param float $rotation_acceleration
   * @param float $time
   * @param string $easing
   */
  void ease_rotation_acceleration(float rotation_acceleration, float time, const std::string& easing);

  /**
   * @deprecated Use the ""particle_rotation_decceleration"" property instead!
   */
  float get_rotation_decceleration() const;
  /**
   * @deprecated Use the ""particle_rotation_decceleration"" property instead!
   * @param float $rotation_decceleration
   */
  void set_rotation_decceleration(float rotation_decceleration);
  /**
   * @param float $rotation_decceleration
   * @param float $time
   */
  void fade_rotation_decceleration(float rotation_decceleration, float time);
  /**
   * @param float $rotation_decceleration
   * @param float $time
   * @param string $easing
   */
  void ease_rotation_decceleration(float rotation_decceleration, float time, const std::string& easing);

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

  SpriteProperties get_random_texture() const;

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
