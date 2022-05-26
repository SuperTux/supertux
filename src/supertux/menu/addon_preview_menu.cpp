//  SuperTux
//  Copyright (C) 2022 Vankata453
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

#include "supertux/menu/addon_preview_menu.hpp"

#include <fmt/format.h>

#include "addon/addon.hpp"
#include "addon/addon_manager.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/download_dialog.hpp"
#include "util/log.hpp"

AddonPreviewMenu::AddonPreviewMenu(const Addon& addon, const std::string addon_type, const std::string addon_string_data, const bool auto_install) :
  m_addon(addon),
  m_addon_type(addon_type),
  m_addon_string_data(addon_string_data),
  m_auto_install(auto_install),
  m_addon_manager(*AddonManager::current())
{
    
  add_label(addon_string_data);
  add_hl();
  const std::string desc = m_addon.get_description();
  const std::string license = m_addon.get_license();
  if (desc == "")
  {
    add_inactive(_("No description available."));
  }
  else
  {
    std::string desc_curr_line = "";
    for (size_t i = 0; i <= desc.size(); i++)
    {
      if (desc[i] != '\n' && i != desc.size())
      {
        desc_curr_line += desc[i];
      }
      else
      {
        add_inactive(_(desc_curr_line));
        desc_curr_line = "";
      }
    }
  }
  add_inactive("");
  add_inactive(fmt::format(fmt::runtime(_("Type: {}")), m_addon_type));
  if (license == "")
  {
    add_inactive(_("License: No license specified."));
  }
  else
  {
    add_inactive(fmt::format(fmt::runtime(_("License: {}")), license));
  }
  add_hl();
  add_entry(1, _("Download"));
  add_back(_("Back"));

  if (m_auto_install) install_addon(m_addon);
}

AddonPreviewMenu::~AddonPreviewMenu()
{
}

void
AddonPreviewMenu::menu_action(MenuItem& item)
{
  if (item.get_id() == 1) install_addon(m_addon);
}

void
AddonPreviewMenu::install_addon(const Addon& addon)
{
  auto addon_id = addon.get_id();
  TransferStatusPtr status = m_addon_manager.request_install_addon(addon_id);
  auto dialog = std::make_unique<DownloadDialog>(status, false, m_auto_install);
  dialog->set_title(fmt::format(fmt::runtime(_("Downloading {}")), m_addon_string_data));
  status->then([this, addon_id](bool success)
  {
    if (success)
    {
      try
      {
        m_addon_manager.enable_addon(addon_id);
        if (m_auto_install)
        {
          MenuManager::instance().set_dialog({});
          MenuManager::instance().clear_menu_stack();
          return;
        }
      }
      catch(const std::exception& err)
      {
        log_warning << "Enabling add-on failed: " << err.what() << std::endl;
      }
      MenuManager::instance().pop_stack();
      MenuManager::instance().current_menu() -> refresh();
    }
    else
    {
      if (m_auto_install)
      {
        MenuManager::instance().set_dialog({});
        MenuManager::instance().clear_menu_stack();
      }
    }
  });
  MenuManager::instance().set_dialog(std::move(dialog));
}

/* EOF */
