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

#pragma once

#include <vector>
#include <memory>
#include <algorithm>

#include "editor/object_option.hpp"
#include "gui/menu_item.hpp"
#include "object/path_walker.hpp"

class Color;
enum class Direction;
class GameObject;
class MovingObject;
class PathObject;
class ReaderMapping;
enum class WalkMode;
class Writer;

namespace worldmap {
enum class Direction;
} // namespace worldmap

namespace sexp {
class Value;
} // namespace sexp

class ObjectSettings final
{
public:
  ObjectSettings(std::string name, UID uid);
  ObjectSettings(ObjectSettings&& other);
  ObjectSettings(ObjectSettings* obj);

  ObjectSettings& operator=(ObjectSettings&&) = default;

  inline const std::string& get_name() const { return m_name; }

  std::unique_ptr<BaseObjectOption>& add_bool(const std::string& text, bool* value_ptr,
                const std::string& key = {},
                const std::optional<bool>& default_value = {},
                unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_float(const std::string& text, float* value_ptr,
                 const std::string& key = {},
                 const std::optional<float>& default_value = {},
                 unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_int(const std::string& text, int* value_ptr,
               const std::string& key = {},
               const std::optional<int>& default_value = {},
               unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_label(const std::string& text, unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_worldmap_direction(const std::string& text, worldmap::Direction* value_ptr,
                              std::optional<worldmap::Direction> default_value = {},
                              const std::string& key = {}, unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_direction(const std::string& text, Direction* value_ptr,
                     std::vector<Direction> possible_directions = {},
                     const std::string& key = {}, unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_walk_mode(const std::string& text, WalkMode* value_ptr,
                     const std::optional<WalkMode>& default_value = {},
                     const std::string& key = {}, unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_objects(const std::string& text, std::vector<std::unique_ptr<GameObject>>* value_ptr,
                   uint8_t get_objects_param = 0, const std::function<void (std::unique_ptr<GameObject>)>& add_object_func = {},
                   const std::string& key = {}, unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_color(const std::string& text, Color* value_ptr,
                 const std::string& key = {},
                 const std::optional<Color>& default_value = {},
                 unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_rgba(const std::string& text, Color* value_ptr,
                const std::string& key = {},
                const std::optional<Color>& default_value = {},
                unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_rgb(const std::string& text, Color* value_ptr,
               const std::string& key = {},
               const std::optional<Color>& default_value = {},
               unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_remove();
  std::unique_ptr<BaseObjectOption>& add_script(UID uid, const std::string& text, std::string* value_ptr,
                  const std::string& key = {}, unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_text(const std::string& text, std::string* value_ptr,
                const std::string& key = {},
                const std::optional<std::string>& default_value = {},
                unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_translatable_text(const std::string& text, std::string* value_ptr,
                             const std::string& key = {},
                             const std::optional<std::string>& default_value = {},
                             unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_multiline_text(UID uid, const std::string& text, std::string* value_ptr,
                          const std::string& key = {},
                          const std::optional<std::string>& default_value = {},
                          unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_multiline_translatable_text(UID uid, const std::string& text, std::string* value_ptr,
                                       const std::string& key = {},
                                       const std::optional<std::string>& default_value = {},
                                       unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_string_select(const std::string& text, int* value_ptr, const std::vector<std::string>& select,
                         const std::optional<int>& default_value = {},
                         const std::string& key = {}, unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_enum(const std::string& text, int* value_ptr,
                const std::vector<std::string>& labels,
                const std::vector<std::string>& symbols,
                const std::optional<int>& default_value = {},
                const std::string& key = {}, unsigned int flags = 0);

  std::unique_ptr<BaseObjectOption>& add_sprite(const std::string& text, std::string* value_ptr,
                  const std::string& key = {},
                  std::optional<std::string> default_value = {},
                  unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_surface(const std::string& text, std::string* value_ptr,
                   const std::string& key = {},
                   std::optional<std::string> default_value = {},
                   unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_sound(const std::string& text, std::string* value_ptr,
                 const std::string& key = {},
                 std::optional<std::string> default_value = {},
                 unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_music(const std::string& text, std::string* value_ptr,
                 const std::string& key = {},
                 std::optional<std::string> default_value = {},
                 unsigned int flags = 0);

  std::unique_ptr<BaseObjectOption>& add_worldmap(const std::string& text, std::string* value_ptr, const std::string& key = {},
                    unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_level(const std::string& text, std::string* value_ptr, const std::string& basedir,
                 const std::string& key = {}, unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_tiles(const std::string& text, TileMap* value_ptr, const std::string& key = {},
                 unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_path(const std::string& text, Path* path, const std::string& key = {},
                 unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_path_ref(const std::string& text, PathObject& target, const std::string& path_ref,
                    const std::string& key = {}, unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_file(const std::string& text, std::string* value_ptr,
                const std::string& key = {},
                const std::optional<std::string>& default_value = {},
                const std::vector<std::string>& filter = {},
                const std::string& basedir = {},
                bool path_relative_to_basedir = true,
                unsigned int flags = 0,
                const std::function<void (MenuItem&, const std::string&, bool)> item_processor = {});
  std::unique_ptr<BaseObjectOption>& add_sexp(const std::string& text, const std::string& key,
                sexp::Value& value, unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_string_array(const std::string& text, const std::string& key, std::vector<std::string>& items);
  std::unique_ptr<BaseObjectOption>& add_test_from_here(const MovingObject* object_ptr);
  std::unique_ptr<BaseObjectOption>& add_particle_editor();
  std::unique_ptr<BaseObjectOption>& add_path_handle(const std::string& text, PathWalker::Handle& handle,
                       const std::string& key = {}, unsigned int flags = 0);
  std::unique_ptr<BaseObjectOption>& add_list(const std::string& text, const std::string& key, const std::vector<std::string>& items, std::string* value_ptr);

  inline const std::vector<std::unique_ptr<BaseObjectOption>>& get_options() const { return m_options; }

  /** Reorder the options in the given order, this is a hack to get
      saving identical to the other editor */
  void reorder(const std::vector<std::string>& order);

  /** Remove an option from the list, this is a hack */
  void remove(const std::string& key);

  /** Parse option properties. */
  void parse(const ReaderMapping& reader);

  /** Save the current states of all options. */
  void save_state();

  /** Check all options for any with a changed state. */
  bool has_state_changed() const;

  /** Parse option properties from an alternative state. */
  void parse_state(const ReaderMapping& reader);

  /** Write the old/new states of all modified options. */
  void save_old_state(std::ostream& out) const;
  void save_new_state(Writer& writer) const;

private:
  std::unique_ptr<BaseObjectOption>& add_option(std::unique_ptr<BaseObjectOption> option);

private:
  std::string m_name;
  UID m_uid;
  std::vector<std::unique_ptr<BaseObjectOption> > m_options;

private:
  ObjectSettings(const ObjectSettings&) = delete;
  ObjectSettings& operator=(const ObjectSettings&) = delete;
};
