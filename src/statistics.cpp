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
#include "statistics.h"
#include "video/drawing_context.h"
#include "resources.h"

Statistics global_stats;

std::string
stat_name_to_string(int stat_enum)
{
  switch(stat_enum)
    {
    case SCORE_STAT:
      return "score";
    case BADGUYS_SQUISHED_STAT:
      return "badguys-squished";
    case SHOTS_STAT:
      return "shots";
    case TIME_NEEDED_STAT:
      return "time-needed";
    case JUMPS_STAT:
      return "jumps";
    }
}

Statistics::Statistics()
{
  reset();
}

Statistics::~Statistics()
{
}

void
Statistics::parse(LispReader& reader)
{
  for(int i = 0; i < NUM_STATS; i++)
    reader.read_int(stat_name_to_string(i).c_str(), stats[i]);
}

void
Statistics::write(LispWriter& writer)
{
  for(int i = 0; i < NUM_STATS; i++)
    writer.write_int(stat_name_to_string(i), stats[i]);
}

void
Statistics::draw_worldmap_info(DrawingContext& context)
{
  char str[128];

  //TODO: this is just a simple message, will be imporved
  sprintf(str, "Level Max Score: %d", stats[SCORE_STAT]);
  context.draw_text(white_small_text, str, Vector(580, 580), LAYER_GUI);
}

void
Statistics::draw_message_info(DrawingContext& context)
{
  // TODO
}

void
Statistics::add_points(int stat, int points)
{
  stats[stat] += points;
}

int
Statistics::get_points(int stat)
{
  return stats[stat];
}

void
Statistics::set_points(int stat, int points)
{
  stats[stat] = points;
}

void
Statistics::reset()
{
  for(int i = 0; i < NUM_STATS; i++)
    stats[i] = 0;
}

void
Statistics::merge(Statistics& stats_)
{
  stats[SCORE_STAT] = std::max(stats[SCORE_STAT], stats_.stats[SCORE_STAT]);
  stats[JUMPS_STAT] = std::min(stats[JUMPS_STAT], stats_.stats[JUMPS_STAT]);
  stats[BADGUYS_SQUISHED_STAT] =
    std::max(stats[BADGUYS_SQUISHED_STAT], stats_.stats[BADGUYS_SQUISHED_STAT]);
  stats[SHOTS_STAT] = std::min(stats[SHOTS_STAT], stats_.stats[SHOTS_STAT]);
  stats[TIME_NEEDED_STAT] =
    std::min(stats[TIME_NEEDED_STAT], stats_.stats[TIME_NEEDED_STAT]);
}

void
Statistics::operator+=(const Statistics& stats_)
{
  for(int i = 0; i < NUM_STATS; i++)
    stats[i] += stats_.stats[i];
}
