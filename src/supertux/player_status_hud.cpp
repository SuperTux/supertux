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

#include <sstream>

#include "supertux/game_object.hpp"
#include "supertux/player_status.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "editor/editor.hpp"

static const int DISPLAYED_COINS_UNSET = -1;

PlayerStatusHUD::PlayerStatusHUD(PlayerStatus& player_status) :
  m_player_status(player_status),
  displayed_coins(DISPLAYED_COINS_UNSET),
  displayed_coins_frame(0),
  coin_surface(Surface::from_file("images/engine/hud/coins-0.png")),
  fire_surface(Surface::from_file("images/objects/bullets/fire-hud.png")),
  ice_surface(Surface::from_file("images/objects/bullets/ice-hud.png"))
{
}

void
PlayerStatusHUD::reset()
{
  displayed_coins = DISPLAYED_COINS_UNSET;
}

void
PlayerStatusHUD::update(float dt_sec)
{
}

void
PlayerStatusHUD::draw(DrawingContext& context)
{
  if (Editor::is_active())
    return;

  if ((displayed_coins == DISPLAYED_COINS_UNSET) ||
      (std::abs(displayed_coins - m_player_status.coins) > 100)) {
    displayed_coins = m_player_status.coins;
    displayed_coins_frame = 0;
  }
  if (++displayed_coins_frame > 2) {
    displayed_coins_frame = 0;
    if (displayed_coins < m_player_status.coins) displayed_coins++;
    if (displayed_coins > m_player_status.coins) displayed_coins--;
  }
  displayed_coins = std::min(std::max(displayed_coins, 0), m_player_status.get_max_coins());

  std::ostringstream ss;
  ss << displayed_coins;
  std::string coins_text = ss.str();

  context.push_transform();
  context.set_translation(Vector(0, 0));
  context.transform().scale = 1.f;
  if (!Editor::is_active())
  {
    if (coin_surface)
    {
      context.color().draw_surface(coin_surface,
                                  Vector(static_cast<float>(context.get_width()) - BORDER_X - static_cast<float>(coin_surface->get_width()) - Resources::fixed_font->get_text_width(coins_text),
                                          BORDER_Y + 1.0f),
                                  LAYER_HUD);
    }

    context.color().draw_text(Resources::fixed_font,
                              coins_text,
                              Vector(static_cast<float>(context.get_width()) - BORDER_X - Resources::fixed_font->get_text_width(coins_text),
                                    BORDER_Y + 14.0f),
                              ALIGN_LEFT,
                              LAYER_HUD,
                              PlayerStatusHUD::text_color);
  }
  std::string ammo_text;

  for (int target = 0; target < InputManager::current()->get_num_users(); target++)
  {
    if (m_player_status.bonus[target] == FIRE_BONUS) {

      ammo_text = std::to_string(m_player_status.max_fire_bullets[target]);

      if (fire_surface) {
        context.color().draw_surface(fire_surface,
                                    Vector(static_cast<float>(context.get_width())
                                                - BORDER_X
                                                - static_cast<float>(fire_surface->get_width())
                                                - Resources::fixed_font->get_text_width(ammo_text),
                                            BORDER_Y
                                                + 1.0f
                                                + (Resources::fixed_font->get_text_height(coins_text) + 5)
                                                + (Resources::fixed_font->get_text_height(ammo_text) + 5)
                                                * static_cast<float>(target)),
                                    LAYER_HUD);
      }

      context.color().draw_text(Resources::fixed_font,
                                ammo_text,
                                Vector(static_cast<float>(context.get_width())
                                          - BORDER_X
                                          - Resources::fixed_font->get_text_width(ammo_text),
                                      BORDER_Y
                                          + (Resources::fixed_font->get_text_height(coins_text) + 5.0f)
                                          + (Resources::fixed_font->get_text_height(ammo_text) + 5.0f)
                                          * static_cast<float>(target)),
                                ALIGN_LEFT,
                                LAYER_HUD,
                                PlayerStatusHUD::text_color);
    }

    if (m_player_status.bonus[target] == ICE_BONUS) {

      ammo_text = std::to_string(m_player_status.max_ice_bullets[target]);

      if (ice_surface) {
        context.color().draw_surface(ice_surface,
                                    Vector(static_cast<float>(context.get_width())
                                                - BORDER_X
                                                - static_cast<float>(ice_surface->get_width())
                                                - Resources::fixed_font->get_text_width(ammo_text),
                                            BORDER_Y
                                                + 1.0f
                                                + (Resources::fixed_font->get_text_height(coins_text) + 5)
                                                + (Resources::fixed_font->get_text_height(ammo_text) + 5)
                                                * static_cast<float>(target)),
                                    LAYER_HUD);
      }

      context.color().draw_text(Resources::fixed_font,
                                ammo_text,
                                Vector(static_cast<float>(context.get_width())
                                          - BORDER_X
                                          - Resources::fixed_font->get_text_width(ammo_text),
                                      BORDER_Y
                                          + (Resources::fixed_font->get_text_height(coins_text) + 5.0f)
                                          + (Resources::fixed_font->get_text_height(ammo_text) + 5.0f)
                                          * static_cast<float>(target)),
                                ALIGN_LEFT,
                                LAYER_HUD,
                                PlayerStatusHUD::text_color);
    }
  }

  context.pop_transform();
}

/* EOF */
