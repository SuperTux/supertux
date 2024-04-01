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

#include "badguy/badguy.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/dispenser.hpp"
#include "editor/editor.hpp"
#include "math/random.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "object/sprite_particle.hpp"
#include "object/water_drop.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

static const float SQUISH_TIME = 2;
static const float GEAR_TIME = 2;
static const float BURN_TIME = 1;

static const float X_OFFSCREEN_DISTANCE = 1280;
static const float Y_OFFSCREEN_DISTANCE = 800;

BadGuy::BadGuy(const Vector& pos, const std::string& sprite_name, int layer,
               const std::string& light_sprite_name, const std::string& ice_sprite_name) :
  BadGuy(pos, Direction::LEFT, sprite_name, layer, light_sprite_name)
{
}

BadGuy::BadGuy(const Vector& pos, Direction direction, const std::string& sprite_name, int layer,
               const std::string& light_sprite_name, const std::string& ice_sprite_name) :
  MovingSprite(pos, sprite_name, layer, COLGROUP_DISABLED),
  ExposedObject<BadGuy, scripting::BadGuy>(this),
  m_physic(),
  m_countMe(true),
  m_is_initialized(false),
  m_start_position(m_col.m_bbox.p1()),
  m_dir(direction),
  m_start_dir(direction),
  m_frozen(false),
  m_ignited(false),
  m_in_water(false),
  m_dead_script(),
  m_melting_time(0),
  m_lightsprite(SpriteManager::current()->create(light_sprite_name)),
  m_freezesprite(SpriteManager::current()->create(ice_sprite_name)),
  m_glowing(false),
  m_unfreeze_timer(),
  m_state(STATE_INIT),
  m_is_active_flag(),
  m_state_timer(),
  m_on_ground_flag(false),
  m_floor_normal(0.0f, 0.0f),
  m_colgroup_active(COLGROUP_MOVING)
{
  SoundManager::current()->preload("sounds/squish.wav");
  SoundManager::current()->preload("sounds/fall.wav");
  SoundManager::current()->preload("sounds/sizzle.ogg");
  SoundManager::current()->preload("sounds/splash.ogg");
  SoundManager::current()->preload("sounds/fire.ogg");

  m_dir = (m_start_dir == Direction::AUTO) ? Direction::LEFT : m_start_dir;
  m_lightsprite->set_blend(Blend::ADD);
}

BadGuy::BadGuy(const ReaderMapping& reader, const std::string& sprite_name, int layer,
               const std::string& light_sprite_name, const std::string& ice_sprite_name) :
  BadGuy(reader, sprite_name, Direction::AUTO, layer, light_sprite_name, ice_sprite_name)
{
}

BadGuy::BadGuy(const ReaderMapping& reader, const std::string& sprite_name,
               Direction default_direction, int layer,
               const std::string& light_sprite_name, const std::string& ice_sprite_name) :
  MovingSprite(reader, sprite_name, layer, COLGROUP_DISABLED),
  ExposedObject<BadGuy, scripting::BadGuy>(this),
  m_physic(),
  m_countMe(true),
  m_is_initialized(false),
  m_start_position(m_col.m_bbox.p1()),
  m_dir(Direction::LEFT),
  m_start_dir(default_direction),
  m_frozen(false),
  m_ignited(false),
  m_in_water(false),
  m_dead_script(),
  m_melting_time(0),
  m_lightsprite(SpriteManager::current()->create(light_sprite_name)),
  m_freezesprite(SpriteManager::current()->create(ice_sprite_name)),
  m_glowing(false),
  m_unfreeze_timer(),
  m_state(STATE_INIT),
  m_is_active_flag(),
  m_state_timer(),
  m_on_ground_flag(false),
  m_floor_normal(0.0f, 0.0f),
  m_colgroup_active(COLGROUP_MOVING)
{
  std::string dir_str;
  if (reader.get("direction", dir_str))
    m_start_dir = string_to_dir(dir_str);
  m_dir = m_start_dir;

  reader.get("dead-script", m_dead_script);

  SoundManager::current()->preload("sounds/squish.wav");
  SoundManager::current()->preload("sounds/fall.wav");
  SoundManager::current()->preload("sounds/sizzle.ogg");
  SoundManager::current()->preload("sounds/splash.ogg");
  SoundManager::current()->preload("sounds/fire.ogg");

  m_dir = (m_start_dir == Direction::AUTO) ? Direction::LEFT : m_start_dir;
  m_lightsprite->set_blend(Blend::ADD);
}

void
BadGuy::draw(DrawingContext& context)
{
  if (!m_sprite.get()) return;

  if (m_state == STATE_INIT || m_state == STATE_INACTIVE)
  {
    if (Editor::is_active()) {
      m_sprite->draw(context.color(), get_pos(), m_layer, m_flip);
    }
  }
  else
  {
    if (m_state == STATE_FALLING)
    {
      context.push_transform();
      context.set_flip(context.get_flip() ^ VERTICAL_FLIP);
      m_sprite->draw(context.color(), get_pos(), m_layer, m_flip);
      context.pop_transform();
    }
    else
    {
      if (m_unfreeze_timer.started() && m_unfreeze_timer.get_timeleft() <= 1.f)
      {
        m_sprite->draw(context.color(), get_pos() + Vector(graphicsRandom.randf(-3, 3), 0.f), m_layer-1, m_flip);
        if (is_portable())
          m_freezesprite->draw(context.color(), get_pos() + Vector(graphicsRandom.randf(-3, 3), 0.f), m_layer);
      }
      else
      {
        if (m_frozen && is_portable())
          m_freezesprite->draw(context.color(), get_pos(), m_layer);
        m_sprite->draw(context.color(), get_pos(), m_layer - (m_frozen ? 1 : 0), m_flip);
      }

      if (m_glowing)
      {
        m_lightsprite->draw(context.light(), m_col.m_bbox.get_middle(), 0);
      }
    }
  }
}

void
BadGuy::update(float dt_sec)
{
  if (m_frozen && !is_grabbed())
  {
    set_colgroup_active(std::abs(m_physic.get_velocity_y()) < 0.2f && std::abs(m_physic.get_velocity_x()) < 0.2f
      ? COLGROUP_MOVING_STATIC : COLGROUP_MOVING);
    if (m_unfreeze_timer.check())
      unfreeze(false);
  }
  if (get_pos().x > Sector::get().get_width() || get_pos().x < -get_bbox().get_width() ||
    get_pos().y > Sector::get().get_height()) {
    auto this_portable = dynamic_cast<Portable*> (this);
    if (!this_portable || !this_portable->is_grabbed())
    {
      run_dead_script();
      m_is_active_flag = false;
      remove_me();
      // This was removed due to fixing a bug. If is it needed somewhere, then I'm sorry. --Hume2
      /**if(countMe) {
        // get badguy name from sprite_name ignoring path and extension
        std::string badguy = sprite_name.substr(0, sprite_name.length() - 7);
        int path_chars = badguy.rfind("/",badguy.length());
        badguy = badguy.substr(path_chars + 1, badguy.length() - path_chars);
        // log warning since badguys_killed can no longer reach total_badguys
        std::string current_level = "[" + Sector::get().get_level()->filename + "] ";
        log_warning << current_level << "Counted badguy " << badguy << " starting at " << start_position << " has left the sector" <<std::endl;
      }*/
      return;
    }
  }

  // Deactivate badguy, if off-screen and not falling down.
  if (m_is_active_flag && is_offscreen() && m_physic.get_velocity_y() <= 0.f)
  {
    deactivate();
    set_state(STATE_INACTIVE);
  }

  switch (m_state) {
    case STATE_ACTIVE:
      m_is_active_flag = true;
      if (Editor::is_active()) {
        break;
      }
      //won't work if defined anywhere else for some reason
      if (m_frozen && is_portable())
        m_freezesprite->set_action(get_overlay_size(), 1);
      else
        m_freezesprite->set_action("default", 1);
        
      active_update(dt_sec);
      break;

    case STATE_INIT:
    case STATE_INACTIVE:
      m_is_active_flag = false;
      m_in_water = !Sector::get().is_free_of_tiles(m_col.get_bbox(), false, Tile::WATER);
      inactive_update(dt_sec);
      try_activate();
      break;

    case STATE_BURNING: {
      m_is_active_flag = false;
      m_col.set_movement(m_physic.get_movement(dt_sec));
      if ( m_sprite->animation_done() ) {
        remove_me();
      }
    } break;

    case STATE_GEAR:
    case STATE_SQUISHED:
      m_is_active_flag = false;
      if (m_state_timer.check()) {
        remove_me();
        break;
      }
      m_col.set_movement(m_physic.get_movement(dt_sec));
      break;

    case STATE_MELTING: {
      m_is_active_flag = false;
      m_col.set_movement(m_physic.get_movement(dt_sec));
      if ( m_sprite->animation_done() || on_ground() ) {
        Sector::get().add<WaterDrop>(m_col.m_bbox.p1(), get_water_sprite(), m_physic.get_velocity());
        remove_me();
        break;
      }
    } break;

    case STATE_GROUND_MELTING:
      m_is_active_flag = false;
      m_col.set_movement(m_physic.get_movement(dt_sec));
      if ( m_sprite->animation_done() ) {
        remove_me();
      }
      break;

    case STATE_INSIDE_MELTING: {
      m_is_active_flag = false;
      m_col.set_movement(m_physic.get_movement(dt_sec));
      if ( on_ground() && m_sprite->animation_done() ) {
        set_action("gear", m_dir, 1);
        set_state(STATE_GEAR);
      }
      int pa = graphicsRandom.rand(0,3);
      float px = graphicsRandom.randf(m_col.m_bbox.get_left(), m_col.m_bbox.get_right());
      float py = graphicsRandom.randf(m_col.m_bbox.get_top(), m_col.m_bbox.get_bottom());
      Vector ppos = Vector(px, py);
      Sector::get().add<SpriteParticle>(get_water_sprite(), "particle_" + std::to_string(pa),
                                             ppos, ANCHOR_MIDDLE,
                                             Vector(0, 0), Vector(0, 100 * Sector::get().get_gravity()),
                                             LAYER_OBJECTS-1);
    } break;

    case STATE_FALLING:
      m_is_active_flag = false;
      m_col.set_movement(m_physic.get_movement(dt_sec));
      break;
  }

  m_on_ground_flag = false;
}

void
BadGuy::initialize()
{
}

void
BadGuy::activate()
{
}

void
BadGuy::deactivate()
{
}

std::vector<Direction>
BadGuy::get_allowed_directions() const
{
  return { Direction::AUTO, Direction::LEFT, Direction::RIGHT };
}

void
BadGuy::active_update(float dt_sec)
{
  if (!is_grabbed())
    m_col.set_movement(m_physic.get_movement(dt_sec));
  if (m_frozen)
    m_sprite->stop_animation();
}

void
BadGuy::inactive_update(float )
{
}

void
BadGuy::collision_tile(uint32_t tile_attributes)
{
  // Don't kill badguys that have already been killed.
  if (!is_active()) return;

  if (tile_attributes & Tile::WATER && !is_in_water())
  {
    m_in_water = true;
    SoundManager::current()->play("sounds/splash.ogg", get_pos());
  }
  if (!(tile_attributes & Tile::WATER) && is_in_water())
  {
    m_in_water = false;
  }

  if (tile_attributes & Tile::HURTS && is_hurtable())
  {
    Rectf hurtbox = get_bbox().grown(-6.f);
    if (!Sector::get().is_free_of_tiles(hurtbox, true, Tile::HURTS) || tile_attributes & Tile::UNISOLID)
    {
      if (tile_attributes & Tile::FIRE)
      {
        if (is_flammable()) ignite();
      }
      // Why is this even here????
      //else if (tile_attributes & Tile::ICE)
      //{
      //  if (is_freezable() && !m_frozen) freeze();
      //}
      else
      {
        kill_fall();
      }
    }
  }
}

HitResponse
BadGuy::collision(GameObject& other, const CollisionHit& hit)
{
  if (!is_active()) return ABORT_MOVE;

  if (is_grabbed())
    return FORCE_MOVE;

  auto badguy = dynamic_cast<BadGuy*> (&other);
  if (badguy && hit.bottom && m_frozen && badguy->get_group() != COLGROUP_TOUCHABLE)
  {
    m_physic.set_velocity_y(badguy->m_physic.get_velocity_y());
    if (!badguy->is_frozen())
    {
      badguy->kill_fall();
    }
    return FORCE_MOVE;
  }
  if (badguy && badguy->is_active() && badguy->m_col.get_group() == COLGROUP_MOVING) {

    /* Badguys don't let badguys squish other badguys. It's bad. */
#if 0
    // Hit from above?
    if (badguy->get_bbox().get_bottom() < (bbox.get_top() + (player->is_sliding() ? 8.f : 16.f))) {
      if (collision_squished(*badguy)) {
        return ABORT_MOVE;
      }
    }
#endif

    return collision_badguy(*badguy, hit);
  }

  auto player = dynamic_cast<Player*> (&other);
  if (player) {

    // Hit from above?
    if (player->get_bbox().get_bottom() < (m_col.m_bbox.get_top() + 16)) {
      if (player->is_stone()) {
        kill_fall();
        return FORCE_MOVE;
      }
      if (collision_squished(*player)) {
        return FORCE_MOVE;
      }
    }

    if (player->is_stone()) {
      if (glm::length(player->get_physic().get_movement(.1f)) > 16.f)
      {
        kill_fall();
        return ABORT_MOVE;
      }
      else
      {
        collision_solid(hit);
        return FORCE_MOVE;
      }
    }

    return collision_player(*player, hit);
  }

  auto bullet = dynamic_cast<Bullet*> (&other);
  if (bullet)
    return collision_bullet(*bullet, hit);

  return FORCE_MOVE;
}

void
BadGuy::collision_solid(const CollisionHit& hit)
{
  if (m_frozen)
  {
    if (hit.top || hit.bottom)
      m_physic.set_velocity_y(0);
    if (hit.left || hit.right)
      m_physic.set_velocity_x(0);
    if ((m_physic.get_velocity_x() > -5.0f) &&
      (m_physic.get_velocity_x() < 5.0f))
    {
      m_physic.set_velocity_x(0);
      m_physic.set_acceleration_x(0.0);
    }
    else
    {
      m_physic.set_velocity_x(m_physic.get_velocity_x() - (m_physic.get_velocity_x() > 0.f ? 5.f : -5.f));
    }
  }
  else
  {
    m_physic.set_velocity(0, 0);
  }
  update_on_ground_flag(hit);
}

void
BadGuy::on_flip(float height)
{
  MovingObject::on_flip(height);
  Vector pos = get_start_position();
  pos.y = height - pos.y;
  set_start_position(pos);
}

HitResponse
BadGuy::collision_player(Player& player, const CollisionHit& hit)
{
  if (player.is_invincible() ||
    (is_snipable() && (player.m_does_buttjump || player.is_sliding()))) {
    kill_fall();
    return ABORT_MOVE;
  }

  if (is_grabbed())
    return FORCE_MOVE;

  if(player.get_grabbed_object() != nullptr && !m_frozen)
  {
      auto badguy = dynamic_cast<BadGuy*>(player.get_grabbed_object());
      if(badguy != nullptr)
      {
        player.get_grabbed_object()->ungrab(player, player.m_dir);
        player.stop_grabbing();
        badguy->kill_fall();
        kill_fall();
        return ABORT_MOVE;
      }
  }
  //TODO: Unfreeze timer.
  if (m_frozen)
  {
    player.collision_solid(hit);
  }
  else
    player.kill(false);
  return FORCE_MOVE;
}

HitResponse
BadGuy::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  if (badguy.is_frozen())
    collision_solid(hit);
  return FORCE_MOVE;
}

bool
BadGuy::collision_squished(GameObject& object)
{
  // Frozen badguys can be killed with butt-jump.
  if (m_frozen)
  {
    auto player = dynamic_cast<Player*>(&object);
    if (player && (player->m_does_buttjump)) {
      player->bounce(*this);
      kill_fall();
      return true;
    }
  }
    return false;
}

HitResponse
BadGuy::collision_bullet(Bullet& bullet, const CollisionHit& hit)
{
  if (is_frozen()) {
    if (bullet.get_type() == FIRE_BONUS) {
      // Fire bullet thaws frozen badguys.
      unfreeze();
      bullet.remove_me();
      return ABORT_MOVE;
    } else {
      // Other bullets ricochet.
      bullet.ricochet(*this, hit);
      return FORCE_MOVE;
    }
  }
  else if (is_ignited()) {
    if (bullet.get_type() == ICE_BONUS) {
      // Ice bullets extinguish ignited badguys.
      extinguish();
      bullet.remove_me();
      return ABORT_MOVE;
    } else {
      // Other bullets are absorbed by ignited badguys.
      bullet.remove_me();
      return FORCE_MOVE;
    }
  }
  else if (bullet.get_type() == FIRE_BONUS && is_flammable()) {
    // Fire bullets ignite flammable badguys.
    ignite();
    bullet.remove_me();
    return ABORT_MOVE;
  }
  else if (bullet.get_type() == ICE_BONUS && is_freezable()) {
    // Ice bullets freeze freezable badguys.
    freeze();
    bullet.remove_me();
    return ABORT_MOVE;
  }
  else {
    // In all other cases, bullets ricochet.
    bullet.ricochet(*this, hit);
    return FORCE_MOVE;
  }
}

void
BadGuy::kill_squished(GameObject& object)
{
  if (!is_active()) return;

  SoundManager::current()->play("sounds/squish.wav", get_pos());
  m_physic.enable_gravity(true);
  m_physic.set_velocity(0, 0);
  set_state(STATE_SQUISHED);
  set_group(COLGROUP_MOVING_ONLY_STATIC);
  auto player = dynamic_cast<Player*>(&object);
  if (player) {
    player->bounce(*this);
  }

  // Start the dead-script.
  run_dead_script();
}

void
BadGuy::kill_fall()
{
  if (!is_active()) return;

  if (m_frozen) {
    SoundManager::current()->play("sounds/brick.wav", get_pos());
    Vector pr_pos(0.0f, 0.0f);
    float cx = m_col.m_bbox.get_width() / 2.f;
    float cy = m_col.m_bbox.get_height() / 2.f;
    for (pr_pos.x = 0.f; pr_pos.x < m_col.m_bbox.get_width(); pr_pos.x +=  18.f) {
      for (pr_pos.y = 0.f; pr_pos.y < m_col.m_bbox.get_height(); pr_pos.y += 18.f) {
        Vector speed = Vector((pr_pos.x - cx) * 3.f, (pr_pos.y - cy) * 2.f);
        Sector::get().add<SpriteParticle>(
            "images/particles/ice_piece"+std::to_string(graphicsRandom.rand(1, 3))+".sprite", "default",
            m_col.m_bbox.p1() + pr_pos, ANCHOR_MIDDLE,
            //SPEED: Add current enemy speed, but do not add downwards velocity because it looks bad.
            Vector(m_physic.get_velocity_x(), m_physic.get_velocity_y() > 0.f ? 0.f : m_physic.get_velocity_y())
            //SPEED: Add specified speed and randomization.
          + speed + Vector(graphicsRandom.randf(-30.f, 30.f), graphicsRandom.randf(-30.f, 30.f)),
            Vector(0, Sector::get().get_gravity() * graphicsRandom.randf(100.f, 120.f)), LAYER_OBJECTS - 1, true);
      }
    }
    // Start the dead-script.
    run_dead_script();
    remove_me();
  } else {
    SoundManager::current()->play("sounds/fall.wav", get_pos());
    m_physic.set_velocity_y(0);
    m_physic.set_acceleration_y(0);
    m_physic.enable_gravity(true);
    set_state(STATE_FALLING);

    // Set the badguy layer to be the foremost, so that
    // this does not reveal secret tilemaps:
    m_layer = Sector::get().get_foremost_opaque_layer() + 1;
    // Start the dead-script.
    run_dead_script();
  }

}

void
BadGuy::run_dead_script()
{
  if(!m_is_active_flag)
     return;

  m_is_active_flag = false;

  if (m_countMe)
    Sector::get().get_level().m_stats.increment_badguys();

  if (m_parent_dispenser != nullptr)
  {
    m_parent_dispenser->notify_dead();
  }

  // Start the dead-script.
  if (!m_dead_script.empty()) {
    Sector::get().run_script(m_dead_script, "dead-script");
  }
}

void
BadGuy::set_state(State state_)
{
  if (m_state == state_)
    return;

  State laststate = m_state;
  m_state = state_;
  switch (state_) {
    case STATE_BURNING:
      m_state_timer.start(BURN_TIME);
      break;
    case STATE_SQUISHED:
      m_state_timer.start(SQUISH_TIME);
      break;
    case STATE_GEAR:
      m_state_timer.start(GEAR_TIME);
      break;
    case STATE_ACTIVE:
      set_group(m_colgroup_active);
      play_looping_sounds();
      //bbox.set_pos(start_position);
      break;
    case STATE_INACTIVE:
      // Was the badguy dead anyway?
      if (laststate == STATE_SQUISHED || laststate == STATE_FALLING) {
        remove_me();
      }
      stop_looping_sounds();
      set_group(COLGROUP_DISABLED);
      break;
    case STATE_FALLING:
      set_group(COLGROUP_DISABLED);
      break;
    default:
      break;
  }
}

bool
BadGuy::is_offscreen() const
{
  Vector cam_dist(0.0f, 0.0f);
  Vector player_dist(0.0f, 0.0f);
  Camera& cam = Sector::get().get_camera();
  cam_dist = cam.get_center() - m_col.m_bbox.get_middle();
  if (Editor::is_active()) {
      if ((fabsf(cam_dist.x) <= X_OFFSCREEN_DISTANCE) && (fabsf(cam_dist.y) <= Y_OFFSCREEN_DISTANCE)) {
        return false;
    }
  }
  auto player = get_nearest_player();
  if (!player)
    return false;
  if (!Editor::is_active()) {
    player_dist = player->get_bbox().get_middle() - m_col.m_bbox.get_middle();
  }
  // In SuperTux 0.1.x, Badguys were activated when Tux<->Badguy center distance was approx. <= ~668px.
  // This doesn't work for wide-screen monitors which give us a virt. res. of approx. 1066px x 600px.
  if (((fabsf(player_dist.x) <= X_OFFSCREEN_DISTANCE) && (fabsf(player_dist.y) <= Y_OFFSCREEN_DISTANCE))
      ||((fabsf(cam_dist.x) <= X_OFFSCREEN_DISTANCE) && (fabsf(cam_dist.y) <= Y_OFFSCREEN_DISTANCE))) {
    return false;
  }
  return true;
}

void
BadGuy::try_activate()
{
  // Don't activate if player is dying.
  auto player = get_nearest_player();
  if (!player) return;

  if (!is_offscreen()) {
    set_state(STATE_ACTIVE);
    if (!m_is_initialized) {

      // If starting direction was set to AUTO, this is our chance to re-orient the badguy.
      if (m_start_dir == Direction::AUTO) {
        auto player_ = get_nearest_player();
        if (player_ && (player_->get_bbox().get_left() > m_col.m_bbox.get_right())) {
          m_dir = Direction::RIGHT;
        } else {
          m_dir = Direction::LEFT;
        }
      }

      initialize();
      m_is_initialized = true;
    }
    activate();
  }
}

bool
BadGuy::might_fall(int height) const
{
  // Make sure we check for at least a 1-pixel fall.
  assert(height > 0);

  float x1;
  float x2;
  float y1 = m_col.m_bbox.get_bottom() + 1;
  float y2 = m_col.m_bbox.get_bottom() + 1 + static_cast<float>(height);
  if (m_dir == Direction::LEFT) {
    x1 = m_col.m_bbox.get_left() - 1;
    x2 = m_col.m_bbox.get_left();
  } else {
    x1 = m_col.m_bbox.get_right();
    x2 = m_col.m_bbox.get_right() + 1;
  }
  const Rectf rect = Rectf(x1, y1, x2, y2);

  return Sector::get().is_free_of_statics(rect) && Sector::get().is_free_of_specifically_movingstatics(rect);
}

Player*
BadGuy::get_nearest_player() const
{
  return Sector::get().get_nearest_player(m_col.m_bbox);
}

void
BadGuy::update_on_ground_flag(const CollisionHit& hit)
{
  if (hit.bottom) {
    m_on_ground_flag = true;
    m_floor_normal = hit.slope_normal;
  }
}

bool
BadGuy::on_ground() const
{
  return m_on_ground_flag;
}

bool
BadGuy::is_active() const
{
  return m_is_active_flag;
}

Vector
BadGuy::get_floor_normal() const
{
  return m_floor_normal;
}

void
BadGuy::grab(MovingObject& object, const Vector& pos, Direction dir_)
{
  Portable::grab(object, pos, dir_);
  m_col.set_movement(pos - get_pos());
  m_dir = dir_;
  if (m_frozen)
  {
    m_unfreeze_timer.stop();
    if (m_sprite->has_action("iced-left"))
    {
      set_action("iced", m_dir, 1);
      // When the sprite doesn't have sepaigrate actions for left and right, it tries to use an universal one.
    }
    else
    {
      if (m_sprite->has_action("iced"))
      {
        set_action("iced", 1);
      }
      // When no iced action exists, default to shading badguy blue.
      else
      {
        m_sprite->set_color(Color(0.60f, 0.72f, 0.88f));
        m_sprite->stop_animation();
      }
    }
  }
  set_colgroup_active(COLGROUP_DISABLED);
}

void
BadGuy::ungrab(MovingObject& object, Direction dir_)
{
  auto player = dynamic_cast<Player*> (&object);
  set_colgroup_active(m_frozen ? COLGROUP_MOVING_STATIC : COLGROUP_MOVING);
  if (m_frozen)
  {
    m_unfreeze_timer.start(8.f);
    if (player->is_swimming() || player->is_water_jumping())
    {
      float swimangle = player->get_swimming_angle();
      m_physic.set_velocity(player->get_velocity() + Vector(std::cos(swimangle), std::sin(swimangle)));
    }
    else
    {
      m_physic.set_velocity_x(fabsf(player->get_physic().get_velocity_x()) < 1.0f ? 0.f :
        player->m_dir == Direction::LEFT ? -200.f : 200.f);
      if (dir_ == Direction::UP)
      {
        m_physic.set_velocity_y(-500.f);
      }
      else if (dir_ == Direction::DOWN)
      {
        Vector mov(0, 32);
        if (Sector::get().is_free_of_statics(get_bbox().moved(mov), this))
        {
          // There is free space, so throw it down.
          m_physic.set_velocity_y(500.f);
        }
      }
      else if (fabsf(player->get_physic().get_velocity_x()) > 1.0f)
        m_physic.set_velocity_y(-200);
      else
        m_physic.set_velocity_y(0.f);
    }
  }
  Portable::ungrab(object, dir_);
}

bool
BadGuy::is_portable() const
{
  return m_frozen;
}

void
BadGuy::freeze()
{
  m_frozen = true;
  m_unfreeze_timer.start(8.f);
  set_colgroup_active(COLGROUP_MOVING_STATIC);
  SoundManager::current()->play("sounds/sizzle.ogg", get_pos());

  float freezesize_x =
    get_overlay_size() == "3x3" ? 96.f :
    get_overlay_size() == "2x2" ? 64.f :
    get_overlay_size() == "2x1" ? 64.f : 45.f;

  float freezesize_y =
    get_overlay_size() == "3x3" ? 94.f :
    get_overlay_size() == "2x2" ? 62.f :
    get_overlay_size() == "2x1" ? 43.f :
    get_overlay_size() == "1x2" ? 62.f : 43.f;

  set_pos(Vector(get_bbox().get_left(), get_bbox().get_bottom() - freezesize_y));

  if (m_sprite->has_action("iced-left"))
    set_action("iced", m_dir, 1);
  // When the sprite doesn't have separate actions for left and right, it tries to use an universal one.
  else
  {
    if (m_sprite->has_action("iced"))
      set_action("iced", 1);
    // When no iced action exists, default to shading badguy blue.
    else
    {
      m_sprite->set_color(Color(0.60f, 0.72f, 0.88f));
      m_sprite->stop_animation();
    }
  }
  m_col.set_size(freezesize_x, freezesize_y);
}

void
BadGuy::unfreeze(bool melt)
{
  if (!m_ignited)
  {
    set_colgroup_active(COLGROUP_MOVING);

    m_sprite->set_color(Color(1.f, 1.f, 1.f));
    m_sprite->set_animation_loops();
  }
  m_frozen = false;
  m_unfreeze_timer.stop();
  m_col.set_size(m_sprite->get_current_hitbox_width(), m_sprite->get_current_hitbox_height());

  SoundManager::current()->play(melt ? "sounds/splash.ogg" : "sounds/brick.wav", get_pos());
  Vector pr_pos(0.0f, 0.0f);
  float cx = m_col.m_bbox.get_width() / 2.f;
  std::string particle_sprite_name = melt ? "images/particles/water_piece" : "images/particles/ice_piece";
  for (pr_pos.x = 0; pr_pos.x < m_col.m_bbox.get_width(); pr_pos.x += 16.f) {
    for (pr_pos.y = 0; pr_pos.y < m_col.m_bbox.get_height(); pr_pos.y += 16.f) {
      Vector speed = Vector((pr_pos.x - cx) * 2.f, 0.f);
      Sector::get().add<SpriteParticle>(
        particle_sprite_name + std::to_string(graphicsRandom.rand(1, 3)) + ".sprite", "default",
        m_col.m_bbox.p1() + pr_pos, ANCHOR_MIDDLE,
        //SPEED: add current enemy speed but do not add downwards velocity because it looks bad.
        Vector(m_physic.get_velocity_x(), m_physic.get_velocity_y() > 0.f ? 0.f : m_physic.get_velocity_y())
        //SPEED: add specified speed and randomization.
        + speed + Vector(graphicsRandom.randf(-30.f, 30.f), 0.f),
        Vector(0.f, Sector::get().get_gravity() * graphicsRandom.randf(100.f, 120.f)), LAYER_OBJECTS + 1, true);
    }
  }
}

bool
BadGuy::is_freezable() const
{
  return false;
}

bool
BadGuy::is_frozen() const
{
  return m_frozen;
}

bool
BadGuy::is_in_water() const
{
  return m_in_water;
}

void
BadGuy::ignite()
{
  if (!is_flammable() || m_ignited) {
    return;
  }

  if (is_frozen())
    unfreeze();

  m_physic.enable_gravity(true);
  m_physic.set_velocity(0, 0);
  set_group(COLGROUP_MOVING_ONLY_STATIC);
  m_sprite->stop_animation();
  m_ignited = true;

  if (m_sprite->has_action("melting-left")) {

    // Melt it!
    if (m_sprite->has_action("ground-melting-left") && on_ground()) {
      set_action("ground-melting", m_dir, 1);
      SoundManager::current()->play("sounds/splash.ogg", get_pos());
      set_state(STATE_GROUND_MELTING);
    } else {
      set_action("melting", m_dir, 1);
      SoundManager::current()->play("sounds/sizzle.ogg", get_pos());
      set_state(STATE_MELTING);
    }

    run_dead_script();

  } else if (m_sprite->has_action("burning-left")) {
    // Burn it!
    m_glowing = true;
    SoundManager::current()->play("sounds/fire.ogg", get_pos());
    set_action("burning", m_dir, 1);
    set_state(STATE_BURNING);
    run_dead_script();
  } else if (m_sprite->has_action("inside-melting-left")) {
    // melt it inside!
    SoundManager::current()->play("sounds/splash.ogg", get_pos());
    set_action("inside-melting", m_dir, 1);
    set_state(STATE_INSIDE_MELTING);
    run_dead_script();
  } else {
    // Let it fall off the screen then.
    kill_fall();
  }
}

void
BadGuy::extinguish()
{
}

bool
BadGuy::is_flammable() const
{
  return true;
}

bool
BadGuy::is_ignited() const
{
  return m_ignited;
}

void
BadGuy::set_colgroup_active(CollisionGroup group_)
{
  m_colgroup_active = group_;
  if (m_state == STATE_ACTIVE) set_group(group_);
}

ObjectSettings
BadGuy::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  if (!get_allowed_directions().empty())
    result.add_direction(_("Direction"), &m_start_dir, get_allowed_directions(), "direction");
  result.add_script(_("Death script"), &m_dead_script, "dead-script");

  result.reorder({"direction", "sprite", "x", "y"});

  return result;
}

void
BadGuy::after_editor_set()
{
  MovingSprite::after_editor_set();

  const std::string direction_str = m_start_dir == Direction::AUTO ? "left" : dir_to_string(m_start_dir);
  const std::string actions[] = {"editor", "normal", "idle", "flying", "walking", "standing", "swim"};
  bool action_set = false;

  for (const auto& action_str : actions)
  {
    const std::string test_action = action_str + "-" + direction_str;
    if (m_sprite->has_action(test_action))
    {
      set_action(test_action);
      action_set = true;
      break;
    }
    else if (m_sprite->has_action(action_str))
    {
      set_action(action_str);
      action_set = true;
      break;
    }
  }

  if (!action_set)
  {
    if (m_sprite->has_action(direction_str))
    {
      set_action(direction_str);
    }
    else
    {
      log_warning << "Couldn't find editor sprite action for badguy direction='"
                  << dir_to_string(m_start_dir) << "': " << get_class_name() << "." << std::endl;
    }
  }
}

bool
BadGuy::can_be_affected_by_wind() const
{
  return !on_ground();
}

void
BadGuy::add_wind_velocity(const Vector& velocity, const Vector& end_speed)
{
  // Only add velocity in the same direction as the wind.
  if (end_speed.x > 0 && m_physic.get_velocity_x() < end_speed.x)
    m_physic.set_velocity_x(std::min(m_physic.get_velocity_x() + velocity.x, end_speed.x));
  if (end_speed.x < 0 && m_physic.get_velocity_x() > end_speed.x)
    m_physic.set_velocity_x(std::max(m_physic.get_velocity_x() + velocity.x, end_speed.x));
  if (end_speed.y > 0 && m_physic.get_velocity_y() < end_speed.y)
    m_physic.set_velocity_y(std::min(m_physic.get_velocity_y() + velocity.y, end_speed.y));
  if (end_speed.y < 0 && m_physic.get_velocity_y() > end_speed.y)
    m_physic.set_velocity_y(std::max(m_physic.get_velocity_y() + velocity.y, end_speed.y));
}

/* EOF */
