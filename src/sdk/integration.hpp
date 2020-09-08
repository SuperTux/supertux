//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SDK_INTEGRATION_HPP
#define HEADER_SUPERTUX_SDK_INTEGRATION_HPP

enum IntegrationStatus {
  MAIN_MENU,
  PLAYING_WORLDMAP,
  PLAYING_LEVEL,
  PLAYING_LEVEL_FROM_WORLDMAP,
  EDITING_WORLDMAP,
  EDITING_LEVEL,
  TESTING_WORLDMAP,
  TESTING_LEVEL,
  TESTING_LEVEL_FROM_WORLDMAP
};

class Integration
{
public:
  static void init_all();
  static void update_all();
  static void close_all();
  static void set_status(IntegrationStatus status);
  static void set_worldmap(const char* worldmap);
  static void set_level(const char* level);

  static IntegrationStatus get_status();

public:
  virtual void init() = 0;
  virtual void update() = 0;
  virtual void close() = 0;
  virtual void update_status(IntegrationStatus status) = 0;
  virtual void update_worldmap(const char* worldmap) = 0;
  virtual void update_level(const char* level) = 0;

protected:
  ~Integration(){};

private:
  static IntegrationStatus m_status;
};

#endif

/* EOF */
