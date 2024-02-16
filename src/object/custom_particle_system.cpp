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

#include <assert.h>
#include <math.h>

#include "collision/collision.hpp"
#include "editor/particle_editor.hpp"
#include "gui/menu_manager.hpp"
#include "math/aatriangle.hpp"
#include "math/easing.hpp"
#include "math/random.hpp"
#include "math/util.hpp"
#include "object/camera.hpp"
#include "object/tilemap.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/game_session.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "util/reader.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/surface_batch.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

CustomParticleSystem::CustomParticleSystem() :
  ExposedObject<CustomParticleSystem, scripting::CustomParticles>(this),
  texture_sum_odds(0.f),
  time_last_remaining(0.f),
  script_easings(),
  m_textures(),
  custom_particles(),
  m_particle_main_texture("/images/engine/editor/sparkle.png"),
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
  reinit_textures();
}

CustomParticleSystem::CustomParticleSystem(const ReaderMapping& reader) :
  ParticleSystem_Interactive(reader),
  ExposedObject<CustomParticleSystem, scripting::CustomParticles>(this),
  texture_sum_odds(0.f),
  time_last_remaining(0.f),
  script_easings(),
  m_textures(),
  custom_particles(),
  m_particle_main_texture("/images/engine/editor/sparkle.png"),
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
  reader.get("main-texture", m_particle_main_texture, "/images/engine/editor/sparkle.png");

  // FIXME: Is there a cleaner way to get a list of textures?
  auto iter = reader.get_iter();
  while (iter.next())
  {
    if (iter.get_key() == "texture")
    {
      ReaderMapping mapping = iter.as_mapping();

      std::string tex;
      if (!mapping.get("surface", tex))
      {
        log_warning << "Texture without surface data ('surface') in " <<
                 mapping.get_doc().get_filename() << ", skipping" << std::endl;
        continue;
      }

      float color_r, color_g, color_b, color_a;
      if (!mapping.get("color_r", color_r))
      {
        log_warning << "Texture without red color field ('color_r') in " <<
                 mapping.get_doc().get_filename() << ", skipping" << std::endl;
        continue;
      }
      if (!mapping.get("color_g", color_g))
      {
        log_warning << "Texture without green color field ('color_g') in " <<
                 mapping.get_doc().get_filename() << ", skipping" << std::endl;
        continue;
      }
      if (!mapping.get("color_b", color_b))
      {
        log_warning << "Texture without blue color field ('color_b') in " <<
                 mapping.get_doc().get_filename() << ", skipping" << std::endl;
        continue;
      }
      if (!mapping.get("color_a", color_a))
      {
        log_warning << "Texture without alpha channel field ('color_a') in " <<
                 mapping.get_doc().get_filename() << ", skipping" << std::endl;
        continue;
      }

      float likeliness;
      if (!mapping.get("likeliness", likeliness))
      {
        log_warning << "Texture without likeliness field in " <<
                 mapping.get_doc().get_filename() << ", skipping" << std::endl;
        continue;
      }

      float scale_x, scale_y;
      if (!mapping.get("scale_x", scale_x))
      {
        log_warning << "Texture without horizontal scale ('scale_x') field in " <<
                 mapping.get_doc().get_filename() << ", skipping" << std::endl;
        continue;
      }
      if (!mapping.get("scale_y", scale_y))
      {
        log_warning << "Texture without vertical scale ('scale_y') field in " <<
                 mapping.get_doc().get_filename() << ", skipping" << std::endl;
        continue;
      }

      auto props = SpriteProperties(Surface::from_file(tex));
      props.likeliness = likeliness;
      props.color = Color(color_r, color_g, color_b, color_a);
      props.scale = Vector(scale_x, scale_y);
      m_textures.push_back(props);
    }
  }

  reader.get("amount", m_max_amount, 25);
  reader.get("delay", m_delay, 0.1f);
  reader.get("lifetime", m_particle_lifetime, 5.f);
  reader.get("lifetime-variation", m_particle_lifetime_variation, 0.f);
  reader.get("birth-time", m_particle_birth_time, 0.f);
  reader.get("birth-time-variation", m_particle_birth_time_variation, 0.f);
  reader.get("death-time", m_particle_death_time, 0.f);
  reader.get("death-time-variation", m_particle_death_time_variation, 0.f);

  reader.get("speed-x", m_particle_speed_x, 0.f);
  reader.get("speed-y", m_particle_speed_y, 0.f);
  reader.get("speed-var-x", m_particle_speed_variation_x, 0.f);
  reader.get("speed-var-y", m_particle_speed_variation_y, 0.f);
  reader.get("acceleration-x", m_particle_acceleration_x, 0.f);
  reader.get("acceleration-y", m_particle_acceleration_y, 0.f);
  reader.get("friction-x", m_particle_friction_x, 0.f);
  reader.get("friction-y", m_particle_friction_y, 0.f);
  reader.get("feather-factor", m_particle_feather_factor, 0.f);

  reader.get("rotation", m_particle_rotation, 0.f);
  reader.get("rotation-variation", m_particle_rotation_variation, 0.f);
  reader.get("rotation-speed", m_particle_rotation_speed, 0.f);
  reader.get("rotation-speed-variation", m_particle_rotation_speed_variation, 0.f);
  reader.get("rotation-acceleration", m_particle_rotation_acceleration, 0.f);
  reader.get("rotation-decceleration", m_particle_rotation_decceleration, 0.f);

  reader.get("cover-screen", m_cover_screen, true);

  std::string rotation_mode;
  if (reader.get("rotation-mode", rotation_mode))
  {
    if (rotation_mode == "wiggling")
    {
      m_particle_rotation_mode = RotationMode::Wiggling;
    }
    else if (rotation_mode == "facing")
    {
      m_particle_rotation_mode = RotationMode::Facing;
    }
    else
    {
      m_particle_rotation_mode = RotationMode::Fixed;
    }
  }
  else
  {
    m_particle_rotation_mode = RotationMode::Fixed;
  }

  std::string birth_mode;
  if (reader.get("birth-mode", birth_mode))
  {
    if (birth_mode == "shrink")
    {
      m_particle_birth_mode = FadeMode::Shrink;
    }
    else if (birth_mode == "fade")
    {
      m_particle_birth_mode = FadeMode::Fade;
    }
    else
    {
      m_particle_birth_mode = FadeMode::None;
    }
  }
  else
  {
    m_particle_birth_mode = FadeMode::None;
  }

  std::string death_mode;
  if (reader.get("death-mode", death_mode))
  {
    if (death_mode == "shrink")
    {
      m_particle_death_mode = FadeMode::Shrink;
    }
    else if (death_mode == "fade")
    {
      m_particle_death_mode = FadeMode::Fade;
    }
    else
    {
      m_particle_death_mode = FadeMode::None;
    }
  }
  else
  {
    m_particle_death_mode = FadeMode::None;
  }

  std::string birth_easing;
  if (reader.get("birth-easing", birth_easing))
  {
    m_particle_birth_easing = EasingMode_from_string(birth_easing);
  }
  else
  {
    m_particle_birth_easing = EasingMode_from_string("");
  }

  std::string death_easing;
  if (reader.get("death-easing", death_easing))
  {
    m_particle_death_easing = EasingMode_from_string(death_easing);
  }
  else
  {
    m_particle_death_easing = EasingMode_from_string("");
  }

  std::string collision_mode;
  if (reader.get("collision-mode", collision_mode))
  {
    if (collision_mode == "stick")
    {
      m_particle_collision_mode = CollisionMode::Stick;
    }
    else if (collision_mode == "stick-forever")
    {
      m_particle_collision_mode = CollisionMode::StickForever;
    }
    else if (collision_mode == "bounce-heavy")
    {
      m_particle_collision_mode = CollisionMode::BounceHeavy;
    }
    else if (collision_mode == "bounce-light")
    {
      m_particle_collision_mode = CollisionMode::BounceLight;
    }
    else if (collision_mode == "destroy")
    {
      m_particle_collision_mode = CollisionMode::Destroy;
    }
    else if (collision_mode == "fade-out")
    {
      m_particle_collision_mode = CollisionMode::FadeOut;
    }
    else
    {
      m_particle_collision_mode = CollisionMode::Ignore;
    }
  }
  else
  {
    m_particle_collision_mode = CollisionMode::Ignore;
  }

  std::string offscreen_mode;
  if (reader.get("offscreen-mode", offscreen_mode))
  {
    if (offscreen_mode == "always")
    {
      m_particle_offscreen_mode = OffscreenMode::Always;
    }
    else if (offscreen_mode == "only-on-exit")
    {
      m_particle_offscreen_mode = OffscreenMode::OnlyOnExit;
    }
    else
    {
      m_particle_offscreen_mode = OffscreenMode::Never;
    }
  }
  else
  {
    m_particle_offscreen_mode = OffscreenMode::Never;
  }

  reinit_textures();
}

CustomParticleSystem::~CustomParticleSystem()
{
}

void
CustomParticleSystem::reinit_textures()
{
  if (!m_textures.size())
  {
    auto props = SpriteProperties(Surface::from_file(m_particle_main_texture));
    props.likeliness = 1.f;
    props.color = Color(1.f, 1.f, 1.f, 1.f);
    props.scale = Vector(1.f, 1.f);
    m_textures.push_back(props);
  }

  texture_sum_odds = 0.f;
  for (const auto& texture : m_textures)
  {
    texture_sum_odds += texture.likeliness;
  }
}

void
CustomParticleSystem::save(Writer& writer)
{
  for (const auto& tex : m_textures)
  {
    writer.start_list("texture");
    writer.write("surface", tex.texture->get_filename());
    writer.write("color_r", tex.color.red);
    writer.write("color_g", tex.color.green);
    writer.write("color_b", tex.color.blue);
    writer.write("color_a", tex.color.alpha);
    writer.write("likeliness", tex.likeliness);
    writer.write("scale_x", tex.scale.x);
    writer.write("scale_y", tex.scale.y);
    writer.end_list("texture");
  }

  GameObject::save(writer);
}

ObjectSettings
CustomParticleSystem::get_settings()
{
  ObjectSettings result = ParticleSystem::get_settings();

  result.add_surface(_("Texture"), &m_particle_main_texture, "main-texture");

  result.add_int(_("Amount"), &m_max_amount, "amount", 25);
  result.add_float(_("Delay"), &m_delay, "delay", 0.1f);
  result.add_float(_("Lifetime"), &m_particle_lifetime, "lifetime", 5.f);
  result.add_float(_("Lifetime variation"), &m_particle_lifetime_variation, "lifetime-variation", 0.f);
  result.add_enum(_("Birth mode"), reinterpret_cast<int*>(&m_particle_birth_mode),
                  {_("None"), _("Fade"), _("Shrink")},
                  {"none", "fade", "shrink"},
                  static_cast<int>(FadeMode::None),
                  "birth-mode");
  result.add_enum(_("Birth easing"), reinterpret_cast<int*>(&m_particle_birth_easing),
                  {
                    _("No easing"),
                    _("Quad in"), _("Quad out"), _("Quad in/out"),
                    _("Cubic in"), _("Cubic out"), _("Cubic in/out"),
                    _("Quart in"), _("Quart out"), _("Quart in/out"),
                    _("Quint in"), _("Quint out"), _("Quint in/out"),
                    _("Sine in"), _("Sine out"), _("Sine in/out"),
                    _("Circular in"), _("Circular out"), _("Circular in/out"),
                    _("Exponential in"), _("Exponential out"), _("Exponential in/out"),
                    _("Elastic in"), _("Elastic out"), _("Elastic in/out"),
                    _("Back in"), _("Back out"), _("Back in/out"),
                    _("Bounce in"), _("Bounce out"), _("Bounce in/out")
                  },
                  {
                    "EaseNone",
                    "EaseQuadIn", "EaseQuadOut", "EaseQuadInOut",
                    "EaseCubicIn", "EaseCubicOut", "EaseCubicInOut",
                    "EaseQuartIn", "EaseQuartOut", "EaseQuartInOut",
                    "EaseQuintIn", "EaseQuintOut", "EaseQuintInOut",
                    "EaseSineIn", "EaseSineOut", "EaseSineInOut",
                    "EaseCircularIn", "EaseCircularOut", "EaseCircularInOut",
                    "EaseExponentialIn", "EaseExponentialOut", "EaseExponentialInOut",
                    "EaseElasticIn", "EaseElasticOut", "EaseElasticInOut",
                    "EaseBackIn", "EaseBackOut", "EaseBackInOut",
                    "EaseBounceIn", "EaseBounceOut", "EaseBounceInOut"
                  },
                  0, "birth-easing");
  result.add_float(_("Birth time"), &m_particle_birth_time, "birth-time", 5.f);
  result.add_float(_("Birth time variation"), &m_particle_birth_time_variation, "birth-time-variation", 0.f);
  result.add_enum(_("Death mode"), reinterpret_cast<int*>(&m_particle_death_mode),
                  {_("None"), _("Fade"), _("Shrink")},
                  {"none", "fade", "shrink"},
                  static_cast<int>(FadeMode::None),
                  "death-mode");
  result.add_enum(_("Death easing"), reinterpret_cast<int*>(&m_particle_death_easing),
                  {
                    _("No easing"),
                    _("Quad in"), _("Quad out"), _("Quad in/out"),
                    _("Cubic in"), _("Cubic out"), _("Cubic in/out"),
                    _("Quart in"), _("Quart out"), _("Quart in/out"),
                    _("Quint in"), _("Quint out"), _("Quint in/out"),
                    _("Sine in"), _("Sine out"), _("Sine in/out"),
                    _("Circular in"), _("Circular out"), _("Circular in/out"),
                    _("Exponential in"), _("Exponential out"), _("Exponential in/out"),
                    _("Elastic in"), _("Elastic out"), _("Elastic in/out"),
                    _("Back in"), _("Back out"), _("Back in/out"),
                    _("Bounce in"), _("Bounce out"), _("Bounce in/out")
                  },
                  {
                    "EaseNone",
                    "EaseQuadIn", "EaseQuadOut", "EaseQuadInOut",
                    "EaseCubicIn", "EaseCubicOut", "EaseCubicInOut",
                    "EaseQuartIn", "EaseQuartOut", "EaseQuartInOut",
                    "EaseQuintIn", "EaseQuintOut", "EaseQuintInOut",
                    "EaseSineIn", "EaseSineOut", "EaseSineInOut",
                    "EaseCircularIn", "EaseCircularOut", "EaseCircularInOut",
                    "EaseExponentialIn", "EaseExponentialOut", "EaseExponentialInOut",
                    "EaseElasticIn", "EaseElasticOut", "EaseElasticInOut",
                    "EaseBackIn", "EaseBackOut", "EaseBackInOut",
                    "EaseBounceIn", "EaseBounceOut", "EaseBounceInOut"
                  },
                  0, "death-easing");
  result.add_float(_("Death time"), &m_particle_death_time, "death-time", 5.f);
  result.add_float(_("Death time variation"), &m_particle_death_time_variation, "death-time-variation", 0.f);
  result.add_float(_("Speed X"), &m_particle_speed_x, "speed-x", 0.f);
  result.add_float(_("Speed Y"), &m_particle_speed_y, "speed-y", 0.f);
  result.add_float(_("Speed X (variation)"), &m_particle_speed_variation_x, "speed-var-x", 0.f);
  result.add_float(_("Speed Y (variation)"), &m_particle_speed_variation_y, "speed-var-y", 0.f);
  result.add_float(_("Acceleration X"), &m_particle_acceleration_x, "acceleration-x", 0.f);
  result.add_float(_("Acceleration Y"), &m_particle_acceleration_y, "acceleration-y", 0.f);
  result.add_float(_("Friction X"), &m_particle_friction_x, "friction-x", 0.f);
  result.add_float(_("Friction Y"), &m_particle_friction_y, "friction-y", 0.f);
  result.add_float(_("Feather factor"), &m_particle_feather_factor, "feather-factor", 0.f);
  result.add_float(_("Rotation"), &m_particle_rotation, "rotation", 0.f);
  result.add_float(_("Rotation (variation)"), &m_particle_rotation_variation, "rotation-variation", 0.f);
  result.add_float(_("Rotation speed"), &m_particle_rotation_speed, "rotation-speed", 0.f);
  result.add_float(_("Rotation speed (variation)"), &m_particle_rotation_speed_variation, "rotation-speed-variation", 0.f);
  result.add_float(_("Rotation acceleration"), &m_particle_rotation_acceleration, "rotation-acceleration", 0.f);
  result.add_float(_("Rotation friction"), &m_particle_rotation_decceleration, "rotation-decceleration", 0.f);
  result.add_enum(_("Rotation mode"), reinterpret_cast<int*>(&m_particle_rotation_mode),
                  {_("Fixed"), _("Facing"), _("Wiggling")},
                  {"fixed", "facing", "wiggling"},
                  static_cast<int>(RotationMode::Fixed),
                  "rotation-mode");
  result.add_enum(_("Collision mode"), reinterpret_cast<int*>(&m_particle_collision_mode),
                  {_("None (pass through)"), _("Stick"), _("Stick Forever"), _("Bounce (heavy)"), _("Bounce (light)"), _("Kill particle"), _("Fade out particle")},
                  {"ignore", "stick", "stick-forever", "bounce-heavy", "bounce-light", "destroy", "fade-out"},
                  static_cast<int>(CollisionMode::Ignore),
                  "collision-mode");
  result.add_enum(_("Delete if off-screen"), reinterpret_cast<int*>(&m_particle_offscreen_mode),
                  {_("Never"), _("Only on exit"), _("Always")},
                  {"never", "only-on-exit", "always"},
                  static_cast<int>(OffscreenMode::Never),
                  "offscreen-mode");
  result.add_bool(_("Cover screen"), &m_cover_screen, "cover-screen", true);

  //result.reorder({"amount", "delay", "lifetime", "lifetime-variation", "enabled", "name"});

  result.add_particle_editor();

  return result;
}

void
CustomParticleSystem::update(float dt_sec)
{
  // The "enabled" flag being false only indicates that new particles shouldn't spawn.
  // However, we still need to update the existing particles, if any.

  // Handle script-based easings.
  for (auto& req : script_easings)
  {
    req.time_remain -= dt_sec;
    if (req.time_remain <= 0)
    {
      req.time_remain = 0;
      *(req.value) = req.end;
    }
    else
    {
      float progress = 1.f - (req.time_remain / req.time_total);
      progress = static_cast<float>(req.func(static_cast<double>(progress)));
      *(req.value) = req.begin + progress * (req.end - req.begin);
    }
  }

  // Update existing particles.
  for (auto& it : custom_particles) {
    auto particle = dynamic_cast<CustomParticle*>(it.get());
    assert(particle);

    if (particle->birth_time > dt_sec) {
      switch(particle->birth_mode) {
      case FadeMode::Shrink:
        particle->scale = static_cast<float>(
                          getEasingByName(particle->birth_easing)(
                            static_cast<double>(
                              1.f - (particle->birth_time / particle->total_birth)
                            )
                          ));
        break;
      case FadeMode::Fade:
        particle->props = SpriteProperties(particle->original_props,
                                           1.f - (particle->birth_time /
                                                  particle->total_birth));
        break;
      default:
        break;
      }
      particle->birth_time -= dt_sec;
    } else if (particle->birth_time > 0.f) {
      particle->birth_time = 0.f;
      switch(particle->birth_mode) {
      case FadeMode::Shrink:
        particle->scale = 1.f;
        break;
      case FadeMode::Fade:
        particle->props = particle->original_props;
        break;
      default:
        break;
      }
    }

    particle->lifetime -= dt_sec;
    if (particle->lifetime < 0.f) {
      particle->lifetime = 0.f;
    }

    if (particle->birth_time <= 0.f && particle->lifetime <= 0.f) {
      if (particle->death_time > dt_sec) {
        switch(particle->death_mode) {
        case FadeMode::Shrink:
          particle->scale = 1.f - static_cast<float>(
                            getEasingByName(particle->death_easing)(
                              static_cast<double>(
                                1.f - (particle->death_time / particle->total_death)
                              )
                            ));
          break;
        case FadeMode::Fade:
          particle->props = SpriteProperties(particle->original_props,
                                                   (particle->death_time /
                                                    particle->total_death));
          break;
        default:
          break;
        }
        particle->death_time -= dt_sec;
      } else {
        particle->death_time = 0.f;
        switch(particle->death_mode) {
        case FadeMode::Shrink:
          particle->scale = 0.f;
          break;
        case FadeMode::Fade:
          particle->props = SpriteProperties(particle->original_props, 0.f);
          break;
        default:
          break;
        }
        particle->ready_for_deletion = true;
      }
    }

    float abs_x = get_abs_x();
    float abs_y = get_abs_y();

    if (!particle->has_been_on_screen) {
      if (particle->pos.y <= static_cast<float>(SCREEN_HEIGHT) + abs_y
          && particle->pos.y >= abs_y
          && particle->pos.x <= static_cast<float>(SCREEN_WIDTH) + abs_x
          && particle->pos.x >= abs_x) {
        particle->has_been_on_screen = true;
      }
    }

    switch(particle->offscreen_mode) {
    case OffscreenMode::Always:
      if (particle->pos.y > static_cast<float>(SCREEN_HEIGHT) + abs_y
          || particle->pos.y < abs_y
          || particle->pos.x > static_cast<float>(SCREEN_WIDTH) + abs_x
          || particle->pos.x < abs_x) {
        particle->ready_for_deletion = true;
      }
      break;
    case OffscreenMode::OnlyOnExit:
      if ((particle->pos.y > static_cast<float>(SCREEN_HEIGHT) + abs_y
          || particle->pos.y < abs_y
          || particle->pos.x > static_cast<float>(SCREEN_WIDTH) + abs_x
          || particle->pos.x < abs_x)
          && particle->has_been_on_screen) {
        particle->ready_for_deletion = true;
      }
      break;
    case OffscreenMode::Never:
      break;
    }

    bool is_in_life_zone = false;
    for (auto& zone : get_zones()) {
      if (zone.get_rect().contains(particle->pos) && zone.get_particle_name() == m_name) {
        switch(zone.get_type()) {
        case ParticleZone::ParticleZoneType::Killer:
          particle->lifetime = 0.f;
          particle->birth_time = 0.f;
          break;

        case ParticleZone::ParticleZoneType::Destroyer:
          particle->ready_for_deletion = true;
          break;

        case ParticleZone::ParticleZoneType::LifeClear:
          particle->last_life_zone_required_instakill = true;
          particle->has_been_in_life_zone = true;
          is_in_life_zone = true;
          break;

        case ParticleZone::ParticleZoneType::Life:
          particle->last_life_zone_required_instakill = false;
          particle->has_been_in_life_zone = true;
          is_in_life_zone = true;
          break;

        // This case is intentionally empty; it serves as a placeholder to prevent a warning.
        case ParticleZone::ParticleZoneType::Spawn:
          break;
        }
      }
    } // For each ParticleZone object.

    if (!is_in_life_zone && particle->has_been_in_life_zone) {
      if (particle->last_life_zone_required_instakill) {
        particle->ready_for_deletion = true;
      } else {
        particle->lifetime = 0.f;
        particle->birth_time = 0.f;
      }
    }

    if (!particle->stuck) {
      particle->speedX += graphicsRandom.randf(-particle->feather_factor,
                                      particle->feather_factor) * dt_sec * 1000.f;
      particle->speedY += graphicsRandom.randf(-particle->feather_factor,
                                      particle->feather_factor) * dt_sec * 1000.f;
      particle->speedX += particle->accX * dt_sec;
      particle->speedY += particle->accY * dt_sec;
      particle->speedX *= 1.f - particle->frictionX * dt_sec;
      particle->speedY *= 1.f - particle->frictionY * dt_sec;

      if (Sector::current() && collision(particle,
                    Vector(particle->speedX,particle->speedY) * dt_sec) > 0) {
        switch(particle->collision_mode) {
        case CollisionMode::Ignore:
          particle->pos.x += particle->speedX * dt_sec;
          particle->pos.y += particle->speedY * dt_sec;
          break;
        case CollisionMode::Stick:
          // Just don't move
          break;
        case CollisionMode::StickForever:
          particle->stuck = true;
          break;
        case CollisionMode::BounceHeavy:
        case CollisionMode::BounceLight:
          {
            auto c = get_collision(particle, Vector(particle->speedX, particle->speedY) * dt_sec);

            float speed_angle = atanf(-particle->speedY / particle->speedX);
            if (c.slope_normal.x == 0.f && c.slope_normal.y == 0.f) {
              auto cX = get_collision(particle, Vector(particle->speedX, 0) * dt_sec);
              if (cX.left != cX.right)
                particle->speedX *= -1;
              auto cY = get_collision(particle, Vector(0, particle->speedY) * dt_sec);
              if (cY.top != cY.bottom)
                particle->speedY *= -1;
            } else {
              float face_angle = atanf(c.slope_normal.y / c.slope_normal.x);
              float dest_angle = face_angle * 2.f - speed_angle; // Reflect the angle around face_angle.
              float dX = cosf(dest_angle),
                    dY = sinf(dest_angle);

              float true_speed = static_cast<float>(sqrt(pow(particle->speedY, 2)
                                                      + pow(particle->speedX, 2)));

              particle->speedX = dX * true_speed;
              particle->speedY = dY * true_speed;
            }

            switch(particle->collision_mode) {
              case CollisionMode::BounceHeavy:
                particle->speedX *= .2f;
                particle->speedY *= .2f;
                break;
              case CollisionMode::BounceLight:
                particle->speedX *= .7f;
                particle->speedY *= .7f;
                break;
              default:
                assert(false);
            }

            particle->pos.x += particle->speedX * dt_sec;
            particle->pos.y += particle->speedY * dt_sec;
          }
          break;
        case CollisionMode::Destroy:
          particle->ready_for_deletion = true;
          break;
        case CollisionMode::FadeOut:
          particle->lifetime = 0.f;
          break;
        }
      } else {
        particle->pos.x += particle->speedX * dt_sec;
        particle->pos.y += particle->speedY * dt_sec;
      }

      switch(particle->angle_mode) {
      case RotationMode::Facing:
        particle->angle = atanf(particle->speedY / particle->speedX) * 180.f / math::PI;
        break;
      case RotationMode::Wiggling:
        particle->angle += graphicsRandom.randf(-particle->angle_speed / 2.f,
                                            particle->angle_speed / 2.f) * dt_sec;
        break;
      case RotationMode::Fixed:
      default:
        particle->angle_speed += particle->angle_acc * dt_sec;
        particle->angle_speed *= 1.f - particle->angle_decc * dt_sec;
        particle->angle += particle->angle_speed * dt_sec;
      }
    }

  } // For each particle.


  // Clear dead particles
  // We iterate through the vector backwards because removing an element affects
  // the index of all elements after it, which can lead to buggy behavior.
  for (int i = static_cast<int>(custom_particles.size()) - 1; i >= 0; --i) {
    auto particle = dynamic_cast<CustomParticle*>(custom_particles.at(i).get());

    if (particle->ready_for_deletion) {
      custom_particles.erase(custom_particles.begin()+i);
    }
  }

  // Add necessary particles.
  float remaining = dt_sec + time_last_remaining;

  if (enabled) {
    int real_max = m_max_amount;
    if (!m_cover_screen) {
      int i = 0;
      for (auto& zone : get_zones()) {
        if (zone.get_type() == ParticleZone::ParticleZoneType::Spawn && zone.get_particle_name() == m_name) {
          i++;
        }
      }
      real_max *= i;
    }
    while (remaining > m_delay && int(custom_particles.size()) < real_max)
    {
      spawn_particles(remaining);
      remaining -= m_delay;
    }
  }

  // Maxes to m_delay, so that if there's already the max amount of particles,
  // it won't store all the time waiting for some particles to go and then
  // spawn a bazillion particles instantly. (Bacisally it means : This will
  // help guarantee there will be at least m_delay between each particle
  // spawn as long as m_delay >= dt_sec).
  time_last_remaining = (remaining > m_delay) ? m_delay : remaining;

}

void
CustomParticleSystem::draw(DrawingContext& context)
{
  // The "enabled" flag being false only indicates that new particles shouldn't spawn.
  // However, we still need to update the existing particles, if any.

  context.push_transform();

  std::unordered_map<SpriteProperties*, SurfaceBatch> batches;
  for (const auto& particle : custom_particles) {
    auto it = batches.find(&(particle->props));
    if (it == batches.end()) {
      const auto& batch_it = batches.emplace(&(particle->props),
        SurfaceBatch(particle->props.texture, particle->props.color));
      batch_it.first->second.draw(Rectf(Vector(
                                               particle->pos.x - particle->scale
                                                 * static_cast<float>(
                                                 particle->props.texture->get_width()
                                               ) * particle->props.scale.x / 2,
                                               particle->pos.y - particle->scale
                                                 * static_cast<float>(
                                                 particle->props.texture->get_height()
                                               ) * particle->props.scale.y / 2
                                        ),
                                        Vector(
                                               particle->pos.x + particle->scale
                                                 * static_cast<float>(
                                                 particle->props.texture->get_width()
                                               ) * particle->props.scale.x / 2,
                                               particle->pos.y + particle->scale
                                                 * static_cast<float>(
                                                 particle->props.texture->get_height()
                                               ) * particle->props.scale.y / 2
                                        )
                                 ), particle->angle);
    } else {
      it->second.draw(Rectf(particle->pos,
                                        Vector(
                                               particle->pos.x + particle->scale
                                                 * static_cast<float>(
                                                 particle->texture->get_width()
                                               ) * particle->props.scale.x,
                                               particle->pos.y + particle->scale
                                                 * static_cast<float>(
                                                 particle->texture->get_height()
                                               ) * particle->props.scale.y
                                        )
                                 ), particle->angle);
    }
  }

  for(auto& it : batches) {
    auto& surface = it.first->texture;
    auto& batch = it.second;
    context.color().draw_surface_batch(surface, batch.move_srcrects(),
      batch.move_dstrects(), batch.move_angles(), it.first->color, z_pos);
  }

  context.pop_transform();
}

// Duplicated from ParticleSystem_Interactive because I intend to bring edits
// sometime in the future, for even more flexibility with particles. (Semphris).
int
CustomParticleSystem::collision(Particle* object, const Vector& movement)
{
  using namespace collision;

  CustomParticle* particle = dynamic_cast<CustomParticle*>(object);
  assert(particle);

  // Calculate rectangle where the object will move.
  float x1, x2;
  float y1, y2;

  x1 = object->pos.x - particle->props.hb_scale.x * static_cast<float>(particle->props.texture->get_width()) / 2
          + particle->props.hb_offset.x * static_cast<float>(particle->props.texture->get_width());
  x2 = x1 + particle->props.hb_scale.x * static_cast<float>(particle->props.texture->get_width()) + movement.x;
  if (x2 < x1) {
    float temp_x = x1;
    x1 = x2;
    x2 = temp_x;
  }

  y1 = object->pos.y - particle->props.hb_scale.y * static_cast<float>(particle->props.texture->get_height()) / 2
          + particle->props.hb_offset.y * static_cast<float>(particle->props.texture->get_height());
  y2 = y1 + particle->props.hb_scale.y * static_cast<float>(particle->props.texture->get_height()) + movement.y;
  if (y2 < y1) {
    float temp_y = y1;
    y1 = y2;
    y2 = temp_y;
  }
  bool water = false;

  // Test with all tiles in this rectangle.
  int starttilex = int(x1-1) / 32;
  int starttiley = int(y1-1) / 32;
  int max_x = int(x2+1);
  int max_y = int(y2+1);

  Rectf dest(x1, y1, x2, y2);
  dest.move(movement);
  Constraints constraints;

  for (const auto& solids : Sector::get().get_solid_tilemaps()) {
    // FIXME: Handle a nonzero tilemap offset.
    // Check if it gets fixed in particlesystem_interactive.cpp.
    for (int x = starttilex; x*32 < max_x; ++x) {
      for (int y = starttiley; y*32 < max_y; ++y) {
        const Tile& tile = solids->get_tile(x, y);

        // Skip non-solid tiles, except water.
        if (! (tile.get_attributes() & (Tile::WATER | Tile::SOLID)))
          continue;

        Rectf rect = solids->get_tile_bbox(x, y);
        if (tile.is_slope ()) { // Slope tile.
          AATriangle triangle = AATriangle(rect, tile.get_data());

          if (rectangle_aatriangle(&constraints, dest, triangle)) {
            if (tile.get_attributes() & Tile::WATER)
              water = true;
          }
        } else { // Normal rectangular tile.
          if (dest.overlaps(rect)) {
            if (tile.get_attributes() & Tile::WATER)
              water = true;
            set_rectangle_rectangle_constraints(&constraints, dest, rect);
          }
        }
      }
    }
  }

  // TODO: Avoid using magic numbers in this section.

  // Check if any constraints exist for collision.
  if (!constraints.has_constraints())
    return -1;

  const CollisionHit& hit = constraints.hit;
  if (water) {
    return 0; // Collision with water tile - no need to draw splash.
  } else {
    if (hit.right || hit.left) {
      return 2; // Collision from the right.
    } else {
      return 1; // Collision from above.
    }
  }
}

CollisionHit
CustomParticleSystem::get_collision(Particle* object, const Vector& movement)
{
  using namespace collision;

  CustomParticle* particle = dynamic_cast<CustomParticle*>(object);
  assert(particle);

  // Calculate rectangle where the object will move.
  float x1, x2;
  float y1, y2;

  x1 = object->pos.x - particle->props.scale.x * static_cast<float>(particle->props.texture->get_width()) / 2;
  x2 = x1 + particle->props.scale.x * static_cast<float>(particle->props.texture->get_width()) + movement.x;
  if (x2 < x1) {
    float temp_x = x1;
    x1 = x2;
    x2 = temp_x;
  }

  y1 = object->pos.y - particle->props.scale.y * static_cast<float>(particle->props.texture->get_height()) / 2;
  y2 = y1 + particle->props.scale.y * static_cast<float>(particle->props.texture->get_height()) + movement.y;
  if (y2 < y1) {
    float temp_y = y1;
    y1 = y2;
    y2 = temp_y;
  }

  // Test with all tiles in this rectangle.
  int starttilex = int(x1-1) / 32;
  int starttiley = int(y1-1) / 32;
  int max_x = int(x2+1);
  int max_y = int(y2+1);

  Rectf dest(x1, y1, x2, y2);
  dest.move(movement);
  Constraints constraints;

  for (const auto& solids : Sector::get().get_solid_tilemaps()) {
    // FIXME: Handle a nonzero tilemap offset.
    // Check if it gets fixed in particlesystem_interactive.cpp.
    for (int x = starttilex; x*32 < max_x; ++x) {
      for (int y = starttiley; y*32 < max_y; ++y) {
        const Tile& tile = solids->get_tile(x, y);

        // Skip non-solid tiles.
        if (! (tile.get_attributes() & (/*Tile::WATER |*/ Tile::SOLID)))
          continue;

        Rectf rect = solids->get_tile_bbox(x, y);
        if (tile.is_slope ()) { // Slope tile.
          AATriangle triangle = AATriangle(rect, tile.get_data());
          rectangle_aatriangle(&constraints, dest, triangle);
        } else { // Normal rectangular tile.
          if (dest.overlaps(rect)) {
            set_rectangle_rectangle_constraints(&constraints, dest, rect);
          }
        }
      }
    }
  }

  return constraints.hit;
}


// =============================================================================
// LOCAL

CustomParticleSystem::SpriteProperties
CustomParticleSystem::get_random_texture()
{
  float val = graphicsRandom.randf(texture_sum_odds);
  for (const auto& texture : m_textures)
  {
    val -= texture.likeliness;
    if (val <= 0)
    {
      return texture;
    }
  }
  return m_textures.at(0);
}

std::vector<ParticleZone::ZoneDetails>
CustomParticleSystem::get_zones()
{
  std::vector<ParticleZone::ZoneDetails> list;

  //if (!!GameSession::current() && Sector::current()) {
  if (!ParticleEditor::current()) {

    // In game or in level editor.
    for (auto& zone : GameSession::current()->get_current_sector().get_objects_by_type<ParticleZone>()) {
      list.push_back(zone.get_details());
    }

  } else {

    // In particle editor.
    list.push_back(ParticleZone::ZoneDetails(m_name,
                                             ParticleZone::ParticleZoneType::Spawn,
                                             Rectf(virtual_width / 2 - 16.f,
                                                   virtual_height / 2 - 16.f,
                                                   virtual_width / 2 + 16.f,
                                                   virtual_height / 2 + 16.f)
                                            ));

  }

  return list;
}

float
CustomParticleSystem::get_abs_x()
{
  return (Sector::current()) ? Sector::get().get_camera().get_translation().x : 0.f;
}

float
CustomParticleSystem::get_abs_y()
{
  return (Sector::current()) ? Sector::get().get_camera().get_translation().y : 0.f;
}

/** Initializes and adds a single particle to the stack. Performs
 *    no check regarding the maximum amount of total particles.
 * @param lifetime The time elapsed since the moment the particle should have been born.
 */
void
CustomParticleSystem::add_particle(float lifetime, float x, float y)
{
  auto particle = std::make_unique<CustomParticle>();
  particle->original_props = get_random_texture();
  particle->props = particle->original_props;

  particle->pos.x = x;
  particle->pos.y = y;

  float life_elapsed = lifetime;
  float birth_delta = m_particle_birth_time_variation / 2;
  particle->total_birth = m_particle_birth_time + graphicsRandom.randf(-birth_delta, birth_delta);
  particle->birth_time = particle->total_birth - life_elapsed;
  if (particle->birth_time < 0.f) {
    life_elapsed = -particle->birth_time;
    particle->birth_time = 0.f;
  } else {
    life_elapsed = 0.f;
  }
  float life_delta = m_particle_lifetime_variation / 2;
  particle->lifetime = m_particle_lifetime - life_elapsed + graphicsRandom.randf(-life_delta, life_delta);
  if (particle->lifetime < 0.f) {
    life_elapsed = -particle->lifetime;
    particle->lifetime = 0.f;
  } else {
    life_elapsed = 0.f;
  }
  float death_delta = m_particle_death_time_variation / 2;
  particle->total_death = m_particle_death_time + graphicsRandom.randf(-death_delta, death_delta);
  particle->death_time = particle->total_death - life_elapsed;

  particle->birth_mode = m_particle_birth_mode;
  particle->death_mode = m_particle_death_mode;

  particle->birth_easing = m_particle_birth_easing;
  particle->death_easing = m_particle_death_easing;

  switch(particle->birth_mode) {
  case FadeMode::Shrink:
    particle->scale = 0.f;
    break;
  default:
    break;
  }

  float speedx_delta = m_particle_speed_variation_x / 2;
  particle->speedX = m_particle_speed_x + graphicsRandom.randf(-speedx_delta, speedx_delta);
  float speedy_delta = m_particle_speed_variation_y / 2;
  particle->speedY = m_particle_speed_y + graphicsRandom.randf(-speedy_delta, speedy_delta);
  particle->accX = m_particle_acceleration_x;
  particle->accY = m_particle_acceleration_y;
  particle->frictionX = m_particle_friction_x;
  particle->frictionY = m_particle_friction_y;

  particle->feather_factor = m_particle_feather_factor;

  float angle_delta = m_particle_rotation_variation / 2;
  particle->angle = m_particle_rotation + graphicsRandom.randf(-angle_delta, angle_delta);
  float angle_speed_delta = m_particle_rotation_speed_variation / 2;
  particle->angle_speed = m_particle_rotation_speed + graphicsRandom.randf(-angle_speed_delta, angle_speed_delta);
  particle->angle_acc = m_particle_rotation_acceleration;
  particle->angle_decc = m_particle_rotation_decceleration;
  particle->angle_mode = m_particle_rotation_mode;

  particle->collision_mode = m_particle_collision_mode;

  particle->offscreen_mode = m_particle_offscreen_mode;

  custom_particles.push_back(std::move(particle));
}

void
CustomParticleSystem::spawn_particles(float lifetime)
{
  if (!m_cover_screen) {
    for (auto& zone : get_zones()) {
      if (zone.get_type() == ParticleZone::ParticleZoneType::Spawn && zone.get_particle_name() == m_name) {
        Rectf rect = zone.get_rect();
        add_particle(lifetime,
                     graphicsRandom.randf(rect.get_width()) + rect.get_left(),
                     graphicsRandom.randf(rect.get_height()) + rect.get_top());
      }
    }
  } else {
    float abs_x = get_abs_x();
    float abs_y = get_abs_y();
    add_particle(lifetime,
                 graphicsRandom.randf(virtual_width) + abs_x,
                 graphicsRandom.randf(virtual_height) + abs_y);
  }
}

// SCRIPTING

void
CustomParticleSystem::ease_value(float* value, float target, float time, easing func) {
  assert(value);

  ease_request req;
  req.value = value;
  req.begin = *value;
  req.end = target;
  req.time_total = time;
  req.time_remain = time;
  req.func = func;

  script_easings.push_back(req);
}

/* EOF */
