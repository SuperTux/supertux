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

AddonPreviewMenu::AddonPreviewMenu(const Addon& addon, const bool auto_install, const bool update) :
  m_addon_manager(*AddonManager::current()),
  m_addon(addon),
  m_addon_enabled(addon.is_enabled()),
  m_auto_install(auto_install),
  m_update(update)
{
  const Addon& repository_addon = m_addon_manager.get_repository_addon(m_addon.get_id());

  const std::string author = repository_addon.get_author();
  const std::string type = addon_string_util::addon_type_to_translated_string(repository_addon.get_type());
  const std::string desc = repository_addon.get_description();
  const std::string license = repository_addon.get_license();

  add_label(fmt::format(fmt::runtime(_("{} \"{}\"")), type, repository_addon.get_title()));
  add_hl();

  add_inactive(author.empty() ? _("No author specified.") : fmt::format(fmt::runtime(_("Author: {}")), author), !author.empty());
  add_inactive(fmt::format(fmt::runtime(_("Type: {}")), type), true);
  add_inactive(license.empty() ? _("No license specified.") : fmt::format(fmt::runtime(_("License: {}")), license), !license.empty());
  add_inactive("");

  add_inactive(_("Description:"), true);
  if (desc.empty())
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
        add_inactive(_(desc_curr_line), true);
        desc_curr_line = "";
      }
    }
  }

  add_hl();

  bool addon_installed = m_addon.is_installed();
  if (!addon_installed || m_update) add_entry(MNID_INSTALL, m_update ? _("Update") : _("Download"));
  if (addon_installed)
  {
    add_entry(MNID_UNINSTALL, _("Uninstall"));
    add_toggle(MNID_TOGGLE, _("Enabled"), &m_addon_enabled, true);
  }

  add_back(_("Back"));

  if (m_auto_install) install_addon();
}

AddonPreviewMenu::~AddonPreviewMenu()
{
}

void
AddonPreviewMenu::menu_action(MenuItem& item)
{
  const int index = item.get_id();
  if (index == MNID_INSTALL)
  {
    install_addon();
  }
  else if (index == MNID_UNINSTALL)
  {
    uninstall_addon();
  }
  else if (index == MNID_TOGGLE)
  {
    toggle_addon();
  }
}

void
AddonPreviewMenu::install_addon()
{
  auto addon_id = m_addon.get_id();
  TransferStatusPtr status = m_addon_manager.request_install_addon(addon_id);
  auto dialog = std::make_unique<DownloadDialog>(status, false, m_auto_install);
  const std::string action = m_update ? "Updating" : "Downloading";
  dialog->set_title(fmt::format(fmt::runtime(_("{} {}")), action, addon_string_util::generate_menu_item_text(m_addon)));
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
      if (!m_update) MenuManager::instance().pop_stack();
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

void
AddonPreviewMenu::uninstall_addon()
{
  const AddonId& addon_id = m_addon.get_id();
  auto dialog = std::make_unique<Dialog>();
  dialog->set_text(_("Uninstalling addon, please wait..."));
  MenuManager::instance().set_dialog(std::move(dialog));
  try
  {
    m_addon_manager.uninstall_addon(addon_id);
    auto success_dialog = std::make_unique<Dialog>();
    success_dialog->set_text(_("Addon uninstalled successfully."));
    success_dialog->add_cancel_button(_("OK"));
    MenuManager::instance().set_dialog(std::move(success_dialog));
    MenuManager::instance().pop_stack();
    MenuManager::instance().current_menu() -> refresh();
  }
  catch (std::exception& err)
  {
    auto failure_dialog = std::make_unique<Dialog>();
    failure_dialog->set_text(std::string("Error uninstalling addon: ") + err.what());
    failure_dialog->add_cancel_button(_("OK"));
    MenuManager::instance().set_dialog(std::move(failure_dialog));
    MenuManager::instance().pop_stack();
    MenuManager::instance().current_menu() -> refresh();
  }
}

void
AddonPreviewMenu::toggle_addon()
{
  const AddonId& addon_id = m_addon.get_id();
  if (m_addon.is_enabled())
  {
    m_addon_manager.disable_addon(addon_id);
  }
  else
  {
    m_addon_manager.enable_addon(addon_id);
  }
  if (m_addon.get_type() == Addon::LANGUAGEPACK)
  {
    Dialog::show_message(_("Please restart SuperTux\nfor these changes to take effect."));
  }
  MenuManager::instance().previous_menu() -> refresh();
}

/* EOF */
