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

#include "scripting/squirrel_util.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

namespace {
const int nv_coins = std::numeric_limits<int>::min();
const int nv_badguys = std::numeric_limits<int>::min();
const float nv_time = std::numeric_limits<float>::max();
const int nv_secrets = std::numeric_limits<int>::min();
}

float WMAP_INFO_LEFT_X;
float WMAP_INFO_RIGHT_X;
float WMAP_INFO_TOP_Y1;
float WMAP_INFO_TOP_Y2;

Statistics::Statistics() :
  m_coins(nv_coins),
  m_total_coins(nv_coins),
  m_badguys(nv_badguys),
  m_total_badguys(nv_badguys),
  m_time(nv_time),
  m_secrets(nv_secrets),
  m_total_secrets(nv_secrets),
  m_valid(true),
  m_max_width(256),
  CAPTION_MAX_COINS(_("Max coins collected:")),
  CAPTION_MAX_FRAGGING(_("Max fragging:")),
  CAPTION_MAX_SECRETS(_("Max secrets found:")),
  CAPTION_BEST_TIME(_("Best time completed:")),
  CAPTION_TARGET_TIME(_("Level target time:"))
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

  for(const auto& caption : captions)
  {
    auto font = Resources::small_font;
    // Add padding the size of lengthiest string:
    auto width = font->get_text_width(caption) +
                 font->get_text_width("XX:XX:XX");
    if(width >= static_cast<float>(m_max_width))
    {
      m_max_width = static_cast<int>(width);
    }
  }
}

void
Statistics::serialize_to_squirrel(HSQUIRRELVM vm)
{
  scripting::begin_table(vm, "statistics");
  if (m_coins != nv_coins) scripting::store_int(vm, "coins-collected", m_coins);
  if (m_total_coins != nv_coins) scripting::store_int(vm, "coins-collected-total", m_total_coins);
  if (m_badguys != nv_badguys) scripting::store_int(vm, "badguys-killed", m_badguys);
  if (m_total_badguys != nv_badguys) scripting::store_int(vm, "badguys-killed-total", m_total_badguys);
  if (m_time != nv_time) scripting::store_float(vm, "time-needed", m_time);
  if (m_secrets != nv_secrets) scripting::store_int(vm, "secrets-found", m_secrets);
  if (m_total_secrets != nv_secrets) scripting::store_int(vm, "secrets-found-total", m_total_secrets);
  scripting::end_table(vm, "statistics");
}

void
Statistics::unserialize_from_squirrel(HSQUIRRELVM vm)
{
  try
  {
    scripting::get_table_entry(vm, "statistics");
    scripting::get_int(vm, "coins-collected", m_coins);
    scripting::get_int(vm, "coins-collected-total", m_total_coins);
    scripting::get_int(vm, "badguys-killed", m_badguys);
    scripting::get_int(vm, "badguys-killed-total", m_total_badguys);
    scripting::get_float(vm, "time-needed", m_time);
    scripting::get_int(vm, "secrets-found", m_secrets);
    scripting::get_int(vm, "secrets-found-total", m_total_secrets);
    sq_pop(vm, 1);
  }
  catch(const std::exception&)
  {
  }
}

void
Statistics::draw_worldmap_info(DrawingContext& context, float target_time)
{
  // skip draw if level was never played
  if (m_coins == nv_coins) return;

  // skip draw if stats were declared invalid
  if (!m_valid) return;

  // no sense drawing stats if there are none
  if (m_total_coins + m_total_badguys + m_total_secrets == 0) return;

  // check to see if screen size has been changed
  if (!(WMAP_INFO_TOP_Y1 == static_cast<float>(SCREEN_HEIGHT - 100))) {
    calculate_max_caption_length();
    WMAP_INFO_LEFT_X = static_cast<float>(context.get_width() - 32 - m_max_width);
    WMAP_INFO_RIGHT_X = WMAP_INFO_LEFT_X + static_cast<float>(m_max_width);
    WMAP_INFO_TOP_Y1 = static_cast<float>(SCREEN_HEIGHT - 100);
    WMAP_INFO_TOP_Y2 = WMAP_INFO_TOP_Y1 + 16;
  }

  context.color().draw_text(
    Resources::small_font, std::string("- ") + _("Best Level Statistics") + " -",
    Vector((WMAP_INFO_LEFT_X + WMAP_INFO_RIGHT_X) / 2, WMAP_INFO_TOP_Y1),
    ALIGN_CENTER, LAYER_HUD,Statistics::header_color);

  std::string caption_buf;
  std::string stat_buf;
  float posy = WMAP_INFO_TOP_Y2;

  for (int stat_no = 0; stat_no < 5; stat_no++) {
    switch (stat_no)
    {
      case 0:
        caption_buf = CAPTION_MAX_COINS;
        stat_buf = coins_to_string(m_coins, m_total_coins);
        break;
      case 1:
        caption_buf = CAPTION_MAX_FRAGGING;
        stat_buf = frags_to_string(m_badguys, m_total_badguys);
        break;
      case 2:
        caption_buf = CAPTION_MAX_SECRETS;
        stat_buf = secrets_to_string(m_secrets, m_total_secrets);
        break;
      case 3:
        caption_buf = CAPTION_BEST_TIME;
        stat_buf = time_to_string(m_time);
        break;
      case 4:
        if(target_time != 0.0f) { // display target time only if defined for level
          caption_buf = CAPTION_TARGET_TIME;
          stat_buf = time_to_string(target_time);
        } else {
          caption_buf = "";
          stat_buf = "";
        }
        break;
      default:
        log_debug << "Invalid stat requested to be drawn" << std::endl;
        break;
    }

    context.color().draw_text(Resources::small_font, caption_buf, Vector(WMAP_INFO_LEFT_X, posy), ALIGN_LEFT, LAYER_HUD, Statistics::header_color);
    context.color().draw_text(Resources::small_font, stat_buf, Vector(WMAP_INFO_RIGHT_X, posy), ALIGN_RIGHT, LAYER_HUD, Statistics::header_color);
    posy += Resources::small_font->get_height() + 2;
  }

}

void
Statistics::draw_endseq_panel(DrawingContext& context, Statistics* best_stats, SurfacePtr backdrop)
{
  // skip draw if stats were declared invalid
  if (!m_valid) return;

  // abort if we have no backdrop
  if (!backdrop) return;

  // no sense drawing stats if there are none
  if (m_total_coins + m_total_badguys + m_total_secrets == 0) return;

  int box_w = 220+110+110;
  int box_h = 30+20+20+20;
  int box_x = static_cast<int>((context.get_width() - box_w) / 2);
  int box_y = static_cast<int>(SCREEN_HEIGHT / 2) - box_h;

  int bd_w = static_cast<int>(backdrop->get_width());
  int bd_h = static_cast<int>(backdrop->get_height());
  int bd_x = static_cast<int>((context.get_width() - bd_w) / 2);
  int bd_y = box_y + (box_h / 2) - (bd_h / 2);

  float col1_x = static_cast<float>(box_x);
  float col2_x = col1_x + 200.0f;
  float col3_x = col2_x + 130.0f;

  float row1_y = static_cast<float>(box_y);
  float row2_y = row1_y + 30.0f;
  float row3_y = row2_y + 20.0f;
  float row4_y = row3_y + 20.0f;
  float row5_y = row4_y + 20.0f;

  context.push_transform();
  context.set_alpha(0.5f);
  context.color().draw_surface(backdrop, Vector(static_cast<float>(bd_x), static_cast<float>(bd_y)), LAYER_HUD);
  context.pop_transform();

  context.color().draw_text(Resources::normal_font, _("You"), Vector(col2_x, row1_y), ALIGN_LEFT, LAYER_HUD, Statistics::header_color);
  if (best_stats)
    context.color().draw_text(Resources::normal_font, _("Best"), Vector(col3_x, row1_y), ALIGN_LEFT, LAYER_HUD, Statistics::header_color);

  context.color().draw_text(Resources::normal_font, _("Coins"), Vector(col2_x - 16.0f, static_cast<float>(row3_y)), ALIGN_RIGHT, LAYER_HUD, Statistics::header_color);
  context.color().draw_text(Resources::normal_font, coins_to_string(m_coins, m_total_coins), Vector(col2_x, static_cast<float>(row3_y)), ALIGN_LEFT, LAYER_HUD, Statistics::text_color);

  if (best_stats) {
    int coins_best = (best_stats->m_coins > m_coins) ? best_stats->m_coins : m_coins;
    int total_coins_best = (best_stats->m_total_coins > m_total_coins) ? best_stats->m_total_coins : m_total_coins;
    context.color().draw_text(Resources::normal_font, coins_to_string(coins_best, total_coins_best), Vector(col3_x, static_cast<float>(row3_y)), ALIGN_LEFT, LAYER_HUD, Statistics::text_color);
  }

  context.color().draw_text(Resources::normal_font, _("Badguys"), Vector(col2_x - 16.0f, static_cast<float>(row4_y)), ALIGN_RIGHT, LAYER_HUD, Statistics::header_color);
  context.color().draw_text(Resources::normal_font, frags_to_string(m_badguys, m_total_badguys), Vector(col2_x, static_cast<float>(row4_y)), ALIGN_LEFT, LAYER_HUD, Statistics::text_color);
  if (best_stats) {
	int badguys_best = (best_stats->m_badguys > m_badguys) ? best_stats->m_badguys : m_badguys;
	int total_badguys_best = (best_stats->m_total_badguys > m_total_badguys) ? best_stats->m_total_badguys : m_total_badguys;
	context.color().draw_text(Resources::normal_font, frags_to_string(badguys_best, total_badguys_best), Vector(col3_x, row4_y), ALIGN_LEFT, LAYER_HUD, Statistics::text_color);
  }

  context.color().draw_text(Resources::normal_font, _("Secrets"), Vector(col2_x-16, row5_y), ALIGN_RIGHT, LAYER_HUD, Statistics::header_color);
  context.color().draw_text(Resources::normal_font, secrets_to_string(m_secrets, m_total_secrets), Vector(col2_x, row5_y), ALIGN_LEFT, LAYER_HUD, Statistics::text_color);
  if (best_stats) {
    int secrets_best = (best_stats->m_secrets > m_secrets) ? best_stats->m_secrets : m_secrets;
    int total_secrets_best = (best_stats->m_total_secrets > m_total_secrets) ? best_stats->m_total_secrets : m_total_secrets;
    context.color().draw_text(Resources::normal_font, secrets_to_string(secrets_best, total_secrets_best), Vector(col3_x, row5_y), ALIGN_LEFT, LAYER_HUD, Statistics::text_color);
  }

  context.color().draw_text(Resources::normal_font, _("Time"), Vector(col2_x - 16, row2_y), ALIGN_RIGHT, LAYER_HUD, Statistics::header_color);
  context.color().draw_text(Resources::normal_font, time_to_string(m_time), Vector(col2_x, row2_y), ALIGN_LEFT, LAYER_HUD, Statistics::text_color);
  if (best_stats) {
    float time_best = (best_stats->m_time < m_time) ? best_stats->m_time : m_time;
    context.color().draw_text(Resources::normal_font, time_to_string(time_best), Vector(col3_x, row2_y), ALIGN_LEFT, LAYER_HUD, Statistics::text_color);
  }
}

void
Statistics::zero()
{
  reset();
  m_total_coins = 0;
  m_total_badguys = 0;
  m_total_secrets = 0;
}

void
Statistics::reset()
{
  m_coins = 0;
  m_badguys = 0;
  m_time = 0;
  m_secrets = 0;
}

void
Statistics::merge(const Statistics& s2)
{
  if (!s2.m_valid) return;
  m_coins = std::max(m_coins, s2.m_coins);
  m_total_coins = s2.m_total_coins;
  m_coins = std::min(m_coins, m_total_coins);
  m_badguys = std::max(m_badguys, s2.m_badguys);
  m_total_badguys = s2.m_total_badguys;
  m_badguys = std::min(m_badguys, m_total_badguys);
  m_time = std::min(m_time, s2.m_time);
  m_secrets = std::max(m_secrets, s2.m_secrets);
  m_total_secrets = s2.m_total_secrets;
  m_secrets = std::min(m_secrets, m_total_secrets);
}

void
Statistics::operator+=(const Statistics& s2)
{
  if (!s2.m_valid) return;
  if (s2.m_coins != nv_coins) m_coins += s2.m_coins;
  if (s2.m_total_coins != nv_coins) m_total_coins += s2.m_total_coins;
  if (s2.m_badguys != nv_badguys) m_badguys += s2.m_badguys;
  if (s2.m_total_badguys != nv_badguys) m_total_badguys += s2.m_total_badguys;
  if (s2.m_time != nv_time) m_time += s2.m_time;
  if (s2.m_secrets != nv_secrets) m_secrets += s2.m_secrets;
  if (s2.m_total_secrets != nv_secrets) m_total_secrets += s2.m_total_secrets;
}

bool
Statistics::completed(const Statistics& stats, const float target_time) const
{
  return (stats.m_coins == stats.m_total_coins &&
      stats.m_badguys == stats.m_total_badguys &&
      stats.m_secrets == stats.m_total_secrets &&
      ((target_time == 0.0f) || (stats.m_time <= target_time)));
}

std::string
Statistics::coins_to_string(int coins, int total_coins) {
  std::ostringstream os;
  os << std::min(std::min(coins, total_coins), 999) << "/" << std::min(total_coins, 999);
  return os.str();
}

std::string
Statistics::frags_to_string(int badguys, int total_badguys) {
  std::ostringstream os;
  os << std::min(std::min(badguys, total_badguys), 999) << "/" << std::min(total_badguys, 999);
  return os.str();
}

std::string
Statistics::time_to_string(float time) {
  int time_csecs = std::min(static_cast<int>(time * 100), 99 * 6000 + 9999);
  int mins = (time_csecs / 6000);
  int secs = (time_csecs % 6000) / 100;
  int cscs = (time_csecs % 6000) % 100;

  std::ostringstream os;
  os << std::setw(2) << std::setfill('0') << mins << ":" << std::setw(2) << std::setfill('0') << secs << "." << std::setw(2) << std::setfill('0') << cscs;
  return os.str();
}

std::string
Statistics::secrets_to_string(int secrets, int total_secrets) {
  std::ostringstream os;
  os << std::min(secrets, 999) << "/" << std::min(total_secrets, 999);
  return os.str();
}

/* EOF */
