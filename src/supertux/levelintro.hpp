//  SuperTux -- LevelIntro screen
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_LEVELINTRO_HPP
#define HEADER_SUPERTUX_SUPERTUX_LEVELINTRO_HPP

#include "supertux/screen.hpp"

#include "sprite/sprite_ptr.hpp"
#include "supertux/timer.hpp"
#include "video/color.hpp"

class DrawingContext;
class Level;
class Player;
class Statistics;

/** Screen that welcomes the player to a level */
class LevelIntro final : public Screen
{
public:
  static void quit();

private:
  static Color s_header_color;
  static Color s_author_color;
  static Color s_stat_hdr_color;
  static Color s_stat_color;
  static Color s_stat_perfect_color;

public:
  LevelIntro(const Level& level, const Statistics* best_level_statistics, bool allow_quit);
  ~LevelIntro() override;

  virtual void setup() override;
  virtual void draw(Compositor& compositor) override;
  virtual void update(float dt_sec, const Controller& controller) override;
  virtual IntegrationStatus get_status() const override;

  void push_player(const Player* player);
  void pop_player(const Player* player);

private:
  void draw_stats_line(DrawingContext& context, int& py, const std::string& name, const std::string& stat, bool isPerfect);

private:
  struct PlayerData final
  {
    PlayerData() = default;

    const Player* player;
    SpritePtr sprite; /**< Sprite representing the player */
    SpritePtr santa_sprite;
    float sprite_py; /**< Position (y axis) for the player sprite */
    float sprite_vy; /**< Velocity (y axis) for the player sprite */
    Timer sprite_jump_timer; /**< When timer fires, the player sprite will "jump" */
  };

private:
  const Level& m_level; /**< The level of which this is the intro screen */
  const Statistics* m_best_level_statistics; /**< Best level statistics of the level of which is the intro screen */
  const bool m_allow_quit;

  std::vector<std::unique_ptr<PlayerData>> m_players;

private:
  LevelIntro(const LevelIntro&) = delete;
  LevelIntro& operator=(const LevelIntro&) = delete;
};

#endif

/* EOF */
