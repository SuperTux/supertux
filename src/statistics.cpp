//  SuperTux
//  Copyright (C) 2004 SuperTux Development Team, see AUTHORS for details
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

Statistics global_stats;

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
  reader.read_int("score", stats[SCORE_STAT]);
}

void
Statistics::write(LispWriter& writer)
{
  writer.write_int("score", stats[SCORE_STAT]);
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
Statistics::reset()
{
  for(int i = 0; i < MAX_STATS; i++)
    stats[i] = 0;
}

void
Statistics::merge(Statistics& stats_)
{
  stats[SCORE_STAT] = std::max(stats[SCORE_STAT], stats_.stats[SCORE_STAT]);
}

void
Statistics::operator+=(const Statistics& stats_)
{
  stats[SCORE_STAT] += stats_.stats[SCORE_STAT];
}
