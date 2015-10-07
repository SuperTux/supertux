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

#include <config.h>

#include <assert.h>
#include <fstream>
#include <iostream>
#include <physfs.h>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "audio/sound_manager.hpp"
#include "control/input_manager.hpp"
#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "lisp/parser.hpp"
#include "object/background.hpp"
#include "object/decal.hpp"
#include "object/tilemap.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "scripting/scripting.hpp"
#include "scripting/squirrel_error.hpp"
#include "scripting/squirrel_util.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/menu/worldmap_menu.hpp"
#include "supertux/player_status.hpp"
#include "supertux/resources.hpp"
#include "supertux/savegame.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/fadein.hpp"
#include "supertux/shrinkfade.hpp"
#include "supertux/spawn_point.hpp"
#include "supertux/textscroller.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/tile_set.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "worldmap/level.hpp"
#include "worldmap/special_tile.hpp"
#include "worldmap/sprite_change.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/worldmap.hpp"

static const float CAMERA_PAN_SPEED = 5.0;

namespace worldmap {

WorldMap* WorldMap::current_ = NULL;

WorldMap::WorldMap(const std::string& filename, Savegame& savegame, const std::string& force_spawnpoint_) :
  tux(),
  m_savegame(savegame),
  tileset(NULL),
  free_tileset(false),
  camera_offset(),
  name(),
  music(),
  init_script(),
  game_objects(),
  solid_tilemaps(),
  passive_message_timer(),
  passive_message(),
  map_filename(),
  levels_path(),
  special_tiles(),
  levels(),
  sprite_changes(),
  spawn_points(),
  teleporters(),
  total_stats(),
  worldmap_table(),
  scripts(),
  ambient_light( 1.0f, 1.0f, 1.0f, 1.0f ),
  force_spawnpoint(force_spawnpoint_),
  in_level(false),
  pan_pos(),
  panning(false)
{
  tux = std::make_shared<Tux>(this);
  add_object(tux);

  name = "<no title>";
  music = "music/salcon.ogg";

  total_stats.reset();

  // create a new squirrel table for the worldmap
  using namespace scripting;

  sq_collectgarbage(global_vm);
  sq_newtable(global_vm);
  sq_pushroottable(global_vm);
  if(SQ_FAILED(sq_setdelegate(global_vm, -2)))
    throw scripting::SquirrelError(global_vm, "Couldn't set worldmap_table delegate");

  sq_resetobject(&worldmap_table);
  if(SQ_FAILED(sq_getstackobj(global_vm, -1, &worldmap_table)))
    throw scripting::SquirrelError(global_vm, "Couldn't get table from stack");

  sq_addref(global_vm, &worldmap_table);
  sq_pop(global_vm, 1);

  SoundManager::current()->preload("sounds/warp.wav");

  // load worldmap objects
  load(filename);
}

WorldMap::~WorldMap()
{
  using namespace scripting;

  if(free_tileset)
    delete tileset;

  for(GameObjects::iterator i = game_objects.begin();
      i != game_objects.end(); ++i) {
    GameObjectPtr& object = *i;
    try_unexpose(object);
  }

  for(SpawnPoints::iterator i = spawn_points.begin();
      i != spawn_points.end(); ++i) {
    delete *i;
  }

  for(ScriptList::iterator i = scripts.begin();
      i != scripts.end(); ++i) {
    HSQOBJECT& object = *i;
    sq_release(global_vm, &object);
  }
  sq_release(global_vm, &worldmap_table);

  sq_collectgarbage(global_vm);

  if(current_ == this)
    current_ = NULL;
}

void
WorldMap::add_object(GameObjectPtr object)
{
  TileMap* tilemap = dynamic_cast<TileMap*>(object.get());
  if(tilemap != 0 && tilemap->is_solid()) {
    solid_tilemaps.push_back(tilemap);
  }

  try_expose(object);
  game_objects.push_back(object);
}

void
WorldMap::try_expose(const GameObjectPtr& object)
{
  ScriptInterface* object_ = dynamic_cast<ScriptInterface*>(object.get());
  if(object_ != NULL) {
    HSQUIRRELVM vm = scripting::global_vm;
    sq_pushobject(vm, worldmap_table);
    object_->expose(vm, -1);
    sq_pop(vm, 1);
  }
}

void
WorldMap::try_unexpose(const GameObjectPtr& object)
{
  ScriptInterface* object_ = dynamic_cast<ScriptInterface*>(object.get());
  if(object_ != NULL) {
    HSQUIRRELVM vm = scripting::global_vm;
    SQInteger oldtop = sq_gettop(vm);
    sq_pushobject(vm, worldmap_table);
    try {
      object_->unexpose(vm, -1);
    } catch(std::exception& e) {
      log_warning << "Couldn't unregister object: " << e.what() << std::endl;
    }
    sq_settop(vm, oldtop);
  }
}

void
WorldMap::move_to_spawnpoint(const std::string& spawnpoint, bool pan)
{
  for(SpawnPoints::iterator i = spawn_points.begin(); i != spawn_points.end(); ++i) {
    SpawnPoint* sp = *i;
    if(sp->name == spawnpoint) {
      Vector p = sp->pos;
      tux->set_tile_pos(p);
      tux->set_direction(sp->auto_dir);
      if(pan) {
        panning = true;
        pan_pos = get_camera_pos_for_tux();
        clamp_camera_position(pan_pos);
      }
      return;
    }
  }
  log_warning << "Spawnpoint '" << spawnpoint << "' not found." << std::endl;
  if (spawnpoint != "main") {
    move_to_spawnpoint("main");
  }
}

void
WorldMap::change(const std::string& filename, const std::string& force_spawnpoint_)
{
  ScreenManager::current()->pop_screen();
  ScreenManager::current()->push_screen(std::unique_ptr<Screen>(new WorldMap(filename, m_savegame, force_spawnpoint_)));
}

void
WorldMap::load(const std::string& filename)
{
  map_filename = filename;
  levels_path = FileSystem::dirname(map_filename);

  try {
    lisp::Parser parser;
    const lisp::Lisp* root = parser.parse(map_filename);

    const lisp::Lisp* level_ = root->get_lisp("supertux-level");
    if(level_ == NULL)
      throw std::runtime_error("file isn't a supertux-level file.");

    level_->get("name", name);

    const lisp::Lisp* sector = level_->get_lisp("sector");
    if(!sector)
      throw std::runtime_error("No sector specified in worldmap file.");

    const lisp::Lisp* tilesets_lisp = level_->get_lisp("tilesets");
    if(tilesets_lisp != NULL) {
      tileset      = TileManager::current()->parse_tileset_definition(*tilesets_lisp).release();
      free_tileset = true;
    }
    std::string tileset_name;
    if(level_->get("tileset", tileset_name)) {
      if(tileset != NULL) {
        log_warning << "multiple tilesets specified in level_" << std::endl;
      } else {
        tileset = TileManager::current()->get_tileset(tileset_name);
      }
    }
    /* load default tileset */
    if(tileset == NULL) {
      tileset = TileManager::current()->get_tileset("images/worldmap.strf");
    }
    current_tileset = tileset;

    lisp::ListIterator iter(sector);
    while(iter.next()) {
      if(iter.item() == "tilemap") {
        add_object(std::make_shared<TileMap>(*(iter.lisp())));
      } else if(iter.item() == "background") {
        add_object(std::make_shared<Background>(*(iter.lisp())));
      } else if(iter.item() == "music") {
        iter.value()->get(music);
      } else if(iter.item() == "init-script") {
        iter.value()->get(init_script);
      } else if(iter.item() == "worldmap-spawnpoint") {
        SpawnPoint* sp = new SpawnPoint(*iter.lisp());
        spawn_points.push_back(sp);
      } else if(iter.item() == "level") {
        auto level = std::make_shared<LevelTile>(levels_path, *iter.lisp());
        load_level_information(*level.get());
        levels.push_back(level.get());
        add_object(level);
      } else if(iter.item() == "special-tile") {
        auto special_tile = std::make_shared<SpecialTile>(*iter.lisp());
        special_tiles.push_back(special_tile.get());
        add_object(special_tile);
      } else if(iter.item() == "sprite-change") {
        auto sprite_change = std::make_shared<SpriteChange>(*iter.lisp());
        sprite_changes.push_back(sprite_change.get());
        add_object(sprite_change);
      } else if(iter.item() == "teleporter") {
        auto teleporter = std::make_shared<Teleporter>(*iter.lisp());
        teleporters.push_back(teleporter.get());
        add_object(teleporter);
      } else if(iter.item() == "decal") {
        auto decal = std::make_shared<Decal>(*iter.lisp());
        add_object(decal);
      } else if(iter.item() == "ambient-light") {
        std::vector<float> vColor;
        sector->get( "ambient-light", vColor );
        if(vColor.size() < 3) {
          log_warning << "(ambient-light) requires a color as argument" << std::endl;
        } else {
          ambient_light = Color( vColor );
        }
      } else if(iter.item() == "name") {
        // skip
      } else {
        log_warning << "Unknown token '" << iter.item() << "' in worldmap" << std::endl;
      }
    }
    current_tileset = NULL;

    if(solid_tilemaps.empty())
      throw std::runtime_error("No solid tilemap specified");

    move_to_spawnpoint("main");

  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when parsing worldmap '" << map_filename << "': " <<
      e.what();
    throw std::runtime_error(msg.str());
  }
}

void
WorldMap::load_level_information(LevelTile& level)
{
  /** get special_tile's title */
  level.title = _("<no title>");
  level.target_time = 0.0f;

  try {
    lisp::Parser parser;
    const lisp::Lisp* root = parser.parse(levels_path + level.get_name());

    const lisp::Lisp* level_lisp = root->get_lisp("supertux-level");
    if(!level_lisp)
      return;

    level_lisp->get("name", level.title);
    level_lisp->get("target-time", level.target_time);

  } catch(std::exception& e) {
    log_warning << "Problem when reading level information: " << e.what() << std::endl;
    return;
  }
}

void WorldMap::calculate_total_stats()
{
  total_stats.zero();
  for(LevelTiles::iterator i = levels.begin(); i != levels.end(); ++i) {
    LevelTile* level = *i;
    if (level->solved) {
      total_stats += level->statistics;
    }
  }
}

void
WorldMap::on_escape_press()
{
  // Show or hide the menu
  if(!MenuManager::instance().is_active()) {
    MenuManager::instance().set_menu(MenuStorage::WORLDMAP_MENU);
    tux->set_direction(D_NONE);  // stop tux movement when menu is called
  } else {
    MenuManager::instance().clear_menu_stack();
  }
}

Vector
WorldMap::get_next_tile(Vector pos, Direction direction) const
{
  switch(direction) {
    case D_WEST:
      pos.x -= 1;
      break;
    case D_EAST:
      pos.x += 1;
      break;
    case D_NORTH:
      pos.y -= 1;
      break;
    case D_SOUTH:
      pos.y += 1;
      break;
    case D_NONE:
      break;
  }
  return pos;
}

bool
WorldMap::path_ok(Direction direction, const Vector& old_pos, Vector* new_pos) const
{
  *new_pos = get_next_tile(old_pos, direction);

  if (!(new_pos->x >= 0 && new_pos->x < get_width()
        && new_pos->y >= 0 && new_pos->y < get_height()))
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
        assert(!"path_ok() can't walk if direction is NONE");
    }
    return false;
  }
}

void
WorldMap::finished_level(Level* gamelevel)
{
  // TODO use Level* parameter here?
  LevelTile* level = at_level();

  if(level == NULL) {
    return;
  }

  bool old_level_state = level->solved;
  level->solved = true;
  level->sprite->set_action("solved");

  // deal with statistics
  level->statistics.merge(gamelevel->stats);
  calculate_total_stats();

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

    int dirdata = available_directions_at(tux->get_tile_pos());
    // first, test for crossroads
    if (dirdata == Tile::WORLDMAP_CNSE ||
        dirdata == Tile::WORLDMAP_CNSW ||
        dirdata == Tile::WORLDMAP_CNEW ||
        dirdata == Tile::WORLDMAP_CSEW ||
        dirdata == Tile::WORLDMAP_CNSEW)
      dir = D_NONE;
    else if (dirdata & Tile::WORLDMAP_NORTH
             && tux->back_direction != D_NORTH)
      dir = D_NORTH;
    else if (dirdata & Tile::WORLDMAP_SOUTH
             && tux->back_direction != D_SOUTH)
      dir = D_SOUTH;
    else if (dirdata & Tile::WORLDMAP_EAST
             && tux->back_direction != D_EAST)
      dir = D_EAST;
    else if (dirdata & Tile::WORLDMAP_WEST
             && tux->back_direction != D_WEST)
      dir = D_WEST;

    if (dir != D_NONE) {
      tux->set_direction(dir);
    }
  }

  if (level->extro_script != "") {
    try {
      std::istringstream in(level->extro_script);
      run_script(in, "worldmap:extro_script");
    } catch(std::exception& e) {
      log_warning << "Couldn't run level-extro-script: " << e.what() << std::endl;
    }
  }
}

Vector
WorldMap::get_camera_pos_for_tux() const {
  Vector camera_offset_;
  Vector tux_pos = tux->get_pos();
  camera_offset_.x = tux_pos.x - SCREEN_WIDTH/2;
  camera_offset_.y = tux_pos.y - SCREEN_HEIGHT/2;
  return camera_offset_;
}

void
WorldMap::clamp_camera_position(Vector& c) {
  if (c.x < 0)
    c.x = 0;
  if (c.y < 0)
    c.y = 0;

  if (c.x > (int)get_width()*32 - SCREEN_WIDTH)
    c.x = (int)get_width()*32 - SCREEN_WIDTH;
  if (c.y > (int)get_height()*32 - SCREEN_HEIGHT)
    c.y = (int)get_height()*32 - SCREEN_HEIGHT;

  if (int(get_width()*32) < SCREEN_WIDTH)
    c.x = get_width()*16.0 - SCREEN_WIDTH/2.0;
  if (int(get_height()*32) < SCREEN_HEIGHT)
    c.y = get_height()*16.0 - SCREEN_HEIGHT/2.0;
}

void
WorldMap::update(float delta)
{
  if (!in_level && !MenuManager::instance().is_active())
  {
    // update GameObjects
    for(size_t i = 0; i < game_objects.size(); ++i) {
      GameObjectPtr& object = game_objects[i];
      if(!panning || object != tux) {
        object->update(delta);
      }
    }

    // remove old GameObjects
    for(GameObjects::iterator i = game_objects.begin();
        i != game_objects.end(); ) {
      GameObjectPtr& object = *i;
      if(!object->is_valid()) {
        try_unexpose(object);
        i = game_objects.erase(i);
      } else {
        ++i;
      }
    }

    /* update solid_tilemaps list */
    //FIXME: this could be more efficient
    solid_tilemaps.clear();
    for(auto i = game_objects.begin(); i != game_objects.end(); ++i)
    {
      TileMap* tm = dynamic_cast<TileMap*>(i->get());
      if (!tm) continue;
      if (tm->is_solid()) solid_tilemaps.push_back(tm);
    }

    Vector requested_pos;

    // position "camera"
    if(!panning) {
      camera_offset = get_camera_pos_for_tux();
    } else {
      Vector delta__ = pan_pos - camera_offset;
      float mag = delta__.norm();
      if(mag > CAMERA_PAN_SPEED) {
        delta__ *= CAMERA_PAN_SPEED/mag;
      }
      camera_offset += delta__;
      if(camera_offset == pan_pos) {
        panning = false;
      }
    }

    requested_pos = camera_offset;
    clamp_camera_position(camera_offset);

    if(panning) {
      if(requested_pos.x != camera_offset.x) {
        pan_pos.x = camera_offset.x;
      }
      if(requested_pos.y != camera_offset.y) {
        pan_pos.y = camera_offset.y;
      }
    }

    // handle input
    Controller *controller = InputManager::current()->get_controller();
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

    // check for teleporters
    Teleporter* teleporter = at_teleporter(tux->get_tile_pos());
    if (teleporter && (teleporter->automatic || (enter_level && (!tux->is_moving())))) {
      enter_level = false;
      if (teleporter->worldmap != "") {
        change(teleporter->worldmap, teleporter->spawnpoint);
      } else {
        // TODO: an animation, camera scrolling or a fading would be a nice touch
        SoundManager::current()->play("sounds/warp.wav");
        tux->back_direction = D_NONE;
        move_to_spawnpoint(teleporter->spawnpoint, true);
      }
    }

    // check for auto-play levels
    LevelTile* level = at_level();
    if (level && (level->auto_play) && (!level->solved) && (!tux->is_moving())) {
      enter_level = true;
      // automatically mark these levels as solved in case player aborts
      level->solved = true;
    }

    if (enter_level && !tux->is_moving())
    {
      /* Check level action */
      LevelTile* level_ = at_level();
      if (!level_) {
        //Respawn if player on a tile with no level and nowhere to go.
        int tile_data = tile_data_at(tux->get_tile_pos());
        if(!( tile_data & ( Tile::WORLDMAP_NORTH |  Tile::WORLDMAP_SOUTH | Tile::WORLDMAP_WEST | Tile::WORLDMAP_EAST ))){
          log_warning << "Player at illegal position " << tux->get_tile_pos().x << ", " << tux->get_tile_pos().y << " respawning." << std::endl;
          move_to_spawnpoint("main");
          return;
        }
        log_warning << "No level to enter at: " << tux->get_tile_pos().x << ", " << tux->get_tile_pos().y << std::endl;
        return;
      }

      if (level_->pos == tux->get_tile_pos()) {
        try {
          Vector shrinkpos = Vector(level_->pos.x*32 + 16 - camera_offset.x,
                                    level_->pos.y*32 +  8 - camera_offset.y);
          std::string levelfile = levels_path + level_->get_name();

          // update state and savegame
          save_state();

          ScreenManager::current()->push_screen(std::unique_ptr<Screen>(new GameSession(levelfile, m_savegame, &level_->statistics)),
                                        std::unique_ptr<ScreenFade>(new ShrinkFade(shrinkpos, 1.0f)));
          in_level = true;
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
}

int
WorldMap::tile_data_at(Vector p) const
{
  int dirs = 0;

  for(std::list<TileMap*>::const_iterator i = solid_tilemaps.begin(); i != solid_tilemaps.end(); ++i) {
    TileMap* tilemap = *i;
    const Tile* tile = tilemap->get_tile((int)p.x, (int)p.y);
    int dirdata = tile->getData();
    dirs |= dirdata;
  }

  return dirs;
}

int
WorldMap::available_directions_at(Vector p) const
{
  return tile_data_at(p) & Tile::WORLDMAP_DIR_MASK;
}

LevelTile*
WorldMap::at_level() const
{
  for(LevelTiles::const_iterator i = levels.begin(); i != levels.end(); ++i) {
    LevelTile* level = *i;
    if (level->pos == tux->get_tile_pos())
      return level;
  }

  return NULL;
}

SpecialTile*
WorldMap::at_special_tile() const
{
  for(SpecialTiles::const_iterator i = special_tiles.begin();
      i != special_tiles.end(); ++i) {
    SpecialTile* special_tile = *i;
    if (special_tile->pos == tux->get_tile_pos())
      return special_tile;
  }

  return NULL;
}

SpriteChange*
WorldMap::at_sprite_change(const Vector& pos) const
{
  for(SpriteChanges::const_iterator i = sprite_changes.begin();
      i != sprite_changes.end(); ++i) {
    SpriteChange* sprite_change = *i;
    if(sprite_change->pos == pos)
      return sprite_change;
  }

  return NULL;
}

Teleporter*
WorldMap::at_teleporter(const Vector& pos) const
{
  for(std::vector<Teleporter*>::const_iterator i = teleporters.begin(); i != teleporters.end(); ++i) {
    Teleporter* teleporter = *i;
    if(teleporter->pos == pos) return teleporter;
  }

  return NULL;
}

void
WorldMap::draw(DrawingContext& context)
{
  if (int(get_width()*32) < SCREEN_WIDTH || int(get_height()*32) < SCREEN_HEIGHT)
    context.draw_filled_rect(Vector(0, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
                             Color(0.0f, 0.0f, 0.0f, 1.0f), LAYER_BACKGROUND0);

  context.set_ambient_color( ambient_light );
  context.push_transform();
  context.set_translation(camera_offset);

  for(auto i = game_objects.begin(); i != game_objects.end(); ++i)
  {
    GameObjectPtr& object = *i;
    if(!panning || object != tux) {
      object->draw(context);
    }
  }

  /*
  // FIXME: make this a runtime switch similar to draw_collrects/show_collrects?
  // draw visual indication of possible walk directions
  static int flipme = 0;
  if (flipme++ & 0x04)
  for (int x = 0; x < get_width(); x++) {
  for (int y = 0; y < get_height(); y++) {
  int data = tile_data_at(Vector(x,y));
  int px = x * 32;
  int py = y * 32;
  const int W = 4;
  if (data & Tile::WORLDMAP_NORTH)    context.draw_filled_rect(Rect(px + 16-W, py       , px + 16+W, py + 16-W), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  if (data & Tile::WORLDMAP_SOUTH)    context.draw_filled_rect(Rect(px + 16-W, py + 16+W, px + 16+W, py + 32  ), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  if (data & Tile::WORLDMAP_EAST)     context.draw_filled_rect(Rect(px + 16+W, py + 16-W, px + 32  , py + 16+W), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  if (data & Tile::WORLDMAP_WEST)     context.draw_filled_rect(Rect(px       , py + 16-W, px + 16-W, py + 16+W), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  if (data & Tile::WORLDMAP_DIR_MASK) context.draw_filled_rect(Rect(px + 16-W, py + 16-W, px + 16+W, py + 16+W), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  if (data & Tile::WORLDMAP_STOP)     context.draw_filled_rect(Rect(px + 4   , py + 4   , px + 28  , py + 28  ), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  }
  }
  */

  draw_status(context);
  context.pop_transform();
}

void
WorldMap::draw_status(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));

  m_savegame.get_player_status()->draw(context);

  if (!tux->is_moving()) {
    for(LevelTiles::iterator i = levels.begin(); i != levels.end(); ++i) {
      LevelTile* level = *i;

      if (level->pos == tux->get_tile_pos()) {
        context.draw_text(Resources::normal_font, level->title,
                          Vector(SCREEN_WIDTH/2,
                                 SCREEN_HEIGHT - Resources::normal_font->get_height() - 10),
                          ALIGN_CENTER, LAYER_HUD, WorldMap::level_title_color);

        // if level is solved, draw level picture behind stats
        /*
          if (level->solved) {
          if (const Surface* picture = level->get_picture()) {
          Vector pos = Vector(SCREEN_WIDTH - picture->get_width(), SCREEN_HEIGHT - picture->get_height());
          context.push_transform();
          context.set_alpha(0.5);
          context.draw_surface(picture, pos, LAYER_FOREGROUND1-1);
          context.pop_transform();
          }
          }
        */
        level->statistics.draw_worldmap_info(context, level->target_time);
        break;
      }
    }

    for(SpecialTiles::iterator i = special_tiles.begin();
        i != special_tiles.end(); ++i) {
      SpecialTile* special_tile = *i;

      if (special_tile->pos == tux->get_tile_pos()) {
        /* Display an in-map message in the map, if any as been selected */
        if(!special_tile->map_message.empty() && !special_tile->passive_message)
          context.draw_text(Resources::normal_font, special_tile->map_message,
                            Vector(SCREEN_WIDTH/2,
                                   SCREEN_HEIGHT - Resources::normal_font->get_height() - 60),
                            ALIGN_CENTER, LAYER_FOREGROUND1, WorldMap::message_color);
        break;
      }
    }

    // display teleporter messages
    Teleporter* teleporter = at_teleporter(tux->get_tile_pos());
    if (teleporter && (teleporter->message != "")) {
      Vector pos = Vector(SCREEN_WIDTH/2, SCREEN_HEIGHT - Resources::normal_font->get_height() - 30);
      context.draw_text(Resources::normal_font, teleporter->message, pos, ALIGN_CENTER, LAYER_FOREGROUND1, WorldMap::teleporter_message_color);
    }

  }

  /* Display a passive message in the map, if needed */
  if(passive_message_timer.started())
    context.draw_text(Resources::normal_font, passive_message,
                      Vector(SCREEN_WIDTH/2, SCREEN_HEIGHT - Resources::normal_font->get_height() - 60),
                      ALIGN_CENTER, LAYER_FOREGROUND1, WorldMap::message_color);

  context.pop_transform();
}

void
WorldMap::setup()
{
  SoundManager::current()->play_music(music);
  MenuManager::instance().clear_menu_stack();
  ScreenManager::current()->set_screen_fade(std::unique_ptr<ScreenFade>(new FadeIn(1)));

  current_ = this;
  load_state();

  // if force_spawnpoint was set, move Tux there, then clear force_spawnpoint
  if (force_spawnpoint != "") {
    move_to_spawnpoint(force_spawnpoint);
    force_spawnpoint = "";
  }

  tux->setup();

  // register worldmap_table as worldmap in scripting
  using namespace scripting;

  sq_pushroottable(global_vm);
  sq_pushstring(global_vm, "worldmap", -1);
  sq_pushobject(global_vm, worldmap_table);
  if(SQ_FAILED(sq_createslot(global_vm, -3)))
    throw SquirrelError(global_vm, "Couldn't set worldmap in roottable");
  sq_pop(global_vm, 1);

  //Run default.nut just before init script
  try {
    IFileStreambuf ins(levels_path + "default.nut");
    std::istream in(&ins);
    run_script(in, "WorldMap::default.nut");
  } catch(std::exception& ) {
    // doesn't exist or erroneous; do nothing
  }

  if(init_script != "") {
    std::istringstream in(init_script);
    run_script(in, "WorldMap::init");
  }
}

void
WorldMap::leave()
{
  using namespace scripting;

  // save state of world and player
  save_state();

  // remove worldmap_table from roottable
  sq_pushroottable(global_vm);
  sq_pushstring(global_vm, "worldmap", -1);
  if(SQ_FAILED(sq_deleteslot(global_vm, -2, SQFalse)))
    throw SquirrelError(global_vm, "Couldn't unset worldmap in roottable");
  sq_pop(global_vm, 1);
}

void
WorldMap::set_levels_solved(bool solved, bool perfect)
{
  for(auto& level : levels)
  {
    level->set_solved(solved);
    level->set_perfect(perfect);
  }
}

void
WorldMap::save_state()
{
  using namespace scripting;

  HSQUIRRELVM vm = global_vm;
  int oldtop = sq_gettop(vm);

  try {
    // get state table
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_or_create_table_entry(vm, "worlds");

    sq_pushstring(vm, map_filename.c_str(), map_filename.length());
    if(SQ_FAILED(sq_deleteslot(vm, -2, SQFalse)))
    {
    }

    // construct new table for this worldmap
    sq_pushstring(vm, map_filename.c_str(), map_filename.length());
    sq_newtable(vm);

    // store tux
    sq_pushstring(vm, "tux", -1);
    sq_newtable(vm);

    store_float(vm, "x", tux->get_tile_pos().x);
    store_float(vm, "y", tux->get_tile_pos().y);
    store_string(vm, "back", direction_to_string(tux->back_direction));

    if(SQ_FAILED(sq_createslot(vm, -3)))
    {
      throw std::runtime_error("failed to create '" + name + "' table entry");
    }

    // levels...
    sq_pushstring(vm, "levels", -1);
    sq_newtable(vm);

    for(LevelTiles::iterator i = levels.begin(); i != levels.end(); ++i) {
      LevelTile* level = *i;

      sq_pushstring(vm, level->get_name().c_str(), -1);
      sq_newtable(vm);

      store_bool(vm, "solved", level->solved);
      store_bool(vm, "perfect", level->perfect);
      level->statistics.serialize_to_squirrel(vm);

      if(SQ_FAILED(sq_createslot(vm, -3)))
      {
        throw std::runtime_error("failed to create '" + name + "' table entry");
      }
    }

    if(SQ_FAILED(sq_createslot(vm, -3)))
    {
      throw std::runtime_error("failed to create '" + name + "' table entry");
    }

    // overall statistics...
    total_stats.serialize_to_squirrel(vm);

    // push world into worlds table
    if(SQ_FAILED(sq_createslot(vm, -3)))
    {
      throw std::runtime_error("failed to create '" + name + "' table entry");
    }
  } catch(std::exception& ) {
    sq_settop(vm, oldtop);
  }

  sq_settop(vm, oldtop);

  m_savegame.save();
}

void
WorldMap::load_state()
{
  log_debug << "loading worldmap state" << std::endl;

  using namespace scripting;

  HSQUIRRELVM vm = global_vm;
  int oldtop = sq_gettop(vm);

  try {
    // get state table
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_table_entry(vm, "worlds");
    get_table_entry(vm, map_filename);

    // load tux
    get_table_entry(vm, "tux");
    Vector p;
    p.x = read_float(vm, "x");
    p.y = read_float(vm, "y");
    std::string back_str = read_string(vm, "back");
    tux->back_direction = string_to_direction(back_str);
    tux->set_tile_pos(p);

    int tile_data = tile_data_at(p);
    if(!( tile_data & ( Tile::WORLDMAP_NORTH | Tile::WORLDMAP_SOUTH | Tile::WORLDMAP_WEST | Tile::WORLDMAP_EAST ))) {
      log_warning << "Player at illegal position " << p.x << ", " << p.y << " respawning." << std::endl;
      move_to_spawnpoint("main");
    }

    sq_pop(vm, 1);

    // load levels
    get_table_entry(vm, "levels");
    for(LevelTiles::iterator i = levels.begin(); i != levels.end(); ++i) {
      LevelTile* level = *i;
      sq_pushstring(vm, level->get_name().c_str(), -1);
      if(SQ_SUCCEEDED(sq_get(vm, -2))) {
        level->solved = read_bool(vm, "solved");
        level->perfect = read_bool(vm, "perfect");
        level->update_sprite_action();
        level->statistics.unserialize_from_squirrel(vm);
        sq_pop(vm, 1);
      }
    }

    // leave levels table
    sq_pop(vm, 1);

    // load overall statistics
    total_stats.unserialize_from_squirrel(vm);

  } catch(std::exception& e) {
    log_debug << "Not loading worldmap state: " << e.what() << std::endl;
    save_state(); // make new initial save
  }
  sq_settop(vm, oldtop);

  in_level = false;
}

size_t
WorldMap::level_count() const
{
  return levels.size();
}

size_t
WorldMap::solved_level_count() const
{
  size_t count = 0;
  for(LevelTiles::const_iterator i = levels.begin(); i != levels.end(); ++i) {
    LevelTile* level = *i;

    if(level->solved)
      count++;
  }

  return count;
}

HSQUIRRELVM
WorldMap::run_script(std::istream& in, const std::string& sourcename)
{
  using namespace scripting;

  // garbage collect thread list
  for(ScriptList::iterator i = scripts.begin();
      i != scripts.end(); ) {
    HSQOBJECT& object = *i;
    HSQUIRRELVM vm = object_to_vm(object);

    if(sq_getvmstate(vm) != SQ_VMSTATE_SUSPENDED) {
      sq_release(global_vm, &object);
      i = scripts.erase(i);
      continue;
    }

    ++i;
  }

  HSQOBJECT object = create_thread(global_vm);
  scripts.push_back(object);

  HSQUIRRELVM vm = object_to_vm(object);

  // set worldmap_table as roottable for the thread
  sq_pushobject(vm, worldmap_table);
  sq_setroottable(vm);

  compile_and_run(vm, in, sourcename);

  return vm;
}

float
WorldMap::get_width() const
{
  float width = 0;
  for(std::list<TileMap*>::const_iterator i = solid_tilemaps.begin(); i != solid_tilemaps.end(); ++i) {
    TileMap* solids = *i;
    if (solids->get_width() > width) width = solids->get_width();
  }
  return width;
}

float
WorldMap::get_height() const
{
  float height = 0;
  for(std::list<TileMap*>::const_iterator i = solid_tilemaps.begin(); i != solid_tilemaps.end(); ++i) {
    TileMap* solids = *i;
    if (solids->get_height() > height) height = solids->get_height();
  }
  return height;
}

} // namespace worldmap

/* EOF */
