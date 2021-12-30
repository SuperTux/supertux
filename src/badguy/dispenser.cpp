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

#include "badguy/dispenser.hpp"

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "math/random.hpp"
#include "object/bullet.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/game_object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

Dispenser::DispenserType
Dispenser::DispenserType_from_string(const std::string& type_string)
{
  if (type_string == "dropper") {
    return DispenserType::DROPPER;
  } else if (type_string == "rocketlauncher") {
    return DispenserType::ROCKETLAUNCHER;
  } else if (type_string == "cannon") {
    return DispenserType::CANNON;
  } else if (type_string == "point") {
    return DispenserType::POINT;
  } else {
    throw std::exception();
  }
}

std::string
Dispenser::DispenserType_to_string(DispenserType type)
{
  switch (type)
  {
    case DispenserType::DROPPER:
      return "dropper";
    case DispenserType::ROCKETLAUNCHER:
      return "rocketlauncher";
    case DispenserType::CANNON:
      return "cannon";
    case DispenserType::POINT:
      return "point";
    default:
      return "unknown";
  }
}

Dispenser::Dispenser(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/dispenser/dispenser.sprite"),
  ExposedObject<Dispenser, scripting::Dispenser>(this),
  m_cycle(),
  m_badguys(),
  m_next_badguy(0),
  m_dispense_timer(),
  m_autotarget(false),
  m_swivel(false),
  m_broken(false),
  m_random(),
  m_gravity(),
  m_type(),
  m_type_str(),
  m_limit_dispensed_badguys(),
  m_max_concurrent_badguys(),
  m_current_badguys(),
  m_flip(NO_FLIP)
{
  set_colgroup_active(COLGROUP_MOVING_STATIC);
  SoundManager::current()->preload("sounds/squish.wav");
  reader.get("cycle", m_cycle, 5.0f);
  if (reader.get("gravity", m_gravity)) m_physic.enable_gravity(true);
  if ( !reader.get("badguy", m_badguys)) m_badguys.clear();
  reader.get("random", m_random, false);
  std::string type_s = "dropper"; //default
  reader.get("type", type_s, "");
  try
  {
    m_type = DispenserType_from_string(type_s);
  }
  catch(std::exception&)
  {
    if (!Editor::is_active())
    {
      if (type_s.empty()) {
        log_warning << "No dispenser type set, setting to dropper." << std::endl;
      }
      else {
        log_warning << "Unknown type of dispenser:" << type_s << ", setting to dropper." << std::endl;
      }
    }
    m_type = DispenserType::DROPPER;
  }

  m_type_str = DispenserType_to_string(m_type);

  reader.get("limit-dispensed-badguys", m_limit_dispensed_badguys, false);
  reader.get("max-concurrent-badguys", m_max_concurrent_badguys, 0);

//  if (badguys.size() <= 0)
//    throw std::runtime_error("No badguys in dispenser.");

  switch (m_type)
  {
    case DispenserType::DROPPER:
      m_sprite->set_action("dropper");
      break;

    case DispenserType::ROCKETLAUNCHER:
      m_sprite->set_action(m_dir == Direction::LEFT ? "working-left" : "working-right");
      set_colgroup_active(COLGROUP_MOVING); //if this were COLGROUP_MOVING_STATIC MrRocket would explode on launch.

      if (m_start_dir == Direction::AUTO) {
        m_autotarget = true;
      }
      break;

    case DispenserType::CANNON:
      m_sprite->set_action("working");
      break;

    case DispenserType::POINT:
      m_sprite->set_action("invisible");
      set_colgroup_active(COLGROUP_DISABLED);
      break;

    default:
      break;
  }

  m_col.m_bbox.set_size(m_sprite->get_current_hitbox_width(), m_sprite->get_current_hitbox_height());
  m_countMe = false;
}

void
Dispenser::draw(DrawingContext& context)
{
  if (m_type != DispenserType::POINT || Editor::is_active()) {
    if (!m_gravity)
      context.set_flip(context.get_flip() ^ m_flip);
    BadGuy::draw(context);
    if (!m_gravity)
      context.set_flip(context.get_flip() ^ m_flip);
  }
}

void
Dispenser::activate()
{
  if (m_broken){
    return;
  }
  if (m_autotarget && !m_swivel){ // auto cannon sprite might be wrong
    auto* player = get_nearest_player();
    if (player) {
      m_dir = (player->get_pos().x > get_pos().x) ? Direction::RIGHT : Direction::LEFT;
      m_sprite->set_action(m_dir == Direction::LEFT ? "working-left" : "working-right");
    }
  }
  m_dispense_timer.start(m_cycle, true);
  launch_badguy();
}

void
Dispenser::deactivate()
{
  m_dispense_timer.stop();
}

//TODO: Add launching velocity to certain badguys
bool
Dispenser::collision_squished(GameObject& object)
{
  //Cannon launching MrRocket can be broken by jumping on it
  //other dispensers are not that fragile.
  if (m_broken || m_type != DispenserType::ROCKETLAUNCHER) {
    return false;
  }

  if (m_frozen) {
    unfreeze();
  }

  m_sprite->set_action(m_dir == Direction::LEFT ? "broken-left" : "broken-right");
  m_dispense_timer.start(0);
  set_colgroup_active(COLGROUP_MOVING_STATIC); // Tux can stand on broken cannon.
  auto player = dynamic_cast<Player*>(&object);
  if (player){
    player->bounce(*this);
  }
  SoundManager::current()->play("sounds/squish.wav", get_pos());
  m_broken = true;
  return true;
}

HitResponse
Dispenser::collision(GameObject& other, const CollisionHit& hit)
{
  auto player = dynamic_cast<Player*> (&other);
  if (player) {
    // hit from above?
    if (player->get_bbox().get_bottom() < (m_col.m_bbox.get_top() + 16)) {
      collision_squished(*player);
      return FORCE_MOVE;
    }
    if (m_frozen && m_type != DispenserType::CANNON){
      unfreeze();
    }
    return FORCE_MOVE;
  }

  auto bullet = dynamic_cast<Bullet*> (&other);
  if (bullet){
    return collision_bullet(*bullet, hit);
  }

  return FORCE_MOVE;
}

void
Dispenser::active_update(float dt_sec)
{
  if (m_gravity)
  {
    BadGuy::active_update(dt_sec);
  }
  if (m_dispense_timer.check()) {
    // auto always shoots in Tux's direction
    if (m_autotarget) {
      if ( m_sprite->animation_done()) {
        m_sprite->set_action(m_dir == Direction::LEFT ? "working-left" : "working-right");
        m_swivel = false;
      }

      auto player = get_nearest_player();
      if (player && !m_swivel){
        Direction targetdir = (player->get_pos().x > get_pos().x) ? Direction::RIGHT : Direction::LEFT;
        if ( m_dir != targetdir ){ // no target: swivel cannon
          m_swivel = true;
          m_dir = targetdir;
          m_sprite->set_action(m_dir == Direction::LEFT ? "swivel-left" : "swivel-right", 1);
        } else { // tux in sight: shoot
          launch_badguy();
        }
      }
    } else {
      launch_badguy();
    }
  }
}

void
Dispenser::launch_badguy()
{
  if (m_badguys.empty()) return;
  if (m_frozen) return;
  if (m_limit_dispensed_badguys &&
      m_current_badguys >= m_max_concurrent_badguys)
      return;

  //FIXME: Does is_offscreen() work right here?
  if (!is_offscreen() && !Editor::is_active()) {
    Direction launchdir = m_dir;
    if ( !m_autotarget && m_start_dir == Direction::AUTO ){
      Player* player = get_nearest_player();
      if ( player ){
        launchdir = (player->get_pos().x > get_pos().x) ? Direction::RIGHT : Direction::LEFT;
      }
    }

    if (m_badguys.size() > 1) {
      if (m_random) {
        m_next_badguy = static_cast<unsigned int>(gameRandom.rand(static_cast<int>(m_badguys.size())));
      }
      else {
        m_next_badguy++;

        if (m_next_badguy >= m_badguys.size())
          m_next_badguy = 0;
      }
    }

    std::string badguy = m_badguys[m_next_badguy];

    if (badguy == "random") {
      log_warning << "random is outdated; use a list of badguys to select from." << std::endl;
      return;
    }
    if (badguy == "goldbomb") {
      log_warning << "goldbomb is not allowed to be dispensed" << std::endl;
      return;
    }

    try {
      /* Need to allocate the badguy first to figure out its bounding box. */
      auto game_object = GameObjectFactory::instance().create(badguy, get_pos(), launchdir);
      if (game_object == nullptr)
        throw std::runtime_error("Creating " + badguy + " object failed.");

      auto& bad_guy = dynamic_cast<BadGuy&>(*game_object);

      Rectf object_bbox = bad_guy.get_bbox();

      Vector spawnpoint(0.0f, 0.0f);
      switch (m_type)
      {
        case DispenserType::DROPPER:
          if (m_flip == NO_FLIP) {
            spawnpoint = get_anchor_pos (m_col.m_bbox, ANCHOR_BOTTOM);
            spawnpoint.x -= 0.5f * object_bbox.get_width();
          }
          else {
            spawnpoint = get_anchor_pos (m_col.m_bbox, ANCHOR_TOP);
            spawnpoint.y -= m_col.m_bbox.get_height();
            spawnpoint.x -= 0.5f * object_bbox.get_width();
          }
          break;

        case DispenserType::ROCKETLAUNCHER:
        case DispenserType::CANNON:
          spawnpoint = get_pos(); /* top-left corner of the cannon */
          if (launchdir == Direction::LEFT)
            spawnpoint.x -= object_bbox.get_width() + 1;
          else
            spawnpoint.x += m_col.m_bbox.get_width() + 1;
          break;

        case DispenserType::POINT:
          spawnpoint = m_col.m_bbox.p1();
          break;

        default:
          break;
      }

      /* Now we set the real spawn position */
      bad_guy.set_pos(spawnpoint);

      /* We don't want to count dispensed badguys in level stats */
      bad_guy.m_countMe = false;

      /* Set reference to dispenser in badguy itself */
      if (m_limit_dispensed_badguys)
      {
        bad_guy.set_parent_dispenser(this);
        m_current_badguys++;
      }

      Sector::get().add_object(std::move(game_object));
    } catch(const std::exception& e) {
      log_warning << "Error dispensing badguy: " << e.what() << std::endl;
      return;
    }
  }
}

void
Dispenser::freeze()
{
  if (m_broken) {
    return;
  }

  set_group(COLGROUP_MOVING_STATIC);
  m_frozen = true;

    if (m_type == DispenserType::ROCKETLAUNCHER && m_sprite->has_action("iced-left"))
    // Only swivel dispensers can use their left/right iced actions.
    m_sprite->set_action(m_dir == Direction::LEFT ? "iced-left" : "iced-right", 1);
    // when the sprite doesn't have separate actions for left and right or isn't a rocketlauncher,
    // it tries to use an universal one.
  else
  {
    if (m_type == DispenserType::CANNON && m_sprite->has_action("iced"))
      m_sprite->set_action("iced", 1);
      // When is the dispenser a cannon, it uses the "iced" action.
    else
    {
      if (m_sprite->has_action("dropper-iced"))
        m_sprite->set_action("dropper-iced", 1);
        // When is the dispenser a dropper, it uses the "dropper-iced".
      else
      {
        m_sprite->set_color(Color(0.6f, 0.72f, 0.88f));
        m_sprite->stop_animation();
        // When is the dispenser something else (unprobable), or has no matching iced sprite, it shades to blue.
      }
    }
  }
  m_dispense_timer.stop();
}

void
Dispenser::unfreeze()
{
  /*set_group(colgroup_active);
  frozen = false;

  sprite->set_color(Color(1.00, 1.00, 1.00f));*/
  BadGuy::unfreeze();

  set_correct_action();
  activate();
}

bool
Dispenser::is_freezable() const
{
  return true;
}

bool
Dispenser::is_flammable() const
{
  return false;
}

void
Dispenser::set_correct_action()
{
  switch (m_type) {
    case DispenserType::DROPPER:
      m_sprite->set_action("dropper");
      break;
    case DispenserType::ROCKETLAUNCHER:
      m_sprite->set_action(m_dir == Direction::LEFT ? "working-left" : "working-right");
      break;
    case DispenserType::CANNON:
      m_sprite->set_action("working");
      break;
    case DispenserType::POINT:
      m_sprite->set_action("invisible");
      break;
    default:
      break;
  }
}

ObjectSettings
Dispenser::get_settings()
{
  ObjectSettings result = BadGuy::get_settings();

  result.add_float(_("Interval (seconds)"), &m_cycle, "cycle");
  result.add_bool(_("Random"), &m_random, "random", false);
  result.add_badguy(_("Enemies"), &m_badguys, "badguy");
  result.add_bool(_("Limit dispensed badguys"), &m_limit_dispensed_badguys,
                  "limit-dispensed-badguys", false);
  result.add_bool(_("Obey Gravity"), &m_gravity,
                  "gravity", false);
  result.add_int(_("Max concurrent badguys"), &m_max_concurrent_badguys,
                 "max-concurrent-badguys", 0);
  result.add_enum(_("Type"), reinterpret_cast<int*>(&m_type),
                  {_("dropper"), _("rocket launcher"), _("cannon"), _("invisible")},
                  {"dropper", "rocketlauncher", "cannon", "point"},
                  static_cast<int>(DispenserType::DROPPER), "type");

  result.reorder({"cycle", "random", "type", "badguy", "direction", "gravity", "limit-dispensed-badguys", "max-concurrent-badguys", "x", "y"});

  return result;
}

void
Dispenser::after_editor_set()
{
  BadGuy::after_editor_set();

  set_correct_action();
}

void
Dispenser::on_flip(float height)
{
  BadGuy::on_flip(height);
  if (!m_gravity)
    FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
