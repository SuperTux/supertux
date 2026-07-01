//  SuperTux
//  Copyright (C) 2026 SuperTux Team
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

#pragma once

#include <memory>
#include <optional>
#include <string>

#include "supertux/global_stats_manager.hpp"
#include "supertux/screen.hpp"
#include "video/color.hpp"
#include "video/surface_ptr.hpp"

class DrawingContext;
class CodeController;
class GameSession;
class Savegame;
class Sector;

class GlobalStatsScreen final : public Screen
{
private:
  static Color s_header_color;
  static Color s_label_color;
  static Color s_value_color;
  static Color s_help_color;

public:
  GlobalStatsScreen();
  ~GlobalStatsScreen() override;

  void setup() override;
  void leave() override;
  void draw(Compositor& compositor) override;
  void update(float dt_sec, const Controller& controller) override;
  IntegrationStatus get_status() const override;

private:
  void draw_background(DrawingContext& context) const;
  void draw_stat_line(DrawingContext& context, int& py, const std::string& label, const std::string& value) const;
  std::string format_count(int value) const;
  std::string format_time(float value) const;
  void load_summary();
  void refresh_title_background();
  void setup_title_sector(Sector& sector);
  void update_title_background(float dt_sec);

private:
  static std::optional<GlobalStatsManager::Summary> s_cached_summary;

private:
  SurfacePtr m_frame;
  SurfacePtr m_backdrop;
  std::unique_ptr<Savegame> m_savegame;
  std::unique_ptr<CodeController> m_controller;
  std::unique_ptr<GameSession> m_titlesession;
  GlobalStatsManager::Summary m_summary;
  bool m_summary_ready;
  bool m_load_requested;
  bool m_jump_was_released;

private:
  GlobalStatsScreen(const GlobalStatsScreen&) = delete;
  GlobalStatsScreen& operator=(const GlobalStatsScreen&) = delete;
};
