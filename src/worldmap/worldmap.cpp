//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmail.com>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "worldmap/worldmap.hpp"

#include <physfs.h>

#include "audio/sound_manager.hpp"
#include "control/input_manager.hpp"
#include "gui/menu_manager.hpp"
#include "object/decal.hpp"
#include "object/tilemap.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "physfs/physfs_file_system.hpp"
#include "sprite/sprite.hpp"
#include "squirrel/squirrel_environment.hpp"
#include "supertux/debug.hpp"
#include "supertux/fadein.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/resources.hpp"
#include "supertux/savegame.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/shrinkfade.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "util/file_system.hpp"
#include "util/reader.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "video/compositor.hpp"
#include "video/video_system.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"
#include "worldmap/level_tile.hpp"
#include "worldmap/spawn_point.hpp"
#include "worldmap/special_tile.hpp"
#include "worldmap/sprite_change.hpp"
#include "worldmap/teleporter.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/worldmap_parser.hpp"
#include "worldmap/worldmap_screen.hpp"
#include "worldmap/worldmap_state.hpp"

static const float CAMERA_PAN_SPEED = 5.0;

namespace worldmap {

WorldMap::WorldMap(const std::string& filename, Savegame& savegame, const std::string& force_spawnpoint_) :
  m_squirrel_environment(new SquirrelEnvironment(SquirrelVirtualMachine::current()->get_vm(), "worldmap")),
  m_tux(),
  m_savegame(savegame),
  m_tileset(nullptr),
  m_camera_offset(),
  m_name("<no title>"),
  m_music("music/salcon.ogg"),
  m_init_script(),
  m_passive_message_timer(),
  m_passive_message(),
  m_map_filename(),
  m_levels_path(),
  m_special_tiles(),
  m_levels(),
  m_sprite_changes(),
  m_spawn_points(),
  m_teleporters(),
  m_ambient_light( 1.0f, 1.0f, 1.0f, 1.0f ),
  m_force_spawnpoint(force_spawnpoint_),
  m_main_is_default(true),
  m_initial_fade_tilemap(),
  m_fade_direction(),
  m_in_level(false),
  m_pan_pos(),
  m_panning(false)
{
  m_tux = add<Tux>(this);

  SoundManager::current()->preload("sounds/warp.wav");

  // load worldmap objects
  WorldMapParser parser(*this);
  parser.load_worldmap(filename);
}

WorldMap::~WorldMap()
{
  clear_objects();
  m_spawn_points.clear();
}

bool
WorldMap::before_object_add(GameObject& object)
{
  m_squirrel_environment->try_expose(object);
  return true;
}

void
WorldMap::before_object_remove(GameObject& object)
{
  m_squirrel_environment->try_unexpose(object);
}

void
WorldMap::move_to_spawnpoint(const std::string& spawnpoint, bool pan, bool main_as_default)
{
  auto sp = get_spawnpoint_by_name(spawnpoint);
  if(sp != nullptr) {
    Vector p = sp->pos;
    m_tux->set_tile_pos(p);
    m_tux->set_direction(sp->auto_dir);
    if(pan) {
      m_panning = true;
      m_pan_pos = get_camera_pos_for_tux();
      clamp_camera_position(m_pan_pos);
    }
    return;
  }

  log_warning << "Spawnpoint '" << spawnpoint << "' not found." << std::endl;
  if (spawnpoint != "main" && main_as_default) {
    move_to_spawnpoint("main");
  }
}

void
WorldMap::change(const std::string& filename, const std::string& force_spawnpoint_)
{
  m_savegame.get_player_status().last_worldmap = filename;
  ScreenManager::current()->pop_screen();
  ScreenManager::current()->push_screen(std::make_unique<WorldMapScreen>(
                                          std::make_unique<WorldMap>(filename, m_savegame, force_spawnpoint_)));
}

void
WorldMap::on_escape_press()
{
  // Show or hide the menu
  if(!MenuManager::instance().is_active()) {
    MenuManager::instance().set_menu(MenuStorage::WORLDMAP_MENU);
    m_tux->set_direction(D_NONE);  // stop tux movement when menu is called
  } else {
    MenuManager::instance().clear_menu_stack();
  }
}

Vector
WorldMap::get_next_tile(const Vector& pos, const Direction& direction) const
{
  auto position = pos;
  switch(direction) {
    case D_WEST:
      position.x -= 1;
      break;
    case D_EAST:
      position.x += 1;
      break;
    case D_NORTH:
      position.y -= 1;
      break;
    case D_SOUTH:
      position.y += 1;
      break;
    case D_NONE:
      break;
  }
  return position;
}

bool
WorldMap::path_ok(const Direction& direction, const Vector& old_pos, Vector* new_pos) const
{
  *new_pos = get_next_tile(old_pos, direction);

  if (!(new_pos->x >= 0 && new_pos->x < get_tiles_width()
        && new_pos->y >= 0 && new_pos->y < get_tiles_height()))
  { // New position is outsite the tilemap
    return false;
  }
  else
  { // Check if the tile allows us to go to new_pos
    int old_tile_data = tile_data_at(old_pos);
    int new_tile_data = tile_data_at(*new_pos);
    switch(direction)
    {
      case D_WEST:
        return (old_tile_data & Tile::WORLDMAP_WEST
                && new_tile_data & Tile::WORLDMAP_EAST);

      case D_EAST:
        return (old_tile_data & Tile::WORLDMAP_EAST
                && new_tile_data & Tile::WORLDMAP_WEST);

      case D_NORTH:
        return (old_tile_data & Tile::WORLDMAP_NORTH
                && new_tile_data & Tile::WORLDMAP_SOUTH);

      case D_SOUTH:
        return (old_tile_data & Tile::WORLDMAP_SOUTH
                && new_tile_data & Tile::WORLDMAP_NORTH);

      case D_NONE:
        log_warning << "path_ok() can't walk if direction is NONE" << std::endl;
        assert(false);
    }
    return false;
  }
}

void
WorldMap::finished_level(Level* gamelevel)
{
  // TODO use Level* parameter here?
  auto level = at_level();

  if(level == nullptr) {
    return;
  }

  bool old_level_state = level->solved;
  level->solved = true;
  level->sprite->set_action("solved");

  // deal with statistics
  level->statistics.update(gamelevel->m_stats);

  if(level->statistics.completed(level->statistics, level->target_time)) {
    level->perfect = true;
    if(level->sprite->has_action("perfect"))
      level->sprite->set_action("perfect");
  }

  save_state();

  if (old_level_state != level->solved) {
    // Try to detect the next direction to which we should walk
    // FIXME: Mostly a hack
    Direction dir = D_NONE;

    int dirdata = available_directions_at(m_tux->get_tile_pos());
    // first, test for crossroads
    if (dirdata == Tile::WORLDMAP_CNSE ||
        dirdata == Tile::WORLDMAP_CNSW ||
        dirdata == Tile::WORLDMAP_CNEW ||
        dirdata == Tile::WORLDMAP_CSEW ||
        dirdata == Tile::WORLDMAP_CNSEW)
      dir = D_NONE;
    else if (dirdata & Tile::WORLDMAP_NORTH
             && m_tux->m_back_direction != D_NORTH)
      dir = D_NORTH;
    else if (dirdata & Tile::WORLDMAP_SOUTH
             && m_tux->m_back_direction != D_SOUTH)
      dir = D_SOUTH;
    else if (dirdata & Tile::WORLDMAP_EAST
             && m_tux->m_back_direction != D_EAST)
      dir = D_EAST;
    else if (dirdata & Tile::WORLDMAP_WEST
             && m_tux->m_back_direction != D_WEST)
      dir = D_WEST;

    if (dir != D_NONE) {
      m_tux->set_direction(dir);
    }
  }

  if (!level->extro_script.empty()) {
    try {
      run_script(level->extro_script, "worldmap:extro_script");
    } catch(std::exception& e) {
      log_warning << "Couldn't run level-extro-script: " << e.what() << std::endl;
    }
  }
}

Vector
WorldMap::get_camera_pos_for_tux() const {
  Vector camera_offset_;
  Vector tux_pos = m_tux->get_pos();
  camera_offset_.x = tux_pos.x - static_cast<float>(SCREEN_WIDTH) / 2.0f;
  camera_offset_.y = tux_pos.y - static_cast<float>(SCREEN_HEIGHT) / 2.0f;
  return camera_offset_;
}

void
WorldMap::clamp_camera_position(Vector& c) const
{
  if (c.x < 0)
    c.x = 0;
  if (c.y < 0)
    c.y = 0;

  if (c.x > get_width() - static_cast<float>(SCREEN_WIDTH))
    c.x = get_width() - static_cast<float>(SCREEN_WIDTH);
  if (c.y > get_height() - static_cast<float>(SCREEN_HEIGHT))
    c.y = get_height() - static_cast<float>(SCREEN_HEIGHT);

  if (get_width() < static_cast<float>(SCREEN_WIDTH))
    c.x = (get_width() - static_cast<float>(SCREEN_WIDTH)) / 2.0f;
  if (get_height() < static_cast<float>(SCREEN_HEIGHT))
    c.y = (get_height() - static_cast<float>(SCREEN_HEIGHT)) / 2.0f;
}

void
WorldMap::update(float delta)
{
  if (m_in_level) return;
  if (MenuManager::instance().is_active()) return;

  GameObjectManager::update(delta);

  Vector requested_pos;

  // position "camera"
  if(!m_panning) {
    m_camera_offset = get_camera_pos_for_tux();
  } else {
    Vector delta__ = m_pan_pos - m_camera_offset;
    float mag = delta__.norm();
    if(mag > CAMERA_PAN_SPEED) {
      delta__ *= CAMERA_PAN_SPEED/mag;
    }
    m_camera_offset += delta__;
    if(m_camera_offset == m_pan_pos) {
      m_panning = false;
    }
  }

  requested_pos = m_camera_offset;
  clamp_camera_position(m_camera_offset);

  if(m_panning) {
    if(requested_pos.x != m_camera_offset.x) {
      m_pan_pos.x = m_camera_offset.x;
    }
    if(requested_pos.y != m_camera_offset.y) {
      m_pan_pos.y = m_camera_offset.y;
    }
  }

  // handle input
  auto controller = InputManager::current()->get_controller();
  bool enter_level = false;
  if(controller->pressed(Controller::ACTION)
     || controller->pressed(Controller::JUMP)
     || controller->pressed(Controller::MENU_SELECT)) {
    /* some people define UP and JUMP on the same key... */
    if(!controller->pressed(Controller::UP))
      enter_level = true;
  }
  if(controller->pressed(Controller::START) ||
     controller->pressed(Controller::ESCAPE))
  {
    on_escape_press();
  }

  if(controller->pressed(Controller::CHEAT_MENU) &&
     g_config->developer_mode)
  {
    MenuManager::instance().set_menu(MenuStorage::WORLDMAP_CHEAT_MENU);
  }

  if(controller->pressed(Controller::DEBUG_MENU) &&
     g_config->developer_mode)
  {
    MenuManager::instance().set_menu(MenuStorage::DEBUG_MENU);
  }

  // check for teleporters
  auto teleporter = at_teleporter(m_tux->get_tile_pos());
  if (teleporter && (teleporter->automatic || (enter_level && (!m_tux->is_moving())))) {
    enter_level = false;
    if (!teleporter->worldmap.empty()) {
      change(teleporter->worldmap, teleporter->spawnpoint);
    } else {
      // TODO: an animation, camera scrolling or a fading would be a nice touch
      SoundManager::current()->play("sounds/warp.wav");
      m_tux->m_back_direction = D_NONE;
      move_to_spawnpoint(teleporter->spawnpoint, true);
    }
  }

  // check for auto-play levels
  auto level = at_level();
  if (level && (level->auto_play) && (!level->solved) && (!m_tux->is_moving())) {
    enter_level = true;
    // automatically mark these levels as solved in case player aborts
    level->solved = true;
  }

  if (enter_level && !m_tux->is_moving())
  {
    /* Check level action */
    auto level_ = at_level();
    if (!level_) {
      //Respawn if player on a tile with no level and nowhere to go.
      int tile_data = tile_data_at(m_tux->get_tile_pos());
      if(!( tile_data & ( Tile::WORLDMAP_NORTH |  Tile::WORLDMAP_SOUTH | Tile::WORLDMAP_WEST | Tile::WORLDMAP_EAST ))){
        log_warning << "Player at illegal position " << m_tux->get_tile_pos().x << ", " << m_tux->get_tile_pos().y << " respawning." << std::endl;
        move_to_spawnpoint("main");
        return;
      }
      log_warning << "No level to enter at: " << m_tux->get_tile_pos().x << ", " << m_tux->get_tile_pos().y << std::endl;
      return;
    }

    if (level_->pos == m_tux->get_tile_pos()) {
      try {
        Vector shrinkpos = Vector(level_->pos.x*32 + 16 - m_camera_offset.x,
                                  level_->pos.y*32 +  8 - m_camera_offset.y);
        std::string levelfile = m_levels_path + level_->get_name();

        // update state and savegame
        save_state();
        ScreenManager::current()->push_screen(std::make_unique<GameSession>(levelfile, m_savegame, &level_->statistics),
                                              std::make_unique<ShrinkFade>(shrinkpos, 1.0f));
        m_in_level = true;
      } catch(std::exception& e) {
        log_fatal << "Couldn't load level: " << e.what() << std::endl;
      }
    }
  }
  else
  {
    //      tux->set_direction(input_direction);
  }
}

int
WorldMap::tile_data_at(const Vector& p) const
{
  int dirs = 0;

  for(const auto& tilemap : get_solid_tilemaps()) {
    const Tile& tile = tilemap->get_tile(static_cast<int>(p.x), static_cast<int>(p.y));
    int dirdata = tile.get_data();
    dirs |= dirdata;
  }

  return dirs;
}

int
WorldMap::available_directions_at(const Vector& p) const
{
  return tile_data_at(p) & Tile::WORLDMAP_DIR_MASK;
}

LevelTile*
WorldMap::at_level() const
{
  for(const auto& level : m_levels) {
    if (level->pos == m_tux->get_tile_pos())
      return level;
  }

  return nullptr;
}

SpecialTile*
WorldMap::at_special_tile() const
{
  for(const auto& special_tile : m_special_tiles) {
    if (special_tile->pos == m_tux->get_tile_pos())
      return special_tile;
  }

  return nullptr;
}

SpriteChange*
WorldMap::at_sprite_change(const Vector& pos) const
{
  for(const auto& sprite_change : m_sprite_changes) {
    if(sprite_change->pos == pos)
      return sprite_change;
  }

  return nullptr;
}

Teleporter*
WorldMap::at_teleporter(const Vector& pos) const
{
  for(const auto& teleporter : m_teleporters) {
    if(teleporter->pos == pos) return teleporter;
  }

  return nullptr;
}

void
WorldMap::draw(DrawingContext& context)
{
  if (get_width() < static_cast<float>(context.get_width()) ||
      get_height() < static_cast<float>(context.get_height()))
  {
    context.color().draw_filled_rect(Vector(0, 0), Vector(static_cast<float>(context.get_width()),
                                                          static_cast<float>(context.get_height())),
                                     Color(0.0f, 0.0f, 0.0f, 1.0f), LAYER_BACKGROUND0);
  }

  context.set_ambient_color( m_ambient_light );
  context.push_transform();
  context.set_translation(m_camera_offset);

  GameObjectManager::draw(context);

  if (g_debug.show_worldmap_path)
  {
    for (int x = 0; x < static_cast<int>(get_tiles_width()); x++) {
      for (int y = 0; y < static_cast<int>(get_tiles_height()); y++) {
        const int data = tile_data_at(Vector(static_cast<float>(x), static_cast<float>(y)));
        const int px = x * 32;
        const int py = y * 32;
        const int W = 4;
        const int layer = LAYER_FOREGROUND1 + 1000;
        const Color color(1.0f, 0.0f, 1.0f, 0.5f);
        if (data & Tile::WORLDMAP_NORTH)    context.color().draw_filled_rect(Rect(px + 16-W, py       , px + 16+W, py + 16-W), color, layer);
        if (data & Tile::WORLDMAP_SOUTH)    context.color().draw_filled_rect(Rect(px + 16-W, py + 16+W, px + 16+W, py + 32  ), color, layer);
        if (data & Tile::WORLDMAP_EAST)     context.color().draw_filled_rect(Rect(px + 16+W, py + 16-W, px + 32  , py + 16+W), color, layer);
        if (data & Tile::WORLDMAP_WEST)     context.color().draw_filled_rect(Rect(px       , py + 16-W, px + 16-W, py + 16+W), color, layer);
        if (data & Tile::WORLDMAP_DIR_MASK) context.color().draw_filled_rect(Rect(px + 16-W, py + 16-W, px + 16+W, py + 16+W), color, layer);
        if (data & Tile::WORLDMAP_STOP)     context.color().draw_filled_rect(Rect(px + 4   , py + 4   , px + 28  , py + 28  ), color, layer);
      }
    }
  }

  draw_status(context);
  context.pop_transform();
}

void
WorldMap::draw_status(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));

  m_savegame.get_player_status().draw(context);

  if (!m_tux->is_moving()) {
    for(const auto& level : m_levels) {
      if (level->pos == m_tux->get_tile_pos()) {
        context.color().draw_text(Resources::normal_font, level->title,
                                  Vector(static_cast<float>(context.get_width()) / 2.0f,
                                         static_cast<float>(context.get_height()) - Resources::normal_font->get_height() - 10),
                                  ALIGN_CENTER, LAYER_HUD, level->title_color);

        // if level is solved, draw level picture behind stats
        /*
          if (level->solved) {
          if (const Surface* picture = level->get_picture()) {
          Vector pos = Vector(context.get_width() - picture->get_width(), context.get_height() - picture->get_height());
          context.push_transform();
          context.set_alpha(0.5);
          context.color().draw_surface(picture, pos, LAYER_FOREGROUND1-1);
          context.pop_transform();
          }
          }
        */
        level->statistics.draw_worldmap_info(context, level->target_time);
        break;
      }
    }

    for(const auto& special_tile : m_special_tiles) {
      if (special_tile->pos == m_tux->get_tile_pos()) {
        /* Display an in-map message in the map, if any as been selected */
        if(!special_tile->map_message.empty() && !special_tile->passive_message)
          context.color().draw_text(Resources::normal_font, special_tile->map_message,
                                    Vector(static_cast<float>(context.get_width()) / 2.0f,
                                           static_cast<float>(context.get_height()) - static_cast<float>(Resources::normal_font->get_height()) - 60.0f),
                                    ALIGN_CENTER, LAYER_FOREGROUND1, WorldMap::message_color);
        break;
      }
    }

    // display teleporter messages
    auto teleporter = at_teleporter(m_tux->get_tile_pos());
    if (teleporter && (!teleporter->message.empty())) {
      Vector pos = Vector(static_cast<float>(context.get_width()) / 2.0f,
                          static_cast<float>(context.get_height()) - Resources::normal_font->get_height() - 30.0f);
      context.color().draw_text(Resources::normal_font, teleporter->message, pos, ALIGN_CENTER, LAYER_FOREGROUND1, WorldMap::teleporter_message_color);
    }
  }

  /* Display a passive message in the map, if needed */
  if(m_passive_message_timer.started())
    context.color().draw_text(Resources::normal_font, m_passive_message,
                              Vector(static_cast<float>(context.get_width()) / 2.0f,
                                     static_cast<float>(context.get_height()) - Resources::normal_font->get_height() - 60.0f),
                              ALIGN_CENTER, LAYER_FOREGROUND1, WorldMap::message_color);

  context.pop_transform();
}

void
WorldMap::setup()
{
  SoundManager::current()->play_music(m_music);
  MenuManager::instance().clear_menu_stack();
  ScreenManager::current()->set_screen_fade(std::make_unique<FadeIn>(1));

  load_state();

  // if force_spawnpoint was set, move Tux there, then clear force_spawnpoint
  if (!m_force_spawnpoint.empty()) {
    move_to_spawnpoint(m_force_spawnpoint, false, m_main_is_default);
    m_force_spawnpoint = "";
    m_main_is_default = true;
  }

  // If we specified a fade tilemap, let's fade it:
  if (!m_initial_fade_tilemap.empty())
  {
    auto tilemap = get_object_by_name<TileMap>(m_initial_fade_tilemap);
    if(tilemap != nullptr)
    {
      if(m_fade_direction == 0)
      {
        tilemap->fade(1.0, 1);
      }
      else
      {
        tilemap->fade(0.0, 1);
      }
    }
    m_initial_fade_tilemap = "";
  }

  m_tux->setup();

  // register worldmap_table as worldmap in scripting
  m_squirrel_environment->expose_self();

  //Run default.nut just before init script
  try {
    IFileStreambuf ins(m_levels_path + "default.nut");
    std::istream in(&ins);
    m_squirrel_environment->run_script(in, "WorldMap::default.nut");
  } catch(std::exception& ) {
    // doesn't exist or erroneous; do nothing
  }

  if(!m_init_script.empty()) {
    m_squirrel_environment->run_script(m_init_script, "WorldMap::init");
  }
  m_tux->process_special_tile( at_special_tile() );
}

void
WorldMap::leave()
{
  // save state of world and player
  save_state();

  // remove worldmap_table from roottable
  m_squirrel_environment->unexpose_self();

  GameManager::current()->load_next_worldmap();
}

void
WorldMap::set_levels_solved(bool solved, bool perfect)
{
  for(auto& level : m_levels)
  {
    level->set_solved(solved);
    level->set_perfect(perfect);
  }
}

size_t
WorldMap::level_count() const
{
  return m_levels.size();
}

size_t
WorldMap::solved_level_count() const
{
  size_t count = 0;
  for(const auto& level : m_levels) {
    if(level->solved)
      count++;
  }

  return count;
}

void
WorldMap::load_state()
{
  WorldMapState state(*this);
  state.load_state();
}

void
WorldMap::save_state()
{
  WorldMapState state(*this);
  state.save_state();
}

void
WorldMap::run_script(const std::string& script, const std::string& sourcename)
{
  m_squirrel_environment->run_script(script, sourcename);
}

} // namespace worldmap

/* EOF */
