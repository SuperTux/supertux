//  $Id$
//
//  SuperTux (Statistics module)
//  Copyright (C) 2004 Ricardo Cruz <rick2@aeiou.pt>
//  Copyright (C) 2006 Ondrej Hosek <ondra.hosek@gmail.com>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#include <config.h>

#include <assert.h>
#include <math.h>
#include "video/drawing_context.hpp"
#include "gettext.hpp"
#include "lisp/lisp.hpp"
#include "resources.hpp"
#include "main.hpp"
#include "statistics.hpp"
#include "log.hpp"

namespace {
  const int nv_coins = std::numeric_limits<int>::min();
  const int nv_badguys = std::numeric_limits<int>::min();
  const float nv_time = std::numeric_limits<float>::max();
  const int nv_secrets = std::numeric_limits<int>::min();
}

Statistics::Statistics() : coins(nv_coins), total_coins(nv_coins), badguys(nv_badguys), total_badguys(nv_badguys), time(nv_time), secrets(nv_secrets), total_secrets(nv_secrets), display_stat(0)
{
}

Statistics::~Statistics()
{
}

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
  writer.write_int("coins-collected", coins);
  writer.write_int("coins-collected-total", total_coins);
  writer.write_int("badguys-killed", badguys);
  writer.write_int("badguys-killed-total", total_badguys);
  writer.write_float("time-needed", time);
  writer.write_int("secrets-found", secrets);
  writer.write_int("secrets-found-total", total_secrets);
}

//define TOTAL_DISPLAY_TIME  3400
//define FADING_TIME          600

#define TOTAL_DISPLAY_TIME  5
#define FADING_TIME         1

#define WMAP_INFO_LEFT_X  520
#define WMAP_INFO_RIGHT_X 740

void
Statistics::draw_worldmap_info(DrawingContext& context)
{
  // skip draw if level was never played
  if (coins == nv_coins) return;

  context.draw_text(white_small_text, _("- Best Level Statistics -"), Vector((WMAP_INFO_LEFT_X + WMAP_INFO_RIGHT_X) / 2, 470), CENTER_ALLIGN, LAYER_GUI);

  float alpha;
  if(timer.get_timegone() < FADING_TIME)
    alpha = (timer.get_timegone() * 1.0f / FADING_TIME);
  else if(timer.get_timeleft() < FADING_TIME)
    alpha = (timer.get_timeleft() * 1.0f / FADING_TIME);
  else
    alpha = 1.0f;

  context.push_transform();
  context.set_alpha(alpha);

  char caption_buf[128];
  char stat_buf[128];
  switch (display_stat) 
  {
    case 0:
      sprintf(caption_buf, _("Max coins collected:"));
      sprintf(stat_buf, "%d/%d", coins, total_coins);
      break;
    case 1:
      sprintf(caption_buf, _("Max fragging:"));
      sprintf(stat_buf, "%d/%d", badguys, total_badguys);
      break;
    case 2:
      sprintf(caption_buf, _("Min time needed:"));
      {
	int csecs = (int)(time * 100);
	int mins = (int)(csecs / 6000);
	int secs = (csecs % 6000) / 100;
	sprintf(stat_buf, "%02d:%02d", mins,secs); 
      }
      break;
    case 3:
      sprintf(caption_buf, _("Max secrets found:"));
      sprintf(stat_buf, "%d/%d", secrets, total_secrets);
      break;
    default:
      log_debug << "Invalid stat requested to be drawn" << std::endl;
      break;
  }

  if (!timer.started())
  {
    timer.start(TOTAL_DISPLAY_TIME);
    display_stat++;
    if (display_stat > 3) display_stat = 0;
  }

  context.draw_text(white_small_text, caption_buf, Vector(WMAP_INFO_LEFT_X, 490), LEFT_ALLIGN, LAYER_GUI);
  context.draw_text(white_small_text, stat_buf, Vector(WMAP_INFO_RIGHT_X, 490), RIGHT_ALLIGN, LAYER_GUI);
  context.pop_transform();
}

void
Statistics::draw_message_info(DrawingContext& context, std::string title)
{
  // skip draw if level was never played
  // TODO: do we need this?
  if (coins == nv_coins) return;

  context.draw_text(gold_text, title, Vector(SCREEN_WIDTH/2, 410), CENTER_ALLIGN, LAYER_GUI);

  char str[128];
  int py = 450 + 18;

  sprintf(str, _("Max coins collected:   %d / %d"), coins, total_coins);
  context.draw_text(white_small_text, str, Vector(SCREEN_WIDTH/2, py), CENTER_ALLIGN, LAYER_GUI); 
  py+=18;

  sprintf(str, _("Max fragging:          %d / %d"), badguys, total_badguys);
  context.draw_text(white_small_text, str, Vector(SCREEN_WIDTH/2, py), CENTER_ALLIGN, LAYER_GUI); 
  py+=18;

  int csecs = (int)(time * 100);
  int mins = (int)(csecs / 6000);
  int secs = (csecs % 6000) / 100;
  sprintf(str, _("Min time needed:       %02d:%02d"), mins,secs);
  context.draw_text(white_small_text, str, Vector(SCREEN_WIDTH/2, py), CENTER_ALLIGN, LAYER_GUI); 
  py+=18;
  
  sprintf(str, _("Max secrets found:     %d / %d"), secrets, total_secrets);
  context.draw_text(white_small_text, str, Vector(SCREEN_WIDTH/2, py), CENTER_ALLIGN, LAYER_GUI); 
  py+=18;
}

void 
Statistics::draw_endseq_panel(DrawingContext& context, Statistics* best_stats, Surface* backdrop)
{
  // skip draw if level was never played
  // TODO: do we need this?
  if (coins == nv_coins) return;

  // abort if we have no backdrop
  if (!backdrop) return;

  int box_w = 130+130+130;
  int box_h = 30+20+20+20;
  int box_x = (int)((SCREEN_WIDTH - box_w) / 2);
  int box_y = (int)(SCREEN_HEIGHT / 2) - box_h;

  int bd_w = (int)backdrop->get_width();
  int bd_h = (int)backdrop->get_height();
  int bd_x = (int)((SCREEN_WIDTH - bd_w) / 2);
  int bd_y = box_y + (box_h / 2) - (bd_h / 2);

  int col1_x = box_x;
  int col2_x = col1_x+130;
  int col3_x = col2_x+130;

  int row1_y = box_y;
  int row2_y = row1_y+30;
  int row3_y = row2_y+20;
  int row4_y = row3_y+20;

  context.draw_surface(backdrop, Vector(bd_x, bd_y), LAYER_GUI);

  char buf[129];
  context.draw_text(white_text, "You", Vector(col2_x, row1_y), LEFT_ALLIGN, LAYER_GUI);
  context.draw_text(white_text, "Best", Vector(col3_x, row1_y), LEFT_ALLIGN, LAYER_GUI);

  context.draw_text(white_text, "Coins", Vector(col1_x, row2_y), LEFT_ALLIGN, LAYER_GUI);
  snprintf(buf, 128, "%d/%d", coins, total_coins);
  context.draw_text(gold_text, buf, Vector(col2_x, row2_y), LEFT_ALLIGN, LAYER_GUI);
  if (best_stats && (best_stats->coins > coins)) {
    snprintf(buf, 128, "%d/%d", best_stats->coins, best_stats->total_coins);
  }
  context.draw_text(gold_text, buf, Vector(col3_x, row2_y), LEFT_ALLIGN, LAYER_GUI);

  context.draw_text(white_text, "Secrets", Vector(col1_x, row4_y), LEFT_ALLIGN, LAYER_GUI);
  snprintf(buf, 128, "%d/%d", secrets, total_secrets);
  context.draw_text(gold_text, buf, Vector(col2_x, row4_y), LEFT_ALLIGN, LAYER_GUI);
  if (best_stats && (best_stats->secrets > secrets)) {
    snprintf(buf, 128, "%d/%d", best_stats->secrets, best_stats->total_secrets);
  }
  context.draw_text(gold_text, buf, Vector(col3_x, row4_y), LEFT_ALLIGN, LAYER_GUI);

  context.draw_text(white_text, "Time", Vector(col1_x, row3_y), LEFT_ALLIGN, LAYER_GUI);
  int csecs = (int)(time * 100);
  int mins = (int)(csecs / 6000);
  int secs = (csecs % 6000) / 100;
  snprintf(buf, 128, "%02d:%02d", mins,secs);
  context.draw_text(gold_text, buf, Vector(col2_x, row3_y), LEFT_ALLIGN, LAYER_GUI);
  if (best_stats && (best_stats->time < time)) {
    int csecs = (int)(best_stats->time * 100);
    int mins = (int)(csecs / 6000);
    int secs = (csecs % 6000) / 100;
    snprintf(buf, 128, "%02d:%02d", mins,secs);
  }
  context.draw_text(gold_text, buf, Vector(col3_x, row3_y), LEFT_ALLIGN, LAYER_GUI);
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
Statistics::merge(Statistics& s2)
{
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
  if (s2.coins != nv_coins) coins += s2.coins;
  if (s2.total_coins != nv_coins) total_coins += s2.total_coins;
  if (s2.badguys != nv_badguys) badguys += s2.badguys;
  if (s2.total_badguys != nv_badguys) total_badguys += s2.total_badguys;
  if (s2.time != nv_time) time += s2.time;
  if (s2.secrets != nv_secrets) secrets += s2.secrets;
  if (s2.total_secrets != nv_secrets) total_secrets += s2.total_secrets;
}
