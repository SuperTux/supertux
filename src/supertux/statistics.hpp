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

#include "video/color.hpp"
#include "video/surface_ptr.hpp"

class DrawingContext;
class Level;
class SquirrelVM;

/** This class is a layer between level and worldmap to keep
    track of stuff like scores, and minor, but funny things, like
    number of jumps and stuff */
class Statistics final
{
private:
  static Color header_color;
  static Color text_color;

public:
  static std::string coins_to_string(int coins, int total_coins);
  static std::string frags_to_string(int badguys, int total_badguys);
  static std::string time_to_string(float time);
  static std::string secrets_to_string(int secrets, int total_secrets);

public:
  enum Status { INVALID, ACCUMULATING, FINAL };

public:
  Statistics(); /**< Creates new statistics, call reset() before counting */

  /** serialize statistics object as squirrel table "statistics" */
  void serialize_to_squirrel(SquirrelVM& vm) const;

  /** unserialize statistics object from squirrel table "statistics" */
  void unserialize_from_squirrel(SquirrelVM& vm);

  void draw_worldmap_info(DrawingContext& context, float target_time); /**< draw worldmap stat HUD */
  void draw_endseq_panel(DrawingContext& context, Statistics* best_stats, const SurfacePtr& backdrop); /**< draw panel shown during level's end sequence */

  void init(const Level& level);
  void finish(float time);
  void invalidate();

  Status get_status() const { return m_status; }

public:
  void update(const Statistics& stats); /**< Given another Statistics object finds the best of each one */
  bool completed(const Statistics& stats, const float target_time) const; /* Check if stats match total stats */

  float get_time() const { return m_time; }

private:
  void calculate_max_caption_length();

private:
  enum Status m_status;

public:
  int m_total_coins; /**< coins in level */
  int m_total_badguys; /**< (vincible) badguys in level */
  int m_total_secrets; /**< secret areas in level */

  int m_coins; /**< coins collected */
  int m_badguys; /**< badguys actively killed */
  int m_secrets; /**< secret areas found */

private:
  float m_time; /**< seconds needed */

private:
  int m_max_width; /** < Gets the max width of a stats line, 255 by default */

  /** Captions */
  std::string CAPTION_MAX_COINS;
  std::string CAPTION_MAX_FRAGGING;
  std::string CAPTION_MAX_SECRETS;
  std::string CAPTION_BEST_TIME;
  std::string CAPTION_TARGET_TIME;

  float WMAP_INFO_LEFT_X;
  float WMAP_INFO_RIGHT_X;
  float WMAP_INFO_TOP_Y1;
  float WMAP_INFO_TOP_Y2;

private:
  Statistics(const Statistics&) = delete;
  Statistics& operator=(const Statistics&) = delete;
};

#endif

/* EOF */
