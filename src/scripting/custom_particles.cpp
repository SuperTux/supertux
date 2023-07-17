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

#include "object/custom_particle_system.hpp"
#include "scripting/custom_particles.hpp"

namespace scripting {

void CustomParticles::set_enabled(bool enable)
{
  SCRIPT_GUARD_VOID;
  object.set_enabled(enable);
}

bool CustomParticles::get_enabled() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_enabled();
}

void CustomParticles::clear()
{
  SCRIPT_GUARD_VOID;
  object.clear();
}

void CustomParticles::spawn_particles(int amount, bool instantly)
{
  SCRIPT_GUARD_VOID;
  if (instantly)
  {
    for (int i  = 0; i < amount; i++)
    {
      object.spawn_particles(0.f);
    }
  }
  else
  {
    // TODO: Implement delayed spawn mode for scripting.
    log_warning << "Delayed spawn mode is not yet implemented for scripting." << std::endl;
  }
}

// =============================================================================
// ============================   ATTRIBUTES   =================================
// =============================================================================

int CustomParticles::get_max_amount()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_max_amount;
}

void CustomParticles::set_max_amount(int amount)
{
  SCRIPT_GUARD_VOID;
  object.m_max_amount = amount;
}

bool CustomParticles::get_cover_screen()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_cover_screen;
}

void CustomParticles::set_cover_screen(bool cover)
{
  SCRIPT_GUARD_VOID;
  object.m_cover_screen = cover;
}

std::string CustomParticles::get_birth_mode()
{
  SCRIPT_GUARD_DEFAULT;
  switch (object.m_particle_birth_mode)
  {
  case CustomParticleSystem::FadeMode::None:
    return "None";

  case CustomParticleSystem::FadeMode::Fade:
    return "Fade";

  case CustomParticleSystem::FadeMode::Shrink:
    return "Shrink";
  
  default:
    return "";
  }
}

void CustomParticles::set_birth_mode(std::string mode)
{
  SCRIPT_GUARD_VOID;
  if (mode == "None")
  {
    object.m_particle_birth_mode = CustomParticleSystem::FadeMode::None;
  }
  else if (mode == "Fade")
  {
    object.m_particle_birth_mode = CustomParticleSystem::FadeMode::Fade;
  }
  else if (mode == "Shrink")
  {
    object.m_particle_birth_mode = CustomParticleSystem::FadeMode::Shrink;
  }
  else
  {
    log_warning << "Invalid option " + mode + "; valid options are: None, Fade, Shrink." << std::endl;
  }
}

std::string CustomParticles::get_death_mode()
{
  SCRIPT_GUARD_DEFAULT;
  switch (object.m_particle_death_mode)
  {
  case CustomParticleSystem::FadeMode::None:
    return "None";

  case CustomParticleSystem::FadeMode::Fade:
    return "Fade";

  case CustomParticleSystem::FadeMode::Shrink:
    return "Shrink";
  
  default:
    return "";
  }
}

void CustomParticles::set_death_mode(std::string mode)
{
  SCRIPT_GUARD_VOID;
  if (mode == "None")
  {
    object.m_particle_death_mode = CustomParticleSystem::FadeMode::None;
  }
  else if (mode == "Fade")
  {
    object.m_particle_death_mode = CustomParticleSystem::FadeMode::Fade;
  }
  else if (mode == "Shrink")
  {
    object.m_particle_death_mode = CustomParticleSystem::FadeMode::Shrink;
  }
  else
  {
    log_warning << "Invalid option " + mode + "; valid options are: None, Fade, Shrink." << std::endl;
  }
}

std::string CustomParticles::get_rotation_mode()
{
  SCRIPT_GUARD_DEFAULT;
  switch (object.m_particle_rotation_mode)
  {
  case CustomParticleSystem::RotationMode::Fixed:
    return "Fixed";

  case CustomParticleSystem::RotationMode::Facing:
    return "Facing";

  case CustomParticleSystem::RotationMode::Wiggling:
    return "Wiggling";
  
  default:
    return "";
  }
}

void CustomParticles::set_rotation_mode(std::string mode)
{
  SCRIPT_GUARD_VOID;
  if (mode == "Fixed")
  {
    object.m_particle_rotation_mode = CustomParticleSystem::RotationMode::Fixed;
  }
  else if (mode == "Facing")
  {
    object.m_particle_rotation_mode = CustomParticleSystem::RotationMode::Facing;
  }
  else if (mode == "Wiggling")
  {
    object.m_particle_rotation_mode = CustomParticleSystem::RotationMode::Wiggling;
  }
  else
  {
    log_warning << "Invalid option " + mode + "; valid options are: Fixed, Facing, Wiggling." << std::endl;
  }
}

std::string CustomParticles::get_collision_mode()
{
  SCRIPT_GUARD_DEFAULT;
  switch (object.m_particle_collision_mode)
  {
  case CustomParticleSystem::CollisionMode::Ignore:
    return "Ignore";

  case CustomParticleSystem::CollisionMode::Stick:
    return "Stick";

  case CustomParticleSystem::CollisionMode::StickForever:
    return "StickForever";

  case CustomParticleSystem::CollisionMode::BounceHeavy:
    return "BounceHeavy";

  case CustomParticleSystem::CollisionMode::BounceLight:
    return "BounceLight";

  case CustomParticleSystem::CollisionMode::Destroy:
    return "Destroy";
  
  default:
    return "";
  }
}

void CustomParticles::set_collision_mode(std::string mode)
{
  SCRIPT_GUARD_VOID;
  if (mode == "Ignore")
  {
    object.m_particle_collision_mode = CustomParticleSystem::CollisionMode::Ignore;
  }
  else if (mode == "Stick")
  {
    object.m_particle_collision_mode = CustomParticleSystem::CollisionMode::Stick;
  }
  else if (mode == "StickForever")
  {
    object.m_particle_collision_mode = CustomParticleSystem::CollisionMode::StickForever;
  }
  else if (mode == "BounceHeavy")
  {
    object.m_particle_collision_mode = CustomParticleSystem::CollisionMode::BounceHeavy;
  }
  else if (mode == "BounceLight")
  {
    object.m_particle_collision_mode = CustomParticleSystem::CollisionMode::BounceLight;
  }
  else if (mode == "Destroy")
  {
    object.m_particle_collision_mode = CustomParticleSystem::CollisionMode::Destroy;
  }
  else
  {
    log_warning << "Invalid option " + mode + "; valid options are: Ignore, Stick, StickForever, BounceHeavy, BounceLight, Destroy." << std::endl;
  }
}

std::string CustomParticles::get_offscreen_mode()
{
  SCRIPT_GUARD_DEFAULT;
  switch (object.m_particle_offscreen_mode)
  {
  case CustomParticleSystem::OffscreenMode::Never:
    return "Never";

  case CustomParticleSystem::OffscreenMode::OnlyOnExit:
    return "OnlyOnExit";

  case CustomParticleSystem::OffscreenMode::Always:
    return "Always";
  
  default:
    return "";
  }
}

void CustomParticles::set_offscreen_mode(std::string mode)
{
  SCRIPT_GUARD_VOID;
  if (mode == "Never")
  {
    object.m_particle_offscreen_mode = CustomParticleSystem::OffscreenMode::Never;
  }
  else if (mode == "OnlyOnExit")
  {
    object.m_particle_offscreen_mode = CustomParticleSystem::OffscreenMode::OnlyOnExit;
  }
  else if (mode == "Always")
  {
    object.m_particle_offscreen_mode = CustomParticleSystem::OffscreenMode::Always;
  }
  else
  {
    log_warning << "Invalid option " + mode + "; valid options are: Never, OnlyOnExit, Always." << std::endl;
  }
}

// =============================================================================
//   Delay
// -----------------------------------------------------------------------------

float CustomParticles::get_delay()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_delay;
}

void CustomParticles::set_delay(float delay)
{
  SCRIPT_GUARD_VOID;
  object.m_delay = delay;
}

void CustomParticles::fade_delay(float delay, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_delay, delay, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_delay(float delay, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_delay, delay, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Lifetime
// -----------------------------------------------------------------------------

float CustomParticles::get_lifetime()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_lifetime;
}

void CustomParticles::set_lifetime(float lifetime)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_lifetime = lifetime;
}

void CustomParticles::fade_lifetime(float lifetime, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_lifetime, lifetime, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_lifetime(float lifetime, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_lifetime, lifetime, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Lifetime_variation
// -----------------------------------------------------------------------------

float CustomParticles::get_lifetime_variation()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_lifetime_variation;
}

void CustomParticles::set_lifetime_variation(float lifetime_variation)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_lifetime_variation = lifetime_variation;
}

void CustomParticles::fade_lifetime_variation(float lifetime_variation, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_lifetime_variation, lifetime_variation, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_lifetime_variation(float lifetime_variation, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_lifetime_variation, lifetime_variation, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Birth_time
// -----------------------------------------------------------------------------

float CustomParticles::get_birth_time()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_birth_time;
}

void CustomParticles::set_birth_time(float birth_time)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_birth_time = birth_time;
}

void CustomParticles::fade_birth_time(float birth_time, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_birth_time, birth_time, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_birth_time(float birth_time, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_birth_time, birth_time, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Birth_time_variation
// -----------------------------------------------------------------------------

float CustomParticles::get_birth_time_variation()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_birth_time_variation;
}

void CustomParticles::set_birth_time_variation(float birth_time_variation)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_birth_time_variation = birth_time_variation;
}

void CustomParticles::fade_birth_time_variation(float birth_time_variation, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_birth_time_variation, birth_time_variation, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_birth_time_variation(float birth_time_variation, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_birth_time_variation, birth_time_variation, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Death_time
// -----------------------------------------------------------------------------

float CustomParticles::get_death_time()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_death_time;
}

void CustomParticles::set_death_time(float death_time)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_death_time = death_time;
}

void CustomParticles::fade_death_time(float death_time, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_death_time, death_time, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_death_time(float death_time, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_death_time, death_time, time, getEasingByName(EasingMode_from_string(easing)));
}



// =============================================================================
//   death_time_variation
// -----------------------------------------------------------------------------
float CustomParticles::get_death_time_variation()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_death_time_variation;
}

void CustomParticles::set_death_time_variation(float death_time_variation)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_death_time_variation = death_time_variation;
}

void CustomParticles::fade_death_time_variation(float death_time_variation, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_death_time_variation, death_time_variation, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_death_time_variation(float death_time_variation, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_death_time_variation, death_time_variation, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Speed_x
// -----------------------------------------------------------------------------

float CustomParticles::get_speed_x()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_speed_x;
}

void CustomParticles::set_speed_x(float speed_x)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_speed_x = speed_x;
}

void CustomParticles::fade_speed_x(float speed_x, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_speed_x, speed_x, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_speed_x(float speed_x, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_speed_x, speed_x, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Speed_y
// -----------------------------------------------------------------------------

float CustomParticles::get_speed_y()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_speed_y;
}

void CustomParticles::set_speed_y(float speed_y)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_speed_y = speed_y;
}

void CustomParticles::fade_speed_y(float speed_y, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_speed_y, speed_y, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_speed_y(float speed_y, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_speed_y, speed_y, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Speed_variation_x
// -----------------------------------------------------------------------------

float CustomParticles::get_speed_variation_x()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_speed_variation_x;
}

void CustomParticles::set_speed_variation_x(float speed_variation_x)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_speed_variation_x = speed_variation_x;
}

void CustomParticles::fade_speed_variation_x(float speed_variation_x, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_speed_variation_x, speed_variation_x, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_speed_variation_x(float speed_variation_x, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_speed_variation_x, speed_variation_x, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Speed_variation_y
// -----------------------------------------------------------------------------

float CustomParticles::get_speed_variation_y()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_speed_variation_y;
}

void CustomParticles::set_speed_variation_y(float speed_variation_y)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_speed_variation_y = speed_variation_y;
}

void CustomParticles::fade_speed_variation_y(float speed_variation_y, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_speed_variation_y, speed_variation_y, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_speed_variation_y(float speed_variation_y, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_speed_variation_y, speed_variation_y, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Acceleration_x
// -----------------------------------------------------------------------------

float CustomParticles::get_acceleration_x()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_acceleration_x;
}

void CustomParticles::set_acceleration_x(float acceleration_x)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_acceleration_x = acceleration_x;
}

void CustomParticles::fade_acceleration_x(float acceleration_x, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_acceleration_x, acceleration_x, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_acceleration_x(float acceleration_x, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_acceleration_x, acceleration_x, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Acceleration_y
// -----------------------------------------------------------------------------

float CustomParticles::get_acceleration_y()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_acceleration_y;
}

void CustomParticles::set_acceleration_y(float acceleration_y)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_acceleration_y = acceleration_y;
}

void CustomParticles::fade_acceleration_y(float acceleration_y, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_acceleration_y, acceleration_y, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_acceleration_y(float acceleration_y, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_acceleration_y, acceleration_y, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Friction_x
// -----------------------------------------------------------------------------

float CustomParticles::get_friction_x()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_friction_x;
}

void CustomParticles::set_friction_x(float friction_x)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_friction_x = friction_x;
}

void CustomParticles::fade_friction_x(float friction_x, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_friction_x, friction_x, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_friction_x(float friction_x, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_friction_x, friction_x, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Friction_y
// -----------------------------------------------------------------------------

float CustomParticles::get_friction_y()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_friction_y;
}

void CustomParticles::set_friction_y(float friction_y)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_friction_y = friction_y;
}

void CustomParticles::fade_friction_y(float friction_y, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_friction_y, friction_y, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_friction_y(float friction_y, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_friction_y, friction_y, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Feather_factor
// -----------------------------------------------------------------------------

float CustomParticles::get_feather_factor()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_feather_factor;
}

void CustomParticles::set_feather_factor(float feather_factor)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_feather_factor = feather_factor;
}

void CustomParticles::fade_feather_factor(float feather_factor, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_feather_factor, feather_factor, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_feather_factor(float feather_factor, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_feather_factor, feather_factor, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Rotation
// -----------------------------------------------------------------------------

float CustomParticles::get_rotation()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_rotation;
}

void CustomParticles::set_rotation(float rotation)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_rotation = rotation;
}

void CustomParticles::fade_rotation(float rotation, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation, rotation, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_rotation(float rotation, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation, rotation, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Rotation_variation
// -----------------------------------------------------------------------------

float CustomParticles::get_rotation_variation()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_rotation_variation;
}

void CustomParticles::set_rotation_variation(float rotation_variation)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_rotation_variation = rotation_variation;
}

void CustomParticles::fade_rotation_variation(float rotation_variation, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation_variation, rotation_variation, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_rotation_variation(float rotation_variation, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation_variation, rotation_variation, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Rotation_speed
// -----------------------------------------------------------------------------

float CustomParticles::get_rotation_speed()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_rotation_speed;
}

void CustomParticles::set_rotation_speed(float rotation_speed)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_rotation_speed = rotation_speed;
}

void CustomParticles::fade_rotation_speed(float rotation_speed, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation_speed, rotation_speed, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_rotation_speed(float rotation_speed, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation_speed, rotation_speed, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Rotation_speed_variation
// -----------------------------------------------------------------------------

float CustomParticles::get_rotation_speed_variation()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_rotation_speed_variation;
}

void CustomParticles::set_rotation_speed_variation(float rotation_speed_variation)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_rotation_speed_variation = rotation_speed_variation;
}

void CustomParticles::fade_rotation_speed_variation(float rotation_speed_variation, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation_speed_variation, rotation_speed_variation, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_rotation_speed_variation(float rotation_speed_variation, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation_speed_variation, rotation_speed_variation, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Rotation_acceleration
// -----------------------------------------------------------------------------

float CustomParticles::get_rotation_acceleration()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_rotation_acceleration;
}

void CustomParticles::set_rotation_acceleration(float rotation_acceleration)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_rotation_acceleration = rotation_acceleration;
}

void CustomParticles::fade_rotation_acceleration(float rotation_acceleration, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation_acceleration, rotation_acceleration, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_rotation_acceleration(float rotation_acceleration, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation_acceleration, rotation_acceleration, time, getEasingByName(EasingMode_from_string(easing)));
}

// =============================================================================
//   Rotation_decceleration
// -----------------------------------------------------------------------------

float CustomParticles::get_rotation_decceleration()
{
  SCRIPT_GUARD_DEFAULT;
  return object.m_particle_rotation_decceleration;
}

void CustomParticles::set_rotation_decceleration(float rotation_decceleration)
{
  SCRIPT_GUARD_VOID;
  object.m_particle_rotation_decceleration = rotation_decceleration;
}

void CustomParticles::fade_rotation_decceleration(float rotation_decceleration, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation_decceleration, rotation_decceleration, time, getEasingByName(EasingMode::EaseNone));
}

void CustomParticles::ease_rotation_decceleration(float rotation_decceleration, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_value(&object.m_particle_rotation_decceleration, rotation_decceleration, time, getEasingByName(EasingMode_from_string(easing)));
}

} // namespace scripting

/* EOF */
