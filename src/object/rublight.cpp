//  SuperTux
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

#include "object/rublight.hpp"

#include "object/explosion.hpp"
#include "badguy/walking_badguy.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/constants.hpp"
#include "video/color.hpp"


RubLight::RubLight(const ReaderMapping& mapping) :
  MovingSprite(mapping, "images/objects/rublight/rublight.sprite", LAYER_TILES,
    COLGROUP_STATIC),
  state(STATE_DARK),
  stored_energy(0),
  light(SpriteManager::current()->create(
    "images/objects/lightmap_light/lightmap_light.sprite"))
{
  m_sprite->set_action("normal");
}

HitResponse
RubLight::collision(GameObject& other, const CollisionHit&)
{
  Player* player = dynamic_cast<Player*>(&other);
  if (player != nullptr &&
      player->get_bbox().get_bottom() < m_col.m_bbox.get_top() + SHIFT_DELTA) {
    Vector vel_player = player->get_velocity();
    float vel_horiz = fabsf(vel_player.x) / 32.0f;
    if (player->is_skidding())
      rub(vel_horiz * 0.3f);
    else
      rub(vel_horiz * 0.01f);
    return FORCE_MOVE;
  }

  if (dynamic_cast<Explosion*> (&other)) {
    rub(1.0f);
    return FORCE_MOVE;
  }

  WalkingBadguy* obj = dynamic_cast<WalkingBadguy*>(&other);
  if (obj != nullptr) {
    float vel_horiz = fabsf(obj->get_velocity_x()) / 32.0f;
    rub(vel_horiz * 0.01f);
  }

  return FORCE_MOVE;
}

void RubLight::rub(float strength)
{
  if (strength <= 0)
    return;
  stored_energy = std::max<float>(stored_energy, strength);
  if (state == STATE_DARK)
    state = STATE_FADING;
}

void
RubLight::update(float dt_sec)
{
  switch (state) {
  case STATE_DARK:
    // Nothing to do
    break;

  case STATE_FADING:
    // exponential fading
    const float fading_speed = 5.0f;
    stored_energy *= expf(-dt_sec * fading_speed);
    if (get_brightness() < 0.000001f) {
      stored_energy = 0;
      state = STATE_DARK;
    }
    break;
  }
}

float
RubLight::get_brightness()
{
  return stored_energy / (1.0f + stored_energy);
}

void
RubLight::draw(DrawingContext& context)
{
  if (state == STATE_FADING) {
    float brightness = get_brightness();
    Color col = Color::from_linear(brightness, brightness * 0.5f,
      brightness * 0.3f);
    light->set_color(col);
    light->set_blend(Blend::ADD);
    light->draw(context.light(), get_pos(), m_layer);
  }

  m_sprite->draw(context.color(), get_pos(), m_layer);
}

/* EOF */
