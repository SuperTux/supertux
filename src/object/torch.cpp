//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "object/torch.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/reader_mapping.hpp"

Torch::Torch(const ReaderMapping& reader) :
  m_torch(),
  m_flame(SpriteManager::current()->create("images/objects/torch/flame.sprite")),
  m_flame_glow(SpriteManager::current()->create("images/objects/torch/flame_glow.sprite")),
  m_flame_light(SpriteManager::current()->create("images/objects/torch/flame_light.sprite")),
  m_burning(true),
  sprite_name("images/objects/torch/torch1.sprite")
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);

  reader.get("sprite", sprite_name);
  reader.get("burning", m_burning, true);

  m_torch = SpriteManager::current()->create(sprite_name);
  bbox.set_size(m_torch->get_width(), m_torch->get_height());
  m_flame_glow->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  m_flame_light->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  set_group(COLGROUP_TOUCHABLE);
}

void
Torch::draw(DrawingContext& context)
{
  if (m_burning)
  {
    m_flame->draw(context, get_pos(), LAYER_TILES - 1);

    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    m_flame_light->draw(context, get_pos(), 0);
    context.pop_target();
  }

  m_torch->draw(context, get_pos(), LAYER_TILES - 1);

  if (m_burning)
  {
    m_flame_glow->draw(context, get_pos(), LAYER_TILES - 1);
  }
}

void
Torch::update(float)
{
}

HitResponse
Torch::collision(GameObject& other, const CollisionHit& )
{
  auto player = dynamic_cast<Player*>(&other);
  if(player != NULL && !m_burning)
  {
    m_burning = true;
  }
  return ABORT_MOVE;
}

ObjectSettings Torch::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();
  ObjectOption burning(MN_TOGGLE, _("Burning"), &m_burning, "burning");
  ObjectOption spr(MN_FILE, _("Sprite"), &sprite_name, "sprite");
  spr.select.push_back(".sprite");

  result.options.push_back(burning);
  result.options.push_back(spr);
  return result;
}

void Torch::after_editor_set()
{
  m_torch = SpriteManager::current()->create(sprite_name);
}

/* EOF */
