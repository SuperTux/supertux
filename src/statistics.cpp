//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Ricardo Cruz <rick2@aeiou.pt>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include "utils/lispreader.h"
#include "utils/lispwriter.h"
#include "video/drawing_context.h"
#include "app/gettext.h"
#include "app/globals.h"
#include "resources.h"
#include "statistics.h"

Statistics global_stats;

std::string
stat_name_to_string(int stat_enum)
{
  switch(stat_enum)
    {
    case SCORE_STAT:
      return "score";
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
  timer.init(true);
  display_stat = 1;

  for(int i = 0; i < NUM_STATS; i++)
    for(int j = 0; j < 2; j++)
      stats[i][j] = -1;
}

Statistics::~Statistics()
{
}

void
Statistics::parse(LispReader& reader)
{
  for(int i = 0; i < NUM_STATS; i++)
    {
    reader.read_int(stat_name_to_string(i).c_str(), stats[i][SPLAYER]);
    reader.read_int((stat_name_to_string(i) + "-total").c_str(), stats[i][STOTAL]);
    }
}

void
Statistics::write(LispWriter& writer)
{
  for(int i = 0; i < NUM_STATS; i++)
    {
    writer.write_int(stat_name_to_string(i), stats[i][SPLAYER]);
    writer.write_int(stat_name_to_string(i) + "-total", stats[i][STOTAL]);
    }
}

#define TOTAL_DISPLAY_TIME 3400
#define FADING_TIME         600

#define WMAP_INFO_LEFT_X  520
#define WMAP_INFO_RIGHT_X 740

void
Statistics::draw_worldmap_info(DrawingContext& context)
{
  if(stats[SCORE_STAT][SPLAYER] == -1)  // not initialized yet
    return;

  if(!timer.check())
    {
    timer.start(TOTAL_DISPLAY_TIME);
    display_stat++;
    if(display_stat >= NUM_STATS)
      display_stat = 1;
    }

  char str[128];

  context.draw_text(white_small_text, _("- Best Level Statistics -"),
                    Vector((WMAP_INFO_LEFT_X + WMAP_INFO_RIGHT_X) / 2, 470),
                    CENTER_ALLIGN, LAYER_GUI);

  sprintf(str, _("Max score:"));
  context.draw_text(white_small_text, str, Vector(WMAP_INFO_LEFT_X, 490), LEFT_ALLIGN, LAYER_GUI);

  sprintf(str, "%d", stats[SCORE_STAT][SPLAYER]);
  context.draw_text(white_small_text, str, Vector(WMAP_INFO_RIGHT_X, 490), RIGHT_ALLIGN, LAYER_GUI);

  // draw other small info

  int alpha;
  if(timer.get_gone() < FADING_TIME)
    alpha = timer.get_gone() * 255 / FADING_TIME;
  else if(timer.get_left() < FADING_TIME)
    alpha = timer.get_left() * 255 / FADING_TIME;
  else
    alpha = 255;

  context.push_transform();
  context.set_alpha(alpha);

  if(display_stat == COINS_COLLECTED_STAT)
    sprintf(str, _("Max coins collected:"));
  else if(display_stat == BADGUYS_KILLED_STAT)
    sprintf(str, _("Max fragging:"));
  else// if(display_stat == TIME_NEEDED_STAT)
    sprintf(str, _("Min time needed:"));

  context.draw_text(white_small_text, str, Vector(WMAP_INFO_LEFT_X, 508), LEFT_ALLIGN, LAYER_GUI);

  if(display_stat == COINS_COLLECTED_STAT)
    sprintf(str, "%d/%d", stats[COINS_COLLECTED_STAT][SPLAYER],
                          stats[COINS_COLLECTED_STAT][STOTAL]);
  else if(display_stat == BADGUYS_KILLED_STAT)
    sprintf(str, "%d/%d", stats[BADGUYS_KILLED_STAT][SPLAYER],
                          stats[BADGUYS_KILLED_STAT][STOTAL]);
  else// if(display_stat == TIME_NEEDED_STAT)
    sprintf(str, "%d/%d", stats[TIME_NEEDED_STAT][SPLAYER],
                          stats[TIME_NEEDED_STAT][STOTAL]);

  context.draw_text(white_small_text, str, Vector(WMAP_INFO_RIGHT_X, 508), RIGHT_ALLIGN, LAYER_GUI);

  context.pop_transform();
}

void
Statistics::draw_message_info(DrawingContext& context, std::string title)
{
  if(stats[SCORE_STAT][SPLAYER] == -1)  // not initialized yet
    return;

  context.draw_text(gold_text, title, Vector(screen->w/2, 410), CENTER_ALLIGN, LAYER_GUI);

  char str[128];

  sprintf(str, _(    "Max score:             %d"), stats[SCORE_STAT][SPLAYER]);
  context.draw_text(white_text, str, Vector(screen->w/2, 450), CENTER_ALLIGN, LAYER_GUI);

  for(int i = 1; i < NUM_STATS; i++)
    {
    if(i == COINS_COLLECTED_STAT)
      sprintf(str, _("Max coins collected:   %d / %d"),
              stats[COINS_COLLECTED_STAT][SPLAYER],
              stats[COINS_COLLECTED_STAT][STOTAL]);
    else if(i == BADGUYS_KILLED_STAT)
      sprintf(str, _("Max fragging:          %d / %d"),
              stats[BADGUYS_KILLED_STAT][SPLAYER],
              stats[BADGUYS_KILLED_STAT][STOTAL]);
    else// if(i == TIME_NEEDED_STAT)
      sprintf(str, _("Min time needed:       %d / %d"),
              stats[TIME_NEEDED_STAT][SPLAYER],
              stats[TIME_NEEDED_STAT][STOTAL]);


    context.draw_text(white_small_text, str, Vector(screen->w/2, 462 + i*18), CENTER_ALLIGN, LAYER_GUI);
    }
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
  stats[SCORE_STAT][SPLAYER] = std::max(stats[SCORE_STAT][SPLAYER], stats_.stats[SCORE_STAT][SPLAYER]);
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
