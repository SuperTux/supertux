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

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "editor/editor.hpp"
#include "math/aatriangle.hpp"
#include "math/rect.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "object/display_effect.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "object/smoke_cloud.hpp"
#include "object/text_object.hpp"
#include "object/text_array_object.hpp"
#include "object/tilemap.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "scripting/sector.hpp"
#include "supertux/collision.hpp"
#include "supertux/collision_system.hpp"
#include "supertux/constants.hpp"
#include "supertux/game_session.hpp"
#include "supertux/level.hpp"
#include "supertux/savegame.hpp"
#include "supertux/spawn_point.hpp"
#include "supertux/tile.hpp"
#include "util/file_system.hpp"
#include "util/writer.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

Sector* Sector::s_current = 0;

bool Sector::s_show_collrects = false;
bool Sector::s_draw_solids_only = false;

Sector::Sector(Level* parent) :
  m_level(parent),
  m_name(),
  m_bullets(),
  m_init_script(),
  m_gameobjects_new(),
  m_currentmusic(LEVEL_MUSIC),
  m_sector_table(),
  m_scripts(),
  m_ambient_light( 1.0f, 1.0f, 1.0f, 1.0f ),
  m_ambient_light_fading(false),
  m_source_ambient_light(1.0f, 1.0f, 1.0f, 1.0f),
  m_target_ambient_light(1.0f, 1.0f, 1.0f, 1.0f),
  m_ambient_light_fade_duration(0.0f),
  m_ambient_light_fade_accum(0.0f),
  m_foremost_layer(),
  m_gameobjects(),
  m_collision_system(new CollisionSystem(*this)),
  m_spawnpoints(),
  m_portables(),
  m_music(),
  m_gravity(10.0),
  m_player(0),
  m_solid_tilemaps(),
  //m_camera(0),
  m_effect(0)
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

  // create players
  auto player1 = std::make_shared<Player>(Player::PLAYER_1,player_status, "Tux");
  auto player2 = std::make_shared<Player>(Player::PLAYER_2,player_status, "Penny");

  player1->get_camera()->set_number(1);
  player2->get_camera()->set_number(2);

  add_object(player1);
  add_object(player2);
  add_object(std::make_shared<DisplayEffect>("Effect"));
  add_object(std::make_shared<TextObject>("Text"));
  add_object(std::make_shared<TextArrayObject>("TextArray"));

  SoundManager::current()->preload("sounds/shoot.wav");

  // create a new squirrel table for the sector
  using namespace scripting;

  sq_collectgarbage(global_vm);

  sq_newtable(global_vm);
  sq_pushroottable(global_vm);
  if(SQ_FAILED(sq_setdelegate(global_vm, -2)))
    throw scripting::SquirrelError(global_vm, "Couldn't set sector_table delegate");

  sq_resetobject(&m_sector_table);
  if(SQ_FAILED(sq_getstackobj(global_vm, -1, &m_sector_table)))
    throw scripting::SquirrelError(global_vm, "Couldn't get sector table");
  sq_addref(global_vm, &m_sector_table);
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

  release_scripts(global_vm, m_scripts, m_sector_table);

  update_game_objects();
  assert(m_gameobjects_new.size() == 0);

  for(const auto& object: m_gameobjects) {
    before_object_remove(object);
  }
}

Level*
Sector::get_level() const
{
  return m_level;
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
    return scripting::run_script(in, "Sector " + m_name + " - " + sourcename,
                                 m_scripts, &m_sector_table);
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
  for(const auto& game_object : m_gameobjects) {
    assert(game_object != object);
  }
  for(const auto& gameobject : m_gameobjects_new) {
    assert(gameobject != object);
  }
#endif

  m_gameobjects_new.push_back(object);
}

void
Sector::activate(const std::string& spawnpoint)
{
  std::shared_ptr<SpawnPoint> sp;
  for(const auto& spawn_point : m_spawnpoints) {
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
  if(s_current != this) {
    if(s_current != NULL)
      s_current->deactivate();
    s_current = this;

    // register sectortable as sector in scripting
    HSQUIRRELVM vm = scripting::global_vm;
    sq_pushroottable(vm);
    scripting::store_object(vm, "sector", m_sector_table);
    sq_pop(vm, 1);

    for(auto& object : m_gameobjects) {
      try_expose(object);
    }
  }
  try_expose_me();


  // two-player hack: move other players to main player's position
  // Maybe specify 2 spawnpoints in the level?
  for(auto& object : m_gameobjects) {
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
      std::string current_level = "[" + Sector::current()->get_level()->m_filename + "] ";
      log_warning << current_level << "Tried spawning Tux in solid matter. Compensating." << std::endl;
      Vector npos = p->get_bbox().p1;
      npos.y-=32;
      p->move(npos);
    }
  }

  //FIXME: This is a really dirty workaround for this strange camera jump
  for(const auto& player_it : get_players())
  {
    auto cam = player_it->get_camera();
    player_it->move(player_it->get_pos() + Vector(-32, 0));
    cam->reset(player_it->get_pos());
    cam->update(1);
    player_it->move(player_it->get_pos() + Vector(32, 0));
    cam->update(1);
  }

  update_game_objects();

  //Run default.nut just before init script
  //Check to see if it's in a levelset (info file)
  std::string basedir = FileSystem::dirname(get_level()->m_filename);
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
  if(!m_init_script.empty() && !Editor::is_active()) {
    run_script(m_init_script, "init-script");
  }
}

void
Sector::deactivate()
{
  if(s_current != this)
    return;

  // remove sector entry from global vm
  HSQUIRRELVM vm = scripting::global_vm;
  sq_pushroottable(vm);
  scripting::delete_table_entry(vm, "sector");
  sq_pop(vm, 1);

  for(const auto& object: m_gameobjects) {
    try_unexpose(object);
  }

  try_unexpose_me();
  s_current = NULL;
}

int
Sector::calculate_foremost_layer() const
{
  int layer = LAYER_BACKGROUND0;
  for(const auto& obj : m_gameobjects)
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
  return m_foremost_layer;
}

void
Sector::update(float elapsed_time)
{
  m_player->check_bounds();

  if(m_ambient_light_fading)
  {
    m_ambient_light_fade_accum += elapsed_time;
    float percent_done = m_ambient_light_fade_accum / m_ambient_light_fade_duration * 1.0f;
    float r = (1.0f - percent_done) * m_source_ambient_light.red + percent_done * m_target_ambient_light.red;
    float g = (1.0f - percent_done) * m_source_ambient_light.green + percent_done * m_target_ambient_light.green;
    float b = (1.0f - percent_done) * m_source_ambient_light.blue + percent_done * m_target_ambient_light.blue;

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

    m_ambient_light = Color(r, g, b);

    if(m_ambient_light_fade_accum >= m_ambient_light_fade_duration)
    {
      m_ambient_light = m_target_ambient_light;
      m_ambient_light_fading = false;
      m_ambient_light_fade_accum = 0;
    }
  }

  /* update objects */
  for(const auto& object : m_gameobjects) {
    if(!object->is_valid())
      continue;

    object->update(elapsed_time);
  }

  /* Handle all possible collisions. */
  m_collision_system->update();
  update_game_objects();
}

void
Sector::update_game_objects()
{
  /** cleanup marked objects */
  for(auto i = m_gameobjects.begin();
      i != m_gameobjects.end(); /* nothing */) {
    const GameObjectPtr& object = *i;

    if(object->is_valid()) {
      ++i;
      continue;
    }

    before_object_remove(object);

    i = m_gameobjects.erase(i);
  }

  /* add newly created objects */
  for(const auto& object : m_gameobjects_new)
  {
    before_object_add(object);

    m_gameobjects.push_back(object);
  }
  m_gameobjects_new.clear();

  /* update solid_tilemaps list */
  //FIXME: this could be more efficient
  m_solid_tilemaps.clear();
  for(const auto& obj : m_gameobjects)
  {
    const auto& tm = dynamic_cast<TileMap*>(obj.get());
    if (!tm) continue;
    if (tm->is_solid()) m_solid_tilemaps.push_back(tm);
  }

}

bool
Sector::before_object_add(GameObjectPtr object)
{
  auto bullet = dynamic_cast<Bullet*>(object.get());
  if (bullet)
  {
    m_bullets.push_back(bullet);
  }

  auto movingobject = dynamic_cast<MovingObject*>(object.get());
  if (movingobject)
  {
    m_collision_system->add(movingobject);
  }

  auto portable = dynamic_cast<Portable*>(object.get());
  if(portable)
  {
    m_portables.push_back(portable);
  }

  auto tilemap = dynamic_cast<TileMap*>(object.get());
  if(tilemap && tilemap->is_solid()) {
    m_solid_tilemaps.push_back(tilemap);
  }

  /*auto camera_ = dynamic_cast<Camera*>(object.get());
  if(camera_) {
    if(m_camera != 0) {
      log_warning << "Multiple cameras added. Ignoring" << std::endl;
      return false;
    }
    this->camera = camera_;
  }*/

  auto player_ = dynamic_cast<Player*>(object.get());
  if(player_) {
    /*
    if(player != 0) {
      log_warning << "Multiple players added. Ignoring" << std::endl;
      return false;
      }*/
    m_player = player_;
  }

  auto effect_ = dynamic_cast<DisplayEffect*>(object.get());
  if(effect_) {
    if(m_effect != 0) {
      log_warning << "Multiple DisplayEffects added. Ignoring" << std::endl;
      return false;
    }
    m_effect = effect_;
  }

  if(s_current == this) {
    try_expose(object);
  }

  return true;
}

void
Sector::try_expose(GameObjectPtr object)
{
  scripting::try_expose(object, m_sector_table);
}

void
Sector::try_expose_me()
{
  HSQUIRRELVM vm = scripting::global_vm;
  sq_pushobject(vm, m_sector_table);
  auto obj = new scripting::Sector(this);
  expose_object(vm, -1, obj, "settings", true);
  sq_pop(vm, 1);
}

void
Sector::before_object_remove(GameObjectPtr object)
{
  auto portable = dynamic_cast<Portable*>(object.get());
  if (portable) {
    m_portables.erase(std::find(m_portables.begin(), m_portables.end(), portable));
  }
  auto bullet = dynamic_cast<Bullet*>(object.get());
  if (bullet) {
    m_bullets.erase(std::find(m_bullets.begin(), m_bullets.end(), bullet));
  }
  auto moving_object = dynamic_cast<MovingObject*>(object.get());
  if (moving_object) {
    m_collision_system->remove(moving_object);
  }

  if(s_current == this)
    try_unexpose(object);
}

void
Sector::try_unexpose(GameObjectPtr object)
{
  scripting::try_unexpose(object, m_sector_table);
}

void
Sector::try_unexpose_me()
{
  HSQUIRRELVM vm = scripting::global_vm;
  SQInteger oldtop = sq_gettop(vm);
  sq_pushobject(vm, m_sector_table);
  try {
    scripting::unexpose_object(vm, -1, "settings");
  } catch(std::exception& e) {
    log_warning << "Couldn't unregister object: " << e.what() << std::endl;
  }
  sq_settop(vm, oldtop);
}
void
Sector::draw(DrawingContext& context, Player* player_it)
{
  if (!player_it)
    player_it = m_player;

  context.set_ambient_color( m_ambient_light );
  context.push_transform();
  context.set_translation(player_it->camera->get_translation());

  for(const auto& object : m_gameobjects) {
    if(!object->is_valid())
      continue;

    if (s_draw_solids_only)
    {
      auto tm = dynamic_cast<TileMap*>(object.get());
      if (tm && !tm->is_solid())
        continue;
    }

    object->draw(context);
  }

  if(s_show_collrects) {
    m_collision_system->draw(context);
  }

  context.pop_transform();
}

void
Sector::on_window_resize()
{
  for(const auto& obj : m_gameobjects)
  {
    obj->on_window_resize();
  }
}

bool
Sector::is_free_of_tiles(const Rectf& rect, const bool ignoreUnisolid) const
{
  return m_collision_system->is_free_of_tiles(rect, ignoreUnisolid);
}

bool
Sector::is_free_of_statics(const Rectf& rect, const MovingObject* ignore_object, const bool ignoreUnisolid) const
{
  return m_collision_system->is_free_of_statics(rect, ignore_object, ignoreUnisolid);
}

bool
Sector::is_free_of_movingstatics(const Rectf& rect, const MovingObject* ignore_object) const
{
  return m_collision_system->is_free_of_movingstatics(rect, ignore_object);
}

bool
Sector::free_line_of_sight(const Vector& line_start, const Vector& line_end, const MovingObject* ignore_object) const
{
  return m_collision_system->free_line_of_sight(line_start, line_end, ignore_object);
}

bool
Sector::can_see_player(const Vector& eye) const
{
  const std::vector<Player*> players = get_players();
  for (const auto& pl : players) {
    // test for free line of sight to any of all four corners and the middle of the player's bounding box
    if (free_line_of_sight(eye, pl->get_bbox().p1, pl)) return true;
    if (free_line_of_sight(eye, Vector(pl->get_bbox().p2.x, pl->get_bbox().p1.y), pl)) return true;
    if (free_line_of_sight(eye, pl->get_bbox().p2, pl)) return true;
    if (free_line_of_sight(eye, Vector(pl->get_bbox().p1.x, pl->get_bbox().p2.y), pl)) return true;
    if (free_line_of_sight(eye, pl->get_bbox().get_middle(), pl)) return true;
  }
  return false;
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
  m_currentmusic = type;
  switch(m_currentmusic) {
    case LEVEL_MUSIC:
      SoundManager::current()->play_music(m_music);
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
  if(SoundManager::current()->get_current_music() == m_music)
  {
    SoundManager::current()->resume_music(3.2f);
  }
  else
  {
    SoundManager::current()->stop_music();
    SoundManager::current()->play_music(m_music, true);
  }
}

MusicType
Sector::get_music_type() const
{
  return m_currentmusic;
}

int
Sector::get_total_badguys() const
{
  int total_badguys = 0;
  for(const auto& object : m_gameobjects) {
    auto badguy = dynamic_cast<BadGuy*>(object.get());
    if (badguy && badguy->countMe)
      total_badguys++;
  }

  return total_badguys;
}

bool
Sector::inside(const Rectf& rect) const
{
  for(const auto& solids : m_solid_tilemaps) {
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
  for(auto& solids: m_solid_tilemaps) {
    width = std::max(width, solids->get_bbox().get_right());
  }

  return width;
}

float
Sector::get_height() const
{
  float height = 0;
  for(const auto& solids: m_solid_tilemaps) {
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
  for(const auto& solids: m_solid_tilemaps) {
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
  for(const auto& object : m_gameobjects) {
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
  for(auto& solids: m_solid_tilemaps) {
    solids->change_all(old_tile_id, new_tile_id);
  }
}

void
Sector::set_ambient_light(float red, float green, float blue)
{
  m_ambient_light.red = red;
  m_ambient_light.green = green;
  m_ambient_light.blue = blue;
}

void
Sector::fade_to_ambient_light(float red, float green, float blue, float seconds)
{
  if(seconds == 0)
  {
    m_ambient_light = Color(red, green, blue);
    return;
  }

  m_ambient_light_fading = true;
  m_ambient_light_fade_accum = 0;
  m_ambient_light_fade_duration = seconds;
  m_source_ambient_light = m_ambient_light;
  m_target_ambient_light = Color(red, green, blue);
}

float
Sector::get_ambient_red() const
{
  return m_ambient_light.red;
}

float
Sector::get_ambient_green() const
{
  return m_ambient_light.green;
}

float
Sector::get_ambient_blue() const
{
  return m_ambient_light.blue;
}

void
Sector::set_gravity(float gravity_)
{
  log_warning << "Changing a Sector's gravitational constant might have unforeseen side-effects" << std::endl;
  m_gravity = gravity_;
}

float
Sector::get_gravity() const
{
  return m_gravity;
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
  return m_collision_system->get_nearby_objects(center, max_distance);
}

void
Sector::stop_looping_sounds()
{
  for(auto& object : m_gameobjects) {
    object->stop_looping_sounds();
  }
}

void Sector::play_looping_sounds()
{
  for(const auto& object : m_gameobjects) {
    object->play_looping_sounds();
  }
}

void
Sector::save(Writer &writer)
{
  writer.start_list("sector", false);

  writer.write("name", m_name, false);
  writer.write("ambient-light", m_ambient_light.toVector());

  if (m_init_script.size()) {
    writer.write("init-script", m_init_script,false);
  }
  if (m_music.size()) {
    writer.write("music", m_music, false);
  }

  if (!Editor::is_active() || !Editor::current()->get_worldmap_mode()) {
    writer.write("gravity", m_gravity);
  }

  // saving spawnpoints
  /*for(auto i = spawnpoints.begin(); i != spawnpoints.end(); ++i) {
    std::shared_ptr<SpawnPoint> spawny = *i;
    spawny->save(writer);
  }*/
  // Do not save spawnpoints since we have spawnpoint markers.

  // saving oběcts (not really)
  for(auto& obj : m_gameobjects) {
    if (obj->is_saveable()) {
      writer.start_list(obj->get_class());
      obj->save(writer);
      writer.end_list(obj->get_class());
    }
  }

  writer.end_list("sector");
}

const std::vector<MovingObject*>&
Sector::get_moving_objects() const
{
  return m_collision_system->get_moving_objects();
}

/* EOF */
