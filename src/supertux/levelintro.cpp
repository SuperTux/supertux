//  SuperTux -- LevelIntro screen
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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

#include "supertux/levelintro.hpp"

#include <fmt/format.h>

#include "control/input_manager.hpp"
#include "math/random.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/game_network_server_user.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/level.hpp"
#include "supertux/player_status.hpp"
#include "supertux/resources.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "video/compositor.hpp"

void
LevelIntro::quit()
{
  ScreenManager::current()->pop_screen(std::make_unique<FadeToBlack>(FadeToBlack::FADEOUT, 0.1f));
}


LevelIntro::LevelIntro(const Level& level, const Statistics* best_level_statistics, bool allow_quit) :
  m_level(level),
  m_best_level_statistics(best_level_statistics),
  m_allow_quit(allow_quit),
  m_players()
{
  for (const Player* player : m_level.get_players())
    push_player(player);
}

LevelIntro::~LevelIntro()
{
}

void
LevelIntro::setup()
{
}

void
LevelIntro::update(float dt_sec, const Controller& controller)
{
  // Check if it's time to exit the screen
  if (m_allow_quit && controller.pressed_any(Control::JUMP, Control::ACTION, Control::MENU_SELECT,
                                             Control::START, Control::ESCAPE))
  {
    quit();
  }

  for (const auto& player : m_players)
  {
    auto bonus_prefix = player->player->get_status().get_bonus_prefix();
    if (player->player->get_status().bonus == FIRE_BONUS && g_config->christmas_mode)
      bonus_prefix = "big";

    player->sprite_py += player->sprite_vy * dt_sec;
    player->sprite_vy += 100 * dt_sec * Sector::get().get_gravity();
    if (player->sprite_py >= 0)
    {
      player->sprite_py = 0;
      player->sprite_vy = 0;
      player->sprite->set_action(bonus_prefix + "-walk-right");
    }
    else
    {
      player->sprite->set_action(bonus_prefix + "-jump-right");
    }
    if (player->sprite_jump_timer.check())
    {
      player->sprite_vy = -300;
      player->sprite_jump_timer.start(graphicsRandom.randf(2,3));
    }
  }
}

void
LevelIntro::draw_stats_line(DrawingContext& context, int& py, const std::string& name, const std::string& stat, bool isPerfect)
{
  std::stringstream ss;
  ss << name << ": " << stat;
  Color tcolor = isPerfect ? s_stat_perfect_color : s_stat_color;
  context.color().draw_center_text(Resources::normal_font, ss.str(), Vector(0, static_cast<float>(py)),
                                   LAYER_FOREGROUND1, tcolor);
  py += static_cast<int>(Resources::normal_font->get_height());
}

void
LevelIntro::draw(Compositor& compositor)
{
  auto& context = compositor.make_context();

  const Statistics& stats = m_level.m_stats;
  int py = static_cast<int>(context.get_height() / 2.0f - Resources::normal_font->get_height() / 2.0f);

  context.set_ambient_color(Color(1.0f, 1.0f, 1.0f, 1.0f));
  context.color().draw_filled_rect(context.get_rect(), Color(0.0f, 0.0f, 0.0f, 1.0f), 0);

  context.color().draw_center_text(Resources::normal_font, m_level.get_name(), Vector(0, static_cast<float>(py)), LAYER_FOREGROUND1, s_header_color);
  py += static_cast<int>(Resources::normal_font->get_height());

  std::string author = m_level.get_author();
  if ((!author.empty()) && (author != "SuperTux Team"))
  {
    std::string author_text = fmt::format(fmt::runtime(_("contributed by {}")), author);
    context.color().draw_center_text(Resources::small_font, author_text, Vector(0, static_cast<float>(py)), LAYER_FOREGROUND1, s_author_color);
    py += static_cast<int>(Resources::small_font->get_height());
  }

  py += 96;

  for (int i = 0; i < static_cast<int>(m_players.size()); i++)
  {
    /*
    if (i != 0 && i < num_users &&
        !InputManager::current()->has_corresponsing_controller(i) &&
        !InputManager::current()->m_uses_keyboard[i])
      context.transform().alpha = 0.25f;
    */

    const PlayerData& player = *m_players[i];
    const GameServerUser* remote_user = player.player->get_remote_user();
    const PlayerStatus::Status& status = player.player->get_status();

    const float offset = (static_cast<float>(i) - static_cast<float>(m_players.size()) / 2.f + 0.5f) * 64.f;

    const Vector sprite_pos((context.get_width() - player.sprite->get_current_hitbox_width()) / 2 - offset,
                            static_cast<float>(py) + player.sprite_py - player.sprite->get_current_hitbox_height());

    player.sprite->draw(context.color(), sprite_pos, LAYER_FOREGROUND1);

    if ((!remote_user && InputManager::current()->get_num_users() > 1) ||
        (remote_user && remote_user->player_controllers.size() > 1))
    {
      context.color().draw_text(Resources::normal_font, std::to_string(player.player->get_id() + 1),
                                sprite_pos + Vector(player.sprite->get_current_hitbox_width() / 2,
                                                    player.sprite->get_current_hitbox_height() / 2),
                                FontAlignment::ALIGN_CENTER, LAYER_LIGHTMAP + 1);
    }

    if (remote_user)
    {
      context.color().draw_text(Resources::normal_font, remote_user->username,
                                sprite_pos + Vector(player.sprite->get_current_hitbox_width() / 2, -32.f),
                                FontAlignment::ALIGN_CENTER, LAYER_LIGHTMAP + 1,
                                remote_user->username_color);
    }

    Color power_color = (status.bonus == FIRE_BONUS ? Color(1.f, 0.7f, 0.5f) :
      status.bonus == ICE_BONUS ? Color(0.7f, 1.f, 1.f) :
      status.bonus == AIR_BONUS ? Color(0.7f, 1.f, 0.5f) :
      status.bonus == EARTH_BONUS ? Color(1.f, 0.9f, 0.6f) :
      Color(1.f, 1.f, 1.f));

    player.sprite->set_color(power_color);
    /*if (status.bonus > GROWUP_BONUS) {
      player.santa_sprite->draw(context.color(), Vector((context.get_width() - player.sprite->get_current_hitbox_width()) / 2 - offset,
                                                  static_cast<float>(py) + player.sprite_py - player.sprite->get_current_hitbox_height()), LAYER_FOREGROUND1);
    }*/

    //context.transform().alpha = 1.f;
  }

  py += 32;

  if (m_best_level_statistics)
  {
    context.color().draw_center_text(Resources::normal_font,
                                     std::string("- ") + _("Best Level Statistics") + std::string(" -"),
                                     Vector(0, static_cast<float>(py)),
                                     LAYER_FOREGROUND1, s_stat_hdr_color);

    py += static_cast<int>(Resources::normal_font->get_height());

    const Statistics::Preferences& preferences = m_level.m_stats.get_preferences();
    if (preferences.enable_coins)
    {
      draw_stats_line(context, py, _("Coins"),
                      Statistics::coins_to_string(m_best_level_statistics->get_coins(), stats.m_total_coins),
                      m_best_level_statistics->get_coins() >= stats.m_total_coins);
    }
    if (preferences.enable_badguys)
    {
      draw_stats_line(context, py, _("Badguys killed"),
                      Statistics::frags_to_string(m_best_level_statistics->get_badguys(), stats.m_total_badguys),
                      m_best_level_statistics->get_badguys() >= stats.m_total_badguys);
    }
    if (preferences.enable_secrets)
    {
      draw_stats_line(context, py, _("Secrets"),
                      Statistics::secrets_to_string(m_best_level_statistics->get_secrets(), stats.m_total_secrets),
                      m_best_level_statistics->get_secrets() >= stats.m_total_secrets);
    }

    bool targetTimeBeaten = m_level.m_target_time == 0.0f || (m_best_level_statistics->get_time() != 0.0f && m_best_level_statistics->get_time() < m_level.m_target_time);
    draw_stats_line(context, py, _("Best time"),
                    Statistics::time_to_string(m_best_level_statistics->get_time()), targetTimeBeaten);

    if (m_level.m_target_time != 0.0f)
    {
      draw_stats_line(context, py, _("Level target time"),
                      Statistics::time_to_string(m_level.m_target_time), targetTimeBeaten);
    }
  }

  py += 32;

  if (!m_level.m_note.empty())
    context.color().draw_center_text(Resources::normal_font, m_level.m_note, Vector(0, py), LAYER_FOREGROUND1);
}

IntegrationStatus
LevelIntro::get_status() const
{
  IntegrationStatus status;
  status.m_details.push_back("Watching a cutscene");
  status.m_details.push_back("In level: " + m_level.get_name());
  return status;
}

void
LevelIntro::push_player(const Player* player)
{
  auto player_data = std::make_unique<PlayerData>();

  player_data->player = player;
  player_data->sprite = SpriteManager::current()->create("images/creatures/tux/tux.sprite");
  player_data->santa_sprite = SpriteManager::current()->create("images/creatures/tux/santahat.sprite");
  player_data->sprite_py = 0;
  player_data->sprite_vy = 0;

  // Show appropriate tux animation for player status.
  if (player->get_status().bonus == FIRE_BONUS && g_config->christmas_mode)
  {
    player_data->sprite->set_action("big-walk-right");
    player_data->santa_sprite->set_action("default");
  }
  else
  {
    player_data->sprite->set_action(player->get_status().get_bonus_prefix() + "-walk-right");
  }

  player_data->sprite_jump_timer.start(graphicsRandom.randf(5,10));

  /* Set Tux powerup sprite action */
  //player_data.santa_sprite->set_action(player_data.sprite->get_action());

  m_players.push_back(std::move(player_data));

  // Sort players by username (local players go first).
  std::sort(m_players.begin(), m_players.end(),
    [](const auto& lhs, const auto& rhs)
    {
      const GameServerUser* lhs_remote = lhs->player->get_remote_user();
      const GameServerUser* rhs_remote = rhs->player->get_remote_user();

      if (lhs_remote == rhs_remote) // Also true if both are nullptr.
        return lhs->player->get_id() > rhs->player->get_id();
      if (lhs_remote && rhs_remote)
        return lhs_remote->username > rhs_remote->username;

      return !rhs_remote;
    });
}

void
LevelIntro::pop_player(const Player* player)
{
  if (m_players.size() <= 1)
  {
    log_warning << "Attempt to pop last player in intro scene" << std::endl;
    return;
  }

  m_players.erase(std::remove_if(m_players.begin(), m_players.end(),
    [player](const auto& player_data)
    {
      return player_data->player == player;
    }),
    m_players.end());
}

/* EOF */
