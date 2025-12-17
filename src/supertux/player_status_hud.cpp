//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/player_status_hud.hpp"

#include <iostream>

#include "object/display_effect.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/debug.hpp"
#include "supertux/game_object.hpp"
#include "supertux/level.hpp"
#include "supertux/player_status.hpp"
#include "supertux/resources.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "editor/editor.hpp"
#include "worldmap/worldmap_sector.hpp"

static const int DISPLAYED_STAT_UNSET = -1;

PlayerStatusHUD::PlayerStatusHUD(PlayerStatus& player_status, bool show_tuxdolls) :
  m_player_status(player_status),
  m_show_tuxdolls(show_tuxdolls),
  displayed_stat(DISPLAYED_STAT_UNSET),
  displayed_stat_frame(0),
  stat_surface(Surface::from_file("images/engine/hud/" + std::string(show_tuxdolls ? "tuxdolls-0.png" : "coins-0.png"))),
  m_stat_value(show_tuxdolls ? m_player_status.tuxdolls : m_player_status.coins),
  m_bonus_sprites(),
  m_item_pocket_border(Surface::from_file("images/engine/hud/item_pocket.png"))
{
  m_bonus_sprites[BONUS_FIRE]  = SpriteManager::current()->create("images/powerups/fireflower/fireflower.sprite");
  m_bonus_sprites[BONUS_ICE]   = SpriteManager::current()->create("images/powerups/iceflower/iceflower.sprite");
  m_bonus_sprites[BONUS_AIR]   = SpriteManager::current()->create("images/powerups/airflower/airflower.sprite");
  m_bonus_sprites[BONUS_EARTH] = SpriteManager::current()->create("images/powerups/earthflower/earthflower.sprite");
}

void
PlayerStatusHUD::reset()
{
  displayed_stat = DISPLAYED_STAT_UNSET;
}

void
PlayerStatusHUD::update(float dt_sec)
{
}

void
PlayerStatusHUD::draw(DrawingContext& context)
{
  if (g_debug.hide_player_hud || Editor::is_active() ||
      (Sector::current() && Sector::current()->get_effect().has_active_borders()))
    return;

  if ((displayed_stat == DISPLAYED_STAT_UNSET) ||
      (std::abs(displayed_stat - m_stat_value) > 100)) {
    displayed_stat = m_stat_value;
    displayed_stat_frame = 0;
  }
  if (++displayed_stat_frame > 2) {
    displayed_stat_frame = 0;
    if (displayed_stat < m_stat_value) displayed_stat++;
    if (displayed_stat > m_stat_value) displayed_stat--;
  }
  if (!m_show_tuxdolls)
    displayed_stat = std::min(std::max(displayed_stat, 0), m_player_status.get_max_coins());

  float hudpos = BORDER_Y + 1.0f;
  const std::string stat_text = std::to_string(displayed_stat);

  context.push_transform();
  context.set_translation(Vector(0, 0));
  context.transform().scale = 1.f;
  if (stat_surface)
  {
    context.color().draw_surface(stat_surface,
                                Vector(context.get_width() - BORDER_X - static_cast<float>(stat_surface->get_width()) - Resources::fixed_font->get_text_width(stat_text),
                                       hudpos),
                                LAYER_HUD);
  }

  context.color().draw_text(Resources::fixed_font,
                            stat_text,
                            Vector(static_cast<float>(context.get_width()) - BORDER_X - Resources::fixed_font->get_text_width(stat_text),
                                  hudpos + 13.f),
                            ALIGN_LEFT,
                            LAYER_HUD,
                            PlayerStatusHUD::text_color);


  if (m_player_status.is_item_pocket_allowed())
  {
    for (int i = 0; i < InputManager::current()->get_num_users(); i++)
    {
      float ypos = static_cast<float>(m_item_pocket_border->get_height() * i);
      Vector pos(BORDER_X, BORDER_Y + ypos);
      context.color().draw_surface(m_item_pocket_border, pos, LAYER_HUD);

      if (m_bonus_sprites.find(m_player_status.m_item_pockets[i]) != m_bonus_sprites.end())
      {
        pos += 20;

        Sprite* sprite = m_bonus_sprites[m_player_status.m_item_pockets[i]].get();
        if (sprite)
          sprite->draw(context.color(), pos, LAYER_HUD);
      }
    }
  }

  context.pop_transform();
}
