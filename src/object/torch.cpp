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
#include "util/gettext.hpp"
#include "util/reader.hpp"

Torch::Torch(const Reader& reader) :
  m_torch(),
  m_flame(),
  m_flame_glow(),
  m_flame_light(),
  m_burning(true)
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);

  std::string sprite_name = "images/objects/torch/torch1.sprite";
  reader.get("sprite", sprite_name);

  bbox.p2.x = bbox.p1.x + 50;
  bbox.p2.y = bbox.p1.y + 50;

  m_torch = SpriteManager::current()->create(sprite_name);
  m_flame = SpriteManager::current()->create("images/objects/torch/flame.sprite");
  m_flame_glow = SpriteManager::current()->create("images/objects/torch/flame_glow.sprite");
  m_flame_glow->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  m_flame_light = SpriteManager::current()->create("images/objects/torch/flame_light.sprite");
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
  // FIXME: this doesn't work, as bbox is wrong
  Player* player = dynamic_cast<Player*>(&other);
  if(player == 0)
  {
    return ABORT_MOVE;
  }
  else
  {
    m_burning = true;
    return ABORT_MOVE;
  }
}

ObjectSettings
Torch::get_settings() {
  ObjectSettings result(_("Torch"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));

  return result;
}

/* EOF */
