//  SuperTux
//  Copyright (C) 2023 MatusGuy
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

#include "badguy/granito.hpp"

#include "badguy/granito_big.hpp"
#include "math/random.hpp"
#include "object/player.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

Granito::Granito(const ReaderMapping& reader, const std::string& sprite_name, int layer) :
  WalkingBadguy(reader, sprite_name, "left", "right", layer),
  m_walk_interval(),
  m_state(STATE_STAND),
  m_original_state(STATE_STAND),
  m_has_waved(false),
  m_has_player_on_top(false),
  m_airborne(false),
  m_detect_script(),
  m_carried_script(),
  m_carrier(nullptr)
{
  parse_type(reader);

  walk_speed = 0;
  set_ledge_behavior(LedgeBehavior::NORMAL);

  m_can_glint = false;

  set_colgroup_active(COLGROUP_MOVING_STATIC);
  m_col.set_unisolid(true);

  reader.get("detect-script", m_detect_script);
  reader.get("carried-script", m_carried_script);
}

void
Granito::active_update_finish(float dt_sec, bool player_on_top)
{
  WalkingBadguy::active_update(dt_sec);
  m_has_player_on_top = player_on_top;

  // If being carried, let the Big Granito carrier do the work of
  // propagating the movement.
  if (m_carrier == nullptr)
    m_col.propagate_movement(m_col.get_movement());
}

void
Granito::active_update(float dt_sec)
{
  if (m_state == STATE_SIT)
  {
    active_update_finish(dt_sec);
    return;
  }

  Rectf airbornebox = get_bbox();
  airbornebox.set_bottom(get_bbox().get_bottom() + 8.f);
  bool airbornebefore = m_airborne;
  m_airborne = (Sector::get().is_free_of_statics(airbornebox));

  if (m_airborne && get_velocity_y() != 0)
  {
    // Choose if falling or jumping
    if (get_velocity_y() < 5)
      set_action("jump", m_dir);
    else if (get_velocity_y() > 5)
      set_action("fall", m_dir);
  }
  else if (!m_airborne && airbornebefore)
  {
    // Go back to normal action
    if (m_state == STATE_STAND)
    {
      set_action("stand", m_dir);
    }
    else if (m_state == STATE_WALK)
    {
      set_action(m_dir);
    }
  }

  if (m_type == WALK)
  {
    active_update_finish(dt_sec);
    return;
  }

  if ((m_state == STATE_LOOKUP && !m_has_player_on_top) ||
      (m_state == STATE_JUMPING && on_ground()))
  {
    restore_original_state();
  }

  if (m_state == STATE_LOOKUP || m_state == STATE_JUMPING)
  {
    active_update_finish(dt_sec);
    return;
  }

  if (!m_has_waved)
  {
    if (m_state == STATE_WAVE)
    {
      if (!m_sprite->animation_done())
      {
        // Still waving
        active_update_finish(dt_sec, true);
        return;
      }
      else
      {
        // Finished waving
        restore_original_state();
        m_has_waved = true;
      }
    }
    else
    {
      try_wave();
    }
  }

  if (m_type == DEFAULT && try_jump())
  {
    active_update_finish(dt_sec);
    return;
  }

  if (m_type == SCRIPTABLE && m_walk_interval.check())
  {
    stand();
  }

  // Only called when timer has finished
  else if (m_type != SCRIPTABLE && !m_walk_interval.started() && !m_walk_interval.check())
  {
    m_walk_interval.start(gameRandom.randf(1.f, 4.f));

    switch (m_type)
    {
      case STAND:
        if (gameRandom.rand(100) > 50)
        {
          // Turn around
          turn(m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
        }

        break;

      case DEFAULT:
      {
        if (gameRandom.rand(100) > 50 && walk_speed == 0)
        {
          // Turn around
          turn(m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
        }
        else
        {
          // Walk/stop
          if (walk_speed > 0)
          {
            stand();
          }
          else
          {
            turn(m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
            walk();
          }
        }

        break;
      }

      default:
        break;
    }
  }

  active_update_finish(dt_sec);
}

HitResponse
Granito::collision_player(Player& player, const CollisionHit& hit)
{
  if (m_state == STATE_SIT || m_type == WALK || m_type == SCRIPTABLE) return FORCE_MOVE;

  if (hit.top)
  {
    m_has_player_on_top = true;

    if (m_state != STATE_LOOKUP)
    {
      m_state = STATE_LOOKUP;
      walk_speed = 0;
      m_physic.set_velocity_x(0);
      set_action("lookup", m_dir);

      // Don't wave again because we've already spotted the player
      m_has_waved = true;
    }
  }

  return FORCE_MOVE;
}

HitResponse
Granito::collision(MovingObject& other, const CollisionHit& hit)
{
  if (hit.bottom)
  {
    if (m_state == STATE_SIT)
      goto granito_collision_end;

    // Yo big granito can i sit on top of your head?
    GranitoBig* granito = dynamic_cast<GranitoBig*>(&other);

    if (!granito)
    {
      // I'm not a big granito.
      goto granito_collision_end;
    }

    if (granito->get_carrying() != nullptr)
    {
      // Sorry, im already carrying this guy.
      goto granito_collision_end;
    }

    // Sure dude.
    granito->carry(this);

    // Yay!
    m_state = STATE_SIT;
    Sector::get().run_script(m_carried_script, "carried-script");
    walk_speed = 0;
    m_physic.reset();
  }

  if (other.get_group() == COLGROUP_MOVING_STATIC &&
      m_dir == Direction::LEFT ? hit.left : hit.right)
  {
    turn(invert_dir(m_dir));
    return ABORT_MOVE;
  }

granito_collision_end:

  // Call other collision functions (collision_player, collision_badguy, ...)
  WalkingBadguy::collision(other, hit);

  return FORCE_MOVE;
}

void
Granito::kill_fall()
{
  return;
}

ObjectSettings
Granito::get_settings()
{
  auto settings = WalkingBadguy::get_settings();

  settings.remove("dead-script");

  settings.add_script(get_uid(), _("Detect script"), &m_detect_script, "detect-script");
  settings.add_script(get_uid(), _("Carried script"), &m_carried_script, "carried-script");

  return settings;
}

void
Granito::activate()
{
  WalkingBadguy::activate();
  reset_detection();
}

GameObjectTypes
Granito::get_types() const
{
  return {
    // Big & small granito
    { "default", _("Default") },
    { "standing", _("Standing") },
    { "walking", _("Walking") },
    { "scriptable", _("Scriptable") },

    // Small granito only
    { "sitting", _("Sitting") }
  };
}

void
Granito::after_editor_set()
{
  WalkingBadguy::after_editor_set();

  switch (m_type)
  {
    case DEFAULT:
      set_action(m_dir);
      break;
    case SIT:
      set_action("sit", m_dir);
      break;
    case SCRIPTABLE:
    case STAND:
      set_action("stand", m_dir);
      break;
  }
}

std::string
Granito::get_carrier_name() const
{
  return m_carrier != nullptr ? m_carrier->get_name() : "";
}

void
Granito::initialize()
{
  WalkingBadguy::initialize();

  if (m_type == WALK)
  {
    walk();
  }
  else if (m_type == SIT)
  {
    sit();
  }

  switch (m_type)
  {
    case DEFAULT:
      set_action(m_dir);
      break;

    case SIT:
      set_action("sit", m_dir);
      break;

    case SCRIPTABLE:
    case STAND:
      set_action("stand", m_dir);
      break;
  }
}

void
Granito::update_hitbox()
{
  WalkingBadguy::update_hitbox();
  m_col.set_unisolid(true);
}

bool
Granito::try_wave()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  if (!on_ground()) return false;

  Player* player = get_nearest_player();
  if (!player)
    return false;

  Vector mid = get_bbox().get_middle();
  Vector plrmid = player->get_bbox().get_middle();

  float xdist = mid.x - plrmid.x;
  if (std::abs(xdist) > 32.f*4.f)
    return false;

  RaycastResult result = Sector::get().get_first_line_intersection(mid, plrmid, false, get_collision_object());

  CollisionObject** resultobj = std::get_if<CollisionObject*>(&result.hit);
  if (resultobj && *resultobj == player->get_collision_object())
  {
    // Only wave if facing player.
    if (xdist == std::abs(xdist) * (m_dir == Direction::LEFT ? -1 : 1))
      return false;

    Sector::get().run_script(m_detect_script, "detect-script");
    if (m_type == SCRIPTABLE)
      m_has_waved = true;
    else
      wave();

    return true;
  }

  return false;
}

void
Granito::wave()
{
  if (m_type == SCRIPTABLE)
    reset_detection();

  walk_speed = 0;
  m_physic.set_velocity_x(0);

  m_state = STATE_WAVE;

  set_action("wave", m_dir, 1);
}

void
Granito::sit()
{
  walk_speed = 0;
  m_state = STATE_SIT;
  m_original_state = STATE_SIT;
  m_physic.set_velocity_x(0);

  if (!m_airborne)
  {
    float oldheight = get_bbox().get_size().height;
    set_action("sit", m_dir);

    float height = get_bbox().get_size().height;
    set_pos(Vector(get_bbox().get_left(), get_bbox().get_top() + oldheight - height));
  }
  else
  {
    set_action("sit", m_dir);
  }
}

void
Granito::turn(const Direction& direction)
{
  m_dir = direction;
  switch (m_state)
  {
    case STATE_WALK:
      walk();
      break;

    case STATE_STAND:
      set_action("stand", m_dir);
      break;

    case STATE_SIT:
      set_action("sit", m_dir);
      break;

    case STATE_LOOKUP:
      set_action("lookup", m_dir);
      break;

    case STATE_JUMPING:
      set_action("jump", m_dir);
      break;

    default:
      break;
  }
}

void
Granito::turn(const std::string& direction)
{
  turn(string_to_dir(direction));
}

void
Granito::walk()
{
  walk_speed = 80;
  m_state = STATE_WALK;
  m_original_state = STATE_WALK;
  m_physic.set_velocity_x(80 * (m_dir == Direction::LEFT ? -1 : 1));
  set_action(m_dir);
}

void
Granito::walk_for(float seconds)
{
  m_walk_interval.start(seconds);
  if (m_original_state != STATE_WALK)
    walk();
}

void
Granito::stand()
{
  walk_speed = 0;
  m_state = STATE_STAND;
  m_original_state = STATE_STAND;
  m_physic.set_velocity_x(0);
  set_action("stand", m_dir);
}

bool
Granito::try_jump()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  if (walk_speed == 0 || m_airborne) return false;

  float eye = (m_dir == Direction::LEFT ? get_bbox().get_left() : get_bbox().get_right());
  float inc = (m_dir == Direction::LEFT ? -32.f : 32.f);

  RaycastResult result = Sector::get().get_first_line_intersection({eye, get_bbox().get_middle().y},
                                                                   {eye + inc, get_bbox().get_middle().y},
                                                                   false,
                                                                   get_collision_object());

  if (!result.is_valid) return false;

  auto result_tile = std::get_if<const Tile*>(&result.hit);
  if (result_tile)
  {
    if ((*result_tile)->is_slope())
      return false;
  }
  else
  {
    auto result_obj = std::get_if<CollisionObject*>(&result.hit);
    if (result_obj && !dynamic_cast<Granito*>(&(*result_obj)->get_parent()))
      return false;
  }

  const Rectf detect(Vector(eye + (m_dir == Direction::LEFT ? -48.f : 16.f),
                            get_bbox().get_top() - (32.f*2)),
                     get_bbox().get_size());

  if (!Sector::get().is_free_of_tiles(detect.grown(-1.f))) return false;

  jump();
  return true;
}

void
Granito::jump()
{
  m_state = STATE_JUMPING;
  m_physic.set_velocity_y(-420.f);
}

void Granito::eject()
{
  if (!m_carrier)
    return;

  m_carrier->eject();
}

void
Granito::restore_original_state()
{
  if (m_state == m_original_state)
    return;

  m_state = m_original_state;

  if (m_state == STATE_WALK)
  {
    walk();
  }
  else
  {
    stand();
  }
}

void
Granito::register_class(ssq::VM& vm)
{
  ssq::Class cls = vm.addAbstractClass<Granito>("Granito", vm.findClass("BadGuy"));

  cls.addFunc("wave", &Granito::wave);
  cls.addFunc("sit", &Granito::sit);
  cls.addFunc("turn", static_cast<void(Granito::*)(const std::string&)>(&Granito::turn));
  cls.addFunc("set_walking", &Granito::set_walking);
  cls.addFunc("walk", &Granito::walk);
  cls.addFunc("walk_for", &Granito::walk_for);
  cls.addFunc("stand", &Granito::stand);
  cls.addFunc("jump", &Granito::jump);
  cls.addFunc("get_state", &Granito::get_state);
  cls.addFunc("get_carrier_name", &Granito::get_carrier_name);
  cls.addFunc("reset_detection", &Granito::reset_detection);
  cls.addFunc("eject", &Granito::eject);

  // Enumerators
  vm.setConst<int>("GRANITO_STATE_SIT", Granito::STATE_SIT);
  vm.setConst<int>("GRANITO_STATE_STAND", Granito::STATE_STAND);
  vm.setConst<int>("GRANITO_STATE_WALK", Granito::STATE_WALK);
  vm.setConst<int>("GRANITO_STATE_WAVE", Granito::STATE_WAVE);
  vm.setConst<int>("GRANITO_STATE_LOOKUP", Granito::STATE_LOOKUP);
  vm.setConst<int>("GRANITO_STATE_JUMPING", Granito::STATE_JUMPING);
}
