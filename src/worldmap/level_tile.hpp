//  SuperTux
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

#ifndef HEADER_SUPERTUX_WORLDMAP_LEVEL_TILE_HPP
#define HEADER_SUPERTUX_WORLDMAP_LEVEL_TILE_HPP

#include "math/vector.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/game_object.hpp"
#include "supertux/statistics.hpp"

class ReaderMapping;

namespace worldmap {

class LevelTile final : public GameObject
{
  friend class WorldMapParser;

public:
  LevelTile(const std::string& basedir, const ReaderMapping& mapping);
  ~LevelTile() override;

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;

  void set_solved(bool v);
  bool is_solved() const { return m_solved; }

  void set_perfect(bool v);
  bool is_perfect() const { return m_perfect; }

  Statistics& get_statistics() { return m_statistics; }
  const Statistics& get_statistics() const { return m_statistics; }

  void update_sprite_action();

  Vector get_pos() const { return m_pos; }

  std::string get_title() const { return m_title; }
  std::string get_level_filename() const { return m_level_filename; }
  std::string get_basedir() const { return m_basedir; }
  Color get_title_color() const { return m_title_color; }
  std::string get_extro_script() const { return m_extro_script; }
  float get_target_time() const { return m_target_time; }
  bool is_auto_play() const { return m_auto_play; }

private:
  Vector m_pos;

  std::string m_basedir;
  std::string m_level_filename;
  std::string m_title;

  /** true if Tux should automatically enter this level if it's unfinished */
  bool m_auto_play;

  float m_target_time;

  /** Script that is run when the level is successfully finished */
  std::string m_extro_script;

  /** Level state */
  bool m_solved;
  bool m_perfect;

  Statistics m_statistics;

  SpritePtr m_sprite;
  Color m_title_color;

private:
  LevelTile(const LevelTile&) = delete;
  LevelTile& operator=(const LevelTile&) = delete;
};

} // namespace worldmap

#endif

/* EOF */
