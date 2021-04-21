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
#include "supertux/menu/download_dialog.hpp"
#include "util/log.hpp"

namespace {

#define IS_REPOSITORY_MENU_ID(idx) (((idx) - MNID_ADDON_LIST_START) % 2 == 0)
#define IS_INSTALLED_MENU_ID(idx) (((idx) - MNID_ADDON_LIST_START) % 2 == 1)

#define MAKE_REPOSITORY_MENU_ID(idx) (MNID_ADDON_LIST_START + 2 * (idx) + 0)
#define MAKE_INSTALLED_MENU_ID(idx) (MNID_ADDON_LIST_START + 2 * (idx) + 1)

#define UNPACK_REPOSITORY_MENU_ID(idx) ((((idx) - MNID_ADDON_LIST_START) - 0) / 2)
#define UNPACK_INSTALLED_MENU_ID(idx) ((((idx) - MNID_ADDON_LIST_START) - 1) / 2)

std::string addon_type_to_translated_string(Addon::Type type)
{
  switch (type)
  {
    case Addon::LEVELSET:
      return _("Levelset");

    case Addon::WORLDMAP:
      return _("Worldmap");

    case Addon::WORLD:
      return _("World");
	  
	case Addon::ADDON:
      return _("Add-on");

    case Addon::LANGUAGEPACK:
      return "";

    default:
      return _("Unknown");
  }
}

std::string generate_menu_item_text(const Addon& addon)
{
  std::string text;
  std::string type = addon_type_to_translated_string(addon.get_type());

  if (!addon.get_author().empty())
  {
    text = fmt::format(_("{} \"{}\" by \"{}\""),
                       type, addon.get_title(), addon.get_author());
  }
  else
  {
    // Only addon type and name, no need for translation.
    text = fmt::format("{} \"{}\"", type, addon.get_title());
  }

  return text;
}

} // namespace

AddonMenu::AddonMenu(bool auto_install_langpack) :
  m_addon_manager(*AddonManager::current()),
  m_installed_addons(),
  m_repository_addons(),
  m_addons_enabled(),
  m_auto_install_langpack(auto_install_langpack)
{
  refresh();
  if (auto_install_langpack)
  {
    const std::string& language = g_dictionary_manager->get_language().get_language();
    if (language == "en")
      return;
    check_online();
  }
}

AddonMenu::~AddonMenu()
{
}

void
AddonMenu::refresh()
{
  m_installed_addons = m_addon_manager.get_installed_addons();
  m_repository_addons = m_addon_manager.get_repository_addons();

  m_addons_enabled.reset(new bool[m_installed_addons.size()]);

  rebuild_menu();
}

void
AddonMenu::rebuild_menu()
{
  clear();
  add_label(_("Add-ons"));
  add_hl();

  if (m_installed_addons.empty())
  {
    if (!m_repository_addons.empty())
    {
      add_inactive(_("No Add-ons installed"));
    }
    else
    {
      add_inactive(_("No Add-ons found"));
    }
  }
  else
  {
    int idx = 0;
    for (const auto& addon_id : m_installed_addons)
    {
      const Addon& addon = m_addon_manager.get_installed_addon(addon_id);
      m_addons_enabled[idx] = addon.is_enabled();
      if (addon_visible(addon))
      {
        std::string text = generate_menu_item_text(addon);
        add_toggle(MAKE_INSTALLED_MENU_ID(idx), text, &m_addons_enabled[idx]);
      }
      idx += 1;
    }
  }

  add_hl();

  {
    bool have_new_stuff = false;
    int idx = 0;
    for (const auto& addon_id : m_repository_addons)
    {
      const Addon& addon = m_addon_manager.get_repository_addon(addon_id);

      try
      {
        // addon is already installed, so check if they are the same
        Addon& installed_addon = m_addon_manager.get_installed_addon(addon_id);
        if (installed_addon.get_md5() == addon.get_md5() ||
            installed_addon.get_version() > addon.get_version())
        {
          log_debug << "ignoring already installed add-on " << installed_addon.get_id() << std::endl;
        }
        else
        {
          log_debug << installed_addon.get_id() << " is installed, but updated: '"
                    << installed_addon.get_md5() << "' vs '" << addon.get_md5() << "'  '"
                    << installed_addon.get_version() << "' vs '" << addon.get_version() << "'"
                    << std::endl;
          if (addon_visible(addon))
          {
            std::string text = generate_menu_item_text(addon);
            add_entry(MAKE_REPOSITORY_MENU_ID(idx), fmt::format( _("Install %s *NEW*"), text));
            have_new_stuff = true;
          }
        }
      }
      catch(const std::exception&)
      {
        // addon is not installed
        if (addon_visible(addon))
        {
          std::string text = generate_menu_item_text(addon);
          add_entry(MAKE_REPOSITORY_MENU_ID(idx), fmt::format( _("Install {}"), text));
          have_new_stuff = true;
        }
      }
      idx += 1;
    }

    if (!have_new_stuff && m_addon_manager.has_been_updated())
    {
      add_inactive(_("No new Add-ons found"));
    }
  }

  if (!m_addon_manager.has_online_support())
  {
    add_inactive(_("Check Online (disabled)"));
  }
  else
  {
    add_entry(MNID_CHECK_ONLINE, _("Check Online"));
  }

  add_hl();
  add_back(_("Back"));
}

void
AddonMenu::menu_action(MenuItem& item)
{
  if (item.get_id() == MNID_CHECK_ONLINE) // check if "Check Online" was chosen
  {
    check_online();
  }
  else if (MNID_ADDON_LIST_START <= item.get_id())
  {
    if (IS_INSTALLED_MENU_ID(item.get_id()))
    {
      int idx = UNPACK_INSTALLED_MENU_ID(item.get_id());
      if (0 <= idx && idx < static_cast<int>(m_installed_addons.size()))
      {
        const Addon& addon = m_addon_manager.get_installed_addon(m_installed_addons[idx]);
        toggle_addon(addon);
      }
    }
    else if (IS_REPOSITORY_MENU_ID(item.get_id()))
    {
      int idx = UNPACK_REPOSITORY_MENU_ID(item.get_id());
      if (0 <= idx && idx < static_cast<int>(m_repository_addons.size()))
      {
        const Addon& addon = m_addon_manager.get_repository_addon(m_repository_addons[idx]);
        install_addon(addon);
      }
    }
  }
  else
  {
    log_warning << "Unknown menu item clicked: " << item.get_id() << std::endl;
  }
}

bool
AddonMenu::addon_visible(const Addon& addon) const
{
  return true;
}

void
AddonMenu::check_online()
{
  try
  {
    TransferStatusPtr status = m_addon_manager.request_check_online();
    status->then([this](bool success)
    {
      if (success)
      {
        if (m_auto_install_langpack)
        {
          const std::string& langpack_id = "language-pack";
          install_addon(m_addon_manager.get_repository_addon(langpack_id));
        }
        else
        {
          refresh();
        }
      }
      else
      {
        if (m_auto_install_langpack)
        {
          MenuManager::instance().set_dialog({});
          MenuManager::instance().clear_menu_stack();
        }
      }
    });
    auto dialog = std::make_unique<DownloadDialog>(status, false, m_auto_install_langpack);
    dialog->set_title(_("Downloading Add-On Repository Index"));
    MenuManager::instance().set_dialog(std::move(dialog));
  }
  catch (std::exception& e)
  {
    log_warning << "Check for available Add-ons failed: " << e.what() << std::endl;
  }
}

void
AddonMenu::install_addon(const Addon& addon)
{
  auto addon_id = addon.get_id();
  TransferStatusPtr status = m_addon_manager.request_install_addon(addon_id);
  auto dialog = std::make_unique<DownloadDialog>(status, false, m_auto_install_langpack);
  dialog->set_title(fmt::format( _("Downloading {}"), generate_menu_item_text(addon)));
  status->then([this, addon_id](bool success)
  {
    if (success)
    {
      try
      {
        m_addon_manager.enable_addon(addon_id);
        if (m_auto_install_langpack)
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
      refresh();
    }
    else
    {
      if (m_auto_install_langpack)
      {
        MenuManager::instance().set_dialog({});
        MenuManager::instance().clear_menu_stack();
      }
    }
  });
  MenuManager::instance().set_dialog(std::move(dialog));
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
