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
#include "editor/editor.hpp"
#include "math/aatriangle.hpp"
#include "math/rect.hpp"
#include "object/ambient_light.hpp"
#include "object/background.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
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
#include "supertux/colorscheme.hpp"
#include "supertux/constants.hpp"
#include "supertux/debug.hpp"
#include "supertux/game_object_factory.hpp"
#include "supertux/level.hpp"
#include "supertux/player_status_hud.hpp"
#include "supertux/resources.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "util/file_system.hpp"
#include "util/writer.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

Sector* Sector::s_current = nullptr;

Sector::Sector(Level& parent) :
  Base::Sector("sector"),
  m_level(parent),
  m_fully_constructed(false),
  m_foremost_layer(),
  m_foremost_opaque_layer(),
  m_gravity(10.0f),
  m_collision_system(new CollisionSystem(*this)),
  m_text_object(add<TextObject>("Text"))
{
  add<DisplayEffect>("Effect");
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

  // FIXME: Is it a good idea to process some resolve requests this early?
  // I added this to fix https://github.com/SuperTux/supertux/issues/1378
  // but I don't know if it's going to introduce other bugs..   ~ Semphris
  try_process_resolve_requests();

  if (!editable) {
    convert_tiles2gameobject();

    if (!m_level.is_worldmap())
    {
      bool has_background = std::any_of(get_objects().begin(), get_objects().end(),
                                        [](const auto& obj) {
                                          return (dynamic_cast<Background*>(obj.get()) ||
                                                  dynamic_cast<Gradient*>(obj.get()));
                                        });
      if (!has_background)
      {
        auto& gradient = add<Gradient>();
        gradient.set_gradient(Color(0.3f, 0.4f, 0.75f), Color(1.f, 1.f, 1.f));
      }
    }
  }

  if (get_solid_tilemaps().empty())
  {
    if (editable)
    {
      log_warning << "sector '" << get_name() << "' does not contain a solid tile layer." << std::endl;
    }
    else
    {
      log_warning << "sector '" << get_name() << "' does not contain a solid tile layer. Creating an empty one." << std::endl;

      TileMap& tilemap = add<TileMap>(TileManager::current()->get_tileset(m_level.get_tileset()));
      tilemap.resize(100, 35);
      tilemap.set_solid();
    }
  }

  if (!get_object_by_type<Camera>()) {
    if (!m_level.is_worldmap())
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

  m_initialized = false;
  flush_game_objects();

  m_foremost_layer = calculate_foremost_layer(false);
  m_foremost_opaque_layer = calculate_foremost_layer();

  process_resolve_requests();

  Base::Sector::finish_construction(editable);

  m_initialized = false;
  flush_game_objects();

  m_fully_constructed = true;
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
    if (!m_level.is_worldmap())
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

  // Make sure all players are moved to this sector.
  for (auto& player : m_level.get_players())
    player->move_to_sector(*this);

  if (s_current != this) {
    if (s_current != nullptr)
      s_current->deactivate();
    s_current = this;

    m_squirrel_environment->expose_self();

    for (const auto& object : get_objects()) {
      m_squirrel_environment->try_expose(*object);
    }
  }

  // The Sector object is called 'settings' as it is accessed as 'sector.settings'
  m_squirrel_environment->expose("settings", std::make_unique<scripting::Sector>(this));

  if (Editor::is_active())
    return;

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

  //FIXME: This is a really dirty workaround for this strange camera jump
  if (get_players().size() > 0)
  {
    Player& player = *(get_players()[0]);
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
Sector::calculate_foremost_layer(bool including_transparent) const
{
  int layer = LAYER_BACKGROUND0;
  for (auto& tm : get_objects_by_type<TileMap>())
  {
    if (tm.get_layer() > layer)
    {
      if ( including_transparent && tm.get_alpha() < 1.0f )
      {
        layer = tm.get_layer() - 1;
      }
      else
      {
        layer = tm.get_layer() + 1;
      }
    }
  }
  log_debug << "Calculated badguy falling layer was: " << layer << std::endl;
  return layer;
}

int
Sector::get_foremost_opaque_layer() const
{
  return m_foremost_opaque_layer;
}

int
Sector::get_foremost_layer() const
{
  return m_foremost_layer;
}

TileSet*
Sector::get_tileset() const
{
  return TileManager::current()->get_tileset(m_level.get_tileset());
}

bool
Sector::in_worldmap() const
{
  return m_level.is_worldmap();
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

  if (auto* movingobject = dynamic_cast<MovingObject*>(&object))
  {
    m_collision_system->add(movingobject->get_collision_object());
  }

  if (auto* tilemap = dynamic_cast<TileMap*>(&object))
  {
    tilemap->set_ground_movement_manager(m_collision_system->get_ground_movement_manager());
  }

  if (s_current == this) {
    m_squirrel_environment->try_expose(object);
  }

  if (m_fully_constructed) {
    try_process_resolve_requests();
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

#if 0
  context.push_transform();

  Rect original_clip = context.get_viewport();
  context.push_transform();
  {
    Camera& camera = get_camera();
    context.set_translation(camera.get_translation());
    context.scale(camera.get_current_scale());

    get_singleton_by_type<PlayerStatusHUD>().set_target_player(0);

    Rect clip = original_clip;
    clip.left = (clip.left + clip.right) / 2 + 16;
    context.set_viewport(clip);

    GameObjectManager::draw(context);

    if (g_debug.show_collision_rects) {
      m_collision_system->draw(context);
    }
  }
  context.set_viewport(original_clip);
  context.pop_transform();

  context.push_transform();
  {
    Camera& camera = get_camera();
    context.set_translation(camera.get_translation());
    context.scale(camera.get_current_scale());

    get_singleton_by_type<PlayerStatusHUD>().set_target_player(1);

    Rect clip = original_clip;
    clip.right = (clip.left + clip.right) / 2 - 16;
    context.set_viewport(clip);

    GameObjectManager::draw(context);

    if (g_debug.show_collision_rects) {
      m_collision_system->draw(context);
    }
  }
  context.set_viewport(original_clip);
  context.pop_transform();

  context.pop_transform();

  Rect midline = original_clip;
  midline.right = (original_clip.left + original_clip.right) / 2 - 16;
  midline.left = (original_clip.left + original_clip.right) / 2 + 16;
  context.color().draw_filled_rect(midline, Color::BLACK, 99999);
#else
  context.push_transform();

  Camera& camera = get_camera();
  context.set_translation(camera.get_translation());
  context.scale(camera.get_current_scale());

  GameObjectManager::draw(context);

  if (g_debug.show_collision_rects) {
    m_collision_system->draw(context);
  }

  context.pop_transform();
#endif

  if (m_level.m_is_in_cutscene && !m_level.m_skip_cutscene)
  {
    context.color().draw_text(Resources::normal_font,
                              _("Press escape to skip"),
                              Vector(32.f, 32.f),
                              ALIGN_LEFT,
                              LAYER_OBJECTS + 1000,
                              ColorScheme::Text::heading_color);
  }
}

bool
Sector::is_free_of_tiles(const Rectf& rect, const bool ignoreUnisolid, uint32_t tiletype) const
{
  return m_collision_system->is_free_of_tiles(rect, ignoreUnisolid, tiletype);
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
Sector::is_free_of_specifically_movingstatics(const Rectf& rect, const MovingObject* ignore_object) const
{
  return m_collision_system->is_free_of_specifically_movingstatics(rect,
                                                      ignore_object ? ignore_object->get_collision_object() : nullptr);
}

CollisionSystem::RaycastResult
Sector::get_first_line_intersection(const Vector& line_start,
                                    const Vector& line_end,
                                    bool ignore_objects,
                                    const CollisionObject* ignore_object) const {
  return m_collision_system->get_first_line_intersection(line_start, line_end, ignore_objects, ignore_object);
}

bool
Sector::free_line_of_sight(const Vector& line_start, const Vector& line_end, bool ignore_objects, const MovingObject* ignore_object) const
{
  return m_collision_system->free_line_of_sight(line_start, line_end, ignore_objects,
                                                ignore_object ? ignore_object->get_collision_object() : nullptr);
}

bool
Sector::can_see_player(const Vector& eye) const
{
  for (auto player_ptr : get_objects_by_type_index(typeid(Player))) {
    Player& player = *static_cast<Player*>(player_ptr);
    // test for free line of sight to any of all four corners and the middle of the player's bounding box
    if (free_line_of_sight(eye, player.get_bbox().p1(), false, &player)) return true;
    if (free_line_of_sight(eye, Vector(player.get_bbox().get_right(), player.get_bbox().get_top()), false, &player)) return true;
    if (free_line_of_sight(eye, player.get_bbox().p2(), false, &player)) return true;
    if (free_line_of_sight(eye, Vector(player.get_bbox().get_left(), player.get_bbox().get_bottom()), false, &player)) return true;
    if (free_line_of_sight(eye, player.get_bbox().get_middle(), false, &player)) return true;
  }
  return false;
}

bool
Sector::inside(const Rectf& rect) const
{
  for (const auto& tilemap : get_all_tilemaps()) {
    Rectf bbox = tilemap->get_bbox();

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
  // Find the tilemap with the greatest surface
  size_t max_surface = 0;
  Size size;
  for (const auto& tilemap : get_all_tilemaps()) {
    size_t surface = tilemap->get_width() * tilemap->get_height();
    if (surface > max_surface) {
      max_surface = surface;
      size = tilemap->get_size();
    }
  }

  return size;
}

void
Sector::resize_sector(const Size& old_size, const Size& new_size, const Size& resize_offset)
{
  BIND_SECTOR(*this);

  bool is_offset = resize_offset.width || resize_offset.height;
  Vector obj_shift = Vector(static_cast<float>(resize_offset.width) * 32.0f,
                            static_cast<float>(resize_offset.height) * 32.0f);

  for (const auto& object : get_objects())
  {
    auto tilemap = dynamic_cast<TileMap*>(object.get());
    if (tilemap)
    {
      if (tilemap->get_size() == old_size)
      {
        tilemap->save_state();
        tilemap->resize(new_size, resize_offset);
        tilemap->check_state();
      }
      else if (is_offset)
      {
        tilemap->save_state();
        tilemap->move_by(obj_shift);
        tilemap->check_state();
      }
    }
    else if (is_offset)
    {
      auto moving_object = dynamic_cast<MovingObject*>(object.get());
      if (moving_object)
      {
        moving_object->save_state();
        moving_object->move_to(moving_object->get_pos() + obj_shift);
        moving_object->check_state();
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
                     return lhs->get_class_name() < rhs->get_class_name();
                   });

  for (auto& obj : objects) {
    if (obj->is_saveable()) {
      writer.start_list(obj->get_class_name());
      obj->save(writer);
      writer.end_list(obj->get_class_name());
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
    // Since object setup is not yet complete, I have to manually add the offset
    // See https://github.com/SuperTux/supertux/issues/1378 for details
    Vector tm_offset = tm.get_path() ? tm.get_path()->get_base() : Vector(0, 0);

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
            Vector pos = tm.get_tile_position(x, y) + tm_offset;
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
          Vector pos = tm.get_tile_position(x, y) + tm_offset;
          Vector center = pos + Vector(16, 16);

          if (attributes & Tile::FIRE) {
            if (attributes & Tile::HURTS) {
              // lava or lavaflow
              // space lights a bit
              if ((tm.get_tile(x-1, y).get_attributes() != attributes || x%3 == 0)
                  && (tm.get_tile(x, y-1).get_attributes() != attributes || y%3 == 0)) {
                float pseudo_rnd = static_cast<float>(static_cast<int>(pos.x) % 10) / 10;
                add<PulsingLight>(center, 1.0f + pseudo_rnd, 0.8f, 1.0f,
                                  (Color(1.0f, 0.3f, 0.0f, 1.0f) * tm.get_current_tint()).validate());
              }
            } else {
              // torch
              float pseudo_rnd = static_cast<float>(static_cast<int>(pos.x) % 10) / 10;
              add<PulsingLight>(center, 1.0f + pseudo_rnd, 0.9f, 1.0f,
                                (Color(1.0f, 1.0f, 0.6f, 1.0f) * tm.get_current_tint()).validate());
            }
          }
        }
      }
    }
  }
}

Camera&
Sector::get_camera() const
{
  return get_singleton_by_type<Camera>();
}

std::vector<Player*>
Sector::get_players() const
{
  return m_level.get_players();
}

DisplayEffect&
Sector::get_effect() const
{
  return get_singleton_by_type<DisplayEffect>();
}

/* EOF */
