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
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/download_dialog.hpp"
#include "util/log.hpp"

AddonPreviewMenu::AddonPreviewMenu(const Addon& addon, bool auto_install, bool update) :
  m_addon_manager(*AddonManager::current()),
  m_screenshot_manager(addon.get_id()),
  m_addon(addon),
  m_addon_enabled(addon.is_enabled()),
  m_auto_install(auto_install),
  m_update(update),
  m_screenshots(),
  m_show_screenshots(false)
{
  rebuild_menu();
}

AddonPreviewMenu::~AddonPreviewMenu()
{
}

void
AddonPreviewMenu::rebuild_menu()
{
  std::string author;
  std::string type;
  std::string desc;
  std::string license;
  bool screenshots_available = false;
  bool info_unavailable = false;
  try
  {
    const Addon& repository_addon = m_addon_manager.get_repository_addon(m_addon.get_id());
    author = repository_addon.get_author();
    type = addon_string_util::addon_type_to_translated_string(repository_addon.get_type());
    desc = repository_addon.get_description();
    license = repository_addon.get_license();
    screenshots_available = repository_addon.get_screenshots().size() > 0;
  }
  catch (std::exception& err)
  {
    log_warning << "Installed addon not available in repository: " << err.what() << std::endl;
    author = m_addon.get_author();
    type = addon_string_util::addon_type_to_translated_string(m_addon.get_type());
    license = m_addon.get_license();
    info_unavailable = true;
  }

  clear();

  add_label(fmt::format(fmt::runtime(_("{} \"{}\"")), type, m_addon.get_title()));
  add_hl();

  if (info_unavailable)
  {
    add_inactive(_("Some information about this add-on is not available."));
    add_inactive(_("Perform a \"Check Online\" to try retrieving it."));
    add_hl();
  }

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
    for (std::size_t i = 0; i <= desc.size(); i++)
    {
      if (desc[i] != '\n' && i != desc.size())
      {
        desc_curr_line += desc[i];
      }
      else
      {
        add_inactive(desc_curr_line, true);
        desc_curr_line = "";
      }
    }
  }
  add_inactive("");

  if (m_screenshots.size() > 0 || screenshots_available)
  {
    if (m_show_screenshots)
    {
      for (std::string path : m_screenshots)
      {
        add_inactive(path); // For testing purposes.
      }
    }
    else
    {
      const std::string show_screenshots_text = _("Show screenshots");
      if (m_auto_install)
      {
        add_inactive(show_screenshots_text);
      }
      else
      {
        add_entry(MNID_SHOW_SCREENSHOTS, show_screenshots_text);
      }
    }
  }
  else
  {
    add_inactive(_("No screenshot previews available."));
  }

  add_hl();

  bool addon_installed = m_addon.is_installed();
  if ((!addon_installed || m_update) && !info_unavailable)
  {
    const std::string action = m_update ? _("Update") : _("Download");
    if (m_auto_install)
    {
      add_inactive(action);
    }
    else
    {
      add_entry(MNID_INSTALL, action);
    }
  }
  if (addon_installed)
  {
    add_toggle(MNID_TOGGLE, _("Enabled"), &m_addon_enabled, true);
    add_entry(MNID_UNINSTALL, _("Uninstall"));
  }

  add_back(_("Back"));

  if (m_auto_install) install_addon();
}

void
AddonPreviewMenu::menu_action(MenuItem& item)
{
  const int index = item.get_id();
  if (index == MNID_SHOW_SCREENSHOTS)
  {
    m_show_screenshots = true;
    auto dialog = std::make_unique<ScreenshotDownloadDialog>(m_screenshot_manager, true);
    dialog->set_text(_("Fetching screenshot previews..."));
    MenuManager::instance().set_dialog(std::move(dialog));
    m_screenshot_manager.request_download_all([this](std::vector<std::string> local_screenshot_urls)
    {
      log_warning << "Fetching screenshots for add-on \"" << m_addon.get_id() << "\" finished." << std::endl;
      m_screenshots = local_screenshot_urls;
      MenuManager::instance().set_dialog({});
      rebuild_menu();
    });
  }
  else if (index == MNID_INSTALL)
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
  else
  {
    log_warning << "Unknown menu item clicked: " << index << std::endl;
  }
}

void
AddonPreviewMenu::install_addon()
{
  auto addon_id = m_addon.get_id();
  TransferStatusPtr status = m_addon_manager.request_install_addon(addon_id);
  auto dialog = std::make_unique<DownloadDialog>(status, false, m_auto_install);
  const std::string action = m_update ? _("Updating") : _("Downloading");
  const Addon& repository_addon = m_addon_manager.get_repository_addon(addon_id);
  dialog->set_title(fmt::format(fmt::runtime("{} {}"), action, addon_string_util::generate_menu_item_text(repository_addon)));
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
      MenuManager::instance().pop_menu(true);
      if (!m_update) MenuManager::instance().pop_menu(true);
      MenuManager::instance().current_menu()->refresh();
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
    Dialog::show_message(_("Addon uninstalled successfully."));
    MenuManager::instance().pop_menu(true);
    MenuManager::instance().current_menu()->refresh();
  }
  catch (std::exception& err)
  {
    Dialog::show_message(fmt::format(fmt::runtime(_("Error uninstalling addon:\n{}")), err.what()));
    MenuManager::instance().pop_menu(true);
    MenuManager::instance().current_menu()->refresh();
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
  MenuManager::instance().previous_menu()->refresh();
}

ScreenshotDownloadDialog::ScreenshotDownloadDialog(AddonScreenshotManager& screenshot_manager, bool passive) :
  Dialog(passive),
  m_screenshot_manager(screenshot_manager)
{
}

ScreenshotDownloadDialog::~ScreenshotDownloadDialog()
{
}

void
ScreenshotDownloadDialog::update()
{
  m_screenshot_manager.update();
}

/* EOF */
