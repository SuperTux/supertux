//  SuperTux
//  Copyright (C) 2018 christ2go <christian@hagemeier.ch>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_ADDON_GALLERY_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_ADDON_GALLERY_HPP

#include "gui/menu.hpp"
#include "addon/downloader.hpp"
#include "addon/addon.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include <boost/format.hpp>
#include "addon/addon_manager.hpp"
#include "util/gettext.hpp"
#include "supertux/menu/download_dialog.hpp"

class AddonGallery : public Menu
{
enum {
  MN_ADDONGALLERY_INSTALL, MN_ADDONGALLERY_UPDATE, MN_ADDONGALLERY_DEACTIVATE
};
private:
  AddonManager& m_addon_manager;
  std::string m_addon;
public:
  void refresh();
  AddonGallery(const std::string& addon):
  m_addon_manager(*AddonManager::current()),
  m_addon(addon)
  {
    refresh();
  }


  void menu_action(MenuItem* item)
  {
    if(item->id == MN_ADDONGALLERY_INSTALL || item->id == MN_ADDONGALLERY_UPDATE)
    {
        auto status = m_addon_manager.request_install_addon(m_addon);
        std::unique_ptr<DownloadDialog> dialog(new DownloadDialog(status, false, false));
        dialog->set_title(str(boost::format( _((item->id == MN_ADDONGALLERY_INSTALL)?"Downloading %s":"Updating %s") ) % m_addon));
        status->then([this](bool success)
        {
          if (success)
          {
            try
            {
              m_addon_manager.enable_addon(m_addon);
            }
            catch(const std::exception& err)
            {
              log_warning << "Enabling add-on failed: " << err.what() << std::endl;
            }
            refresh();
            MenuManager::instance().refresh();
          }

        });
        MenuManager::instance().set_dialog(std::move(dialog));
        return;
    }
    if(item->id == MN_ADDONGALLERY_DEACTIVATE)
    {
      bool activated = m_addon_manager.get_installed_addon(m_addon).is_enabled();
      if(activated)
      {
        m_addon_manager.disable_addon(m_addon);
      }else{
        m_addon_manager.enable_addon(m_addon);
      }
      clear();
      refresh();
    }


  }

  ~AddonGallery();


};

#endif
