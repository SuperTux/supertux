//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include <physfs.h>

#include "editor/editor.hpp"
#include "editor/worldmap_objects.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

namespace worldmap_editor {

WorldmapObject::WorldmapObject (const ReaderMapping& lisp, const std::string& default_sprite) :
  MovingSprite(lisp, default_sprite)
{
  bbox.p1.x = 32 * bbox.p1.x;
  bbox.p1.y = 32 * bbox.p1.y;
  bbox.set_size(32, 32);
}

WorldmapObject::WorldmapObject (const ReaderMapping& lisp) :
  MovingSprite(lisp)
{
  bbox.p1.x = 32 * bbox.p1.x;
  bbox.p1.y = 32 * bbox.p1.y;
  bbox.set_size(32, 32);
}

WorldmapObject::WorldmapObject (const Vector& pos, const std::string& default_sprite) :
  MovingSprite(pos, default_sprite)
{
  bbox.p1.x = 32 * bbox.p1.x;
  bbox.p1.y = 32 * bbox.p1.y;
  bbox.set_size(32, 32);
}

WorldmapObject::~WorldmapObject() {

}

void
WorldmapObject::move_to(const Vector& pos) {
  Vector new_pos;
  new_pos.x = 32 * int(pos.x / 32);
  new_pos.y = 32 * int(pos.y / 32);
  set_pos(new_pos);
}

void
WorldmapObject::save(Writer& writer) {
  writer.write("x", int(bbox.p1.x / 32));
  writer.write("y", int(bbox.p1.y / 32));
}

LevelDot::LevelDot (const ReaderMapping& lisp) :
  WorldmapObject(lisp, "images/worldmap/common/leveldot.sprite"),
  level(),
  extro_script(),
  auto_play(false),
  title_color(1, 1, 1)
{
  lisp.get("name", name);
  lisp.get("extro-script", extro_script);
  lisp.get("auto-play", auto_play);

  std::vector<float> vColor;
  if (lisp.get("color", vColor)) {
    title_color = Color(vColor);
  }

  level = Editor::current()->get_world() ?
    FileSystem::join(Editor::current()->get_world()->get_basedir(), name) : name;
}

LevelDot::~LevelDot() { }

void
LevelDot::draw(DrawingContext& context)
{
  sprite->draw(context, bbox.p1 + Vector(16, 16), layer);
}

ObjectSettings
LevelDot::get_settings() {
  ObjectSettings result(_("Level"));

  ObjectOption lvl(MN_FILE, _("Level"), &level);
  lvl.select.push_back(".stl");
  result.options.push_back(lvl);

  result.options.push_back( ObjectOption(MN_SCRIPT, _("Extro script"), &extro_script));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Auto play"), &auto_play));

  ObjectOption spr(MN_FILE, _("Sprite"), &sprite_name);
  spr.select.push_back(".sprite");
  result.options.push_back(spr);

  result.options.push_back( ObjectOption(MN_COLOR, _("Title colour"), &title_color));

  return result;
}

void
LevelDot::save(Writer& writer) {
  WorldmapObject::save(writer);
  writer.write("name", name, false);
  writer.write("sprite", sprite_name, false);
  writer.write("extro-script", extro_script, false);
  writer.write("auto-play", auto_play);
  writer.write("color", title_color.toVector());
}

void
LevelDot::after_editor_set() {
  // Extract the level file to be relative to world directory
  name = FileSystem::basename(level);
  level = FileSystem::dirname(level);
  level.erase(level.end()-1); // Erase the slash at the end
  if (level[0] == '/' || level[0] == '\\') {
    level.erase(level.begin()); // Erase the slash at the begin
  }
  std::string basedir = Editor::current()->get_world()->get_basedir();
  int c = 100;
  while (level.size() && level != basedir && c > 0) {
    name = FileSystem::join(FileSystem::basename(level), name);
    level = FileSystem::dirname(level);
    level.erase(level.end()-1); // Erase the slash at the end
    c--; //Do not cycle forever if something has failed.
  }

  // Forbid the players to use levels of other levelsets
  level = FileSystem::join(Editor::current()->get_world()->get_basedir(), name);
  if (!PHYSFS_exists(level.c_str())) {
    log_warning << "Using levels of other level subsets is not allowed!" << std::endl;
    level = basedir + "/";
    name = "";
  }
}

Teleporter::Teleporter (const ReaderMapping& lisp) :
  WorldmapObject(lisp, "images/worldmap/common/teleporterdot.sprite"),
  worldmap(),
  spawnpoint(),
  message(),
  automatic(),
  change_worldmap()
{
  lisp.get("worldmap", worldmap);
  lisp.get("spawnpoint", spawnpoint);
  lisp.get("message", message);

  lisp.get("automatic", automatic);

  change_worldmap = worldmap.size();
}

Teleporter::~Teleporter() { }

void
Teleporter::draw(DrawingContext& context)
{
  sprite->draw(context, bbox.p1 + Vector(16, 16), layer);
}

void
Teleporter::save(Writer& writer) {
  WorldmapObject::save(writer);
  writer.write("spawnpoint", spawnpoint, false);
  writer.write("message", message, true);
  writer.write("sprite", sprite_name, false);
  writer.write("automatic", automatic);

  if (change_worldmap) {
    writer.write("worldmap", worldmap, false);
  }
}

ObjectSettings
Teleporter::get_settings() {
  ObjectSettings result(_("Teleporter"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Spawnpoint"), &spawnpoint));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Message"), &message));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Automatic"), &automatic));

  result.options.push_back( ObjectOption(MN_TOGGLE, _("Change worldmap"), &change_worldmap));
  ObjectOption wm(MN_FILE, _("Target worldmap"), &worldmap);
  wm.select.push_back(".stwm");
  result.options.push_back(wm);

  ObjectOption spr(MN_FILE, _("Sprite"), &sprite_name);
  spr.select.push_back(".sprite");
  result.options.push_back(spr);

  return result;
}

WorldmapSpawnPoint::WorldmapSpawnPoint (const ReaderMapping& lisp) :
  WorldmapObject(lisp, "images/worldmap/common/tux.png"),
  dir(worldmap::D_NONE)
{
  lisp.get("name", name);

  std::string auto_dir_str;
  if (lisp.get("auto-dir", auto_dir_str)) {
    dir = worldmap::string_to_direction(auto_dir_str);
  }
}

WorldmapSpawnPoint::WorldmapSpawnPoint (const std::string& name_, const Vector& pos) :
  WorldmapObject(pos, "images/worldmap/common/tux.png"),
  dir(worldmap::D_NONE)
{
  name = name_;
}

WorldmapSpawnPoint::~WorldmapSpawnPoint() { }

void
WorldmapSpawnPoint::save(Writer& writer) {
  WorldmapObject::save(writer);
  writer.write("name", name, false);
  writer.write("auto-dir", worldmap::direction_to_string(dir), false);
}

ObjectSettings
WorldmapSpawnPoint::get_settings() {
  ObjectSettings result(_("Spawn point"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( worldmap::dir_option(&dir));
  return result;
}

SpriteChange::SpriteChange (const ReaderMapping& lisp) :
  WorldmapObject(lisp, "images/engine/editor/spritechange.png"),
  target_sprite(sprite_name),
  stay_action(),
  initial_stay_action(false),
  stay_group(),
  change_on_touch(true)
{
  // To make obvious where the sprite change is, let's use an universal 32×32 sprite
  sprite = SpriteManager::current()->create("images/engine/editor/spritechange.png");

  lisp.get("stay-action", stay_action);
  lisp.get("initial-stay-action", initial_stay_action);
  lisp.get("stay-group", stay_group);

  lisp.get("change-on-touch", change_on_touch);
}

SpriteChange::~SpriteChange() { }

void
SpriteChange::save(Writer& writer) {
  WorldmapObject::save(writer);
  writer.write("stay-action", stay_action, false);
  writer.write("initial-stay-action", initial_stay_action);
  writer.write("stay-group", stay_group, false);
  writer.write("sprite", target_sprite, false);
  writer.write("change-on-touch", change_on_touch);
}

ObjectSettings
SpriteChange::get_settings() {
  ObjectSettings result(_("Sprite change"));

  ObjectOption spr(MN_FILE, _("Sprite"), &target_sprite);
  spr.select.push_back(".sprite");
  result.options.push_back(spr);

  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Stay action"), &stay_action));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Initial stay action"), &initial_stay_action));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Stay group"), &stay_group));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Change on touch"), &change_on_touch));

  return result;
}

SpecialTile::SpecialTile (const ReaderMapping& lisp) :
  WorldmapObject(lisp, "images/worldmap/common/messagedot.png"),
  map_message(),
  script(),
  passive_message(false),
  invisible_tile(true),
  apply_to_direction(worldmap::D_NONE)
{
  lisp.get("map-message", map_message);
  lisp.get("script", script);

  lisp.get("passive-message", passive_message);
  lisp.get("invisible-tile", invisible_tile);

  std::string dir_str;
  if (lisp.get("apply-to-direction", dir_str)) {
    apply_to_direction = worldmap::string_to_direction(dir_str);
  }
}

SpecialTile::~SpecialTile() { }

void
SpecialTile::save(Writer& writer) {
  WorldmapObject::save(writer);
  writer.write("map-message", map_message, true);
  writer.write("script", script, false);

  if (sprite_name != "images/worldmap/common/messagedot.png") {
    writer.write("sprite", sprite_name, false);
  }

  writer.write("passive-message", passive_message);
  writer.write("invisible-tile", invisible_tile);

  writer.write("apply-to-direction", worldmap::direction_to_string(apply_to_direction), false);
}

ObjectSettings
SpecialTile::get_settings() {
  ObjectSettings result(_("Special tile"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Message"), &map_message));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Show message"), &passive_message));
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Script"), &script));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Invisible"), &invisible_tile));
  result.options.push_back( worldmap::dir_option(&apply_to_direction));

  ObjectOption spr(MN_FILE, _("Sprite"), &sprite_name);
  spr.select.push_back(".sprite");
  result.options.push_back(spr);

  return result;
}

} // namespace worldmap_editor

/* EOF */
