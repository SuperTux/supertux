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

#ifndef HEADER_SUPERTUX_EDITOR_WORLDMAP_OBJECTS_HPP
#define HEADER_SUPERTUX_EDITOR_WORLDMAP_OBJECTS_HPP

#include "object/moving_sprite.hpp"
#include "video/color.hpp"
#include "worldmap/direction.hpp"

namespace worldmap_editor {

class WorldmapObject : public MovingSprite
{
public:
  WorldmapObject(const ReaderMapping& mapping, const std::string& default_sprite);
  WorldmapObject(const ReaderMapping& mapping);
  WorldmapObject(const Vector& pos, const std::string& default_sprite);

  virtual ObjectSettings get_settings() override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override { return FORCE_MOVE; }
  static std::string class_name() { return "worldmap-object"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual void move_to(const Vector& pos) override;

private:
  // FIXME: purely used for saving, is not updated normally, don't use.
  int m_tile_x;
  int m_tile_y;

private:
  WorldmapObject(const WorldmapObject&) = delete;
  WorldmapObject& operator=(const WorldmapObject&) = delete;
};

class LevelDot final : public WorldmapObject
{
public:
  LevelDot(const ReaderMapping& mapping);

  virtual void draw(DrawingContext& context) override;

  static std::string class_name() { return "level"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Level"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual ObjectSettings get_settings() override;

private:
  std::string m_level_filename;
  std::string m_extro_script;
  bool m_auto_play;
  Color m_title_color;

private:
  LevelDot(const LevelDot&) = delete;
  LevelDot& operator=(const LevelDot&) = delete;
};

class Teleporter final : public WorldmapObject
{
public:
  Teleporter(const ReaderMapping& mapping);

  virtual void draw(DrawingContext& context) override;

  static std::string class_name() { return "teleporter"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Teleporter"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual ObjectSettings get_settings() override;

private:
  std::string m_worldmap;
  std::string m_sector;
  std::string m_spawnpoint;
  std::string m_message;
  bool m_automatic;
  bool m_change_worldmap;

private:
  Teleporter(const Teleporter&) = delete;
  Teleporter& operator=(const Teleporter&) = delete;
};

class WorldmapSpawnPoint final : public WorldmapObject
{
public:
  WorldmapSpawnPoint(const ReaderMapping& mapping);
  WorldmapSpawnPoint(const std::string& name_, const Vector& pos);

  static std::string class_name() { return "worldmap-spawnpoint"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Spawn point"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;

private:
  worldmap::Direction m_dir;

private:
  WorldmapSpawnPoint(const WorldmapSpawnPoint&) = delete;
  WorldmapSpawnPoint& operator=(const WorldmapSpawnPoint&) = delete;
};

class SpriteChange final : public WorldmapObject
{
public:
  SpriteChange(const ReaderMapping& mapping);

  static std::string class_name() { return "sprite-change"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Sprite Change"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual ObjectSettings get_settings() override;

private:
  std::string m_target_sprite;
  std::string m_stay_action;
  bool m_initial_stay_action;
  std::string m_stay_group;
  bool m_change_on_touch;

private:
  SpriteChange(const SpriteChange&) = delete;
  SpriteChange& operator=(const SpriteChange&) = delete;
};

class SpecialTile final : public WorldmapObject
{
public:
  SpecialTile(const ReaderMapping& mapping);

  static std::string class_name() { return "special-tile"; }
  virtual std::string get_class_name() const override { return class_name(); }
    static std::string display_name() { return _("Special tile"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual ObjectSettings get_settings() override;

private:
  std::string m_map_message;
  std::string m_script;
  bool m_passive_message;
  bool m_invisible_tile;

  std::string m_apply_to_directions;

private:
  SpecialTile(const SpecialTile&) = delete;
  SpecialTile& operator=(const SpecialTile&) = delete;
};

} // namespace worldmap_editor

#endif

/* EOF */
