//  SuperTux
//  Copyright (C) 2022-2023 Vankata453
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

#include "supertux/menu/world_preview_menu.hpp"

#include <sstream>

#include <fmt/format.h>
#include <physfs.h>

#include "gui/item_action.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_item.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/menu/contrib_levelset_menu.hpp"
#include "supertux/resources.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

WorldPreviewMenu::WorldPreviewMenu() :
  m_world_entries()
{
}

ItemAction&
WorldPreviewMenu::add_world(const std::string& title, const std::string& folder,
                            Savegame::Progress progress, SurfacePtr preview)
{
  ItemAction& item = add_entry(static_cast<int>(m_world_entries.size()), title);

  if (g_config->show_world_previews && preview)
  {
    item.set_preview(preview);
    m_force_previews = true;
  }

  std::string progress_text;
  if (progress.total > 0) // Only show progress, if provided.
  {
    progress_text = fmt::format(fmt::runtime(_("{}/{} finished, {}/{} perfected ({}%)")),
        progress.solved, progress.total, progress.perfect, progress.total, progress.get_percentage());
    m_force_previews = true;
  }

  m_world_entries.push_back({ folder, progress_text });

  return item;
}

SurfacePtr
WorldPreviewMenu::find_preview(const std::string& preview_file, const std::string& basedir)
{
  if (!g_config->show_world_previews)
    return nullptr;

  std::string preview_path = FileSystem::join("profile" + std::to_string(g_config->profile), preview_file);
  if (PHYSFS_exists(preview_path.c_str())) // A preview exists.
    return Surface::from_file(preview_path);

  // Preview doesn't exist, so check for a placeholder.
  preview_path = FileSystem::join(basedir, "preview.png");
  if (PHYSFS_exists(preview_path.c_str())) // A preview placeholder exists.
    return Surface::from_file(preview_path);

  return nullptr;
}

void
WorldPreviewMenu::draw_preview_data(DrawingContext& context, const MenuItem& item, const Rectf& preview_rect, float alpha)
{
  // Draw world progress.
  context.color().draw_text(Resources::normal_font, m_world_entries[item.get_id()].progress_text,
                            Vector(preview_rect.get_left() + preview_rect.get_width() / 2, preview_rect.get_bottom() * 1.03f),
                            ALIGN_CENTER, LAYER_GUI, Color(1, 1, 1, alpha));
}

bool
WorldPreviewMenu::is_preview_item_valid(const MenuItem& item) const
{
  const int index = item.get_id();
  if (index < 0 || index >= static_cast<int>(m_world_entries.size())) // Not a valid world index.
    return false;

  return !m_world_entries[index].progress_text.empty();
}

void
WorldPreviewMenu::menu_action(MenuItem& item)
{
  const int index = item.get_id();
  if (index >= 0 && index < static_cast<int>(m_world_entries.size())) // Valid world index.
  {
    std::unique_ptr<World> world = World::from_directory(m_world_entries[index].folder);
    if (world->is_levelset())
    {
      MenuManager::instance().push_menu(std::make_unique<ContribLevelsetMenu>(std::move(world)));
    }
    else
    {
      GameManager::current()->start_worldmap(*world);
    }
  }
}

/* EOF */
