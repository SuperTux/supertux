//  SuperTux
//  Copyright (C) 2026 SuperTux Team
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

#include "supertux/screen/global_stats_screen.hpp"

#include <memory>
#include <optional>

#include <fmt/format.h>

#include "collision/collision_object.hpp"
#include "control/codecontroller.hpp"
#include "control/controller.hpp"
#include "gui/mousecursor.hpp"
#include "math/util.hpp"
#include "object/camera.hpp"
#include "object/music_object.hpp"
#include "object/player.hpp"
#include "supertux/constants.hpp"
#include "supertux/d_scope.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/profile_manager.hpp"
#include "supertux/resources.hpp"
#include "supertux/savegame.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/statistics.hpp"
#include "supertux/tile.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"
#include "video/compositor.hpp"
#include "video/drawing_context.hpp"
#include "video/layer.hpp"
#include "video/surface.hpp"

static const std::string DEFAULT_TITLE_LEVEL = "levels/misc/menu.stl";

Color GlobalStatsScreen::s_header_color(1.f, 1.f, 1.f);
Color GlobalStatsScreen::s_label_color(0.90f, 0.95f, 1.00f);
Color GlobalStatsScreen::s_value_color(0.72f, 0.95f, 1.00f);
Color GlobalStatsScreen::s_help_color(0.76f, 0.84f, 0.92f);

std::optional<GlobalStatsManager::Summary> GlobalStatsScreen::s_cached_summary;

GlobalStatsScreen::GlobalStatsScreen() :
  m_frame(Surface::from_file("images/engine/menu/frame.png")),
  m_backdrop(Surface::from_file("images/engine/menu/score-backdrop.png")),
  m_savegame(std::make_unique<Savegame>(ProfileManager::current()->get_current_profile(), "")),
  m_controller(new CodeController()),
  m_titlesession(),
  m_summary(),
  m_summary_ready(false),
  m_load_requested(false),
  m_jump_was_released(false)
{
}

GlobalStatsScreen::~GlobalStatsScreen()
{
}

void
GlobalStatsScreen::setup()
{
  MouseCursor::current()->set_visible(true);
  refresh_title_background();
  s_cached_summary.reset();
}

void
GlobalStatsScreen::leave()
{
  if (m_titlesession)
  {
    m_titlesession->get_current_sector().deactivate();
    m_titlesession->leave();
  }

  MouseCursor::current()->set_visible(true);
}

void
GlobalStatsScreen::update(float dt_sec, const Controller& controller)
{
  update_title_background(dt_sec * 0.6f);

  if (!m_summary_ready)
  {
    if (!m_load_requested)
    {
      m_load_requested = true;
      return;
    }

    load_summary();
    return;
  }

  if (controller.pressed_any(Control::JUMP, Control::ACTION, Control::MENU_SELECT,
                             Control::START, Control::ESCAPE))
  {
    ScreenManager::current()->pop_screen(std::make_unique<FadeToBlack>(FadeToBlack::FADEOUT, 0.1f));
  }
}

void
GlobalStatsScreen::draw(Compositor& compositor)
{
  constexpr int panel_layer = LAYER_GUI;
  constexpr int text_layer = LAYER_GUI + 1;

  auto& context = compositor.make_context();
  draw_background(context);

  if (!m_summary_ready)
  {
    context.color().draw_center_text(Resources::normal_font,
                                     _("Loading statistics..."),
                                     Vector(0, context.get_height() / 2.f - Resources::normal_font->get_height() / 2.f),
                                     text_layer,
                                     s_header_color);

    context.color().draw_center_text(Resources::small_font,
                                     _("Please wait a moment"),
                                     Vector(0, context.get_height() / 2.f + Resources::small_font->get_height()),
                                     text_layer,
                                     s_help_color);
    return;
  }

  int py = static_cast<int>(context.get_height() / 2.f - Resources::normal_font->get_height() * 5.f);

  context.color().draw_center_text(Resources::normal_font,
                                   std::string("- ") + _("Global Statistics") + std::string(" -"),
                                   Vector(0, static_cast<float>(py)),
                                   text_layer,
                                   s_header_color);
  py += static_cast<int>(Resources::normal_font->get_height() * 2.f);

  draw_stat_line(context, py, _("Coins collected"), format_count(m_summary.total_coins));
  draw_stat_line(context, py, _("Secrets found"), format_count(m_summary.total_secrets));
  draw_stat_line(context, py, _("Tux Dolls collected"), format_count(m_summary.total_tuxdolls));
  draw_stat_line(context, py, _("Levels completed"), fmt::format("{} / {}", m_summary.solved_levels, m_summary.total_levels));
  draw_stat_line(context, py, _("Perfect levels"), format_count(m_summary.perfect_levels));
  draw_stat_line(context, py, _("Total play time"), format_time(m_summary.total_time));

  py += static_cast<int>(Resources::normal_font->get_height());
  context.color().draw_center_text(Resources::small_font,
                                   _("Press jump, action, start or escape to return"),
                                   Vector(0, static_cast<float>(py)),
                                   text_layer,
                                   s_help_color);

  if (m_frame)
    context.color().draw_surface_scaled(m_frame, context.get_rect(), panel_layer + 2);
}

IntegrationStatus
GlobalStatsScreen::get_status() const
{
  IntegrationStatus status;
  status.m_details.push_back("Viewing global statistics");
  return status;
}

void
GlobalStatsScreen::draw_background(DrawingContext& context) const
{
  constexpr int panel_layer = LAYER_GUI;

  if (m_titlesession)
  {
    m_titlesession->get_current_sector().draw(context);
  }
  else
  {
    context.set_ambient_color(Color(1.f, 1.f, 1.f, 1.f));
    context.color().draw_gradient(Color(0.53f, 0.71f, 0.96f, 1.f),
                                  Color(0.94f, 0.97f, 1.00f, 1.f),
                                  LAYER_BACKGROUND0,
                                  VERTICAL,
                                  context.get_rect());
  }

  const float panel_width = std::min(context.get_width() - 120.f, 760.f);
  const float panel_height = 360.f;
  const Rectf panel(Vector((context.get_width() - panel_width) / 2.f,
                           (context.get_height() - panel_height) / 2.f - 10.f),
                    Sizef(panel_width, panel_height));

  if (m_backdrop)
  {
    context.color().draw_surface_scaled(m_backdrop, panel, panel_layer);
  }
  else
  {
    context.color().draw_filled_rect(panel,
                                     Color(0.18f, 0.25f, 0.35f, 0.72f),
                                     18.f,
                                     panel_layer);
  }
}

void
GlobalStatsScreen::draw_stat_line(DrawingContext& context, int& py,
                                  const std::string& label, const std::string& value) const
{
  constexpr int text_layer = LAYER_GUI + 1;
  const float center_x = context.get_width() / 2.f;
  const float label_x = center_x - 24.f;
  const float value_x = center_x + 24.f;

  context.color().draw_text(Resources::normal_font,
                            label + ":",
                            Vector(label_x, static_cast<float>(py)),
                            ALIGN_RIGHT,
                            text_layer,
                            s_label_color);
  context.color().draw_text(Resources::normal_font,
                            value,
                            Vector(value_x, static_cast<float>(py)),
                            ALIGN_LEFT,
                            text_layer,
                            s_value_color);

  py += static_cast<int>(Resources::normal_font->get_height());
}

std::string
GlobalStatsScreen::format_count(int value) const
{
  return fmt::format("{}", value);
}

std::string
GlobalStatsScreen::format_time(float value) const
{
  if (value <= 0.0f)
    return _("Not available");

  return Statistics::time_to_string(value);
}

void
GlobalStatsScreen::load_summary()
{
  m_summary = GlobalStatsManager().aggregate();
  s_cached_summary = m_summary;
  m_summary_ready = true;
}

void
GlobalStatsScreen::refresh_title_background()
{
  bool level_init = false;
  std::string title_level = DEFAULT_TITLE_LEVEL;

  if (g_config->custom_title_levels)
  {
    const std::string last_world = ProfileManager::current()->get_current_profile().get_last_world();
    if (!last_world.empty())
    {
      const std::string savegame_title_level = Savegame::from_current_profile(last_world, true)->get_player_status().title_level;
      if (savegame_title_level.empty())
      {
        const auto world = World::from_directory("levels/" + last_world);
        if (world)
          title_level = world->get_title_level();
      }
      else
      {
        title_level = savegame_title_level;
      }
    }
  }

  if (title_level.empty())
    title_level = DEFAULT_TITLE_LEVEL;

  if (!m_titlesession || m_titlesession->get_level_file() != title_level)
  {
    std::unique_ptr<GameSession> new_session;
    try
    {
      new_session = std::make_unique<GameSession>(title_level, *m_savegame, nullptr);
      new_session->restart_level(false, true);
    }
    catch (const std::exception&)
    {
      if (!m_titlesession || m_titlesession->get_level_file() != DEFAULT_TITLE_LEVEL)
      {
        new_session = std::make_unique<GameSession>(DEFAULT_TITLE_LEVEL, *m_savegame, nullptr);
        new_session->restart_level(false, true);
      }
    }

    if (new_session)
    {
      m_titlesession = std::move(new_session);
      level_init = true;
    }
  }

  if (!m_titlesession)
    return;

  Sector& sector = m_titlesession->get_current_sector();
  if (level_init || Sector::current() != &sector)
    setup_title_sector(sector);
}

void
GlobalStatsScreen::setup_title_sector(Sector& sector)
{
  auto& music = sector.get_singleton_by_type<MusicObject>();
  music.resume_music(true);

  Player& player = *(sector.get_players()[0]);
  sector.activate(player.get_pos() - Vector(0.f, player.is_big() ? 0.f : 32.f));
  player.set_controller(m_controller.get());
  player.set_speedlimit(230.f);
}

void
GlobalStatsScreen::update_title_background(float dt_sec)
{
  if (!m_titlesession)
    return;

  Sector& sector = m_titlesession->get_current_sector();
  Player& player = *(sector.get_players()[0]);

  if (player.is_dying())
  {
    m_titlesession->restart_level(true, true);
    setup_title_sector(m_titlesession->get_current_sector());
    return;
  }

  BIND_SECTOR(sector);
  sector.update(dt_sec);

  m_controller->update();
  m_controller->press(Control::RIGHT);

  const Rectf& bbox = player.get_bbox();
  const Vector eye(bbox.get_right(), bbox.get_top() + bbox.get_height() / 2);
  const Vector end(eye.x + 46.f, eye.y);

  auto result = sector.get_first_line_intersection(eye, end, false, player.get_collision_object());

  bool shouldjump = result.is_valid;
  if (shouldjump)
  {
    if (auto tile = std::get_if<const Tile*>(&result.hit))
      shouldjump = !(*tile)->is_slope();
    else if (auto obj = std::get_if<CollisionObject*>(&result.hit))
      shouldjump = ((*obj)->get_group() == COLGROUP_STATIC ||
                    (*obj)->get_group() == COLGROUP_MOVING_STATIC);
  }

  if (player.m_fall_mode == Player::FallMode::JUMPING ||
      (m_jump_was_released && shouldjump))
  {
    m_controller->press(Control::JUMP);
    m_jump_was_released = false;
  }
  else
  {
    m_jump_was_released = true;
  }

  if (sector.get_width() - 320.f < player.get_pos().x)
  {
    sector.activate(DEFAULT_SECTOR_NAME);
    sector.get_camera().reset(player.get_pos());
  }
}
