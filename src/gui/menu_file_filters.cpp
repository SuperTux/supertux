//  SuperTux
//  Copyright (C) 2025 bruhmoent
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

#include "gui/dialog.hpp"
#include "gui/item_textfield.hpp"
#include "gui/menu_file_filters.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "util/string_util.hpp"

namespace
{
  const std::vector<std::string> IMAGE_FILES = { ".jpg", ".png" };
  const std::vector<std::string> SPRITE_FILES = { ".sprite" };
  const std::vector<std::string> AUDIO_FILES = { ".wav", ".ogg" };
}

MenuFileFilters::MenuFileFilters(FileSystemMenu::MenuParams* params, std::function<void()> on_apply_callback)
  : m_original_params(params),
  m_on_apply_callback(on_apply_callback),
  m_filename_filter(params->filename_filter),
  m_active_filter_type(FilterType::NONE),
  m_initial_filename_filter(params->filename_filter),
  m_initial_additional_extensions(params->additional_extensions)
{
  for (const auto& ext : m_original_params->additional_extensions)
  {
    m_custom_extensions_str += ext + " ";
  }
  if (!m_custom_extensions_str.empty())
  {
    m_custom_extensions_str.pop_back();
  }

  add_label(_("Filters"));
  add_hl();

  add_textfield(_("File Name contains"), &m_filename_filter);

  std::string allowed_extensions_str = _("Allowed extensions ");
  if (m_original_params->extensions.empty())
  {
    allowed_extensions_str += _("Any");
  }
  else
  {
    for (const auto& ext : m_original_params->extensions)
    {
      allowed_extensions_str += ext + " ";
    }
    allowed_extensions_str.pop_back();
  }
  add_inactive(allowed_extensions_str);

  add_string_select(MNID_FILTER_TYPE_CHANGED, _("Filter by extensions"), reinterpret_cast<int*>(&m_active_filter_type),
                    { _("None (use allowed)"), _("Image Files"), _("Sprite Files"), _("Audio Files"), _("Custom") });

  add_textfield(_("Custom"), &m_custom_extensions_str);

  add_hl();
  add_entry(MNID_APPLY, _("Apply"));
  add_entry(MNID_RESET, _("Reset"));
  add_hl();
  add_back(_("Back"));
}

bool
MenuFileFilters::is_sensitive() const
{
  if (m_active_item < 0 || static_cast<size_t>(m_active_item) >= m_items.size())
  {
    return false;
  }

  const MenuItem* active_item = m_items[m_active_item].get();

  return (dynamic_cast<const ItemTextField*>(active_item) != nullptr);
}

void
MenuFileFilters::menu_action(MenuItem& item)
{
  switch (item.get_id())
  {
    case MNID_APPLY:
      if (apply_changes())
      {
        MenuManager::instance().pop_menu();
      }
      break;
    case MNID_RESET:
      reset_to_defaults();
      break;
    case MNID_FILTER_TYPE_CHANGED:
    default:
      break;
  }
}

bool
is_subset(const std::vector<std::string>& subset, const std::vector<std::string>& superset)
{
  if (superset.empty())
  {
    return true;
  }
  return std::all_of(subset.begin(), subset.end(), [&](const std::string& item)
    {
    return std::find(superset.begin(), superset.end(), item) != superset.end();
    });
}

bool
MenuFileFilters::apply_changes()
{
  m_original_params->filename_filter = m_filename_filter;

  std::vector<std::string> new_filter_extensions;
  switch (m_active_filter_type)
  {
    case FilterType::IMAGE:
        new_filter_extensions = IMAGE_FILES;
        break;
      case FilterType::SPRITE:
        new_filter_extensions = SPRITE_FILES;
        break;
      case FilterType::AUDIO:
        new_filter_extensions = AUDIO_FILES;
        break;
      case FilterType::CUSTOM:
        if (!m_custom_extensions_str.empty())
        {
          StringUtil::split(new_filter_extensions, m_custom_extensions_str, ' ');
        }
        break;
      case FilterType::NONE:
      default:
        break;
  }

  if (!is_subset(new_filter_extensions, m_original_params->extensions))
  {
    Dialog::show_message(_("Selected filter extensions are not allowed."));
    return false;
  }

  m_original_params->additional_extensions = new_filter_extensions;

  if (m_on_apply_callback)
  {
    m_on_apply_callback();
  }
  return true;
}

bool
MenuFileFilters::on_back_action()
{
  reset_to_defaults();

  if (m_on_apply_callback)
  {
    m_on_apply_callback();
  }

  return true;
}

void
MenuFileFilters::reset_to_defaults()
{
  m_original_params->filename_filter = m_initial_filename_filter;
  m_original_params->additional_extensions = m_initial_additional_extensions;

  m_filename_filter = "";
  m_active_filter_type = FilterType::NONE;

  m_custom_extensions_str.clear();
  for (const auto& ext : m_original_params->extensions)
  {
    m_custom_extensions_str += ext + " ";
  }
  if (!m_custom_extensions_str.empty())
  {
    m_custom_extensions_str.pop_back();
  }
}

/* EOF */
