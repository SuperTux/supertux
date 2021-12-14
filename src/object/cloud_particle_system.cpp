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

#include "object/cloud_particle_system.hpp"

#include "math/random.hpp"
#include "object/camera.hpp"
#include "supertux/sector.hpp"
#include "supertux/globals.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/surface_batch.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

CloudParticleSystem::CloudParticleSystem() :
  ParticleSystem(128),
  ExposedObject<CloudParticleSystem, scripting::Clouds>(this),
  cloudimage(Surface::from_file("images/particles/cloud.png")),

  m_current_speed(1.f),
  m_target_speed(1.f),
  m_speed_fade_time_remaining(0.f),

  m_current_amount(15.f),
  //m_target_amount(15.f),
  //m_amount_fade_time_remaining(0.f),
  m_current_real_amount(0)
{
  init();
}

CloudParticleSystem::CloudParticleSystem(const ReaderMapping& reader) :
  ParticleSystem(reader, 128),
  ExposedObject<CloudParticleSystem, scripting::Clouds>(this),
  cloudimage(Surface::from_file("images/particles/cloud.png")),

  m_current_speed(1.f),
  m_target_speed(1.f),
  m_speed_fade_time_remaining(0.f),

  m_current_amount(15.f),
  //m_target_amount(15.f),
  //m_amount_fade_time_remaining(0.f),
  m_current_real_amount(0)
{
  init();
}

CloudParticleSystem::~CloudParticleSystem()
{
}

void CloudParticleSystem::init()
{
  virtual_width = 2000.0;

  // create some random clouds
  add_clouds(15, 0.f);
}

ObjectSettings CloudParticleSystem::get_settings()
{
  ObjectSettings result = ParticleSystem::get_settings();

  result.add_float(_("Intensity"), &m_current_amount, "intensity", 15.f);

  result.reorder({"intensity", "enabled", "name"});

  return result;
}


void CloudParticleSystem::update(float dt_sec)
{
  if (!enabled)
    return;

  // Update speed
  if (m_speed_fade_time_remaining > 0.f) {
    if (dt_sec >= m_speed_fade_time_remaining) {
      m_current_speed = m_target_speed;
      m_speed_fade_time_remaining = 0.f;
    } else {
      float amount = dt_sec / m_speed_fade_time_remaining;
      m_current_speed += (m_target_speed - m_current_speed) * amount;
      m_speed_fade_time_remaining -= dt_sec;
    }
  }

  auto& cam = Sector::get().get_singleton_by_type<Camera>();

  for (auto& particle : particles) {
    auto cloudParticle = dynamic_cast<CloudParticle*>(particle.get());
    if (!cloudParticle)
      continue;
    cloudParticle->pos.x += cloudParticle->speed * dt_sec * m_current_speed;
    while (cloudParticle->pos.x < cam.get_translation().x - static_cast<float>(cloudParticle->texture->get_width()))
      cloudParticle->pos.x += virtual_width;
    while (cloudParticle->pos.x > cam.get_translation().x + static_cast<float>(SCREEN_WIDTH))
      cloudParticle->pos.x -= virtual_width;
    while (cloudParticle->pos.y < cam.get_translation().y - static_cast<float>(cloudParticle->texture->get_height()))
      cloudParticle->pos.y += virtual_height;
    while (cloudParticle->pos.y > cam.get_translation().y + static_cast<float>(SCREEN_HEIGHT))
      cloudParticle->pos.y -= virtual_height;

    // Update alpha
    if (cloudParticle->target_time_remaining > 0.f) {
      if (dt_sec >= cloudParticle->target_time_remaining) {
        cloudParticle->alpha = cloudParticle->target_alpha;
        cloudParticle->target_time_remaining = 0.f;
        if (cloudParticle->alpha == 0.f) {
          // Remove this particle
          // But not right here, else it'd mess with the iterator
        }
      } else {
        float amount = dt_sec / cloudParticle->target_time_remaining;
        cloudParticle->alpha += (cloudParticle->target_alpha - cloudParticle->alpha) * amount;
        cloudParticle->target_time_remaining -= dt_sec;
      }
    }
  }

  // Clear dead clouds
  // Scroll through the vector backwards, because removing an element affects
  //   the index of all elements after it (prevents buggy behavior)
  for (int i = static_cast<int>(particles.size()) - 1; i >= 0; --i) {
    auto particle = dynamic_cast<CloudParticle*>(particles.at(i).get());
    
    if (particle->target_alpha == 0.f && particle->target_time_remaining == 0.f)
      particles.erase(particles.begin()+i);
  }
}

int CloudParticleSystem::add_clouds(int amount, float fade_time)
{
  int target_amount = m_current_real_amount + amount;

  if (target_amount > max_amount)
    target_amount = max_amount;

  int amount_to_add = target_amount - m_current_real_amount;

  for (int i = 0; i < amount_to_add; ++i) {
    auto particle = std::make_unique<CloudParticle>();
    // Don't consider the camera, because the Sector might not exist yet
    // Instead, rely on update() to correct this when it will be called
    particle->pos.x = graphicsRandom.randf(virtual_width);
    particle->pos.y = graphicsRandom.randf(virtual_height);
    particle->texture = cloudimage;
    particle->speed = -graphicsRandom.randf(25.0, 54.0);
    particle->alpha = (fade_time == 0.f) ? 1.f : 0.f;
    particle->target_alpha = 1.f;
    particle->target_time_remaining = fade_time;

    particles.push_back(std::move(particle));
  }

  m_current_real_amount = target_amount;
  return m_current_real_amount;
}

int CloudParticleSystem::remove_clouds(int amount, float fade_time)
{
  int target_amount = m_current_real_amount - amount;

  if (target_amount < min_amount)
    target_amount = min_amount;

  int amount_to_remove = m_current_real_amount - target_amount;

  int i = 0;
  for (; i < amount_to_remove && i < static_cast<int>(particles.size()); ++i) {
  
    auto particle = dynamic_cast<CloudParticle*>(particles.at(i).get());
    if (particle->target_alpha != 1.f || particle->target_time_remaining != 0.f) {
      // Skip that one, it doesn't count
      --i;
    } else {
      particle->target_alpha = 0.f;
      particle->target_time_remaining = fade_time;
    }
  }

  return i;
}

void CloudParticleSystem::fade_speed(float new_speed, float fade_time)
{
  // No check to enabled; change the fading even if it's disabled

  // If time is 0 (or smaller?), then update() will never change m_current_speed
  if (fade_time <= 0.f)
  {
    m_current_speed = new_speed;
  }
  
  m_target_speed = new_speed;
  m_speed_fade_time_remaining = fade_time;
}

void CloudParticleSystem::fade_amount(int new_amount, float fade_time, float time_between)
{
  // No check to enabled; change the fading even if it's disabled

  int delta = new_amount - m_current_real_amount;

  if (delta < 0)
  {
    remove_clouds(-delta, fade_time);
  }
  else if (delta > 0)
  {
    add_clouds(delta, fade_time);
  } // Else delta == 0, in which case there is nothing to do
}


void CloudParticleSystem::draw(DrawingContext& context)
{
  if (!enabled)
    return;

  const auto& region = Sector::current()->get_active_region();

  context.push_transform();

  std::unordered_map<SurfacePtr, SurfaceBatch> batches;
  for (const auto& particle : particles) {

    if(!region.contains(particle->pos))
      continue;

    if (particle->alpha != 1.f) {
      const auto& batch_it = batches.emplace(
          particle->texture->clone(),
          SurfaceBatch(
              particle->texture,
              Color(1.f, 1.f, 1.f, particle->alpha)
          ));
      batch_it.first->second.draw(particle->pos, particle->angle);
    } else {
      auto it = batches.find(particle->texture);
      if (it == batches.end()) {
        const auto& batch_it = batches.emplace(particle->texture,
          SurfaceBatch(particle->texture));
        batch_it.first->second.draw(particle->pos, particle->angle);
      } else {
        it->second.draw(particle->pos, particle->angle);
      }
    }
  }

  for(auto& it : batches) {
    auto& surface = it.first;
    auto& batch = it.second;
    // FIXME: What is the colour used for?
    // RESOLVED : That's the tint and the alpha
    context.color().draw_surface_batch(surface, batch.move_srcrects(),
      batch.move_dstrects(), batch.move_angles(), batch.get_color(), z_pos);
  }

  context.pop_transform();
}

/* EOF */
