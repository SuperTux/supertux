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

#include "editor/worldmap_objects.hpp"

#include <physfs.h>

#include "editor/editor.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

namespace worldmap_editor {

WorldmapObject::WorldmapObject (const ReaderMapping& mapping, const std::string& default_sprite) :
  MovingSprite(mapping, default_sprite),
  m_tile_x(),
  m_tile_y()
{
  m_col.m_bbox = Rectf(Vector(32 * m_col.m_bbox.get_left(),
                              32 * m_col.m_bbox.get_top()),
                       Sizef(32.0f, 32.0f));
}

WorldmapObject::WorldmapObject (const ReaderMapping& mapping) :
  MovingSprite(mapping),
  m_tile_x(),
  m_tile_y()
{
  m_col.m_bbox.set_left(32 * m_col.m_bbox.get_left());
  m_col.m_bbox.set_top(32 * m_col.m_bbox.get_top());
  m_col.m_bbox.set_size(32, 32);
}

WorldmapObject::WorldmapObject (const Vector& pos, const std::string& default_sprite) :
  MovingSprite(pos, default_sprite),
  m_tile_x(),
  m_tile_y()
{
  m_col.m_bbox.set_left(32 * m_col.m_bbox.get_left());
  m_col.m_bbox.set_top(32 * m_col.m_bbox.get_top());
  m_col.m_bbox.set_size(32, 32);
}

ObjectSettings
WorldmapObject::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  m_tile_x = static_cast<int>(m_col.m_bbox.get_left()) / 32;
  m_tile_y = static_cast<int>(m_col.m_bbox.get_top()) / 32;

  result.remove("x");
  result.remove("y");

  result.add_int(_("X"), &m_tile_x, "x", {}, OPTION_HIDDEN);
  result.add_int(_("Y"), &m_tile_y, "y", {}, OPTION_HIDDEN);

  return result;
}

void
WorldmapObject::move_to(const Vector& pos)
{
  set_pos(Vector(32.0f * static_cast<float>(pos.x / 32),
                 32.0f * static_cast<float>(pos.y / 32)));
}

LevelDot::LevelDot(const ReaderMapping& mapping) :
  WorldmapObject(mapping, "images/worldmap/common/leveldot.sprite"),
  m_level_filename(),
  m_extro_script(),
  m_auto_play(false),
  m_title_color(1, 1, 1)
{
  mapping.get("extro-script", m_extro_script);
  mapping.get("auto-play", m_auto_play);
  if (!mapping.get("level", m_level_filename)) {
    // Hack for backward compatibility with 0.5.x level
    m_level_filename = std::move(m_name);
  }

  std::vector<float> vColor;
  if (mapping.get("color", vColor)) {
    m_title_color = Color(vColor);
  }
}

void
LevelDot::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), m_col.m_bbox.p1() + Vector(16, 16), m_layer);
}

ObjectSettings
LevelDot::get_settings()
{
  ObjectSettings result = WorldmapObject::get_settings();

  std::string basedir = (Editor::current() && Editor::current()->get_world()) ?
    Editor::current()->get_world()->get_basedir() : std::string();

  // FIXME: hack to make the basedir absolute, making
  // World::get_basedir() itself absolute would be correct, but
  // invalidate savefiles.
  if (!basedir.empty() && basedir.front() != '/') {
    basedir = "/" + basedir;
  }

  result.add_level(_("Level"), &m_level_filename, "level", basedir);
  result.add_script(_("Outro script"), &m_extro_script, "extro-script");
  result.add_bool(_("Auto play"), &m_auto_play, "auto-play", false);
  //result.add_sprite(_("Sprite"), &m_sprite_name, "sprite");
  result.add_color(_("Title colour"), &m_title_color, "color", Color::WHITE);

  result.reorder({"name", "sprite", "x", "y"});

  return result;
}

Teleporter::Teleporter (const ReaderMapping& mapping) :
  WorldmapObject(mapping, "images/worldmap/common/teleporterdot.sprite"),
  m_worldmap(),
  m_sector(),
  m_spawnpoint(),
  m_message(),
  m_automatic(),
  m_change_worldmap()
{
  mapping.get("worldmap", m_worldmap);
  mapping.get("sector", m_sector);
  mapping.get("spawnpoint", m_spawnpoint);
  mapping.get("message", m_message);

  mapping.get("automatic", m_automatic);

  m_change_worldmap = m_worldmap.size() > 0;
}

void
Teleporter::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), m_col.m_bbox.p1() + Vector(16, 16), m_layer);
}

ObjectSettings
Teleporter::get_settings()
{
  ObjectSettings result = WorldmapObject::get_settings();

  result.add_text(_("Sector"), &m_sector, "sector");
  result.add_text(_("Spawnpoint"), &m_spawnpoint, "spawnpoint");
  result.add_translatable_text(_("Message"), &m_message, "message");
  result.add_bool(_("Automatic"), &m_automatic, "automatic", false);
  // result.add_bool(_("Change worldmap"), &m_change_worldmap, "worldmap", true);
  result.add_worldmap(_("Target worldmap"), &m_worldmap, "worldmap");
  //result.add_sprite(_("Sprite"), &m_sprite_name, "sprite");

  result.reorder({"sector", "spawnpoint", "automatic", "message", "sprite", "x", "y"});

  return result;
}

WorldmapSpawnPoint::WorldmapSpawnPoint (const ReaderMapping& mapping) :
  WorldmapObject(mapping, "images/worldmap/common/tux.png"),
  m_dir(worldmap::Direction::NONE)
{
  mapping.get("name", m_name);

  std::string auto_dir_str;
  if (mapping.get("auto-dir", auto_dir_str)) {
    m_dir = worldmap::string_to_direction(auto_dir_str);
  }
}

WorldmapSpawnPoint::WorldmapSpawnPoint (const std::string& name_, const Vector& pos) :
  WorldmapObject(pos, "images/worldmap/common/tux.png"),
  m_dir(worldmap::Direction::NONE)
{
  m_name = name_;
}

ObjectSettings
WorldmapSpawnPoint::get_settings()
{
  ObjectSettings result = WorldmapObject::get_settings();

  result.add_worldmap_direction(_("Direction"), &m_dir, worldmap::Direction::NONE, "auto-dir");
  result.remove("sprite");

  result.reorder({"auto-dir", "name", "x", "y"});

  return result;
}

SpriteChange::SpriteChange (const ReaderMapping& mapping) :
  WorldmapObject(mapping, "images/engine/editor/spritechange.png"),
  m_target_sprite(m_sprite_name),
  m_stay_action(),
  m_initial_stay_action(false),
  m_stay_group(),
  m_change_on_touch(true)
{
  // To make obvious where the sprite change is, let's use an universal 32×32 sprite
  m_sprite = SpriteManager::current()->create("images/engine/editor/spritechange.png");

  mapping.get("stay-action", m_stay_action);
  mapping.get("initial-stay-action", m_initial_stay_action);
  mapping.get("stay-group", m_stay_group);

  mapping.get("change-on-touch", m_change_on_touch);
}

ObjectSettings
SpriteChange::get_settings()
{
  ObjectSettings result = WorldmapObject::get_settings();

  //result.add_sprite(_("Sprite"), &m_target_sprite, "sprite");
  result.add_text(_("Stay action"), &m_stay_action, "stay-action");
  result.add_bool(_("Initial stay action"), &m_initial_stay_action, "initial-stay-action");
  result.add_text(_("Stay group"), &m_stay_group, "stay-group");
  result.add_bool(_("Change on touch"), &m_change_on_touch, "change-on-touch");

  result.reorder({"change-on-touch", "initial-stay-action", "stay-group", "sprite", "x", "y"});

  return result;
}

SpecialTile::SpecialTile (const ReaderMapping& mapping) :
  WorldmapObject(mapping, "images/worldmap/common/messagedot.png"),
  m_map_message(),
  m_script(),
  m_passive_message(false),
  m_invisible_tile(false),
  m_apply_to_directions("north-east-south-west")
{
  mapping.get("map-message", m_map_message);
  mapping.get("script", m_script);

  mapping.get("passive-message", m_passive_message);
  mapping.get("invisible-tile", m_invisible_tile);

  mapping.get("apply-to-direction", m_apply_to_directions);
}

ObjectSettings
SpecialTile::get_settings()
{
  ObjectSettings result = WorldmapObject::get_settings();

  result.add_translatable_text(_("Message"), &m_map_message, "map-message");
  result.add_bool(_("Show message"), &m_passive_message, "passive-message", false);
  result.add_script(_("Script"), &m_script, "script");
  result.add_bool(_("Invisible"), &m_invisible_tile, "invisible-tile", false);
  result.add_text(_("Direction"), &m_apply_to_directions, "apply-to-direction", std::string("north-east-south-west"));
  //result.add_sprite(_("Sprite"), &m_sprite_name, "sprite");

  result.reorder({"map-message", "invisible-tile", "script", "passive-message", "apply-to-direction", "sprite", "x", "y"});

  return result;
}

} // namespace worldmap_editor

/* EOF */
