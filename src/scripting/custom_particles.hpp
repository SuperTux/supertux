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

#ifndef HEADER_SUPERTUX_SCRIPTING_CUSTOM_PARTICLES_HPP
#define HEADER_SUPERTUX_SCRIPTING_CUSTOM_PARTICLES_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class CustomParticleSystem;
#endif

namespace scripting {

class CustomParticles final
#ifndef SCRIPTING_API
  : public GameObject<::CustomParticleSystem>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  CustomParticles(const CustomParticles&) = delete;
  CustomParticles& operator=(const CustomParticles&) = delete;
#endif

public:
  void set_enabled(bool enable);
  bool get_enabled() const;

  /** Instantly removes all particles of that type on the screen */
  void clear();

  /**
   * Spawns particles regardless of whether or not the particles are enabled.
   * If "instantly" is true, disregard the delay settings.
   */
  void spawn_particles(int amount, bool instantly);



  int get_max_amount();
  void set_max_amount(int amount);

  std::string get_birth_mode();
  void set_birth_mode(std::string mode);

  std::string get_death_mode();
  void set_death_mode(std::string mode);

  std::string get_rotation_mode();
  void set_rotation_mode(std::string mode);

  std::string get_collision_mode();
  void set_collision_mode(std::string mode);

  std::string get_offscreen_mode();
  void set_offscreen_mode(std::string mode);

  bool get_cover_screen();
  void set_cover_screen(bool cover);

  float get_delay();
  void set_delay(float delay);
  void fade_delay(float delay, float time);
  void ease_delay(float delay, float time, std::string easing);

  float get_lifetime();
  void set_lifetime(float lifetime);
  void fade_lifetime(float lifetime, float time);
  void ease_lifetime(float lifetime, float time, std::string easing);

  float get_lifetime_variation();
  void set_lifetime_variation(float lifetime_variation);
  void fade_lifetime_variation(float lifetime_variation, float time);
  void ease_lifetime_variation(float lifetime_variation, float time, std::string easing);

  float get_birth_time();
  void set_birth_time(float birth_time);
  void fade_birth_time(float birth_time, float time);
  void ease_birth_time(float birth_time, float time, std::string easing);

  float get_birth_time_variation();
  void set_birth_time_variation(float birth_time_variation);
  void fade_birth_time_variation(float birth_time_variation, float time);
  void ease_birth_time_variation(float birth_time_variation, float time, std::string easing);

  float get_death_time();
  void set_death_time(float death_time);
  void fade_death_time(float death_time, float time);
  void ease_death_time(float death_time, float time, std::string easing);

  float get_death_time_variation();
  void set_death_time_variation(float death_time_variation);
  void fade_death_time_variation(float death_time_variation, float time);
  void ease_death_time_variation(float death_time_variation, float time, std::string easing);

  float get_speed_x();
  void set_speed_x(float speed_x);
  void fade_speed_x(float speed_x, float time);
  void ease_speed_x(float speed_x, float time, std::string easing);

  float get_speed_y();
  void set_speed_y(float speed_y);
  void fade_speed_y(float speed_y, float time);
  void ease_speed_y(float speed_y, float time, std::string easing);

  float get_speed_variation_x();
  void set_speed_variation_x(float speed_variation_x);
  void fade_speed_variation_x(float speed_variation_x, float time);
  void ease_speed_variation_x(float speed_variation_x, float time, std::string easing);

  float get_speed_variation_y();
  void set_speed_variation_y(float speed_variation_y);
  void fade_speed_variation_y(float speed_variation_y, float time);
  void ease_speed_variation_y(float speed_variation_y, float time, std::string easing);

  float get_acceleration_x();
  void set_acceleration_x(float acceleration_x);
  void fade_acceleration_x(float acceleration_x, float time);
  void ease_acceleration_x(float acceleration_x, float time, std::string easing);

  float get_acceleration_y();
  void set_acceleration_y(float acceleration_y);
  void fade_acceleration_y(float acceleration_y, float time);
  void ease_acceleration_y(float acceleration_y, float time, std::string easing);

  float get_friction_x();
  void set_friction_x(float friction_x);
  void fade_friction_x(float friction_x, float time);
  void ease_friction_x(float friction_x, float time, std::string easing);

  float get_friction_y();
  void set_friction_y(float friction_y);
  void fade_friction_y(float friction_y, float time);
  void ease_friction_y(float friction_y, float time, std::string easing);

  float get_feather_factor();
  void set_feather_factor(float feather_factor);
  void fade_feather_factor(float feather_factor, float time);
  void ease_feather_factor(float feather_factor, float time, std::string easing);

  float get_rotation();
  void set_rotation(float rotation);
  void fade_rotation(float rotation, float time);
  void ease_rotation(float rotation, float time, std::string easing);

  float get_rotation_variation();
  void set_rotation_variation(float rotation_variation);
  void fade_rotation_variation(float rotation_variation, float time);
  void ease_rotation_variation(float rotation_variation, float time, std::string easing);

  float get_rotation_speed();
  void set_rotation_speed(float rotation_speed);
  void fade_rotation_speed(float rotation_speed, float time);
  void ease_rotation_speed(float rotation_speed, float time, std::string easing);

  float get_rotation_speed_variation();
  void set_rotation_speed_variation(float rotation_speed_variation);
  void fade_rotation_speed_variation(float rotation_speed_variation, float time);
  void ease_rotation_speed_variation(float rotation_speed_variation, float time, std::string easing);

  float get_rotation_acceleration();
  void set_rotation_acceleration(float rotation_acceleration);
  void fade_rotation_acceleration(float rotation_acceleration, float time);
  void ease_rotation_acceleration(float rotation_acceleration, float time, std::string easing);

  float get_rotation_decceleration();
  void set_rotation_decceleration(float rotation_decceleration);
  void fade_rotation_decceleration(float rotation_decceleration, float time);
  void ease_rotation_decceleration(float rotation_decceleration, float time, std::string easing);

};

} // namespace scripting

#endif

/* EOF */
