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
#include <cmath>

#include "gui/menu_manager.hpp"
#include "object/display_effect.hpp"
#include "sprite/sprite_manager.hpp"
#include "object/player.hpp"
#include "supertux/debug.hpp"
#include "supertux/game_object.hpp"
#include "supertux/level.hpp"
#include "supertux/player_status.hpp"
#include "supertux/resources.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/sector.hpp"
#include "supertux/counter.hpp"
#include "supertux/globals.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "editor/editor.hpp"
#include "worldmap/worldmap_sector.hpp"
#include "math/util.hpp"
#include "math/anchor_point.hpp"

static const int DISPLAYED_STAT_UNSET = -1;
static constexpr float ITEM_POCKET_TIME = 6.f;

PlayerStatusHUD::PlayerStatusHUD(PlayerStatus& player_status, bool show_tuxdolls) :
  m_player_status(player_status),
  m_show_tuxdolls(show_tuxdolls),
  displayed_stat(DISPLAYED_STAT_UNSET),
  displayed_stat_frame(0),
  stat_surface(Surface::from_file("images/engine/hud/" + std::string(show_tuxdolls ? "tuxdolls-0.png" : "coins-0.png"))),
  m_stat_value(show_tuxdolls ? m_player_status.tuxdolls : m_player_status.coins),
  m_bonus_sprites(),
  m_item_pocket_border(Surface::from_file("images/engine/hud/item_pocket.png")),
  m_counters(),
  m_item_pocket_fade()
{
  m_player_status.set_hud_hint(this);
  // aesthetic choice: hint to players their item pocket
  m_item_pocket_fade.start(ITEM_POCKET_TIME);
  m_bonus_sprites[BONUS_FIRE]  = SpriteManager::current()->create("images/powerups/fireflower/fireflower.sprite");
  m_bonus_sprites[BONUS_ICE]   = SpriteManager::current()->create("images/powerups/iceflower/iceflower.sprite");
  m_bonus_sprites[BONUS_AIR]   = SpriteManager::current()->create("images/powerups/airflower/airflower.sprite");
  m_bonus_sprites[BONUS_EARTH] = SpriteManager::current()->create("images/powerups/earthflower/earthflower.sprite");

  m_counters.resize(COUNTER_COUNT);

  for (int type = 0; type < COUNTER_COUNT; type++)
  {
    const CounterDef& def = Counter::get_def(static_cast<CounterType>(type));
    m_counters[type].center_sprite = SpriteManager::current()->create(def.center_sprite_path);
    m_counters[type].counter_sprite = SpriteManager::current()->create(def.counter_sprite_path);
    m_counters[type].counter_parts = m_counters[type].counter_sprite->get_action_surfaces("default").value_or(std::vector<SurfacePtr>());
    m_counters[type].effect_sprite = SpriteManager::current()->create(def.effect_sprite_path);
  }
}

void
PlayerStatusHUD::on_item_pocket_change([[maybe_unused]] Player* player)
{
  m_item_pocket_fade.start(ITEM_POCKET_TIME);
}

void
PlayerStatusHUD::reset()
{
  displayed_stat = DISPLAYED_STAT_UNSET;
}

void
PlayerStatusHUD::update(float dt_sec)
{
  m_item_pocket_fade.check();
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
      constexpr float POCKET_FADE_AT = .7f;
      constexpr float POCKET_FADE_MULT = 1.4f;
      float prog = m_item_pocket_fade.started() ? m_item_pocket_fade.get_progress() : 1.f;
      float fade = 1.f - std::fabs(POCKET_FADE_AT - prog) * POCKET_FADE_MULT;
      if (m_item_pocket_fade.started())
      {
        context.set_alpha(prog < POCKET_FADE_AT ? 1.f : fade);

      }
      else
        context.set_alpha(fade);

      // If in some menu (i.e. pause) or not in game (worldmap) then just show anyway
      if ((MenuManager::current() && MenuManager::current()->get_menu_stack_size() > 0) ||
          (GameSession::current() && !GameSession::current()->is_active()))
        context.set_alpha(1.f);

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

      context.set_alpha(1.f);
    }
  }

  if (Sector::current()) // draw only if in level
  {
    for (auto& player : Sector::current()->get_players())
    {
      const std::vector<Counter>& counters = player->m_counters;

      for (int i = 0; i < counters.size(); i++)
      {
        const Counter& counter = counters[i];
        int type = counter.get_type();

        if (!counter.is_counter_on() || !m_counters[type].counter_sprite)
          continue;

        float xpos = static_cast<float>((m_counters[type].counter_sprite->get_width() + 10) * (i + 1));
        float ypos = static_cast<float>(m_counters[type].counter_sprite->get_height() * player->get_id());
        Vector pos(BORDER_X + xpos, BORDER_Y + ypos);

        context.color().draw_surface(m_counters[type].counter_parts[counter.counter_stage()], pos, LAYER_HUD);

        if (!m_counters[type].center_sprite)
          continue;

        Vector center_sprite_pos = pos + Vector(21.f, 20.f);
        m_counters[type].center_sprite->draw(context.color(), center_sprite_pos, LAYER_HUD);

        if (!m_counters[type].effect_sprite)
          continue;

        Vector center_sprite_center = center_sprite_pos + Vector(
          m_counters[type].center_sprite->get_width()  * 0.5f,
          m_counters[type].center_sprite->get_height() * 0.5f
        );

        float angle = counter.effect_angle();
        Vector dir(std::cos(angle), std::sin(angle));
        const float radius = 29.f;
        Vector anchor_off = get_anchor_pos(m_counters[type].effect_sprite->get_current_hitbox(), ANCHOR_MIDDLE);

        float pulse = 0.55f + 0.45f * std::sin(g_game_time * 3.0f);
        context.set_alpha(pulse);

        Vector base_pos = center_sprite_center + (dir * radius) - anchor_off;

        for (int k = -2; k <= 2; ++k) 
        {
          Vector draw_pos = base_pos + (dir * (k * 2.0f)); 
          m_counters[type].effect_sprite->draw(context.color(), draw_pos, LAYER_HUD + 1);
        }
        context.set_alpha(1.0f);
      }
    }
  }
  
  
  context.pop_transform();
}
