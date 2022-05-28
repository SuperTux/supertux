//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/menu/addon_menu.hpp"

#include <fmt/format.h>

#include "addon/addon.hpp"
#include "addon/addon_manager.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/addon_browse_menu.hpp"
#include "util/log.hpp"

namespace {

#define IS_INSTALLED_MENU_ID(idx) (((idx) - MNID_ADDON_LIST_START) % 2 == 1)

#define MAKE_INSTALLED_MENU_ID(idx) (MNID_ADDON_LIST_START + 2 * (idx) + 1)

#define UNPACK_INSTALLED_MENU_ID(idx) ((((idx) - MNID_ADDON_LIST_START) - 1) / 2)

} // namespace

AddonMenu::AddonMenu(const bool language_packs_only) :
  m_addon_manager(*AddonManager::current()),
  m_installed_addons(),
  m_addons_enabled(),
  m_langpacks_only(language_packs_only)
{
  refresh();
}

AddonMenu::~AddonMenu()
{
}

void
AddonMenu::refresh()
{
  m_installed_addons = m_addon_manager.get_installed_addons();

  m_addons_enabled.reset(new bool[m_installed_addons.size()]);

  rebuild_menu();
}

void
AddonMenu::rebuild_menu()
{
  clear();
  add_label(m_langpacks_only ? _("Installed Language Packs") : _("Installed Add-ons"));
  add_hl();

  if (m_installed_addons.empty())
  {
    add_inactive(m_langpacks_only ? _("No language packs installed") : _("No Add-ons installed"));
  }
  else
  {
    bool langpacks_installed = false;
    int idx = 0;
    for (const auto& addon_id : m_installed_addons)
    {
      const Addon& addon = m_addon_manager.get_installed_addon(addon_id);
      m_addons_enabled[idx] = addon.is_enabled();
      if (addon.is_visible())
      {
        const Addon::Type addon_type = addon.get_type();
        if ((m_langpacks_only && addon_type == Addon::LANGUAGEPACK) || !m_langpacks_only)
        {
          if (addon_type == Addon::LANGUAGEPACK) langpacks_installed = true;
          std::string text = addon_string_util::generate_menu_item_text(addon);
          add_toggle(MAKE_INSTALLED_MENU_ID(idx), text, &m_addons_enabled[idx]);
        }
      }
      idx += 1;
    }
    if (!langpacks_installed && m_langpacks_only) add_inactive(_("No language packs installed"));
  }

  add_hl();
  add_entry(MNID_BROWSE, m_langpacks_only ? _("Browse language packs") : _("Browse Add-ons"));
  add_hl();
  add_back(_("Back"));
}

void
AddonMenu::menu_action(MenuItem& item)
{
  int index = item.get_id();
  if (index == MNID_BROWSE) {
    MenuManager::instance().push_menu(std::make_unique<AddonBrowseMenu>(m_langpacks_only, false));
  }
  else if (IS_INSTALLED_MENU_ID(index))
  {
    int idx = UNPACK_INSTALLED_MENU_ID(index);
    if (0 <= idx && idx < static_cast<int>(m_installed_addons.size()))
    {
      const Addon& addon = m_addon_manager.get_installed_addon(m_installed_addons[idx]);
      toggle_addon(addon);
    }
  }
  else
  {
    log_warning << "Unknown menu item clicked: " << index << std::endl;
  }
}

void
AddonMenu::toggle_addon(const Addon& addon)
{
  if (addon.is_enabled())
  {
    m_addon_manager.disable_addon(addon.get_id());
  }
  else
  {
    m_addon_manager.enable_addon(addon.get_id());
  }
  if (addon.get_type() == Addon::LANGUAGEPACK)
  {
    auto dialog = std::make_unique<Dialog>();
    dialog->set_text(_("Please restart SuperTux\nfor these changes to take effect."));
    dialog->add_cancel_button(_("OK"));
    MenuManager::instance().set_dialog(std::move(dialog));
  }
}

/* EOF */
