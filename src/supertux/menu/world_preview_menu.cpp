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

#include "supertux/menu/world_preview_menu.hpp"

#include <physfs.h>

#include "gui/menu_manager.hpp"
#include "gui/menu_item.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "supertux/menu/contrib_levelset_menu.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

const Sizef WorldPreviewMenu::s_preview_size(426.f, 240.f);
const float WorldPreviewMenu::s_preview_fade_time = 0.1f;

WorldPreviewMenu::WorldPreviewMenu(float center_x_offset, float center_y_offset) :
  m_world_entries(),
  m_last_world_index(-1),
  m_preview_fade_timer(),
  m_preview_fade_active(false),
  m_preview_fading_out(false)
{
  // Adjust center position to give space for displaying previews.
  set_center_pos(static_cast<float>(SCREEN_WIDTH) / center_x_offset,
                 static_cast<float>(SCREEN_HEIGHT) / center_y_offset);

  m_preview_fade_timer.start(s_preview_fade_time);
}

SurfacePtr
WorldPreviewMenu::find_preview(const std::string& preview_file, const std::string& basedir)
{
  std::string preview_path = FileSystem::join("profile" + std::to_string(g_config->profile), preview_file);
  if (PHYSFS_exists(preview_path.c_str())) // A preview exists.
  {
    return Surface::from_file(preview_path);
  }
  else // Preview doesn't exist, so check for a placeholder.
  {
    preview_path = FileSystem::join(basedir, "preview.png");
    if (PHYSFS_exists(preview_path.c_str())) // A preview placeholder exists.
      return Surface::from_file(preview_path);
  }

  return nullptr;
}

void
WorldPreviewMenu::draw_additional(DrawingContext& context)
{
  const int index = get_active_item_id();
  bool valid_last_index = is_valid_index();

  // Update fade.
  if (index != m_last_world_index && !m_preview_fade_active) // Index has changed, there is no current fade.
  {
    if (valid_last_index) // Fade out only if the last index is valid.
      m_preview_fade_timer.start(s_preview_fade_time);
    m_preview_fading_out = true;
    m_preview_fade_active = true;
  }
  float timeleft = m_preview_fade_timer.get_timeleft();
  if (timeleft < 0 && m_preview_fade_active) // Current fade is over.
  {
    m_last_world_index = index;
    valid_last_index = is_valid_index(); // Repeat valid last index check
    if (m_preview_fading_out) // After a fade-out, a fade-in should follow up.
    {
      m_preview_fade_timer.start(s_preview_fade_time);
      timeleft = m_preview_fade_timer.get_timeleft();
      m_preview_fading_out = false;
    }
    else
    {
      m_preview_fade_active = false;
    }
  }

  // Set alpha according to fade.
  float alpha = 1.f;
  if (timeleft > 0)
  {
    const float alpha_val = timeleft * (1.f / s_preview_fade_time);
    alpha = m_preview_fading_out ? alpha_val : 1.f - alpha_val;
  }
  const Color color_white(1, 1, 1, alpha);

  // Perform actions only if current index is a valid world index.
  if (valid_last_index)
  {
    // Draw progress preview of current world.
    Rectf preview_rect(Vector(context.get_width() * 0.73f - s_preview_size.width / 2,
                            context.get_height() / 2 - s_preview_size.height / 2), s_preview_size);
    if (m_world_entries[m_last_world_index].preview)
    {
      PaintStyle style;
      style.set_alpha(alpha);
      context.color().draw_surface_scaled(m_world_entries[m_last_world_index].preview, preview_rect, LAYER_GUI, style);
    }
    else // If preview is not available, draw a filled rect with text in its place.
    {
      context.color().draw_filled_rect(preview_rect, Color(0, 0, 0, alpha), LAYER_GUI);
      context.color().draw_text(Resources::normal_font, _("No preview available."),
                                preview_rect.get_middle(), ALIGN_CENTER, LAYER_GUI, color_white);
    }

    // Draw world progress.
    const Savegame::Progress& progress = m_world_entries[m_last_world_index].progress;
    if (progress.progress > -2) // Progress should be drawn.
      context.color().draw_text(Resources::normal_font,
                                progress.progress > -1 ? std::to_string(progress.progress) + "/" + std::to_string(progress.total) : _("*NEW*"),
                                Vector(preview_rect.get_left() + s_preview_size.width / 2, preview_rect.get_bottom() * 1.05f),
                                ALIGN_CENTER, LAYER_GUI, color_white);
  }
}

bool
WorldPreviewMenu::is_valid_index() const
{
  return m_last_world_index >= 0 && m_last_world_index < static_cast<int>(m_world_entries.size()) &&
         m_world_entries[m_last_world_index].is_worldmap;
}

void
WorldPreviewMenu::menu_action(MenuItem& item)
{
  int index = item.get_id();
  if (index >= 0)
  {
    std::unique_ptr<World> world = World::from_directory(m_world_entries[index].folder);
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

/* EOF */
