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

#include "editor/object_settings.hpp"

#include <assert.h>

#include "video/color.hpp"

ObjectSettings::ObjectSettings(const std::string& name) :
  m_name(name),
  m_options()
{
}

void
ObjectSettings::copy_from(const ObjectSettings& other)
{
  auto it1 = m_options.begin();
  auto it2 = other.m_options.begin();
  while (it1 != m_options.end() && it2 != other.m_options.end()) {
    auto oo1 = &*it1;
    auto oo2 = &*it2;

    switch (oo1->m_type)
    {
      case MN_TEXTFIELD:
      case MN_SCRIPT:
      case MN_FILE:
        assert(oo1->m_type == oo2->m_type);
        *(static_cast<std::string*>(oo1->m_option)) = *(static_cast<std::string*>(oo2->m_option));
        break;

      case MN_FLOATFIELD:
        assert(oo1->m_type == oo2->m_type);
        *(static_cast<float*>(oo1->m_option)) = *(static_cast<float*>(oo2->m_option));
        break;

      case MN_INTFIELD:
      case MN_STRINGSELECT:
        assert(oo1->m_type == oo2->m_type);
        *(static_cast<int*>(oo1->m_option)) = *(static_cast<int*>(oo2->m_option));
        break;

      case MN_TOGGLE:
        assert(oo1->m_type == oo2->m_type);
        *(static_cast<bool*>(oo1->m_option)) = *(static_cast<bool*>(oo2->m_option));
        break;

      case MN_BADGUYSELECT:
        assert(oo1->m_type == oo2->m_type);
        *(static_cast<std::vector<std::string>*>(oo1->m_option)) = *(static_cast<std::vector<std::string>*>(oo2->m_option));
        break;

      case MN_COLOR:
        assert(oo1->m_type == oo2->m_type);
        *(static_cast<Color*>(oo1->m_option)) = *(static_cast<Color*>(oo2->m_option));
        break;

      default:
        //Do not assert here!
        break;
    }

    it1++;
    it2++;
  }
}

void
ObjectSettings::add_option(const ObjectOption& option)
{
  m_options.push_back(option);
}

void
ObjectSettings::add_badguy(const std::string& text, std::vector<std::string>* value_ptr,
                           const std::string& key, int flags)
{
  add(MN_BADGUYSELECT, text, value_ptr, key, flags);
}

void
ObjectSettings::add_color(const std::string& text, Color* value_ptr,
                          const std::string& key, int flags)
{
  add(MN_COLOR, text, value_ptr, key, flags);
}

void
ObjectSettings::add_bool(const std::string& text, bool* value_ptr,
                         const std::string& key, int flags)
{
  add(MN_TOGGLE, text, value_ptr, key, flags);
}

void
ObjectSettings::add_float(const std::string& text, float* value_ptr,
                          const std::string& key, int flags)
{
  add(MN_FLOATFIELD, text, value_ptr, key, flags);
}

void
ObjectSettings::add_int(const std::string& text, int* value_ptr,
                        const std::string& key, int flags)
{
  add(MN_INTFIELD, text, value_ptr, key, flags);
}

void
ObjectSettings::add_remove()
{
  add(MN_REMOVE, "", nullptr);
}

void
ObjectSettings::add_script(const std::string& text, std::string* value_ptr,
                           const std::string& key, int flags)
{
  add(MN_SCRIPT, text, value_ptr, key, flags);
}

void
ObjectSettings::add_text(const std::string& text, std::string* value_ptr,
                         const std::string& key, int flags)
{
  add(MN_TEXTFIELD, text, value_ptr, key, flags);
}

void
ObjectSettings::add_file(const std::string& text, std::string* value_ptr, const std::string& key,
                         const std::vector<std::string>& filter, int flags)
{
  ObjectOption option(MN_FILE, text, value_ptr);
  option.m_select = filter;
  add_option(option);
}

void
ObjectSettings::add_level(const std::string& text, std::string* value_ptr, const std::string& key,
                          int flags)
{
  add_file(text, value_ptr, key, {".stl"}, flags);
}

void
ObjectSettings::add_sprite(const std::string& text, std::string* value_ptr, const std::string& key,
                           int flags)
{
  add_file(text, value_ptr, key, {".png", ".sprite"}, flags);
}

void
ObjectSettings::add_surface(const std::string& text, std::string* value_ptr, const std::string& key,
                            int flags)
{
  add_file(text, value_ptr, key, {".png", ".surface"}, flags);
}

void
ObjectSettings::add_sound(const std::string& text, std::string* value_ptr, const std::string& key,
                          int flags)
{
  add_file(text, value_ptr, key, {".wav", ".ogg"}, flags);
}

void
ObjectSettings::add_music(const std::string& text, std::string* value_ptr, const std::string& key,
                          int flags)
{
  add_file(text, value_ptr, key, {".ogg", ".music"}, flags);
}

void
ObjectSettings::add_worldmap(const std::string& text, std::string* value_ptr, const std::string& key,
                            int flags)
{
  add_file(text, value_ptr, key, {".stwm"}, flags);
}

/* EOF */
