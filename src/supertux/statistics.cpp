//  SuperTux (Statistics module)
//  Copyright (C) 2004 Ricardo Cruz <rick2@aeiou.pt>
//  Copyright (C) 2006 Ondrej Hosek <ondra.hosek@gmail.com>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "supertux/statistics.hpp"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <numeric>

#include <simplesquirrel/table.hpp>

#include "audio/sound_manager.hpp"
#include "gui/menu.hpp"
#include "math/util.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/resources.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

static const float INGAME_STATS_DISTANCE = 1.7f;

Statistics::Preferences::Preferences() :
  enable_coins(true),
  enable_secrets(true)
{
}

void
Statistics::Preferences::parse(const ReaderMapping& reader)
{
  reader.get("enable-coins", enable_coins);
  reader.get("enable-secrets", enable_secrets);
}

void
Statistics::Preferences::write(Writer& writer) const
{
  writer.write("enable-coins", enable_coins);
  writer.write("enable-secrets", enable_secrets);
}

void
Statistics::Preferences::add_to_menu(Menu& menu)
{
  menu.add_toggle(-1, _("Enable Coins Statistic"), &enable_coins);
  menu.add_toggle(-1, _("Enable Secrets Statistic"), &enable_secrets);
}


Statistics::Statistics() :
  m_status(INVALID),
  m_total_coins(),
  m_total_secrets(),
  m_coins(),
  m_tuxdolls(),
  m_secrets(),
  m_time(),
  m_cleared_coins(false),
  m_cleared_secrets(false),
  m_coins_time(0.f),
  m_secrets_time(0.f),
  m_preferences(),
  m_max_width(256),
  CAPTION_MAX_COINS(_("Max coins collected:")),
  CAPTION_MAX_FRAGGING(_("Max fragging:")),
  CAPTION_MAX_SECRETS(_("Max secrets found:")),
  CAPTION_BEST_TIME(_("Best time completed:")),
  CAPTION_TARGET_TIME(_("Level target time:")),
  CAPTION_TUXDOLLS(_("Tux Dolls collected:")),
  WMAP_INFO_LEFT_X(),
  WMAP_INFO_RIGHT_X(),
  WMAP_INFO_TOP_Y1(),
  WMAP_INFO_TOP_Y2(),
  coin_icon(Surface::from_file("/images/engine/hud/coin-icon.png")),
  secret_icon(Surface::from_file("/images/engine/hud/secret-icon.png"))
{
  calculate_max_caption_length();
  WMAP_INFO_LEFT_X = static_cast<float>(SCREEN_WIDTH) - 32.0f - static_cast<float>(m_max_width);
  WMAP_INFO_RIGHT_X = WMAP_INFO_LEFT_X + static_cast<float>(m_max_width);
  WMAP_INFO_TOP_Y1 = static_cast<float>(SCREEN_HEIGHT) - 100.0f;
  WMAP_INFO_TOP_Y2 = WMAP_INFO_TOP_Y1 + 16.0f;
}

void
Statistics::calculate_max_caption_length()
{
  auto captions = {CAPTION_MAX_COINS, CAPTION_MAX_FRAGGING, CAPTION_MAX_SECRETS,
                   CAPTION_BEST_TIME, CAPTION_TARGET_TIME};

  m_max_width = 256;

  for (const auto& caption : captions)
  {
    auto font = Resources::small_font;
    // Add padding the size of lengthiest string:
    auto width = font->get_text_width(caption) +
                 font->get_text_width("XX:XX:XX");
    if (width >= static_cast<float>(m_max_width))
    {
      m_max_width = static_cast<int>(width);
    }
  }
}

void
Statistics::serialize_to_squirrel(ssq::Table& table) const
{
  if (m_status != FINAL) return;

  ssq::Table statistics = table.addTable("statistics");
  statistics.set("coins-collected", m_coins);
  statistics.set("tuxdolls-collected", m_tuxdolls);
  statistics.set("secrets-found", m_secrets);
  statistics.set("time-needed", m_time);
  statistics.set("coins-collected-total", m_total_coins);
  statistics.set("secrets-found-total", m_total_secrets);
}

void
Statistics::unserialize_from_squirrel(const ssq::Table& table)
{
  try
  {
    const ssq::Table statistics = table.findTable("statistics");
    statistics.get("coins-collected", m_coins);
    statistics.get("tuxdolls-collected", m_tuxdolls);
    statistics.get("secrets-found", m_secrets);
    statistics.get("time-needed", m_time);
    statistics.get("coins-collected-total", m_total_coins);
    statistics.get("secrets-found-total", m_total_secrets);

    m_status = FINAL;
  }
  catch(const ssq::NotFoundException&)
  {
    // ignore non-existing or malformed statistics table
  }
}

void
Statistics::add_preferences_to_menu(Menu& menu)
{
  m_preferences.add_to_menu(menu);
}

void
Statistics::draw_worldmap_info(DrawingContext& context, float target_time)
{
  if (m_status != FINAL) return;

  // check to see if screen size has been changed
  if (!(WMAP_INFO_TOP_Y1 == static_cast<float>(SCREEN_HEIGHT - 100))) {
    calculate_max_caption_length();
    WMAP_INFO_LEFT_X = context.get_width() - 32.f - static_cast<float>(m_max_width);
    WMAP_INFO_RIGHT_X = WMAP_INFO_LEFT_X + static_cast<float>(m_max_width);
    WMAP_INFO_TOP_Y1 = static_cast<float>(SCREEN_HEIGHT - 100);
    WMAP_INFO_TOP_Y2 = WMAP_INFO_TOP_Y1 + 16;
  }

  context.color().draw_text(
    Resources::small_font, "- " + _("Best Level Statistics") + " -",
    Vector((WMAP_INFO_LEFT_X + WMAP_INFO_RIGHT_X) / 2, WMAP_INFO_TOP_Y1),
    ALIGN_CENTER, LAYER_HUD,Statistics::header_color);

  std::string caption_buf;
  std::string stat_buf;
  float posy = WMAP_INFO_TOP_Y2;
  Color hcolor = Statistics::header_color;
  Color tcolor;

  for (int stat_no = 0; stat_no < 5; stat_no++) {
    tcolor = Statistics::header_color;
    switch (stat_no)
    {
      case 0:
        if (!m_preferences.enable_coins)
          continue;

        caption_buf = CAPTION_MAX_COINS;
        stat_buf = coins_to_string(m_coins, m_total_coins);
        if (m_coins >= m_total_coins)
          tcolor = Statistics::perfect_color;
        break;
      case 1:
        if (!m_preferences.enable_secrets)
          continue;

        caption_buf = CAPTION_MAX_SECRETS;
        stat_buf = secrets_to_string(m_secrets, m_total_secrets);
        if (m_secrets >= m_total_secrets)
          tcolor = Statistics::perfect_color;
        break;
      case 2:
        caption_buf = CAPTION_BEST_TIME;
        stat_buf = time_to_string(m_time);
        if ((m_time < target_time) || (target_time == 0.0f))
          tcolor = Statistics::perfect_color;
        break;
      case 3:
        if (target_time != 0.0f) { // display target time only if defined for level
          caption_buf = CAPTION_TARGET_TIME;
          stat_buf = time_to_string(target_time);
          if ((m_time < target_time) || (target_time == 0.0f))
            tcolor = Statistics::perfect_color;
        } else {
          caption_buf = "";
          stat_buf = "";
        }
        break;
      case 4:
        if (m_tuxdolls <= 0)
          continue;

        caption_buf = CAPTION_TUXDOLLS;
        stat_buf = std::to_string(m_tuxdolls);
        hcolor = Statistics::special_color;
        tcolor = Statistics::special_color;
        break;
      default:
        assert(false);
        break;
    }

    context.color().draw_text(Resources::small_font, caption_buf, Vector(WMAP_INFO_LEFT_X, posy), ALIGN_LEFT, LAYER_HUD, hcolor);
    context.color().draw_text(Resources::small_font, stat_buf, Vector(WMAP_INFO_RIGHT_X, posy), ALIGN_RIGHT, LAYER_HUD, tcolor);
    posy += Resources::small_font->get_height() + 2;
  }
}

void
Statistics::draw_endseq_panel(DrawingContext& context, Statistics* best_stats, const SurfacePtr& backdrop, float target_time)
{
  if (m_status != FINAL) return;

  constexpr int layer = LAYER_GUI + 50;
  constexpr float row_height = 22.f;
  constexpr float header_padding_top = 10.f;
  constexpr float padding_top = 40.f;
  constexpr float padding_bottom = 10.f;
  constexpr float label_indent = 16.f;

  int visible_rows = 1;
  if (m_preferences.enable_coins)   ++visible_rows;
  if (m_preferences.enable_secrets) ++visible_rows;

  const std::vector<float> column_widths = { 215.f, 130.f, 130.f };

  const float box_w = std::accumulate(column_widths.begin(), column_widths.end(), 0.f);
  const float box_h = padding_top + (static_cast<float>(visible_rows) * row_height) + padding_bottom;
  const float box_x = ((context.get_width() - box_w) / 2.f);

  const float box_y = (static_cast<float>(SCREEN_HEIGHT) - box_h) / 2.f;

  std::vector<float> col_x_positions;
  col_x_positions.reserve(column_widths.size());
  float current_x = box_x;
  for (const float width : column_widths)
  {
    col_x_positions.push_back(current_x);
    current_x += width;
  }

  const float bd_w = static_cast<float>(backdrop->get_width());
  const float bd_h = static_cast<float>(backdrop->get_height());
  const float bd_x = (context.get_width() - bd_w) / 2.f;
  const float bd_y = box_y + (box_h / 2.f) - (bd_h / 2.f);

  context.push_transform();
  context.set_alpha(0.5f);
  context.color().draw_surface(backdrop, Vector(bd_x, bd_y), layer);
  context.pop_transform();

  const float header_y = box_y + header_padding_top;
  context.color().draw_text(Resources::normal_font, _("You"), Vector(col_x_positions[1], header_y), ALIGN_LEFT, layer, Statistics::header_color);
  if (best_stats)
    context.color().draw_text(Resources::normal_font, _("Best"), Vector(col_x_positions[2], header_y), ALIGN_LEFT, layer, Statistics::header_color);

  float y = box_y + padding_top;

  Color tcolor = Statistics::text_color;
  if (target_time == 0.0f || (m_time != 0.0f && m_time < target_time))
    tcolor = Statistics::perfect_color;

  context.color().draw_text(Resources::normal_font, _("Time"), Vector(col_x_positions[1] - label_indent, y), ALIGN_RIGHT, layer, Statistics::header_color);
  context.color().draw_text(Resources::normal_font, time_to_string(m_time), Vector(col_x_positions[1], y), ALIGN_LEFT, layer, tcolor);
  if (best_stats)
  {
    float time_best = (best_stats->m_time < m_time && best_stats->m_time > 0.0f) ? best_stats->m_time : m_time;
    if (target_time == 0.0f || (time_best != 0.0f && time_best < target_time))
      tcolor = Statistics::perfect_color;
    else
      tcolor = Statistics::text_color;
    context.color().draw_text(Resources::normal_font, time_to_string(time_best), Vector(col_x_positions[2], y), ALIGN_LEFT, layer, tcolor);
  }

  if (m_preferences.enable_coins)
  {
    y += row_height;

    context.color().draw_text(Resources::normal_font, _("Coins"), Vector(col_x_positions[1] - label_indent, y), ALIGN_RIGHT, layer, Statistics::header_color);

    if (m_coins >= m_total_coins)
      tcolor = Statistics::perfect_color;
    else
      tcolor = Statistics::text_color;
    context.color().draw_text(Resources::normal_font, coins_to_string(m_coins, m_total_coins), Vector(col_x_positions[1], y), ALIGN_LEFT, layer, tcolor);

    if (best_stats)
    {
      int coins_best = (best_stats->m_coins > m_coins) ? best_stats->m_coins : m_coins;
      int total_coins_best = (best_stats->m_total_coins > m_total_coins) ? best_stats->m_total_coins : m_total_coins;
      if (coins_best >= total_coins_best)
        tcolor = Statistics::perfect_color;
      else
        tcolor = Statistics::text_color;
      context.color().draw_text(Resources::normal_font, coins_to_string(coins_best, total_coins_best), Vector(col_x_positions[2], y), ALIGN_LEFT, layer, tcolor);
    }
  }

  if (m_preferences.enable_secrets)
  {
    y += row_height;

    if (m_secrets >= m_total_secrets)
      tcolor = Statistics::perfect_color;
    else
      tcolor = Statistics::text_color;
    context.color().draw_text(Resources::normal_font, _("Secrets"), Vector(col_x_positions[1] - label_indent, y), ALIGN_RIGHT, layer, Statistics::header_color);
    context.color().draw_text(Resources::normal_font, secrets_to_string(m_secrets, m_total_secrets), Vector(col_x_positions[1], y), ALIGN_LEFT, layer, tcolor);

    if (best_stats)
    {
      int secrets_best = (best_stats->m_secrets > m_secrets) ? best_stats->m_secrets : m_secrets;
      int total_secrets_best = (best_stats->m_total_secrets > m_total_secrets) ? best_stats->m_total_secrets : m_total_secrets;
      if (secrets_best >= total_secrets_best)
        tcolor = Statistics::perfect_color;
      else
        tcolor = Statistics::text_color;
      context.color().draw_text(Resources::normal_font, secrets_to_string(secrets_best, total_secrets_best), Vector(col_x_positions[2], y), ALIGN_LEFT, layer, tcolor);
    }
  }
}

void
Statistics::draw_ingame_stats(DrawingContext& context, bool on_pause_menu)
{
  const float height = Resources::normal_font->get_height();
  float y = context.get_height() - height * 4.f;

  if (m_preferences.enable_secrets)
  {
    if (on_pause_menu || (m_cleared_secrets && m_secrets_time < 5.f))
    {
      std::string text(secrets_to_string(m_secrets, m_total_secrets));
      float width = Resources::normal_font->get_text_width(text),
            x_offset = width + 75.f;

      if (!on_pause_menu)
        x_offset *= std::min(1.f, -std::abs(m_secrets_time - 2.5f) + 2.5f);

      Vector pos(context.get_width() - x_offset, y);

      context.color().draw_filled_rect(Rectf(pos.x, pos.y, pos.x + width + 37.f,
                                             pos.y + height).grown(5.f),
                                       Color(0.f, 0.f, 0.f, 0.5f),
                                       10.f, LAYER_HUD - 1);
      context.color().draw_text(Resources::normal_font, text, pos,
                                FontAlignment::ALIGN_LEFT, LAYER_HUD,
                                (m_secrets < m_total_secrets)
                                      ? Statistics::text_color
                                      : Statistics::perfect_color
                                );
      context.color().draw_surface_scaled(secret_icon,
                                          Rectf(Vector(pos.x + width + 3.f, pos.y - 5.f), Sizef(32.f, 32.f)),
                                          LAYER_HUD);
    }

    y -= height * INGAME_STATS_DISTANCE;
  }

  if (m_preferences.enable_coins && (on_pause_menu || (m_cleared_coins && m_coins_time < 5.f)))
  {
    std::string text(coins_to_string(m_coins, m_total_coins));
    float width = Resources::normal_font->get_text_width(text),
          x_offset = width + 75.f;

    if (!on_pause_menu)
      x_offset *= std::min(1.f, -std::abs(m_coins_time - 2.5f) + 2.5f);

    Vector pos(context.get_width() - x_offset, y);

    context.color().draw_filled_rect(Rectf(pos.x, pos.y, pos.x + width + 37.f,
                                           pos.y + height).grown(5.f),
                                     Color(0.f, 0.f, 0.f, 0.5f),
                                     10.f, LAYER_HUD - 1);
    context.color().draw_text(Resources::normal_font, text, pos,
                              FontAlignment::ALIGN_LEFT, LAYER_HUD,
                              (m_coins < m_total_coins)
                                    ? Statistics::text_color
                                    : Statistics::perfect_color
                              );
    context.color().draw_surface_scaled(coin_icon,
                                        Rectf(Vector(pos.x + width + 3.f, pos.y - 5.f), Sizef(32.f, 32.f)),
                                        LAYER_HUD);
  }
}

void
Statistics::update_timers(float dt_sec)
{
  if (m_cleared_coins)
    m_coins_time += dt_sec;

  if (m_cleared_secrets)
    m_secrets_time += dt_sec;
}

void
Statistics::init(const Level& level)
{
  m_status = ACCUMULATING;

  m_coins = 0;
  m_secrets = 0;

  m_total_coins = level.get_total_coins();
  m_total_secrets = level.get_total_secrets();
}

void
Statistics::finish(float time)
{
  m_status = FINAL;
  m_time = time;
}

void
Statistics::invalidate()
{
  m_status = INVALID;
}

void
Statistics::update(const Statistics& other)
{
  if (other.m_status != FINAL) return;

  m_coins = std::max(m_coins, other.m_coins);
  m_tuxdolls = std::max(m_tuxdolls, other.m_tuxdolls);
  m_secrets = std::max(m_secrets, other.m_secrets);
  if (m_time == 0)
    m_time = other.m_time;
  else
    m_time = std::min(m_time, other.m_time);

  m_total_coins = other.m_total_coins;
  m_total_secrets = other.m_total_secrets;

  m_preferences = other.m_preferences;

  m_coins = math::clamp(m_coins, 0, m_total_coins);
  m_secrets = math::clamp(m_secrets, 0, m_total_secrets);
  m_status = FINAL;
}

bool
Statistics::completed(const float target_time) const
{
  return ((!m_preferences.enable_coins || m_coins == m_total_coins) &&
          (!m_preferences.enable_secrets || m_secrets == m_total_secrets) &&
          (target_time == 0.0f || m_time <= target_time));
}

std::string
Statistics::coins_to_string(int coins, int total_coins)
{
  std::ostringstream os;
  os << std::min(std::min(coins, total_coins), 999) << "/" << std::min(total_coins, 999);
  return os.str();
}

std::string
Statistics::secrets_to_string(int secrets, int total_secrets)
{
  std::ostringstream os;
  os << std::min(secrets, 999) << "/" << std::min(total_secrets, 999);
  return os.str();
}

std::string
Statistics::time_to_string(float time)
{
  std::ostringstream os;
  if (time == 0.0f)
  {
    os << "--:--:--";
  }
  else
  {
    int time_csecs = static_cast<int>(time * 100);
    int mins = (time_csecs / 6000);
    int secs = (time_csecs % 6000) / 100;
    int cscs = (time_csecs % 6000) % 100;
    os << std::setw(2) << std::setfill('0') << mins << ":" << std::setw(2) << std::setfill('0') << secs << "." << std::setw(2) << std::setfill('0') << cscs;
  }

  return os.str();
}

void
Statistics::check_coins()
{
  if (!m_preferences.enable_coins || m_cleared_coins)
    return;

  if (m_coins >= m_total_coins)
  {
    m_cleared_coins = true;
    SoundManager::current()->play("/sounds/coins_cleared.ogg", 0.5f);
  }
}

void
Statistics::check_secrets()
{
  if (!m_preferences.enable_secrets || m_cleared_secrets)
    return;

  if (m_secrets >= m_total_secrets)
  {
    m_cleared_secrets = true;
    SoundManager::current()->play("/sounds/tada.ogg", 0.5f);
  }
}
