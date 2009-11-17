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

#ifndef HEADER_SUPERTUX_SCRIPTING_TILEMAP_HPP
#define HEADER_SUPERTUX_SCRIPTING_TILEMAP_HPP

#ifndef SCRIPTING_API
class TileMap;
typedef TileMap _TileMap;
#endif

namespace Scripting {

class TileMap
{
public:
#ifndef SCRIPTING_API
  TileMap(_TileMap* tilemap);
  ~TileMap();
#endif

  /** Move tilemap until at given node, then stop */
  void goto_node(int node_no);

  /** Start moving tilemap */
  void start_moving();

  /** Stop tilemap at next node */
  void stop_moving();

  /**
   * Start fading the tilemap to opacity given by @c alpha.
   * Destination opacity will be reached after @c seconds seconds. Also influences solidity.
   */
  void fade(float alpha, float seconds);

  /**
   * Instantly switch tilemap's opacity to @c alpha. Also influences solidity.
   */
  void set_alpha(float alpha);

  /**
   * Return tilemap's opacity. Note that while the tilemap is fading in or out, this will return the current alpha value, not the target alpha.
   */
  float get_alpha();

#ifndef SCRIPTING_API
  _TileMap* tilemap;

private:
  TileMap(const TileMap&);
  TileMap& operator=(const TileMap&);
#endif
};

}

#endif

/* EOF */
