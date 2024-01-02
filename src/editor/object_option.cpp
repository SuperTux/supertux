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

#include "editor/object_option.hpp"

#include <string>
#include <utility>

#include <vector>
#include <sstream>

#include "editor/object_menu.hpp"
#include "gui/item_stringselect.hpp"
#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_object_select.hpp"
#include "object/tilemap.hpp"
#include "supertux/direction.hpp"
#include "supertux/moving_object.hpp"
#include "util/gettext.hpp"
#include "util/writer.hpp"
#include "video/color.hpp"

namespace {

template<typename T>
std::string fmt_to_string(const T& v)
{
  std::ostringstream out;
  out << v;
  return out.str();
}

} // namespace

BaseObjectOption::BaseObjectOption(const std::string& text, const std::string& key, unsigned int flags) :
  m_text(text),
  m_key(key),
  m_flags(flags)
{
}

template<typename T>
ObjectOption<T>::ObjectOption(const std::string& text, const std::string& key, unsigned int flags, T* pointer) :
  BaseObjectOption(text, key, flags),
  m_value_pointer(pointer)
{
}

BoolObjectOption::BoolObjectOption(const std::string& text, bool* pointer, const std::string& key,
                                   std::optional<bool> default_value,
                                   unsigned int flags) :
  ObjectOption(text, key, flags, pointer),
  m_default_value(std::move(default_value))
{
}

void
BoolObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_toggle(-1, get_text(), m_value_pointer);
}

void
BoolObjectOption::save(Writer& writer) const
{
  if (!get_key().empty()) {
    if (m_default_value && *m_default_value == *m_value_pointer) {
      // skip
    } else {
      writer.write(get_key(), *m_value_pointer);
    }
  }
}

std::string
BoolObjectOption::to_string() const
{
  return *m_value_pointer ? _("true") : _("false");
}

IntObjectOption::IntObjectOption(const std::string& text, int* pointer, const std::string& key,
                                 std::optional<int> default_value,
                                 unsigned int flags) :
  ObjectOption(text, key, flags, pointer),
  m_default_value(std::move(default_value))
{
}

void
IntObjectOption::save(Writer& writer) const
{
  if (!get_key().empty()) {
    if (m_default_value && *m_default_value == *m_value_pointer) {
      // skip
    } else {
      writer.write(get_key(), *m_value_pointer);
    }
  }
}

std::string
IntObjectOption::to_string() const
{
  return fmt_to_string(*m_value_pointer);
}

void
IntObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_intfield(get_text(), m_value_pointer);
}

LabelObjectOption::LabelObjectOption(const std::string& text,
                                 unsigned int flags) :
  ObjectOption(text, "", flags)
{
}

void
LabelObjectOption::save(Writer& writer) const
{
}

std::string
LabelObjectOption::to_string() const
{
  return "";
}

void
LabelObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_label(m_text);
}

RectfObjectOption::RectfObjectOption(const std::string& text, Rectf* pointer, const std::string& key,
                                     unsigned int flags) :
  ObjectOption(text, key, flags, pointer),
  m_width(m_value_pointer->get_width()),
  m_height(m_value_pointer->get_height())
{
}

void
RectfObjectOption::save(Writer& write) const
{
  write.write("width", m_width);
  write.write("height", m_height);
  // write.write("x", &pointer->p1.x);
  // write.write("y", &pointer->p1.y);
}

std::string
RectfObjectOption::to_string() const
{
  std::ostringstream out;
  out << *m_value_pointer;
  return out.str();
}

void
RectfObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_floatfield(_("Width"), const_cast<float*>(&m_width));
  menu.add_floatfield(_("Height"), const_cast<float*>(&m_height));
}

FloatObjectOption::FloatObjectOption(const std::string& text, float* pointer, const std::string& key,
                                     std::optional<float> default_value,
                                     unsigned int flags) :
  ObjectOption(text, key, flags, pointer),
  m_default_value(std::move(default_value))
{
}

void
FloatObjectOption::save(Writer& writer) const
{
  if (!get_key().empty()) {
    if (m_default_value && *m_default_value == *m_value_pointer) {
      // skip
    } else {
      writer.write(get_key(), *m_value_pointer);
    }
  }
}

std::string
FloatObjectOption::to_string() const
{
  return fmt_to_string(*m_value_pointer);
}

void
FloatObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_floatfield(get_text(), m_value_pointer);
}

StringObjectOption::StringObjectOption(const std::string& text, std::string* pointer, const std::string& key,
                                       std::optional<std::string> default_value,
                                       unsigned int flags) :
  ObjectOption(text, key, flags, pointer),
  m_default_value(std::move(default_value))
{
}

void
StringObjectOption::save(Writer& writer) const
{
  if (!get_key().empty()) {
    if ((m_default_value && *m_default_value == *m_value_pointer) || m_value_pointer->empty()) {
      // skip
    } else {
      writer.write(get_key(), *m_value_pointer, (get_flags() & OPTION_TRANSLATABLE));
    }
  }
}

std::string
StringObjectOption::to_string() const
{
  return *m_value_pointer;
}

void
StringObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_textfield(get_text(), m_value_pointer);
}

StringMultilineObjectOption::StringMultilineObjectOption(const std::string& text, std::string* pointer, const std::string& key,
                                       std::optional<std::string> default_value,
                                       unsigned int flags) :
  ObjectOption(text, key, flags, pointer),
  m_default_value(std::move(default_value))
{
}

void
StringMultilineObjectOption::save(Writer& writer) const
{
  if (!get_key().empty()) {
    if ((m_default_value && *m_default_value == *m_value_pointer) || m_value_pointer->empty()) {
      // skip
    } else {
      writer.write(get_key(), *m_value_pointer, (get_flags() & OPTION_TRANSLATABLE));
    }
  }
}

std::string
StringMultilineObjectOption::to_string() const
{
  if (!m_value_pointer->empty()) {
    return "...";
  }
  return "";
}

void
StringMultilineObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_script(get_text(), m_value_pointer);
}

StringSelectObjectOption::StringSelectObjectOption(const std::string& text, int* pointer,
                                                   const std::vector<std::string>& select,
                                                   std::optional<int> default_value,
                                                   const std::string& key, unsigned int flags) :
  ObjectOption(text, key, flags, pointer),
  m_select(select),
  m_default_value(std::move(default_value))
{
}

void
StringSelectObjectOption::save(Writer& writer) const
{
  if (!get_key().empty()) {
    if (m_default_value && *m_default_value == *m_value_pointer) {
      // skip
    } else {
      writer.write(get_key(), *m_value_pointer);
    }
  }
}

std::string
StringSelectObjectOption::to_string() const
{
  int* selected_id = static_cast<int*>(m_value_pointer);
  if (*selected_id >= int(m_select.size()) || *selected_id < 0) {
    return _("invalid"); //Test whether the selected ID is valid
  } else {
    return m_select[*selected_id];
  }
}

void
StringSelectObjectOption::add_to_menu(Menu& menu) const
{
  int& selected_id = *m_value_pointer;
  if ( selected_id >= static_cast<int>(m_select.size()) || selected_id < 0 ) {
    selected_id = 0; // Set the option to zero when not selectable
  }
  menu.add_string_select(-1, get_text(), m_value_pointer, m_select);
}

EnumObjectOption::EnumObjectOption(const std::string& text, int* pointer,
                                   const std::vector<std::string>& labels,
                                   const std::vector<std::string>& symbols,
                                   std::optional<int> default_value,
                                   const std::string& key, unsigned int flags) :
  ObjectOption(text, key, flags, pointer),
  m_labels(labels),
  m_symbols(symbols),
  m_default_value(std::move(default_value))
{
}

void
EnumObjectOption::save(Writer& writer) const
{
  if (0 <= *m_value_pointer && *m_value_pointer < int(m_symbols.size()) &&
      !get_key().empty())
  {
    if (m_default_value && *m_default_value == *m_value_pointer) {
      // skip
    } else {
      writer.write(get_key(), m_symbols[*m_value_pointer]);
    }
  }
}

std::string
EnumObjectOption::to_string() const
{
  if (0 <= *m_value_pointer && *m_value_pointer < int(m_labels.size())) {
    return m_labels[*m_value_pointer];
  } else {
    return _("invalid");
  }
}

void
EnumObjectOption::add_to_menu(Menu& menu) const
{
  if (*m_value_pointer >= static_cast<int>(m_labels.size()) || *m_value_pointer < 0 ) {
    *m_value_pointer = 0; // Set the option to zero when not selectable
  }
  menu.add_string_select(-1, get_text(), m_value_pointer, m_labels);
}


ScriptObjectOption::ScriptObjectOption(const std::string& text, std::string* pointer, const std::string& key,
                                       unsigned int flags) :
  ObjectOption(text, key, flags, pointer)
{
}

void
ScriptObjectOption::save(Writer& writer) const
{
  auto& value = *m_value_pointer;
  if (!value.empty())
  {
    if (!get_key().empty()) {
      writer.write(get_key(), value);
    }
  }
}

std::string
ScriptObjectOption::to_string() const
{
  if (!m_value_pointer->empty()) {
    return "...";
  }
  return "";
}

void
ScriptObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_script(get_text(), m_value_pointer);
}

FileObjectOption::FileObjectOption(const std::string& text, std::string* pointer,
                                   std::optional<std::string> default_value,
                                   const std::string& key,
                                   std::vector<std::string> filter,
                                   const std::string& basedir,
                                   bool path_relative_to_basedir,
                                   unsigned int flags) :
  ObjectOption(text, key, flags, pointer),
  m_default_value(std::move(default_value)),
  m_filter(std::move(filter)),
  m_basedir(basedir),
  m_path_relative_to_basedir(path_relative_to_basedir)
{
}

void
FileObjectOption::save(Writer& writer) const
{
  if (m_default_value && *m_default_value == *m_value_pointer) {
    // skip
  } else {
    auto& value = *m_value_pointer;
    if (!value.empty())
    {
      if (!get_key().empty()) {
        writer.write(get_key(), value);
      }
    }
  }
}

std::string
FileObjectOption::to_string() const
{
  return *m_value_pointer;
}

void
FileObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_file(get_text(), m_value_pointer, m_filter, m_basedir, m_path_relative_to_basedir);
}

ColorObjectOption::ColorObjectOption(const std::string& text, Color* pointer, const std::string& key,
                                     std::optional<Color> default_value, bool use_alpha,
                                     unsigned int flags) :
  ObjectOption(text, key, flags, pointer),
  m_default_value(std::move(default_value)),
  m_use_alpha(use_alpha)
{
}

void
ColorObjectOption::save(Writer& writer) const
{
  if (!get_key().empty()) {
    if (m_default_value && *m_default_value == *m_value_pointer) {
      // skip
    } else {
      auto vec = m_value_pointer->toVector();
      if (!m_use_alpha || vec.back() == 1.0f) {
        vec.pop_back();
      }
      writer.write(get_key(), vec);
    }
  }
}

std::string
ColorObjectOption::to_string() const
{
  return m_value_pointer->to_string();
}

void
ColorObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_color(get_text(), m_value_pointer);
}

ObjectSelectObjectOption::ObjectSelectObjectOption(const std::string& text, std::vector<std::unique_ptr<GameObject>>* pointer,
                                                   uint8_t get_objects_param, const std::function<void (std::unique_ptr<GameObject>)>& add_object_func,
                                                   const std::string& key, unsigned int flags) :
  ObjectOption(text, key, flags, pointer),
  m_get_objects_param(get_objects_param),
  m_add_object_function(add_object_func)
{
}

void
ObjectSelectObjectOption::save(Writer& writer) const
{
  if (get_key().empty())
    return;

  writer.start_list(get_key());
  for (auto it = m_value_pointer->begin(); it != m_value_pointer->end(); it++)
  {
    auto& obj = *it;
    writer.start_list(obj->get_class_name());

    // Rectangle properties should not be saved.
    auto settings = obj->get_settings();
    settings.remove("region");
    settings.remove("x");
    settings.remove("y");

    for (const auto& option : settings.get_options())
      option->save(writer);

    writer.end_list(obj->get_class_name());
  }
  writer.end_list(get_key());
}

std::string
ObjectSelectObjectOption::to_string() const
{
  return fmt_to_string(m_value_pointer->size());
}

void
ObjectSelectObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_entry(get_text(), [pointer = m_value_pointer, get_objects_param = m_get_objects_param,
                              add_object_func = m_add_object_function]() {
    MenuManager::instance().push_menu(std::make_unique<ObjectSelectMenu>(*pointer, get_objects_param, add_object_func));
  });
}

TilesObjectOption::TilesObjectOption(const std::string& text, TileMap* tilemap, const std::string& key,
                                     unsigned int flags) :
  ObjectOption(text, key, flags),
  m_tilemap(tilemap)
{
}

void
TilesObjectOption::save(Writer& write) const
{
  write.write("width", m_tilemap->get_width());
  write.write("height", m_tilemap->get_height());
  write.write("tiles", m_tilemap->get_tiles(), m_tilemap->get_width());
}

std::string
TilesObjectOption::to_string() const
{
  return {};
}

void
TilesObjectOption::add_to_menu(Menu& menu) const
{
}

PathObjectOption::PathObjectOption(const std::string& text, Path* path, const std::string& key,
                                   unsigned int flags) :
  ObjectOption(text, key, flags, path)
{
}

void
PathObjectOption::save(Writer& write) const
{
  m_value_pointer->save(write);
}

std::string
PathObjectOption::to_string() const
{
  return {};
}

void
PathObjectOption::add_to_menu(Menu& menu) const
{
}

PathRefObjectOption::PathRefObjectOption(const std::string& text, PathObject& target, const std::string& path_ref,
                                         const std::string& key, unsigned int flags) :
  ObjectOption(text, key, flags, &target),
  m_path_ref(path_ref)
{
}

void
PathRefObjectOption::save(Writer& writer) const
{
  if (!m_path_ref.empty()) {
    writer.write(get_key(), m_path_ref);
  }
}

std::string
PathRefObjectOption::to_string() const
{
  return m_path_ref;
}

void
PathRefObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_path_settings(m_text, *m_value_pointer, m_path_ref);
}

SExpObjectOption::SExpObjectOption(const std::string& text, const std::string& key, sexp::Value& value,
                                   unsigned int flags) :
  ObjectOption(text, key, flags, &value)
{
}

void
SExpObjectOption::save(Writer& writer) const
{
  if (!m_value_pointer->is_nil()) {
    writer.write(get_key(), *m_value_pointer);
  }
}

std::string
SExpObjectOption::to_string() const
{
  return m_value_pointer->str();
}

void
SExpObjectOption::add_to_menu(Menu& menu) const
{
}

PathHandleOption::PathHandleOption(const std::string& text, PathWalker::Handle& handle,
                                   const std::string& key, unsigned int flags) :
  ObjectOption(text, key, flags),
  m_target(handle)
{
}

void
PathHandleOption::save(Writer& writer) const
{
  writer.start_list(get_key());
  writer.write("scale_x", m_target.m_scalar_pos.x);
  writer.write("scale_y", m_target.m_scalar_pos.y);
  writer.write("offset_x", m_target.m_pixel_offset.x);
  writer.write("offset_y", m_target.m_pixel_offset.y);
  writer.end_list(get_key());
}

std::string
PathHandleOption::to_string() const
{
  return "("
        + std::to_string(m_target.m_scalar_pos.x) + ", "
        + std::to_string(m_target.m_scalar_pos.y) + "), ("
        + std::to_string(m_target.m_pixel_offset.x) + ", "
        + std::to_string(m_target.m_pixel_offset.y) + ")";
}

void
PathHandleOption::add_to_menu(Menu& menu) const
{
  menu.add_floatfield(get_text() + " (" + _("Scale X") + ")", &m_target.m_scalar_pos.x);
  menu.add_floatfield(get_text() + " (" + _("Scale Y") + ")", &m_target.m_scalar_pos.y);
  menu.add_floatfield(get_text() + " (" + _("Offset X") + ")", &m_target.m_pixel_offset.x);
  menu.add_floatfield(get_text() + " (" + _("Offset Y") + ")", &m_target.m_pixel_offset.y);
}

RemoveObjectOption::RemoveObjectOption() :
  ObjectOption(_("Remove"), "", 0)
{
}

std::string
RemoveObjectOption::to_string() const
{
  return {};
}

void
RemoveObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_entry(ObjectMenu::MNID_REMOVE, get_text());
}

TestFromHereOption::TestFromHereOption() :
  ObjectOption(_("Test from here"), "", 0)
{
}

std::string
TestFromHereOption::to_string() const
{
  return {};
}

void
TestFromHereOption::add_to_menu(Menu& menu) const
{
  menu.add_entry(ObjectMenu::MNID_TEST_FROM_HERE, get_text());
}

ParticleEditorOption::ParticleEditorOption() :
  ObjectOption(_("Open Particle Editor"), "", 0)
{
}

std::string
ParticleEditorOption::to_string() const
{
  return {};
}

void
ParticleEditorOption::add_to_menu(Menu& menu) const
{
  menu.add_entry(ObjectMenu::MNID_OPEN_PARTICLE_EDITOR, get_text());
}

ButtonOption::ButtonOption(const std::string& text, std::function<void()> callback) :
  ObjectOption(text, "", 0),
  m_callback(std::move(callback))
{
}

std::string
ButtonOption::to_string() const
{
  return {};
}

void
ButtonOption::add_to_menu(Menu& menu) const
{
  menu.add_entry(get_text(), m_callback);
}

StringArrayOption::StringArrayOption(const std::string& text, const std::string& key, std::vector<std::string>& items) :
  ObjectOption(text, key, 0),
  m_items(items)
{}

void
StringArrayOption::save(Writer& write) const
{
  write.write("strings", m_items);
}

void
StringArrayOption::add_to_menu(Menu& menu) const
{
  menu.add_string_array(get_text(), m_items);
}

ListOption::ListOption(const std::string& text, const std::string& key, const std::vector<std::string>& items, std::string* value_ptr) :
  ObjectOption(text, key, 0, value_ptr),
  m_items(items)
{}

void
ListOption::save(Writer& writer) const
{
  writer.write(get_key(), *m_value_pointer);
}

void
ListOption::add_to_menu(Menu& menu) const
{
  menu.add_list(get_text(), m_items, m_value_pointer);
}

DirectionOption::DirectionOption(const std::string& text, Direction* value_ptr,
                                 std::vector<Direction> possible_directions,
                                 const std::string& key, unsigned int flags) :
  ObjectOption(text, key, flags, value_ptr),
  m_possible_directions(std::move(possible_directions))
{
  if (m_possible_directions.empty())
    m_possible_directions = { Direction::AUTO, Direction::NONE, Direction::LEFT,
                              Direction::RIGHT, Direction::UP, Direction::DOWN };
}

void
DirectionOption::save(Writer& writer) const
{
  if (*m_value_pointer == m_possible_directions.at(0))
    return;

  writer.write(get_key(), dir_to_string(*m_value_pointer));
}

std::string
DirectionOption::to_string() const
{
  return dir_to_translated_string(*m_value_pointer);
}

void
DirectionOption::add_to_menu(Menu& menu) const
{
  int selected = 0;
  std::vector<std::string> labels;
  for (size_t i = 0; i < m_possible_directions.size(); i++)
  {
    const auto& dir = m_possible_directions.at(i);
    labels.push_back(dir_to_translated_string(dir));

    if (dir == *m_value_pointer)
      selected = static_cast<int>(i);
  }

  menu.add_string_select(-1, get_text(), selected, labels)
    .set_callback([value_ptr = m_value_pointer, possible_directions = m_possible_directions](int index) {
                    *value_ptr = possible_directions.at(index);
                  });
}

/* EOF */
