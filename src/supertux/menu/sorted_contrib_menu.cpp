//  SuperTux
//  Copyright (C) 2021 mrkubax10 <mrkubax10@onet.pl>
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

#include "supertux/menu/sorted_contrib_menu.hpp"

#include <sstream>

#include <fmt/format.h>

#include "gui/item_action.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_item.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/levelset.hpp"
#include "supertux/menu/contrib_levelset_menu.hpp"
#include "supertux/player_status.hpp"
#include "supertux/resources.hpp"
#include "supertux/savegame.hpp"
#include "util/gettext.hpp"
#include "video/surface.hpp"

class ItemLockedWorld final : public MenuItem
{
public:
  ItemLockedWorld(const std::string& text, int collected, int required) :
    MenuItem(text),
    m_count_text(fmt::format("{}/{}", collected, required)),
    m_lock_icon(Surface::from_file("images/engine/menu/lock.png")),
    m_tuxdoll_icon(Surface::from_file("images/powerups/1up/1up.png"))
  {
    set_help(fmt::format(fmt::runtime(_("Collect {} more Tux Dolls in Story Mode to unlock!")), required - collected));
  }

  virtual void draw(DrawingContext& context, const Vector& pos, int menu_width, bool active) override
  {
    const Vector middle(pos.x + static_cast<float>(menu_width) / 2.f,
                        pos.y - Resources::normal_font->get_height() / 2.f);

    context.color().draw_surface_scaled(m_lock_icon,
                                 Rectf(Vector(10.f + pos.x + Resources::normal_font->get_height() * 0.8f / 2.f,
                                              middle.y + Resources::normal_font->get_height() * 0.1f),
                                       Sizef(Resources::normal_font->get_height(),
                                             Resources::normal_font->get_height()) * 0.8f),
                                 LAYER_GUI);
    context.color().draw_text(Resources::normal_font, get_text(),
                              middle - Vector(Resources::normal_font->get_height() // For Tux Doll icon
                                                + Resources::normal_font->get_text_width(m_count_text), 0) / 2.f,
                              ALIGN_CENTER, LAYER_GUI, ColorScheme::Menu::inactive_color);

    context.color().draw_surface_scaled(m_tuxdoll_icon,
                                 Rectf(middle + Vector(20.f + Resources::normal_font->get_text_width(get_text())
                                                         - Resources::normal_font->get_text_width(m_count_text),
                                                       Resources::normal_font->get_height() * 0.2f) / 2.f,
                                       Sizef(Resources::normal_font->get_height(),
                                             Resources::normal_font->get_height()) * 0.8f),
                                 LAYER_GUI);
    context.color().draw_text(Resources::normal_font, m_count_text,
                              middle + Vector(32.f + Resources::normal_font->get_text_width(get_text())
                                                + Resources::normal_font->get_height() * 0.8f * 2, // For both icons
                                              0) / 2.f,
                              ALIGN_CENTER, LAYER_GUI, ColorScheme::Menu::warning_color);
  }

  virtual int get_width() const override
  {
    return static_cast<int>(Resources::normal_font->get_text_width(get_text()))
      + static_cast<int>(Resources::normal_font->get_text_width(m_count_text))
      + static_cast<int>(Resources::normal_font->get_height() * 0.8f) * 2 // For both icons
      + 32 // Spacing
      + 16;
  }

private:
  const std::string m_count_text;
  SurfacePtr m_lock_icon;
  SurfacePtr m_tuxdoll_icon;

private:
  ItemLockedWorld(const ItemLockedWorld&) = delete;
  ItemLockedWorld& operator=(const ItemLockedWorld&) = delete;
};


SortedContribMenu::SortedContribMenu(std::vector<std::unique_ptr<World>>& worlds, const std::string& contrib_type, const std::string& title, const std::string& empty_message) :
  m_world_folders()
{
  add_label(title);
  add_hl();

  int story_tuxdolls = std::numeric_limits<int>::max();
  if (!g_config->developer_mode)
  {
    const auto story_savegame = Savegame::from_current_profile("world1");
    story_tuxdolls = story_savegame->get_player_status().tuxdolls;
  }

  int world_id = 0;
  for (unsigned int i = 0; i < worlds.size(); i++)
  {
    if (worlds[i]->get_contrib_type() == contrib_type)
    {
      m_world_folders.push_back(worlds[i]->get_basedir());
      std::string title_str = worlds[i]->get_title();
      if (worlds[i]->is_levelset())
        title_str = "[" + title_str + "]";

      if (story_tuxdolls >= worlds[i]->get_tuxdolls_required())
        add_entry(world_id, title_str).set_help(worlds[i]->get_description());
      else
        add_item(std::make_unique<ItemLockedWorld>(title_str, story_tuxdolls, worlds[i]->get_tuxdolls_required()));

      ++world_id;
    }
  }
  if (world_id == 0)
  {
    add_inactive(empty_message);
  }

  add_hl();
  add_back(_("Back"));
}

void
SortedContribMenu::menu_action(MenuItem& item)
{
  int index = item.get_id();
  if (index >= 0)
  {
    std::unique_ptr<World> world = World::from_directory(m_world_folders[index]);
    if (world->is_levelset())
    {
      MenuManager::instance().push_menu(std::unique_ptr<Menu>(new ContribLevelsetMenu(std::move(world))));
    }
    else
    {
      GameManager::current()->start_worldmap(*world);
    }
  }
}
