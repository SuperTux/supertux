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

#include "object/powerup.hpp"

#include "audio/sound_manager.hpp"
#include "math/random.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "scripting/level.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

PowerUp::PowerUp(const ReaderMapping& mapping) :
  MovingSprite(mapping, "images/powerups/egg/egg.sprite", LAYER_OBJECTS, COLGROUP_MOVING),
  physic(),
  script(),
  no_physics(),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-small.sprite")),
  m_flip(NO_FLIP)
{
  mapping.get("script", script, "");
  mapping.get("disable-physics", no_physics, false);
  initialize();
}

PowerUp::PowerUp(const Vector& pos, const std::string& sprite_name_) :
  MovingSprite(pos, sprite_name_, LAYER_OBJECTS, COLGROUP_MOVING),
  physic(),
  script(),
  no_physics(false),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-small.sprite")),
  m_flip(NO_FLIP)
{
  initialize();
}

void
PowerUp::initialize()
{
  physic.enable_gravity(true);
  SoundManager::current()->preload("sounds/grow.ogg");
  SoundManager::current()->preload("sounds/fire-flower.wav");
   SoundManager::current()->preload("sounds/gulp.wav");
  //set default light for glow effect for standard sprites
  lightsprite->set_blend(Blend::ADD);
  lightsprite->set_color(Color(0.0f, 0.0f, 0.0f));
  if (m_sprite_name == "images/powerups/egg/egg.sprite" || m_sprite_name == "/images/powerups/egg/egg.sprite") {
    lightsprite->set_color(Color(0.2f, 0.2f, 0.0f));
  } else if (m_sprite_name == "images/powerups/fireflower/fireflower.sprite" || m_sprite_name == "/images/powerups/fireflower/fireflower.sprite") {
    lightsprite->set_color(Color(0.3f, 0.0f, 0.0f));
  } else if (m_sprite_name == "images/powerups/iceflower/iceflower.sprite" || m_sprite_name == "/images/powerups/iceflower/iceflower.sprite") {
    lightsprite->set_color(Color(0.0f, 0.1f, 0.2f));
  } else if (m_sprite_name == "images/powerups/airflower/airflower.sprite" || m_sprite_name == "/images/powerups/airflower/airflower.sprite") {
    lightsprite->set_color(Color(0.15f, 0.0f, 0.15f));
  } else if (m_sprite_name == "images/powerups/earthflower/earthflower.sprite" || m_sprite_name == "/images/powerups/earthflower/earthflower.sprite") {
    lightsprite->set_color(Color(0.0f, 0.3f, 0.0f));
  } else if (m_sprite_name == "images/powerups/star/star.sprite" || m_sprite_name == "/images/powerups/star/star.sprite") {
    lightsprite->set_color(Color(0.4f, 0.4f, 0.4f));
  }
}

void
PowerUp::collision_solid(const CollisionHit& hit)
{
  if (hit.bottom) {
    physic.set_velocity_y(0);
  }
  if (hit.right || hit.left) {
    physic.set_velocity_x(-physic.get_velocity_x());
  }
}

HitResponse
PowerUp::collision(GameObject& other, const CollisionHit&)
{
  Player* player = dynamic_cast<Player*>(&other);
  if (player == nullptr)
    return FORCE_MOVE;

  if (m_sprite_name == "images/powerups/potions/blue-potion.sprite" ||
      m_sprite_name == "images/powerups/potions/red-potion.sprite" ||
	  m_sprite_name == "/images/powerups/potions/blue-potion.sprite" || 
	  m_sprite_name == "/images/powerups/potions/red-potion.sprite") {
      SoundManager::current()->play("sounds/gulp.wav");
  }

  if (!script.empty()) {
    Sector::get().run_script(script, "powerup-script");
    remove_me();
    return ABORT_MOVE;
  }

  // some defaults if no script has been set
  if (m_sprite_name == "images/powerups/egg/egg.sprite" || m_sprite_name == "/images/powerups/egg/egg.sprite") {
    if (!player->add_bonus(GROWUP_BONUS, true))
      return FORCE_MOVE;
    SoundManager::current()->play("sounds/grow.ogg");
  } else if (m_sprite_name == "images/powerups/fireflower/fireflower.sprite" || m_sprite_name == "/images/powerups/fireflower/fireflower.sprite") {
    if (!player->add_bonus(FIRE_BONUS, true))
      return FORCE_MOVE;
    SoundManager::current()->play("sounds/fire-flower.wav");
  } else if (m_sprite_name == "images/powerups/iceflower/iceflower.sprite" || m_sprite_name == "/images/powerups/iceflower/iceflower.sprite") {
    if (!player->add_bonus(ICE_BONUS, true))
      return FORCE_MOVE;
    SoundManager::current()->play("sounds/fire-flower.wav");
  } else if (m_sprite_name == "images/powerups/airflower/airflower.sprite" || m_sprite_name == "/images/powerups/airflower/airflower.sprite") {
    if (!player->add_bonus(AIR_BONUS, true))
      return FORCE_MOVE;
    SoundManager::current()->play("sounds/fire-flower.wav");
  } else if (m_sprite_name == "images/powerups/earthflower/earthflower.sprite" || m_sprite_name == "/images/powerups/earthflower/earthflower.sprite") {
    if (!player->add_bonus(EARTH_BONUS, true))
      return FORCE_MOVE;
    SoundManager::current()->play("sounds/fire-flower.wav");
  } else if (m_sprite_name == "images/powerups/star/star.sprite" || m_sprite_name == "/images/powerups/star/star.sprite") {
    player->make_invincible();
  } else if (m_sprite_name == "images/powerups/1up/1up.sprite" || m_sprite_name == "/images/powerups/1up/1up.sprite") {
    player->get_status().add_coins(100);
  } else if (m_sprite_name == "images/powerups/potions/red-potion.sprite" || m_sprite_name == "/images/powerups/potions/red-potion.sprite") {
    scripting::Level_flip_vertically();
  }

  remove_me();
  return ABORT_MOVE;
}

void
PowerUp::update(float dt_sec)
{
  if (!no_physics)
    m_col.set_movement(physic.get_movement(dt_sec));
  //Stars sparkle when close to Tux
  if (m_sprite_name == "images/powerups/star/star.sprite" || m_sprite_name == "/images/powerups/star/star.sprite"){
    if (auto* player = Sector::get().get_nearest_player(m_col.m_bbox)) {
      float disp_x = player->get_bbox().get_left() - m_col.m_bbox.get_left();
      float disp_y = player->get_bbox().get_top() - m_col.m_bbox.get_top();
      if (disp_x*disp_x + disp_y*disp_y <= 256*256)
      {
        if (graphicsRandom.rand(0, 2) == 0) {
          float px = graphicsRandom.randf(m_col.m_bbox.get_left() * 1.0f, m_col.m_bbox.get_right() * 1.0f);
          float py = graphicsRandom.randf(m_col.m_bbox.get_top() * 1.0f, m_col.m_bbox.get_bottom() * 1.0f);
          Vector ppos = Vector(px, py);
          Vector pspeed = Vector(0, 0);
          Vector paccel = Vector(0, 0);
          Sector::get().add<SpriteParticle>(
            "images/particles/sparkle.sprite",
            // draw bright sparkles when very close to Tux, dark sparkles when slightly further
            (disp_x*disp_x + disp_y*disp_y <= 128*128) ?
            // make every other a longer sparkle to make trail a bit fuzzy
            (size_t(g_game_time*20)%2) ? "small" : "medium" : "dark",
            ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS+1+5);
        }
      }
    }
  }
}

void
PowerUp::draw(DrawingContext& context)
{
  if (no_physics)
    context.set_flip(context.get_flip() ^ m_flip);

  m_sprite->draw(context.color(), get_pos(), m_layer);

  // Stars are brighter
  if (m_sprite_name == "images/powerups/star/star.sprite" || m_sprite_name == "/images/powerups/star/star.sprite")
  {
    m_sprite->draw(context.color(), get_pos(), m_layer);
  }

  lightsprite->draw(context.light(), m_col.m_bbox.get_middle(), 0);

  if (no_physics)
    context.set_flip(context.get_flip() ^ m_flip);
}

ObjectSettings
PowerUp::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_script(_("Script"), &script, "script");
  result.add_bool(_("Disable gravity"), &no_physics, "disable-physics", false);

  result.reorder({"script", "disable-physics", "sprite", "x", "y"});

  return result;
}

void
PowerUp::on_flip(float height)
{
  MovingSprite::on_flip(height);
  if (no_physics)
    FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
