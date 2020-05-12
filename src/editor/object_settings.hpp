//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_EDITOR_OBJECT_SETTINGS_HPP
#define HEADER_SUPERTUX_EDITOR_OBJECT_SETTINGS_HPP

#include <vector>
#include <memory>

#include "editor/object_option.hpp"

class Color;
enum class Direction;
enum class WalkMode;
namespace worldmap {
enum class Direction;
} // namespace worldmap
namespace sexp {
class Value;
} // namespace sexp

class ObjectSettings final
{
public:
  ObjectSettings(const std::string& name);
  ObjectSettings(ObjectSettings&&) = default;

  const std::string& get_name() const { return m_name; }

  void add_bool(const std::string& text, bool* value_ptr,
                const std::string& key = {},
                boost::optional<bool> default_value = {},
                unsigned int flags = 0);
  void add_float(const std::string& text, float* value_ptr,
                 const std::string& key = {},
                 boost::optional<float> default_value = {},
                 unsigned int flags = 0);
  void add_int(const std::string& text, int* value_ptr,
               const std::string& key = {},
               boost::optional<int> default_value = {},
               unsigned int flags = 0);
  void add_rectf(const std::string& text, Rectf* value_ptr,
                 const std::string& key = {},
                 unsigned int flags = 0);
  void add_worldmap_direction(const std::string& text, worldmap::Direction* value_ptr,
                              boost::optional<worldmap::Direction> default_value = {},
                              const std::string& key = {}, unsigned int flags = 0);
  void add_direction(const std::string& text, Direction* value_ptr,
                     boost::optional<Direction> default_value = {},
                     const std::string& key = {}, unsigned int flags = 0);
  void add_walk_mode(const std::string& text, WalkMode* value_ptr,
                     boost::optional<WalkMode> default_value = {},
                     const std::string& key = {}, unsigned int flags = 0);
  void add_badguy(const std::string& text, std::vector<std::string>* value_ptr,
                  const std::string& key = {}, unsigned int flags = 0);
  void add_color(const std::string& text, Color* value_ptr,
                 const std::string& key = {},
                 boost::optional<Color> default_value = {},
                 unsigned int flags = 0);
  void add_rgba(const std::string& text, Color* value_ptr,
                const std::string& key = {},
                boost::optional<Color> default_value = {},
                unsigned int flags = 0);
  void add_rgb(const std::string& text, Color* value_ptr,
               const std::string& key = {},
               boost::optional<Color> default_value = {},
               unsigned int flags = 0);
  void add_remove();
  void add_script(const std::string& text, std::string* value_ptr,
                  const std::string& key = {}, unsigned int flags = 0);
  void add_text(const std::string& text, std::string* value_ptr,
                const std::string& key = {},
                boost::optional<std::string> default_value = {},
                unsigned int flags = 0);
  void add_translatable_text(const std::string& text, std::string* value_ptr,
                             const std::string& key = {},
                             boost::optional<std::string> default_value = {},
                             unsigned int flags = 0);
  void add_string_select(const std::string& text, int* value_ptr, const std::vector<std::string>& select,
                         boost::optional<int> default_value = {},
                         const std::string& key = {}, unsigned int flags = 0);
  void add_enum(const std::string& text, int* value_ptr,
                const std::vector<std::string>& labels,
                const std::vector<std::string>& symbols,
                boost::optional<int> default_value = {},
                const std::string& key = {}, unsigned int flags = 0);

  void add_sprite(const std::string& text, std::string* value_ptr,
                  const std::string& key = {},
                  boost::optional<std::string> default_value = {},
                  unsigned int flags = 0);
  void add_surface(const std::string& text, std::string* value_ptr,
                   const std::string& key = {},
                   boost::optional<std::string> default_value = {},
                   unsigned int flags = 0);
  void add_sound(const std::string& text, std::string* value_ptr,
                 const std::string& key = {},
                 boost::optional<std::string> default_value = {},
                 unsigned int flags = 0);
  void add_music(const std::string& text, std::string* value_ptr,
                 const std::string& key = {},
                 boost::optional<std::string> default_value = {},
                 unsigned int flags = 0);

  void add_worldmap(const std::string& text, std::string* value_ptr, const std::string& key = {},
                    unsigned int flags = 0);
  void add_level(const std::string& text, std::string* value_ptr, const std::string& basedir,
                 const std::string& key = {}, unsigned int flags = 0);
  void add_tiles(const std::string& text, TileMap* value_ptr, const std::string& key = {},
                 unsigned int flags = 0);
  void add_path(const std::string& text, Path* path, const std::string& key = {},
                 unsigned int flags = 0);
  void add_path_ref(const std::string& text, const std::string& path_ref, const std::string& key = {},
                    unsigned int flags = 0);
  void add_file(const std::string& text, std::string* value_ptr,
                const std::string& key = {},
                boost::optional<std::string> default_value = {},
                const std::vector<std::string>& filter = {},
                const std::string& basedir = {},
                unsigned int flags = 0);
  void add_sexp(const std::string& text, const std::string& key,
                sexp::Value& value, unsigned int flags = 0);

  const std::vector<std::unique_ptr<ObjectOption> >& get_options() const { return m_options; }

  /** Reorder the options in the given order, this is a hack to get
      saving identical to the other editor */
  void reorder(const std::vector<std::string>& order);

  /** Remove an option from the list, this is a hack */
  void remove(const std::string& key);

private:
  void add_option(std::unique_ptr<ObjectOption> option);

private:
  std::string m_name;
  std::vector<std::unique_ptr<ObjectOption> > m_options;

private:
  ObjectSettings(const ObjectSettings&) = delete;
  ObjectSettings& operator=(const ObjectSettings&) = delete;
};

#endif

/* EOF */
