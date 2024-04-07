//  SuperTux - BicyclePlatform
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#include "object/bicycle_platform.hpp"

#include <algorithm>
#include <math.h>

#include "editor/editor.hpp"
#include "math/util.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/debug.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface.hpp"

static const std::string PLATFORM_SPRITE = "images/objects/platforms/small.sprite";

BicyclePlatformChild::BicyclePlatformChild(const Vector& pos, const std::string& sprite, float angle_offset, BicyclePlatform& parent) :
  MovingSprite(pos, sprite, LAYER_OBJECTS, COLGROUP_STATIC),
  m_parent(parent),
  m_angle_offset(angle_offset),
  m_momentum(),
  m_contacts()
{
  m_col.set_unisolid(true);
}

void
BicyclePlatformChild::update(float dt_sec)
{
  float angle = m_parent.m_angle + m_angle_offset;
  angle = math::positive_fmodf(angle, math::TAU);

  Vector dest = m_parent.get_pos() + Vector(cosf(angle), sinf(angle)) * m_parent.m_radius - (m_col.m_bbox.get_size().as_vector() * 0.5f);
  Vector movement = dest - get_pos();
  m_col.set_movement(movement);
  m_col.propagate_movement(movement);
}

HitResponse
BicyclePlatformChild::collision(GameObject& other, const CollisionHit& )
{
  const float gravity = Sector::get().get_gravity();

  // Somehow the hit parameter does not get filled in, so to determine (hit.top == true) we do this:
  auto mo = dynamic_cast<MovingObject*>(&other); if (!mo) return FORCE_MOVE;
  if ((mo->get_bbox().get_bottom()) > (m_col.m_bbox.get_top() + 2)) return FORCE_MOVE;

  auto pl = dynamic_cast<Player*>(mo);
  if (pl) {
    if (pl->is_big()) m_momentum += m_parent.m_momentum_change_rate * gravity;
    auto po = pl->get_grabbed_object();
    auto pomo = dynamic_cast<MovingObject*>(po);
    if (m_contacts.insert(pomo).second) {
      m_momentum += m_parent.m_momentum_change_rate * gravity;
    }
  }

  if (m_contacts.insert(&other).second) {
    m_momentum += m_parent.m_momentum_change_rate * Sector::get().get_gravity();
  }

  return FORCE_MOVE;
}

void
BicyclePlatformChild::editor_delete()
{
  // Removing a child removes the whole platform.
  m_parent.editor_delete();
}

void
BicyclePlatformChild::on_flip(float height)
{
  MovingSprite::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

BicyclePlatform::BicyclePlatform(const ReaderMapping& reader) :
  Platform(reader, PLATFORM_SPRITE),
  m_radius(128),
  m_angle(0),
  m_angular_speed(0.0f),
  m_momentum_change_rate(0.1f),
  m_children(),
  m_platforms(2)
{
  set_group(COLGROUP_DISABLED);
  m_col.set_size(32.f, 32.f);

  reader.get("radius", m_radius, 128.0f);
  reader.get("momentum-change-rate", m_momentum_change_rate, 0.1f);

  reader.get("platforms", m_platforms);
  m_platforms = std::max(1, m_platforms);
}

void
BicyclePlatform::finish_construction()
{
  Platform::finish_construction();

  get_path()->m_mode = WalkMode::ONE_SHOT;
  get_path()->m_adapt_speed = true;
  get_walker()->m_running = true;

  if (Editor::is_active()) return;

  for (int i = 0; i < m_platforms; i++)
  {
    const float offset = static_cast<float>(i) * (math::TAU / static_cast<float>(m_platforms));
    m_children.push_back(&d_sector->add<BicyclePlatformChild>(get_pos(), m_sprite_name, offset, *this));
  }
}

void
BicyclePlatform::draw(DrawingContext& context)
{
  if (g_debug.show_collision_rects || Editor::is_active())
    context.color().draw_filled_rect(Rectf::from_center(get_pos(), Sizef(16, 16)), Color::RED, 8.f, get_layer());

  if (Editor::is_active())
  {
    for (int i = 0; i < m_platforms; i++)
    {
      const float offset = static_cast<float>(i) * (math::TAU / static_cast<float>(m_platforms));
      float angle = m_angle + offset;
      angle = math::positive_fmodf(angle, math::TAU);

      Sizef size = m_sprite->get_current_hitbox().get_size();
      Vector dest = get_pos() + Vector(cosf(angle), sinf(angle)) * m_radius - (size.as_vector() * 0.5f);
      m_sprite->draw(context.color(), dest, get_layer());
    }
  }
}

void
BicyclePlatform::update(float dt_sec)
{
  if (!get_path()) return;
  if (!get_path()->is_valid()) return;

  float total_angular_momentum = 0.0f;
  for (const auto& child : m_children)
  {
    const float child_angle = m_angle + child->m_angle_offset;
    const float angular_momentum = cosf(child_angle) * child->m_momentum;
    total_angular_momentum += angular_momentum;
    child->m_momentum = 0.0f;
    child->m_contacts.clear();
  }

  m_angular_speed += (total_angular_momentum * dt_sec) * math::PI;
  m_angular_speed *= 1.0f - dt_sec * 0.2f;
  m_angle += m_angular_speed * dt_sec;
  m_angle = math::positive_fmodf(m_angle, math::TAU);

  m_angular_speed = std::min(std::max(m_angular_speed, -128.f * math::PI * dt_sec),
                             128.f * math::PI * dt_sec);

  get_walker()->m_walking_speed = m_angular_speed;

  std::cout << m_angular_speed << std::endl;
  get_walker()->m_stop_at_node_nr = -1;
  get_walker()->update(dt_sec * m_angular_speed * 0.1f);
  Vector movement = get_walker()->get_pos(get_bbox().get_size(), m_path_handle) - get_pos();
  m_col.set_movement(movement);

}

void
BicyclePlatform::on_flip(float height)
{
  set_pos(Vector(get_pos().x, height - get_pos().y));
}

ObjectSettings
BicyclePlatform::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  if (get_path_gameobject())
    result.add_path_ref(_("Path"), *this, get_path_ref(), "path-ref");

  result.add_bool(_("Running"), &get_walker()->m_running, "running", true, 0);
  result.add_int(_("Starting Node"), &m_starting_node, "starting-node", 0, 0U);
  result.add_path_handle(_("Handle"), m_path_handle, "handle");

  result.add_int(_("Platforms"), &m_platforms, "platforms", 2);
  result.add_float(_("Radius"), &m_radius, "radius", 128.0f);
  result.add_float(_("Momentum change rate"), &m_momentum_change_rate, "momentum-change-rate", 0.1f);

  result.reorder({"running", "name", "path-ref", "starting-node", "sprite", "x", "y",
                  "platforms", "radius", "momentum-change-rate"});

  return result;
}

/* EOF */
