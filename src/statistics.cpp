//  $Id$
//
//  SuperTux (Statistics module)
//  Copyright (C) 2004 Ricardo Cruz <rick2@aeiou.pt>
//  Copyright (C) 2006 Ondrej Hosek <white.timberwolf@aon.at>
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

#include "video/drawing_context.hpp"
#include "gettext.hpp"
#include "lisp/lisp.hpp"
#include "resources.hpp"
#include "main.hpp"
#include "statistics.hpp"

Statistics global_stats;

std::string
stat_name_to_string(int stat_enum)
{
  switch(stat_enum)
    {
//    case SCORE_STAT:
//      return "score";
    case COINS_COLLECTED_STAT:
      return "coins-collected";
    case BADGUYS_KILLED_STAT:
      return "badguys-killed";
    case TIME_NEEDED_STAT:
      return "time-needed";;
    }
  return "";
}

int
my_min(int a, int b)
{
if(a == -1)
  return b;
if(b == -1)
  return a;
return std::min(a, b);
}

Statistics::Statistics()
{
  display_stat = 1;

  for(int i = 0; i < NUM_STATS; i++)
    for(int j = 0; j < 2; j++)
      stats[i][j] = -1;
}

Statistics::~Statistics()
{
}

void
Statistics::parse(const lisp::Lisp& reader)
{
  for(int i = 0; i < NUM_STATS; i++) {
    reader.get(stat_name_to_string(i).c_str(), stats[i][SPLAYER]);
    reader.get((stat_name_to_string(i) + "-total").c_str(), stats[i][STOTAL]);
  }
}

void
Statistics::write(lisp::Writer& writer)
{
  for(int i = 0; i < NUM_STATS; i++) {
    writer.write_int(stat_name_to_string(i), stats[i][SPLAYER]);
    writer.write_int(stat_name_to_string(i) + "-total", stats[i][STOTAL]);
  }
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
  if(stats[COINS_COLLECTED_STAT][SPLAYER] == -1)  // not initialized yet
    return;

//  if(timer.check())
  if (!timer.started())
  {
    timer.start(TOTAL_DISPLAY_TIME);
    display_stat++;
    if(display_stat >= NUM_STATS)
      display_stat = 0;

    if((display_stat == TIME_NEEDED_STAT) && (stats[TIME_NEEDED_STAT][STOTAL] == -1))
    { // no timer in level
      display_stat++;
      if(display_stat >= NUM_STATS)
        display_stat = 0;
    }
  }

  char str[128];

  context.draw_text(white_small_text, _("- Best Level Statistics -"),
                    Vector((WMAP_INFO_LEFT_X + WMAP_INFO_RIGHT_X) / 2, 470),
                    CENTER_ALLIGN, LAYER_GUI);

  // Score has been removed
  //sprintf(str, _("Max score:"));
  //context.draw_text(white_small_text, str, Vector(WMAP_INFO_LEFT_X, 490), LEFT_ALLIGN, LAYER_GUI);

  //sprintf(str, "%d", stats[SCORE_STAT][SPLAYER]);
  //context.draw_text(white_small_text, str, Vector(WMAP_INFO_RIGHT_X, 490), RIGHT_ALLIGN, LAYER_GUI);

  float alpha;
  if(timer.get_timegone() < FADING_TIME)
    alpha = (timer.get_timegone() * 1.0f / FADING_TIME);
  else if(timer.get_timeleft() < FADING_TIME)
    alpha = (timer.get_timeleft() * 1.0f / FADING_TIME);
  else
    alpha = 1.0f;

  context.push_transform();
  context.set_alpha(alpha);

  if(display_stat == COINS_COLLECTED_STAT)
    sprintf(str, _("Max coins collected:"));
  else if(display_stat == BADGUYS_KILLED_STAT)
    sprintf(str, _("Max fragging:"));
  else// if(display_stat == TIME_NEEDED_STAT)
    sprintf(str, _("Min time needed:"));

  // y == 508 before score was removed
  context.draw_text(white_small_text, str, Vector(WMAP_INFO_LEFT_X, 490), LEFT_ALLIGN, LAYER_GUI);

  if(display_stat == COINS_COLLECTED_STAT)
    sprintf(str, "%d/%d", stats[COINS_COLLECTED_STAT][SPLAYER],
                          stats[COINS_COLLECTED_STAT][STOTAL]);
  else if(display_stat == BADGUYS_KILLED_STAT)
    sprintf(str, "%d/%d", stats[BADGUYS_KILLED_STAT][SPLAYER],
                          stats[BADGUYS_KILLED_STAT][STOTAL]);
  else// if(display_stat == TIME_NEEDED_STAT)
    sprintf(str, "%d/%d", stats[TIME_NEEDED_STAT][SPLAYER],
                          stats[TIME_NEEDED_STAT][STOTAL]);

  context.draw_text(white_small_text, str, Vector(WMAP_INFO_RIGHT_X, 490), RIGHT_ALLIGN, LAYER_GUI);

  context.pop_transform();
}

void
Statistics::draw_message_info(DrawingContext& context, std::string title)
{
  if(stats[COINS_COLLECTED_STAT][SPLAYER] == -1)  // not initialized yet
    return;

  context.draw_text(gold_text, title, Vector(SCREEN_WIDTH/2, 410), CENTER_ALLIGN, LAYER_GUI);

  char str[128];

  //sprintf(str, _(    "Max score:             %d"), stats[SCORE_STAT][SPLAYER]);
  //context.draw_text(white_text, str, Vector(SCREEN_WIDTH/2, 450), CENTER_ALLIGN, LAYER_GUI);

  for(int i = 0; i < NUM_STATS; i++)
    {
    if(i == COINS_COLLECTED_STAT)
      sprintf(str, _("Max coins collected:   %d / %d"),
              stats[COINS_COLLECTED_STAT][SPLAYER],
              stats[COINS_COLLECTED_STAT][STOTAL]);
    else if(i == BADGUYS_KILLED_STAT)
      sprintf(str, _("Max fragging:          %d / %d"),
              stats[BADGUYS_KILLED_STAT][SPLAYER],
              stats[BADGUYS_KILLED_STAT][STOTAL]);
    else if((i == TIME_NEEDED_STAT) && (stats[TIME_NEEDED_STAT][STOTAL] != -1))
      sprintf(str, _("Min time needed:       %d / %d"),
              stats[TIME_NEEDED_STAT][SPLAYER],
              stats[TIME_NEEDED_STAT][STOTAL]);
    else
      continue;


    // y == (462 + i*18) before score removal
    context.draw_text(white_small_text, str, Vector(SCREEN_WIDTH/2, 450 + (i+1)*18), CENTER_ALLIGN, LAYER_GUI);
    }
}

void 
Statistics::draw_endseq_panel(DrawingContext& context, Statistics* best_stats, Surface* backdrop)
{
  // abort if statistics are not yet initialized
  if(stats[COINS_COLLECTED_STAT][SPLAYER] == -1) return;

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
  snprintf(buf, 128, "%d/%d", stats[COINS_COLLECTED_STAT][SPLAYER], stats[COINS_COLLECTED_STAT][STOTAL]);
  context.draw_text(gold_text, buf, Vector(col2_x, row2_y), LEFT_ALLIGN, LAYER_GUI);
  if (best_stats && (best_stats->stats[COINS_COLLECTED_STAT][SPLAYER] > stats[COINS_COLLECTED_STAT][SPLAYER])) {
    snprintf(buf, 128, "%d/%d", best_stats->stats[COINS_COLLECTED_STAT][SPLAYER], best_stats->stats[COINS_COLLECTED_STAT][STOTAL]);
  }
  context.draw_text(gold_text, buf, Vector(col3_x, row2_y), LEFT_ALLIGN, LAYER_GUI);

  context.draw_text(white_text, "Time", Vector(col1_x, row3_y), LEFT_ALLIGN, LAYER_GUI);
  snprintf(buf, 128, "%d:%02d", stats[TIME_NEEDED_STAT][SPLAYER] / 60, stats[TIME_NEEDED_STAT][SPLAYER] % 60);
  context.draw_text(gold_text, buf, Vector(col2_x, row3_y), LEFT_ALLIGN, LAYER_GUI);
  if (best_stats && (best_stats->stats[TIME_NEEDED_STAT][SPLAYER] < stats[TIME_NEEDED_STAT][SPLAYER])) {
    snprintf(buf, 128, "%d:%02d", best_stats->stats[TIME_NEEDED_STAT][SPLAYER] / 60, best_stats->stats[TIME_NEEDED_STAT][SPLAYER] % 60);
  }
  context.draw_text(gold_text, buf, Vector(col3_x, row3_y), LEFT_ALLIGN, LAYER_GUI);
  
  context.draw_text(white_text, "Badguys", Vector(col1_x, row4_y), LEFT_ALLIGN, LAYER_GUI);
  snprintf(buf, 128, "%d/%d", stats[BADGUYS_KILLED_STAT][SPLAYER], stats[BADGUYS_KILLED_STAT][STOTAL]);
  context.draw_text(gold_text, buf, Vector(col2_x, row4_y), LEFT_ALLIGN, LAYER_GUI);
  if (best_stats && (best_stats->stats[BADGUYS_KILLED_STAT][SPLAYER] > stats[BADGUYS_KILLED_STAT][SPLAYER])) {
    snprintf(buf, 128, "%d/%d", best_stats->stats[BADGUYS_KILLED_STAT][SPLAYER], best_stats->stats[BADGUYS_KILLED_STAT][STOTAL]);
  }
  context.draw_text(gold_text, buf, Vector(col3_x, row4_y), LEFT_ALLIGN, LAYER_GUI);

}

void
Statistics::add_points(int stat, int points)
{
  stats[stat][SPLAYER] += points;
}

int
Statistics::get_points(int stat)
{
  return stats[stat][SPLAYER];
}

void
Statistics::set_points(int stat, int points)
{
  stats[stat][SPLAYER] = points;
}

void
Statistics::set_total_points(int stat, int points)
{
  stats[stat][STOTAL] = points;
}

void
Statistics::reset()
{
  for(int i = 0; i < NUM_STATS; i++)
    stats[i][SPLAYER] = 0;
}

void
Statistics::merge(Statistics& stats_)
{
//  stats[SCORE_STAT][SPLAYER] = std::max(stats[SCORE_STAT][SPLAYER], stats_.stats[SCORE_STAT][SPLAYER]);
  stats[COINS_COLLECTED_STAT][SPLAYER] = std::max(stats[COINS_COLLECTED_STAT][SPLAYER], stats_.stats[COINS_COLLECTED_STAT][SPLAYER]);
  stats[BADGUYS_KILLED_STAT][SPLAYER] =
    std::max(stats[BADGUYS_KILLED_STAT][SPLAYER], stats_.stats[BADGUYS_KILLED_STAT][SPLAYER]);
  stats[TIME_NEEDED_STAT][SPLAYER] =
    my_min(stats[TIME_NEEDED_STAT][SPLAYER], stats_.stats[TIME_NEEDED_STAT][SPLAYER]);

  stats[COINS_COLLECTED_STAT][STOTAL] = stats_.stats[COINS_COLLECTED_STAT][STOTAL];
  stats[BADGUYS_KILLED_STAT][STOTAL] = stats_.stats[BADGUYS_KILLED_STAT][STOTAL];
  stats[TIME_NEEDED_STAT][STOTAL] = stats_.stats[TIME_NEEDED_STAT][STOTAL];
}

void
Statistics::operator+=(const Statistics& stats_)
{
  for(int i = 0; i < NUM_STATS; i++)
    {
    if(stats_.stats[i][SPLAYER] == -1)
      continue;
    stats[i][SPLAYER] += stats_.stats[i][SPLAYER];
    if(stats_.stats[i][STOTAL] != -1)
      stats[i][STOTAL] += stats_.stats[i][STOTAL];
    }
}
