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
#include <config.h>

#include <assert.h>
#include <math.h>
#include <sstream>
#include <iomanip>
#include <limits>
#include "video/drawing_context.hpp"
#include "gettext.hpp"
#include "lisp/writer.hpp"
#include "lisp/lisp.hpp"
#include "resources.hpp"
#include "main.hpp"
#include "statistics.hpp"
#include "log.hpp"
#include "scripting/squirrel_util.hpp"

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

Statistics::Statistics() : coins(nv_coins), total_coins(nv_coins), badguys(nv_badguys), total_badguys(nv_badguys), time(nv_time), secrets(nv_secrets), total_secrets(nv_secrets), valid(true) 
{
  WMAP_INFO_LEFT_X = (SCREEN_WIDTH/2 + 80) + 32;
  WMAP_INFO_RIGHT_X = SCREEN_WIDTH/2 + 368;
  WMAP_INFO_TOP_Y1 = SCREEN_HEIGHT/2 + 172 - 16;
  WMAP_INFO_TOP_Y2 = SCREEN_HEIGHT/2 + 172;
}

Statistics::~Statistics()
{
}

/*
void
Statistics::parse(const lisp::Lisp& reader)
{
  reader.get("coins-collected", coins);
  reader.get("coins-collected-total", total_coins);
  reader.get("badguys-killed", badguys);
  reader.get("badguys-killed-total", total_badguys);
  reader.get("time-needed", time);
  reader.get("secrets-found", secrets);
  reader.get("secrets-found-total", total_secrets);
}

void
Statistics::write(lisp::Writer& writer)
{
  writer.write("coins-collected", coins);
  writer.write("coins-collected-total", total_coins);
  writer.write("badguys-killed", badguys);
  writer.write("badguys-killed-total", total_badguys);
  writer.write("time-needed", time);
  writer.write("secrets-found", secrets);
  writer.write("secrets-found-total", total_secrets);
}
*/

void
Statistics::serialize_to_squirrel(HSQUIRRELVM vm)
{
  // TODO: there's some bug in the unserialization routines that breaks stuff when an empty statistics table is written, so -- as a workaround -- let's make sure we will actually write something first
  if (!((coins != nv_coins) || (total_coins != nv_coins) || (badguys != nv_badguys) || (total_badguys != nv_badguys) || (time != nv_time) || (secrets != nv_secrets) || (total_secrets != nv_secrets))) return;

  sq_pushstring(vm, "statistics", -1);
  sq_newtable(vm);
  if (coins != nv_coins) Scripting::store_int(vm, "coins-collected", coins);
  if (total_coins != nv_coins) Scripting::store_int(vm, "coins-collected-total", total_coins);
  if (badguys != nv_badguys) Scripting::store_int(vm, "badguys-killed", badguys);
  if (total_badguys != nv_badguys) Scripting::store_int(vm, "badguys-killed-total", total_badguys);
  if (time != nv_time) Scripting::store_float(vm, "time-needed", time);
  if (secrets != nv_secrets) Scripting::store_int(vm, "secrets-found", secrets);
  if (total_secrets != nv_secrets) Scripting::store_int(vm, "secrets-found-total", total_secrets);
  sq_createslot(vm, -3);
}

void
Statistics::unserialize_from_squirrel(HSQUIRRELVM vm)
{
  sq_pushstring(vm, "statistics", -1);
  if(SQ_FAILED(sq_get(vm, -2))) {
    return;
  }
  Scripting::get_int(vm, "coins-collected", coins);
  Scripting::get_int(vm, "coins-collected-total", total_coins);
  Scripting::get_int(vm, "badguys-killed", badguys);
  Scripting::get_int(vm, "badguys-killed-total", total_badguys);
  Scripting::get_float(vm, "time-needed", time);
  Scripting::get_int(vm, "secrets-found", secrets);
  Scripting::get_int(vm, "secrets-found-total", total_secrets);
  sq_pop(vm, 1);
}

void
Statistics::draw_worldmap_info(DrawingContext& context)
{
  // skip draw if level was never played
  if (coins == nv_coins) return;

  // skip draw if stats were declared invalid
  if (!valid) return;

  context.draw_text(small_font, std::string("- ") + _("Best Level Statistics") + " -", Vector((WMAP_INFO_LEFT_X + WMAP_INFO_RIGHT_X) / 2, WMAP_INFO_TOP_Y1), ALIGN_CENTER, LAYER_GUI,Statistics::header_color);

  std::string caption_buf;
  std::string stat_buf;
  float posy = WMAP_INFO_TOP_Y2;
  for (int stat_no = 0; stat_no < 4; stat_no++) {
    switch (stat_no)
    {
      case 0:
          caption_buf = _("Max coins collected:");
          stat_buf = coins_to_string(coins, total_coins);
          break;
      case 1:
          caption_buf = _("Max fragging:");
          stat_buf = frags_to_string(badguys, total_badguys);
          break;
      case 2:
          caption_buf = _("Min time needed:");
          stat_buf = time_to_string(time);
          break;
      case 3:
          caption_buf = _("Max secrets found:");
          stat_buf = secrets_to_string(secrets, total_secrets);
          break;
      default:
          log_debug << "Invalid stat requested to be drawn" << std::endl;
          break;
    }

    context.draw_text(small_font, caption_buf, Vector(WMAP_INFO_LEFT_X, posy), ALIGN_LEFT, LAYER_GUI, Statistics::header_color);
    context.draw_text(small_font, stat_buf, Vector(WMAP_INFO_RIGHT_X, posy), ALIGN_RIGHT, LAYER_GUI, Statistics::header_color);
    posy += small_font->get_height() + 2;
  }

}

void
Statistics::draw_endseq_panel(DrawingContext& context, Statistics* best_stats, Surface* backdrop)
{
  // skip draw if level was never played
  // TODO: do we need this?
  if (coins == nv_coins) return;

  // skip draw if stats were declared invalid
  if (!valid) return;

  // abort if we have no backdrop
  if (!backdrop) return;

  int box_w = 220+110+110;
  int box_h = 30+20+20+20;
  int box_x = (int)((SCREEN_WIDTH - box_w) / 2);
  int box_y = (int)(SCREEN_HEIGHT / 2) - box_h;

  int bd_w = (int)backdrop->get_width();
  int bd_h = (int)backdrop->get_height();
  int bd_x = (int)((SCREEN_WIDTH - bd_w) / 2);
  int bd_y = box_y + (box_h / 2) - (bd_h / 2);

  int col1_x = box_x;
  int col2_x = col1_x+200;
  int col3_x = col2_x+130;

  int row1_y = box_y;
  int row2_y = row1_y+30;
  int row3_y = row2_y+20;
  int row4_y = row3_y+20;

  context.push_transform();
  context.set_alpha(0.5);
  context.draw_surface(backdrop, Vector(bd_x, bd_y), LAYER_GUI);
  context.pop_transform();

  context.draw_text(normal_font, _("You"), Vector(col2_x, row1_y), ALIGN_LEFT, LAYER_GUI, Statistics::header_color);
  context.draw_text(normal_font, _("Best"), Vector(col3_x, row1_y), ALIGN_LEFT, LAYER_GUI, Statistics::header_color);

  context.draw_text(normal_font, _("Coins"), Vector(col2_x-16, row3_y), ALIGN_RIGHT, LAYER_GUI, Statistics::header_color);
  int coins_best = (best_stats && (best_stats->coins > coins)) ? best_stats->coins : coins;
  int total_coins_best = (best_stats && (best_stats->total_coins > total_coins)) ? best_stats->total_coins : total_coins;
  context.draw_text(normal_font, coins_to_string(coins, total_coins), Vector(col2_x, row3_y), ALIGN_LEFT, LAYER_GUI, Statistics::text_color);
  context.draw_text(normal_font, coins_to_string(coins_best, total_coins_best), Vector(col3_x, row3_y), ALIGN_LEFT, LAYER_GUI, Statistics::text_color);

  context.draw_text(normal_font, _("Secrets"), Vector(col2_x-16, row4_y), ALIGN_RIGHT, LAYER_GUI, Statistics::header_color);
  int secrets_best = (best_stats && (best_stats->secrets > secrets)) ? best_stats->secrets : secrets;
  int total_secrets_best = (best_stats && (best_stats->total_secrets > total_secrets)) ? best_stats->total_secrets : total_secrets;
  context.draw_text(normal_font, secrets_to_string(secrets, total_secrets), Vector(col2_x, row4_y), ALIGN_LEFT, LAYER_GUI, Statistics::text_color);
  context.draw_text(normal_font, secrets_to_string(secrets_best, total_secrets_best), Vector(col3_x, row4_y), ALIGN_LEFT, LAYER_GUI, Statistics::text_color);

  context.draw_text(normal_font, _("Time"), Vector(col2_x-16, row2_y), ALIGN_RIGHT, LAYER_GUI, Statistics::header_color);
  float time_best = (best_stats && (best_stats->time < time)) ? best_stats->time : time;
  context.draw_text(normal_font, time_to_string(time), Vector(col2_x, row2_y), ALIGN_LEFT, LAYER_GUI, Statistics::text_color);
  context.draw_text(normal_font, time_to_string(time_best), Vector(col3_x, row2_y), ALIGN_LEFT, LAYER_GUI, Statistics::text_color);
}

void
Statistics::zero()
{
  reset();
  total_coins = 0;
  total_badguys = 0;
  total_secrets = 0;
}

void
Statistics::reset()
{
  coins = 0;
  badguys = 0;
  time = 0;
  secrets = 0;
}

void
Statistics::merge(const Statistics& s2)
{
  if (!s2.valid) return;
  coins = std::max(coins, s2.coins);
  total_coins = s2.total_coins;
  badguys = std::max(badguys, s2.badguys);
  total_badguys = s2.total_badguys;
  time = std::min(time, s2.time);
  secrets = std::max(secrets, s2.secrets);
  total_secrets = s2.total_secrets;
}

void
Statistics::operator+=(const Statistics& s2)
{
  if (!s2.valid) return;
  if (s2.coins != nv_coins) coins += s2.coins;
  if (s2.total_coins != nv_coins) total_coins += s2.total_coins;
  if (s2.badguys != nv_badguys) badguys += s2.badguys;
  if (s2.total_badguys != nv_badguys) total_badguys += s2.total_badguys;
  if (s2.time != nv_time) time += s2.time;
  if (s2.secrets != nv_secrets) secrets += s2.secrets;
  if (s2.total_secrets != nv_secrets) total_secrets += s2.total_secrets;
}

void
Statistics::declare_invalid()
{
  valid = false;
}

std::string
Statistics::coins_to_string(int coins, int total_coins) {
  std::ostringstream os;
  os << std::min(coins, 999) << "/" << std::min(total_coins, 999);
  return os.str();
}

std::string
Statistics::frags_to_string(int badguys, int total_badguys) {
  std::ostringstream os;
  os << std::min(badguys, 999) << "/" << std::min(total_badguys, 999);
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

