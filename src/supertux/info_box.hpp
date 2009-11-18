//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_INFO_BOX_HPP
#define HEADER_SUPERTUX_SUPERTUX_INFO_BOX_HPP

#include <map>
#include <string>
#include <vector>

class DrawingContext;
class InfoBoxLine;
class Surface;

/** This class is displaying a box with information text inside the game */
class InfoBox
{
public:
  InfoBox(const std::string& text);
  ~InfoBox();

  void draw(DrawingContext& context);
  void scrolldown();
  void scrollup();
  void pagedown();
  void pageup();

private:
  size_t firstline;
  std::vector<InfoBoxLine*> lines;
  std::map<std::string, Surface*> images;
  Surface* arrow_scrollup;
  Surface* arrow_scrolldown;

private:
  InfoBox(const InfoBox&);
  InfoBox& operator=(const InfoBox&);
};

#endif

/* EOF */
