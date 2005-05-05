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

#ifndef SUPERTUX_STATISTICS_H
#define SUPERTUX_STATISTICS_H

#include "timer.h"
#include "lisp/lisp.h"
#include "lisp/writer.h"

class DrawingContext;

#define SPLAYER 0
#define STOTAL  1

enum {
  SCORE_STAT,
  COINS_COLLECTED_STAT,
  BADGUYS_KILLED_STAT,
  TIME_NEEDED_STAT,
  NUM_STATS
};

/** This class is a layer between level and worldmap to keep
 *  track of stuff like scores, and minor, but funny things, like
 *  number of jumps and stuff */

class Statistics
{
public:
  // don't forget to call reset() to init stat
  Statistics();
  ~Statistics();

  /// read statistics from lisp file
  void parse(const lisp::Lisp& lisp);
  /// write statistics to lisp file
  void write(lisp::Writer& writer);

  /* Draw to the worldmap or a game message */
  // TODO: make this functions working
  void draw_worldmap_info(DrawingContext& context);
  void draw_message_info(DrawingContext& context, std::string title);

  /* Add / Set / Get points to/from one of the stats this can keep track of */
  void add_points(int stat, int points);
  void set_points(int stat, int points);
  int get_points(int stat);

  void set_total_points(int stat, int points);

  /* Reset statistics */
  void reset();

  /* Give another Statistics object, find the best of each one */
  void merge(Statistics& stats);

  /* Add two statistic objects */
  void operator+=(const Statistics& o);

private:
  int stats[NUM_STATS][2];

  Timer timer;
  int display_stat;
};

extern Statistics global_stats;

#endif /*SUPERTUX_STATISTICS_H*/
