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
#include "math/random_generator.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "scripting/level.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

#include <sstream>

PowerUp::PowerUp(const ReaderMapping& lisp) :
  MovingSprite(lisp, "images/powerups/egg/egg.sprite", LAYER_OBJECTS, COLGROUP_MOVING),
  physic(),
  script(),
  no_physics(),
  light(0.0f,0.0f,0.0f),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-small.sprite"))
{
  if (!lisp.get("script", script)) script = "";
  if (!lisp.get("disable-physics", no_physics)) no_physics = false;
  physic.enable_gravity(true);
  SoundManager::current()->preload("sounds/grow.ogg");
  SoundManager::current()->preload("sounds/fire-flower.wav");
  SoundManager::current()->preload("sounds/gulp.wav");
  //set default light for glow effect for standard sprites
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.0f, 0.0f, 0.0f));
  if (sprite_name == "images/powerups/egg/egg.sprite") {
    lightsprite->set_color(Color(0.2f, 0.2f, 0.0f));
  } else if (sprite_name == "images/powerups/fireflower/fireflower.sprite") {
    lightsprite->set_color(Color(0.3f, 0.0f, 0.0f));
  } else if (sprite_name == "images/powerups/iceflower/iceflower.sprite") {
    lightsprite->set_color(Color(0.0f, 0.1f, 0.2f));
  } else if (sprite_name == "images/powerups/airflower/airflower.sprite") {
    lightsprite->set_color(Color(0.15f, 0.0f, 0.15f));
  } else if (sprite_name == "images/powerups/earthflower/earthflower.sprite") {
    lightsprite->set_color(Color(0.0f, 0.3f, 0.0f));
  } else if (sprite_name == "images/powerups/star/star.sprite") {
    lightsprite->set_color(Color(0.4f, 0.4f, 0.4f));
  }

}

PowerUp::PowerUp(const Vector& pos, const std::string& sprite_name_) :
  MovingSprite(pos, sprite_name_, LAYER_OBJECTS, COLGROUP_MOVING),
  physic(),
  script(),
  no_physics(false),
  light(0.0f,0.0f,0.0f),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-small.sprite"))
{
  physic.enable_gravity(true);
  SoundManager::current()->preload("sounds/grow.ogg");
  SoundManager::current()->preload("sounds/fire-flower.wav");
  //set default light for glow effect for standard sprites
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.0f, 0.0f, 0.0f));
  if (sprite_name == "images/powerups/egg/egg.sprite") {
    lightsprite->set_color(Color(0.2f, 0.2f, 0.0f));
  } else if (sprite_name == "images/powerups/fireflower/fireflower.sprite") {
    lightsprite->set_color(Color(0.3f, 0.0f, 0.0f));
  } else if (sprite_name == "images/powerups/iceflower/iceflower.sprite") {
    lightsprite->set_color(Color(0.0f, 0.1f, 0.2f));
  } else if (sprite_name == "images/powerups/airflower/airflower.sprite") {
    lightsprite->set_color(Color(0.15f, 0.0f, 0.15f));
  } else if (sprite_name == "images/powerups/earthflower/earthflower.sprite") {
    lightsprite->set_color(Color(0.0f, 0.3f, 0.0f));
  } else if (sprite_name == "images/powerups/star/star.sprite") {
    lightsprite->set_color(Color(0.4f, 0.4f, 0.4f));
  }
}

void
PowerUp::collision_solid(const CollisionHit& hit)
{
  if(hit.bottom) {
    physic.set_velocity_y(0);
  }
  if(hit.right || hit.left) {
    physic.set_velocity_x(-physic.get_velocity_x());
  }
}

HitResponse
PowerUp::collision(GameObject& other, const CollisionHit&)
{
  Player* player = dynamic_cast<Player*>(&other);
  if(player == 0)
    return FORCE_MOVE;

  if (sprite_name == "images/powerups/potions/blue-potion.sprite" ||
      sprite_name == "images/powerups/potions/red-potion.sprite") {
      SoundManager::current()->play("sounds/gulp.wav");
  }

  if (!script.empty()) {
    Sector::current()->run_script(script, "powerup-script");
    remove_me();
    return ABORT_MOVE;
  }

  // some defaults if no script has been set
  if (sprite_name == "images/powerups/egg/egg.sprite") {
    if(!player->add_bonus(GROWUP_BONUS, true))
      return FORCE_MOVE;
    SoundManager::current()->play("sounds/grow.ogg");
  } else if (sprite_name == "images/powerups/fireflower/fireflower.sprite") {
    if(!player->add_bonus(FIRE_BONUS, true))
      return FORCE_MOVE;
    SoundManager::current()->play("sounds/fire-flower.wav");
  } else if (sprite_name == "images/powerups/iceflower/iceflower.sprite") {
    if(!player->add_bonus(ICE_BONUS, true))
      return FORCE_MOVE;
    SoundManager::current()->play("sounds/fire-flower.wav");
  } else if (sprite_name == "images/powerups/airflower/airflower.sprite") {
    if(!player->add_bonus(AIR_BONUS, true))
      return FORCE_MOVE;
    SoundManager::current()->play("sounds/fire-flower.wav");
  } else if (sprite_name == "images/powerups/earthflower/earthflower.sprite") {
    if(!player->add_bonus(EARTH_BONUS, true))
      return FORCE_MOVE;
    SoundManager::current()->play("sounds/fire-flower.wav");
  } else if (sprite_name == "images/powerups/star/star.sprite") {
    player->make_invincible();
  } else if (sprite_name == "images/powerups/1up/1up.sprite") {
    player->get_status()->add_coins(100);
  } else if (sprite_name == "images/powerups/potions/red-potion.sprite") {
    scripting::Level_flip_vertically();
  }

  remove_me();
  return ABORT_MOVE;
}

void
PowerUp::update(float elapsed_time)
{
  if (!no_physics)
    movement = physic.get_movement(elapsed_time);
  //Stars sparkle when close to Tux
  if (sprite_name == "images/powerups/star/star.sprite"){
    Player* player = Sector::current()->get_nearest_player(bbox);
    if (player) {
      float disp_x = player->get_bbox().p1.x - bbox.p1.x;
      float disp_y = player->get_bbox().p1.y - bbox.p1.y;
      if (disp_x*disp_x + disp_y*disp_y <= 256*256)
      {
        if (graphicsRandom.rand(0, 2) == 0) {
          float px = graphicsRandom.randf(bbox.p1.x+0, bbox.p2.x-0);
          float py = graphicsRandom.randf(bbox.p1.y+0, bbox.p2.y-0);
          Vector ppos = Vector(px, py);
          Vector pspeed = Vector(0, 0);
          Vector paccel = Vector(0, 0);
          Sector::current()->add_object(std::make_shared<SpriteParticle>(
                                          "images/objects/particles/sparkle.sprite",
                                          // draw bright sparkles when very close to Tux, dark sparkles when slightly further
                                          (disp_x*disp_x + disp_y*disp_y <= 128*128) ?
                                          // make every other a longer sparkle to make trail a bit fuzzy
                                          (size_t(game_time*20)%2) ? "small" : "medium" : "dark",
                                          ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS+1+5));
        }
      }
    }
  }
}

void
PowerUp::draw(DrawingContext& context){
  //Draw the Sprite.
  sprite->draw(context, get_pos(), layer);
  //Draw light when dark for defaults
  context.get_light( bbox.get_middle(), &light );
  if (light.red + light.green + light.blue < 3.0){
    //Stars are brighter
    if (sprite_name == "images/powerups/star/star.sprite") {
      sprite->draw(context, get_pos(), layer);
    }
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    lightsprite->draw(context, bbox.get_middle(), 0);
    context.pop_target();
  }
}

ObjectSettings
PowerUp::get_settings() {
  ObjectSettings result = MovingSprite::get_settings();
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Script"), &script,
                                         "script"));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Disable gravity"), &no_physics,
                                         "disable-physics"));

  return result;
}

/* EOF */
