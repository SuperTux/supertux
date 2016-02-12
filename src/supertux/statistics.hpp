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

#ifndef HEADER_SUPERTUX_SUPERTUX_STATISTICS_HPP
#define HEADER_SUPERTUX_SUPERTUX_STATISTICS_HPP

#include <squirrel.h>

#include "video/color.hpp"
#include "video/surface_ptr.hpp"

namespace lisp { class Lisp; }
class DrawingContext;

/** This class is a layer between level and worldmap to keep
 *  track of stuff like scores, and minor, but funny things, like
 *  number of jumps and stuff */
class Statistics
{
  static Color header_color;
  static Color text_color;
public:
  int coins; /**< coins collected */
  int total_coins; /**< coins in level */
  int badguys; /**< badguys actively killed */
  int total_badguys; /**< (vincible) badguys in level */
  float time; /**< seconds needed */
  int secrets; /**< secret areas found */
  int total_secrets; /**< secret areas in level */

public:
  Statistics(); /**< Creates new statistics, call reset() before counting */
  ~Statistics();

  /**
   * serialize statistics object as squirrel table "statistics"
   */
  void serialize_to_squirrel(HSQUIRRELVM vm);

  /**
   * unserialize statistics object from squirrel table "statistics"
   */
  void unserialize_from_squirrel(HSQUIRRELVM vm);

  void draw_worldmap_info(DrawingContext& context, float target_time); /**< draw worldmap stat HUD */
  void draw_endseq_panel(DrawingContext& context, Statistics* best_stats, SurfacePtr backdrop); /**< draw panel shown during level's end sequence */

  void zero(); /**< Set stats to zero */
  void reset(); /**< Set stats (but not totals) to zero */
  void merge(const Statistics& stats); /**< Given another Statistics object finds the best of each one */
  void operator+=(const Statistics& o); /**< Add two Statistics objects */
  bool completed(const Statistics& stats, const float target_time) const; /* Check if stats match total stats */

  void declare_invalid(); /**< marks statistics as invalid for their entire lifetime (e.g. after cheating). Invalid statistics will not be merged or drawn. */

  static std::string coins_to_string(int coins, int total_coins);
  static std::string frags_to_string(int badguys, int total_badguys);
  static std::string time_to_string(float time);
  static std::string secrets_to_string(int secrets, int total_secrets);

private:
  bool valid; /**< stores whether these statistics can be trusted */

};

#endif /*SUPERTUX_STATISTICS_H*/

/* EOF */
