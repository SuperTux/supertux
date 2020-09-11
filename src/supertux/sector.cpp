//  SuperTux -  A Jump'n Run
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

#include "supertux/sector.hpp"

#include <physfs.h>
#include <algorithm>

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "collision/collision.hpp"
#include "collision/collision_system.hpp"
#include "editor/editor.hpp"
#include "math/aatriangle.hpp"
#include "math/rect.hpp"
#include "object/ambient_light.hpp"
#include "object/background.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "object/cutscene_info.hpp"
#include "object/display_effect.hpp"
#include "object/gradient.hpp"
#include "object/music_object.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "object/pulsing_light.hpp"
#include "object/smoke_cloud.hpp"
#include "object/spawnpoint.hpp"
#include "object/text_array_object.hpp"
#include "object/text_object.hpp"
#include "object/tilemap.hpp"
#include "object/vertical_stripes.hpp"
#include "physfs/ifile_stream.hpp"
#include "scripting/sector.hpp"
#include "squirrel/squirrel_environment.hpp"
#include "supertux/constants.hpp"
#include "supertux/debug.hpp"
#include "supertux/game_object_factory.hpp"
#include "supertux/game_session.hpp"
#include "supertux/level.hpp"
#include "supertux/player_status_hud.hpp"
#include "supertux/savegame.hpp"
#include "supertux/tile.hpp"
#include "util/file_system.hpp"
#include "util/writer.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

Sector* Sector::s_current = nullptr;

namespace {

PlayerStatus dummy_player_status;

} // namespace

Sector::Sector(Level& parent) :
  m_level(parent),
  m_name(),
  m_fully_constructed(false),
  m_init_script(),
  m_foremost_layer(),
  m_squirrel_environment(new SquirrelEnvironment(SquirrelVirtualMachine::current()->get_vm(), "sector")),
  m_collision_system(new CollisionSystem(*this)),
  m_gravity(10.0)
{
  Savegame* savegame = (Editor::current() && Editor::is_active()) ?
    Editor::current()->m_savegame.get() :
    GameSession::current() ? &GameSession::current()->get_savegame() : nullptr;
  PlayerStatus& player_status = savegame ? savegame->get_player_status() : dummy_player_status;

  if (savegame && !m_level.m_suppress_pause_menu && !savegame->is_title_screen()) {
    add<PlayerStatusHUD>(player_status);
  }
  add<Player>(player_status, "Tux");
  add<DisplayEffect>("Effect");
  add<TextObject>("Text");
  add<TextArrayObject>("TextArray");

  SoundManager::current()->preload("sounds/shoot.wav");
}

Sector::~Sector()
{
  try
  {
    deactivate();
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  clear_objects();
}

void
Sector::finish_construction(bool editable)
{
  flush_game_objects();

  if (!editable) {
    convert_tiles2gameobject();

    bool has_background = std::any_of(get_objects().begin(), get_objects().end(),
                                      [](const auto& obj) {
                                        return (dynamic_cast<Background*>(obj.get()) ||
                                                dynamic_cast<Gradient*>(obj.get()));
                                      });
    if (!has_background) {
      auto& gradient = add<Gradient>();
      gradient.set_gradient(Color(0.3f, 0.4f, 0.75f), Color(1.f, 1.f, 1.f));
    }
  }

  if (get_solid_tilemaps().empty()) {
    log_warning << "sector '" << get_name() << "' does not contain a solid tile layer." << std::endl;
  }

  if (!get_object_by_type<Camera>()) {
    log_warning << "sector '" << get_name() << "' does not contain a camera." << std::endl;
    add<Camera>("Camera");
  }

  if (!get_object_by_type<AmbientLight>()) {
    add<AmbientLight>(Color::WHITE);
  }

  if (!get_object_by_type<MusicObject>()) {
    add<MusicObject>();
  }

  if (!get_object_by_type<VerticalStripes>()) {
    add<VerticalStripes>();
  }

  flush_game_objects();

  m_foremost_layer = calculate_foremost_layer();

  process_resolve_requests();

  for (auto& object : get_objects()) {
    object->finish_construction();
  }

  flush_game_objects();

  auto cutscene_text = new CutsceneInfo(get_camera(), _("Press escape to skip"), m_level);
  add_object(std::unique_ptr<GameObject> (cutscene_text));

  m_fully_constructed = true;
}

Level&
Sector::get_level() const
{
  return m_level;
}

void
Sector::activate(const std::string& spawnpoint)
{
  SpawnPointMarker* sp = nullptr;
  for (auto& spawn_point : get_objects_by_type<SpawnPointMarker>()) {
    if (spawn_point.get_name() == spawnpoint) {
      sp = &spawn_point;
      break;
    }
  }

  if (!sp) {
    log_warning << "Spawnpoint '" << spawnpoint << "' not found." << std::endl;
    if (spawnpoint != "main") {
      activate("main");
    } else {
      activate(Vector(0, 0));
    }
  } else {
    activate(sp->get_pos());
  }
}

void
Sector::activate(const Vector& player_pos)
{
  BIND_SECTOR(*this);

  if (s_current != this) {
    if (s_current != nullptr)
      s_current->deactivate();
    s_current = this;

    m_squirrel_environment->expose_self();

    for (auto& object : get_objects()) {
      m_squirrel_environment->try_expose(*object);
    }
  }

  // The Sector object is called 'settings' as it is accessed as 'sector.settings'
  m_squirrel_environment->expose("settings", std::make_unique<scripting::Sector>(this));

  // two-player hack: move other players to main player's position
  // Maybe specify 2 spawnpoints in the level?
  for (auto player_ptr : get_objects_by_type_index(typeid(Player))) {
    Player& player = *static_cast<Player*>(player_ptr);
    // spawn smalltux below spawnpoint
    if (!player.is_big()) {
      player.move(player_pos + Vector(0,32));
    } else {
      player.move(player_pos);
    }

    // spawning tux in the ground would kill him
    if (!is_free_of_tiles(player.get_bbox())) {
      std::string current_level = "[" + Sector::get().get_level().m_filename + "] ";
      log_warning << current_level << "Tried spawning Tux in solid matter. Compensating." << std::endl;
      Vector npos = player.get_bbox().p1();
      npos.y-=32;
      player.move(npos);
    }
  }

  { //FIXME: This is a really dirty workaround for this strange camera jump
    Player& player = get_player();
    Camera& camera = get_camera();
    player.move(player.get_pos()+Vector(-32, 0));
    camera.reset(player.get_pos());
    camera.update(1);
    player.move(player.get_pos()+(Vector(32, 0)));
    camera.update(1);
  }

  flush_game_objects();

  //Run default.nut just before init script
  //Check to see if it's in a levelset (info file)
  std::string basedir = FileSystem::dirname(get_level().m_filename);
  if (PHYSFS_exists((basedir + "/info").c_str())) {
    try {
      IFileStream in(basedir + "/default.nut");
      m_squirrel_environment->run_script(in, "default.nut");
    } catch(std::exception& ) {
      // doesn't exist or erroneous; do nothing
    }
  }

  // Run init script
  if (!m_init_script.empty() && !Editor::is_active()) {
    run_script(m_init_script, "init-script");
  }
}

void
Sector::deactivate()
{
  BIND_SECTOR(*this);

  if (s_current != this)
    return;

  m_squirrel_environment->unexpose_self();

  for (const auto& object: get_objects()) {
    m_squirrel_environment->try_unexpose(*object);
  }

  m_squirrel_environment->unexpose("settings");

  s_current = nullptr;
}

Rectf
Sector::get_active_region() const
{
  Camera& camera = get_camera();
  return Rectf(
    camera.get_translation() - Vector(1600, 1200),
    camera.get_translation() + Vector(1600, 1200) + Vector(static_cast<float>(SCREEN_WIDTH),
                                                           static_cast<float>(SCREEN_HEIGHT)));
}

int
Sector::calculate_foremost_layer() const
{
  int layer = LAYER_BACKGROUND0;
  for (auto& tm : get_objects_by_type<TileMap>())
  {
    if (tm.get_layer() > layer)
    {
      if ( (tm.get_alpha() < 1.0f) )
      {
        layer = tm.get_layer() - 1;
      }
      else
      {
        layer = tm.get_layer() + 1;
      }
    }
  }
  log_debug << "Calculated baduy falling layer was: " << layer << std::endl;
  return layer;
}

int
Sector::get_foremost_layer() const
{
  return m_foremost_layer;
}

void
Sector::update(float dt_sec)
{
  assert(m_fully_constructed);

  BIND_SECTOR(*this);

  m_squirrel_environment->update(dt_sec);

  GameObjectManager::update(dt_sec);

  /* Handle all possible collisions. */
  m_collision_system->update();
  flush_game_objects();
}

bool
Sector::before_object_add(GameObject& object)
{
  if (object.is_singleton())
  {
    const auto& objects = get_objects_by_type_index(std::type_index(typeid(object)));
    if (!objects.empty())
    {
      log_warning << "Can't insert multiple GameObject of type '" << typeid(object).name() << "', ignoring" << std::endl;
      return false;
    }
  }

  auto movingobject = dynamic_cast<MovingObject*>(&object);
  if (movingobject)
  {
    m_collision_system->add(movingobject->get_collision_object());
  }

  if (s_current == this) {
    m_squirrel_environment->try_expose(object);
  }

  if (m_fully_constructed) {
    // if the sector is already fully constructed, finish the object
    // constructions, as there should be no more named references to resolve
    object.finish_construction();
  }

  return true;
}

void
Sector::before_object_remove(GameObject& object)
{
  auto moving_object = dynamic_cast<MovingObject*>(&object);
  if (moving_object) {
    m_collision_system->remove(moving_object->get_collision_object());
  }

  if (s_current == this)
    m_squirrel_environment->try_unexpose(object);
}

void
Sector::draw(DrawingContext& context)
{
  BIND_SECTOR(*this);

  Camera& camera = get_camera();

  context.push_transform();
  context.set_translation(camera.get_translation());

  GameObjectManager::draw(context);

  if (g_debug.show_collision_rects) {
    m_collision_system->draw(context);
  }

  context.pop_transform();
}

bool
Sector::is_free_of_tiles(const Rectf& rect, const bool ignoreUnisolid) const
{
  return m_collision_system->is_free_of_tiles(rect, ignoreUnisolid);
}

bool
Sector::is_free_of_statics(const Rectf& rect, const MovingObject* ignore_object, const bool ignoreUnisolid) const
{
  return m_collision_system->is_free_of_statics(rect,
                                                ignore_object ? ignore_object->get_collision_object() : nullptr,
                                                ignoreUnisolid);
}

bool
Sector::is_free_of_movingstatics(const Rectf& rect, const MovingObject* ignore_object) const
{
  return m_collision_system->is_free_of_movingstatics(rect,
                                                      ignore_object ? ignore_object->get_collision_object() : nullptr);
}

bool
Sector::free_line_of_sight(const Vector& line_start, const Vector& line_end, const MovingObject* ignore_object) const
{
  return m_collision_system->free_line_of_sight(line_start, line_end,
                                                ignore_object ? ignore_object->get_collision_object() : nullptr);
}

bool
Sector::can_see_player(const Vector& eye) const
{
  for (auto player_ptr : get_objects_by_type_index(typeid(Player))) {
    Player& player = *static_cast<Player*>(player_ptr);
    // test for free line of sight to any of all four corners and the middle of the player's bounding box
    if (free_line_of_sight(eye, player.get_bbox().p1(), &player)) return true;
    if (free_line_of_sight(eye, Vector(player.get_bbox().get_right(), player.get_bbox().get_top()), &player)) return true;
    if (free_line_of_sight(eye, player.get_bbox().p2(), &player)) return true;
    if (free_line_of_sight(eye, Vector(player.get_bbox().get_left(), player.get_bbox().get_bottom()), &player)) return true;
    if (free_line_of_sight(eye, player.get_bbox().get_middle(), &player)) return true;
  }
  return false;
}

bool
Sector::inside(const Rectf& rect) const
{
  for (const auto& solids : get_solid_tilemaps()) {
    Rectf bbox = solids->get_bbox();

    // the top of the sector extends to infinity
    if (bbox.get_left() <= rect.get_left() &&
        rect.get_right() <= bbox.get_right() &&
        rect.get_bottom() <= bbox.get_bottom()) {
      return true;
    }
  }
  return false;
}

Size
Sector::get_editor_size() const
{
  // Find the solid tilemap with the greatest surface
  size_t max_surface = 0;
  Size size;
  for (const auto& solids: get_solid_tilemaps()) {
    size_t surface = solids->get_width() * solids->get_height();
    if (surface > max_surface) {
      max_surface = surface;
      size = solids->get_size();
    }
  }

  return size;
}

void
Sector::resize_sector(const Size& old_size, const Size& new_size, const Size& resize_offset)
{
  bool is_offset = resize_offset.width || resize_offset.height;
  Vector obj_shift = Vector(static_cast<float>(resize_offset.width) * 32.0f,
                            static_cast<float>(resize_offset.height) * 32.0f);
  for (const auto& object : get_objects()) {
    auto tilemap = dynamic_cast<TileMap*>(object.get());
    if (tilemap) {
      if (tilemap->get_size() == old_size) {
        tilemap->resize(new_size, resize_offset);
      } else if (is_offset) {
        tilemap->move_by(obj_shift);
      }
    } else if (is_offset) {
      auto moving_object = dynamic_cast<MovingObject*>(object.get());
      if (moving_object) {
        moving_object->move_to(moving_object->get_pos() + obj_shift);
      }
    }
  }
}

void
Sector::change_solid_tiles(uint32_t old_tile_id, uint32_t new_tile_id)
{
  for (auto& solids: get_solid_tilemaps()) {
    solids->change_all(old_tile_id, new_tile_id);
  }
}

void
Sector::set_gravity(float gravity)
{
  if (gravity != 10.0f)
  {
    log_warning << "Changing a Sector's gravitational constant might have unforeseen side-effects: " << gravity << std::endl;
  }

  m_gravity = gravity;
}

float
Sector::get_gravity() const
{
  return m_gravity;
}

Player*
Sector::get_nearest_player (const Vector& pos) const
{
  Player *nearest_player = nullptr;
  float nearest_dist = std::numeric_limits<float>::max();

  for (auto player_ptr : get_objects_by_type_index(typeid(Player)))
  {
    Player& player = *static_cast<Player*>(player_ptr);
    if (player.is_dying() || player.is_dead())
      continue;

    float dist = player.get_bbox ().distance(pos);

    if (dist < nearest_dist) {
      nearest_player = &player;
      nearest_dist = dist;
    }
  }

  return nearest_player;
}

std::vector<MovingObject*>
Sector::get_nearby_objects(const Vector& center, float max_distance) const
{
  std::vector<MovingObject*> result;
  for (auto& object : m_collision_system->get_nearby_objects(center, max_distance))
  {
    auto* moving_object = dynamic_cast<MovingObject*>(&object->get_listener());
    if (moving_object) {
      result.push_back(moving_object);
    }
  }
  return result;
}

void
Sector::stop_looping_sounds()
{
  for (auto& object : get_objects()) {
    object->stop_looping_sounds();
  }
}

void Sector::play_looping_sounds()
{
  for (const auto& object : get_objects()) {
    object->play_looping_sounds();
  }
}

void
Sector::save(Writer &writer)
{
  BIND_SECTOR(*this);

  writer.start_list("sector", false);

  writer.write("name", m_name, false);

  if (!m_level.is_worldmap()) {
    if (m_gravity != 10.0f) {
      writer.write("gravity", m_gravity);
    }
  }

  if (m_init_script.size()) {
    writer.write("init-script", m_init_script,false);
  }

  // saving objects;
  std::vector<GameObject*> objects(get_objects().size());
  std::transform(get_objects().begin(), get_objects().end(), objects.begin(), [] (auto& obj) {
    return obj.get();
  });

  std::stable_sort(objects.begin(), objects.end(),
                   [](const GameObject* lhs, GameObject* rhs) {
                     return lhs->get_class() < rhs->get_class();
                   });

  for (auto& obj : objects) {
    if (obj->is_saveable()) {
      writer.start_list(obj->get_class());
      obj->save(writer);
      writer.end_list(obj->get_class());
    }
  }

  writer.end_list("sector");
}

void
Sector::convert_tiles2gameobject()
{
  // add lights for special tiles
  for (auto& tm : get_objects_by_type<TileMap>())
  {
    for (int x=0; x < tm.get_width(); ++x)
    {
      for (int y=0; y < tm.get_height(); ++y)
      {
        const Tile& tile = tm.get_tile(x, y);

        if (!tile.get_object_name().empty())
        {
          // If a tile is associated with an object, insert that
          // object and remove the tile
          if (tile.get_object_name() == "decal" ||
              tm.is_solid())
          {
            Vector pos = tm.get_tile_position(x, y);
            try {
              auto object = GameObjectFactory::instance().create(tile.get_object_name(), pos, Direction::AUTO, tile.get_object_data());
              add_object(std::move(object));
              tm.change(x, y, 0);
            } catch(std::exception& e) {
              log_warning << e.what() << "" << std::endl;
            }
          }
        }
        else
        {
          // add lights for fire tiles
          uint32_t attributes = tile.get_attributes();
          Vector pos = tm.get_tile_position(x, y);
          Vector center = pos + Vector(16, 16);

          if (attributes & Tile::FIRE) {
            if (attributes & Tile::HURTS) {
              // lava or lavaflow
              // space lights a bit
              if ((tm.get_tile(x-1, y).get_attributes() != attributes || x%3 == 0)
                  && (tm.get_tile(x, y-1).get_attributes() != attributes || y%3 == 0)) {
                float pseudo_rnd = static_cast<float>(static_cast<int>(pos.x) % 10) / 10;
                add<PulsingLight>(center, 1.0f + pseudo_rnd, 0.8f, 1.0f,
                                  Color(1.0f, 0.3f, 0.0f, 1.0f));
              }
            } else {
              // torch
              float pseudo_rnd = static_cast<float>(static_cast<int>(pos.x) % 10) / 10;
              add<PulsingLight>(center, 1.0f + pseudo_rnd, 0.9f, 1.0f,
                                Color(1.0f, 1.0f, 0.6f, 1.0f));
            }
          }
        }
      }
    }
  }
}

void
Sector::run_script(const std::string& script, const std::string& sourcename)
{
  m_squirrel_environment->run_script(script, sourcename);
}

Camera&
Sector::get_camera() const
{
  return get_singleton_by_type<Camera>();
}

Player&
Sector::get_player() const
{
  return *static_cast<Player*>(get_objects_by_type_index(typeid(Player)).at(0));
}

DisplayEffect&
Sector::get_effect() const
{
  return get_singleton_by_type<DisplayEffect>();
}

/* EOF */
