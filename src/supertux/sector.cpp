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

#include <algorithm>
#include <math.h>
#include <vector>

#include "scripting/scripting.hpp"
#include "scripting/squirrel_util.hpp"
#include "scripting/sector.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/jumpy.hpp"
#include "editor/editor.hpp"
#include "math/aatriangle.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "object/display_effect.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "object/smoke_cloud.hpp"
#include "object/text_object.hpp"
#include "object/tilemap.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "supertux/collision.hpp"
#include "supertux/constants.hpp"
#include "supertux/game_session.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/player_status.hpp"
#include "supertux/savegame.hpp"
#include "supertux/spawn_point.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "trigger/secretarea_trigger.hpp"
#include "trigger/sequence_trigger.hpp"
#include "util/file_system.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

Sector* Sector::_current = 0;

bool Sector::show_collrects = false;
bool Sector::draw_solids_only = false;

Sector::Sector(Level* parent) :
  level(parent),
  name(),
  bullets(),
  init_script(),
  gameobjects_new(),
  currentmusic(LEVEL_MUSIC),
  sector_table(),
  scripts(),
  ambient_light( 1.0f, 1.0f, 1.0f, 1.0f ),
  ambient_light_fading(false),
  source_ambient_light(1.0f, 1.0f, 1.0f, 1.0f),
  target_ambient_light(1.0f, 1.0f, 1.0f, 1.0f),
  ambient_light_fade_duration(0.0f),
  ambient_light_fade_accum(0.0f),
  foremost_layer(),
  gameobjects(),
  moving_objects(),
  spawnpoints(),
  portables(),
  music(),
  gravity(10.0),
  player(0),
  solid_tilemaps(),
  camera(0),
  effect(0),
  collision_detector(moving_objects, solid_tilemaps, get_players())
{
  PlayerStatus* player_status;
  if (Editor::is_active()) {
    player_status = Editor::current()->m_savegame->get_player_status();
  } else {
    player_status = GameSession::current()->get_savegame().get_player_status();
  }
  if (!player_status) {
    log_warning << "Player status is not initialized." << std::endl;
  }
  add_object(std::make_shared<Player>(player_status, "Tux"));
  add_object(std::make_shared<DisplayEffect>("Effect"));
  add_object(std::make_shared<TextObject>("Text"));
  
  collision_detector.set_players(get_players());

  SoundManager::current()->preload("sounds/shoot.wav");

  // create a new squirrel table for the sector
  using namespace scripting;

  sq_collectgarbage(global_vm);

  sq_newtable(global_vm);
  sq_pushroottable(global_vm);
  if(SQ_FAILED(sq_setdelegate(global_vm, -2)))
    throw scripting::SquirrelError(global_vm, "Couldn't set sector_table delegate");

  sq_resetobject(&sector_table);
  if(SQ_FAILED(sq_getstackobj(global_vm, -1, &sector_table)))
    throw scripting::SquirrelError(global_vm, "Couldn't get sector table");
  sq_addref(global_vm, &sector_table);
  sq_pop(global_vm, 1);
}

Sector::~Sector()
{
  using namespace scripting;
  try
  {
    deactivate();
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  release_scripts(global_vm, scripts, sector_table);

  update_game_objects();
  assert(gameobjects_new.size() == 0);

  for(const auto& object: gameobjects) {
    before_object_remove(object);
  }
}

Level*
Sector::get_level() const
{
  return level;
}

HSQUIRRELVM
Sector::run_script(const std::string& script, const std::string& sourcename)
{
  if(script.empty())
  {
    return NULL;
  }
  std::istringstream stream(script);
  return run_script(stream, sourcename);
}

HSQUIRRELVM
Sector::run_script(std::istream& in, const std::string& sourcename)
{
  try {
    return scripting::run_script(in, "Sector " + name + " - " + sourcename,
                                 scripts, &sector_table);
  }
  catch(const std::exception& e)
  {
    log_warning << "Error running sector script: " << e.what() << std::endl;
    return NULL;
  }
}

void
Sector::add_object(GameObjectPtr object)
{
  // make sure the object isn't already in the list
#ifndef NDEBUG
  for(const auto& game_object : gameobjects) {
    assert(game_object != object);
  }
  for(const auto& gameobject : gameobjects_new) {
    assert(gameobject != object);
  }
#endif

  gameobjects_new.push_back(object);
}

void
Sector::activate(const std::string& spawnpoint)
{
  std::shared_ptr<SpawnPoint> sp;
  for(const auto& spawn_point : spawnpoints) {
    if(spawn_point->name == spawnpoint) {
      sp = spawn_point;
      break;
    }
  }
  if(!sp) {
    log_warning << "Spawnpoint '" << spawnpoint << "' not found." << std::endl;
    if(spawnpoint != "main") {
      activate("main");
    } else {
      activate(Vector(0, 0));
    }
  } else {
    activate(sp->pos);
  }
}

void
Sector::activate(const Vector& player_pos)
{
  if(_current != this) {
    if(_current != NULL)
      _current->deactivate();
    _current = this;

    // register sectortable as sector in scripting
    HSQUIRRELVM vm = scripting::global_vm;
    sq_pushroottable(vm);
    scripting::store_object(vm, "sector", sector_table);
    sq_pop(vm, 1);

    for(auto& object : gameobjects) {
      try_expose(object);
    }
  }
  try_expose_me();


  // two-player hack: move other players to main player's position
  // Maybe specify 2 spawnpoints in the level?
  for(auto& object : gameobjects) {
    auto p = dynamic_cast<Player*>(object.get());
    if (!p) continue;

    // spawn smalltux below spawnpoint
    if (!p->is_big()) {
      p->move(player_pos + Vector(0,32));
    } else {
      p->move(player_pos);
    }

    // spawning tux in the ground would kill him
    if(!is_free_of_tiles(p->get_bbox())) {
      std::string current_level = "[" + Sector::current()->get_level()->filename + "] ";
      log_warning << current_level << "Tried spawning Tux in solid matter. Compensating." << std::endl;
      Vector npos = p->get_bbox().p1;
      npos.y-=32;
      p->move(npos);
    }
  }

  //FIXME: This is a really dirty workaround for this strange camera jump
  player->move(player->get_pos()+Vector(-32, 0));
  camera->reset(player->get_pos());
  camera->update(1);
  player->move(player->get_pos()+(Vector(32, 0)));
  camera->update(1);

  update_game_objects();

  //Run default.nut just before init script
  //Check to see if it's in a levelset (info file)
  std::string basedir = FileSystem::dirname(get_level()->filename);
  if(PHYSFS_exists((basedir + "/info").c_str())) {
    try {
      IFileStreambuf ins(basedir + "/default.nut");
      std::istream in(&ins);
      run_script(in, "default.nut");
    } catch(std::exception& ) {
      // doesn't exist or erroneous; do nothing
    }
  }

  // Run init script
  if(!init_script.empty() && !Editor::is_active()) {
    run_script(init_script, "init-script");
  }
}

void
Sector::deactivate()
{
  if(_current != this)
    return;

  // remove sector entry from global vm
  HSQUIRRELVM vm = scripting::global_vm;
  sq_pushroottable(vm);
  scripting::delete_table_entry(vm, "sector");
  sq_pop(vm, 1);

  for(const auto& object: gameobjects) {
    try_unexpose(object);
  }

  try_unexpose_me();
  _current = NULL;
}

Rectf
Sector::get_active_region() const
{
  return Rectf(
    camera->get_translation() - Vector(1600, 1200),
    camera->get_translation() + Vector(1600, 1200) + Vector(SCREEN_WIDTH,SCREEN_HEIGHT));
}

int
Sector::calculate_foremost_layer() const
{
  int layer = LAYER_BACKGROUND0;
  for(const auto& obj : gameobjects)
  {
    const auto& tm = dynamic_cast<TileMap*>(obj.get());
    if (!tm) continue;
    if(tm->get_layer() > layer)
    {
      if( (tm->get_alpha() < 1.0) )
      {
        layer = tm->get_layer() - 1;
      }
      else
      {
        layer = tm->get_layer() + 1;
      }
    }
  }
  log_debug << "Calculated baduy falling layer was: " << layer << std::endl;
  return layer;
}

int
Sector::get_foremost_layer() const
{
  return foremost_layer;
}

void
Sector::update(float elapsed_time)
{
  player->check_bounds();

  if(ambient_light_fading)
  {
    ambient_light_fade_accum += elapsed_time;
    float percent_done = ambient_light_fade_accum / ambient_light_fade_duration * 1.0f;
    float r = (1.0f - percent_done) * source_ambient_light.red + percent_done * target_ambient_light.red;
    float g = (1.0f - percent_done) * source_ambient_light.green + percent_done * target_ambient_light.green;
    float b = (1.0f - percent_done) * source_ambient_light.blue + percent_done * target_ambient_light.blue;
    
    if(r > 1.0)
      r = 1.0;
    if(g > 1.0)
      g = 1.0;
    if(b > 1.0)
      b = 1.0;

    if(r < 0)
      r = 0;
    if(g < 0)
      g = 0;
    if(b < 0)
      b = 0;
    
    ambient_light = Color(r, g, b);

    if(ambient_light_fade_accum >= ambient_light_fade_duration)
    {
      ambient_light = target_ambient_light;
      ambient_light_fading = false;
      ambient_light_fade_accum = 0;
    }
  }

  /* update objects */
  for(const auto& object : gameobjects) {
    if(!object->is_valid())
      continue;

    object->update(elapsed_time);
  }

  /* Handle all possible collisions. */
  collision_detector.handle_collisions();
  update_game_objects();
}

void
Sector::update_game_objects()
{
  /** cleanup marked objects */
  for(auto i = gameobjects.begin();
      i != gameobjects.end(); /* nothing */) {
    const GameObjectPtr& object = *i;

    if(object->is_valid()) {
      ++i;
      continue;
    }

    before_object_remove(object);

    i = gameobjects.erase(i);
  }

  /* add newly created objects */
  for(const auto& object : gameobjects_new)
  {
    before_object_add(object);

    gameobjects.push_back(object);
  }
  gameobjects_new.clear();

  /* update solid_tilemaps list */
  //FIXME: this could be more efficient
  solid_tilemaps.clear();
  for(const auto& obj : gameobjects)
  {
    const auto& tm = dynamic_cast<TileMap*>(obj.get());
    if (!tm) continue;
    if (tm->is_solid()) solid_tilemaps.push_back(tm);
  }

}

bool
Sector::before_object_add(GameObjectPtr object)
{
  auto bullet = dynamic_cast<Bullet*>(object.get());
  if (bullet)
  {
    bullets.push_back(bullet);
  }

  auto movingobject = dynamic_cast<MovingObject*>(object.get());
  if (movingobject)
  {
    moving_objects.push_back(movingobject);
  }

  auto portable = dynamic_cast<Portable*>(object.get());
  if(portable)
  {
    portables.push_back(portable);
  }

  auto tilemap = dynamic_cast<TileMap*>(object.get());
  if(tilemap && tilemap->is_solid()) {
    solid_tilemaps.push_back(tilemap);
  }

  auto camera_ = dynamic_cast<Camera*>(object.get());
  if(camera_) {
    if(this->camera != 0) {
      log_warning << "Multiple cameras added. Ignoring" << std::endl;
      return false;
    }
    this->camera = camera_;
  }

  auto player_ = dynamic_cast<Player*>(object.get());
  if(player_) {
    if(this->player != 0) {
      log_warning << "Multiple players added. Ignoring" << std::endl;
      return false;
    }
    this->player = player_;
  }

  auto effect_ = dynamic_cast<DisplayEffect*>(object.get());
  if(effect_) {
    if(this->effect != 0) {
      log_warning << "Multiple DisplayEffects added. Ignoring" << std::endl;
      return false;
    }
    this->effect = effect_;
  }

  if(_current == this) {
    try_expose(object);
  }

  return true;
}

void
Sector::try_expose(GameObjectPtr object)
{
  scripting::try_expose(object, sector_table);
}

void
Sector::try_expose_me()
{
  HSQUIRRELVM vm = scripting::global_vm;
  sq_pushobject(vm, sector_table);
  auto obj = new scripting::Sector(this);
  expose_object(vm, -1, obj, "settings", true);
  sq_pop(vm, 1);
}

void
Sector::before_object_remove(GameObjectPtr object)
{
  auto portable = dynamic_cast<Portable*>(object.get());
  if (portable) {
    portables.erase(std::find(portables.begin(), portables.end(), portable));
  }
  auto bullet = dynamic_cast<Bullet*>(object.get());
  if (bullet) {
    bullets.erase(std::find(bullets.begin(), bullets.end(), bullet));
  }
  auto moving_object = dynamic_cast<MovingObject*>(object.get());
  if (moving_object) {
    moving_objects.erase(
      std::find(moving_objects.begin(), moving_objects.end(), moving_object));
  }

  if(_current == this)
    try_unexpose(object);
}

void
Sector::try_unexpose(GameObjectPtr object)
{
  scripting::try_unexpose(object, sector_table);
}

void
Sector::try_unexpose_me()
{
  HSQUIRRELVM vm = scripting::global_vm;
  SQInteger oldtop = sq_gettop(vm);
  sq_pushobject(vm, sector_table);
  try {
    scripting::unexpose_object(vm, -1, "settings");
  } catch(std::exception& e) {
    log_warning << "Couldn't unregister object: " << e.what() << std::endl;
  }
  sq_settop(vm, oldtop);
}
void
Sector::draw(DrawingContext& context)
{
  context.set_ambient_color( ambient_light );
  context.push_transform();
  context.set_translation(camera->get_translation());

  for(const auto& object : gameobjects) {
    if(!object->is_valid())
      continue;

    if (draw_solids_only)
    {
      auto tm = dynamic_cast<TileMap*>(object.get());
      if (tm && !tm->is_solid())
        continue;
    }

    object->draw(context);
  }

  if(show_collrects) {
    Color color(1.0f, 0.0f, 0.0f, 0.75f);
    for(auto& object : moving_objects) {
      const Rectf& rect = object->get_bbox();

      context.draw_filled_rect(rect, color, LAYER_FOREGROUND1 + 10);
    }
  }

  context.pop_transform();
}

void
Sector::on_window_resize()
{
  for(const auto& obj : gameobjects)
  {
    obj->on_window_resize();
  }
}

void Sector::collision_tilemap(collision::Constraints* constraints,
                         const Vector& movement, const Rectf& dest,
                         MovingObject& object) const
{
  return collision_detector.collision_tilemap(constraints, movement, dest, 
                                              object);
}
                          
bool Sector::is_free_of_tiles(const Rectf& rect, 
                      const bool ignoreUnisolid) const
{
  return collision_detector.is_free_of_tiles(rect, ignoreUnisolid);
}
                      
bool Sector::is_free_of_statics(const Rectf& rect, 
                        const MovingObject* ignore_object, 
                        const bool ignoreUnisolid) const
{
  return collision_detector.is_free_of_statics(rect, ignore_object, 
                                                ignoreUnisolid);
}
                        
bool Sector::is_free_of_movingstatics(const Rectf& rect, 
                                 const MovingObject* ignore_object) const
{
  return collision_detector.is_free_of_movingstatics(rect, ignore_object);
}
                              
bool Sector::free_line_of_sight(const Vector& line_start, 
                                const Vector& line_end, 
                                const MovingObject* ignore_object) const
{
  return collision_detector.free_line_of_sight(line_start, line_end, 
                                               ignore_object);
}

bool Sector::can_see_player(const Vector& eye)
{
  collision_detector.set_players(get_players());
  return collision_detector.can_see_player(eye);
}

bool
Sector::add_smoke_cloud(const Vector& pos)
{
  add_object(std::make_shared<SmokeCloud>(pos));
  return true;
}

void
Sector::play_music(MusicType type)
{
  currentmusic = type;
  switch(currentmusic) {
    case LEVEL_MUSIC:
      SoundManager::current()->play_music(music);
      break;
    case HERRING_MUSIC:
      SoundManager::current()->play_music("music/invincible.ogg");
      break;
    case HERRING_WARNING_MUSIC:
      SoundManager::current()->stop_music(TUX_INVINCIBLE_TIME_WARNING);
      break;
    default:
      SoundManager::current()->play_music("");
      break;
  }
}

void
Sector::resume_music()
{
  if(SoundManager::current()->get_current_music() == music)
  {
    SoundManager::current()->resume_music(3.2f);
  }
  else
  {
    SoundManager::current()->stop_music();
    SoundManager::current()->play_music(music, true);
  }
}

MusicType
Sector::get_music_type() const
{
  return currentmusic;
}

int
Sector::get_total_badguys() const
{
  int total_badguys = 0;
  for(const auto& object : gameobjects) {
    auto badguy = dynamic_cast<BadGuy*>(object.get());
    if (badguy && badguy->countMe)
      total_badguys++;
  }

  return total_badguys;
}

bool
Sector::inside(const Rectf& rect) const
{
  for(const auto& solids : solid_tilemaps) {
    Rectf bbox = solids->get_bbox();
    bbox.p1.y = -INFINITY; // pretend the tilemap extends infinitely far upwards

    if (bbox.contains(rect))
      return true;
  }
  return false;
}

float
Sector::get_width() const
{
  float width = 0;
  for(auto& solids: solid_tilemaps) {
    width = std::max(width, solids->get_bbox().get_right());
  }

  return width;
}

float
Sector::get_height() const
{
  float height = 0;
  for(const auto& solids: solid_tilemaps) {
    height = std::max(height, solids->get_bbox().get_bottom());
  }

  return height;
}

Size
Sector::get_editor_size() const
{
  // Find the solid tilemap with the greatest surface
  size_t max_surface = 0;
  Size size;
  for(const auto& solids: solid_tilemaps) {
    size_t surface = solids->get_width() * solids->get_height();
    if (surface > max_surface) {
      max_surface = surface;
      size = solids->get_size();
    }
  }

  return size;
}

void
Sector::resize_sector(Size& old_size, Size& new_size)
{
  for(const auto& object : gameobjects) {
    auto tilemap = dynamic_cast<TileMap*>(object.get());
    if (tilemap && tilemap->get_size() == old_size) {
      tilemap->resize(new_size);
    }
  }
}

void
Sector::change_solid_tiles(uint32_t old_tile_id, uint32_t new_tile_id)
{
  for(auto& solids: solid_tilemaps) {
    solids->change_all(old_tile_id, new_tile_id);
  }
}

void
Sector::set_ambient_light(float red, float green, float blue)
{
  ambient_light.red = red;
  ambient_light.green = green;
  ambient_light.blue = blue;
}

void
Sector::fade_to_ambient_light(float red, float green, float blue, float seconds)
{
  if(seconds == 0)
  {
    ambient_light = Color(red, green, blue);
    return;
  }

  ambient_light_fading = true;
  ambient_light_fade_accum = 0;
  ambient_light_fade_duration = seconds;
  source_ambient_light = ambient_light;
  target_ambient_light = Color(red, green, blue);
}

float
Sector::get_ambient_red() const
{
  return ambient_light.red;
}

float
Sector::get_ambient_green() const
{
  return ambient_light.green;
}

float
Sector::get_ambient_blue() const
{
  return ambient_light.blue;
}

void
Sector::set_gravity(float gravity_)
{
  log_warning << "Changing a Sector's gravitational constant might have unforeseen side-effects" << std::endl;
  this->gravity = gravity_;
}

float
Sector::get_gravity() const
{
  return gravity;
}

Player*
Sector::get_nearest_player (const Vector& pos) const
{
  Player *nearest_player = NULL;
  float nearest_dist = std::numeric_limits<float>::max();

  std::vector<Player*> players = get_players();
  for (auto& this_player : players)
  {
    if (this_player->is_dying() || this_player->is_dead())
      continue;

    float this_dist = this_player->get_bbox ().distance(pos);

    if (this_dist < nearest_dist) {
      nearest_player = this_player;
      nearest_dist = this_dist;
    }
  }

  return nearest_player;
} /* Player *get_nearest_player */

std::vector<MovingObject*>
Sector::get_nearby_objects (const Vector& center, float max_distance) const
{
  std::vector<MovingObject*> ret;
  std::vector<Player*> players = Sector::current()->get_players();

  for(const auto& player_ : players) {
    float distance = player_->get_bbox().distance(center);
    if (distance <= max_distance)
      ret.push_back(player_);
  }

  for (const auto& object_ : moving_objects) {
    float distance = object_->get_bbox().distance(center);
    if (distance <= max_distance)
      ret.push_back(object_);
  }

  return ret;
}

void
Sector::stop_looping_sounds()
{
  for(auto& object : gameobjects) {
    object->stop_looping_sounds();
  }
}

void Sector::play_looping_sounds()
{
  for(const auto& object : gameobjects) {
    object->play_looping_sounds();
  }
}

void
Sector::save(Writer &writer)
{
  writer.start_list("sector", false);

  writer.write("name", name, false);
  writer.write("ambient-light", ambient_light.toVector());

  if (init_script.size()) {
    writer.write("init-script", init_script,false);
  }
  if (music.size()) {
    writer.write("music", music, false);
  }

  if (!Editor::is_active() || !Editor::current()->get_worldmap_mode()) {
    writer.write("gravity", gravity);
  }

  // saving spawnpoints
  /*for(auto i = spawnpoints.begin(); i != spawnpoints.end(); ++i) {
    std::shared_ptr<SpawnPoint> spawny = *i;
    spawny->save(writer);
  }*/
  // Do not save spawnpoints since we have spawnpoint markers.

  // saving oběcts (not really)
  for(auto& obj : gameobjects) {
    if (obj->is_saveable()) {
      writer.start_list(obj->get_class());
      obj->save(writer);
      writer.end_list(obj->get_class());
    }
  }

  writer.end_list("sector");
}

/* vim: set sw=2 sts=2 et : */
/* EOF */
