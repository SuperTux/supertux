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

#ifndef HEADER_SUPERTUX_SCRIPTING_ANCHOR_POINTS_HPP
#define HEADER_SUPERTUX_SCRIPTING_ANCHOR_POINTS_HPP

namespace scripting {

#ifdef DOXYGEN_SCRIPTING
/**
 * @summary This module contains global anchor constants.
 */
class AnchorPoints
{
public:
#endif

// TODO get these from the definitions in anchor.h (needs miniswig update)
static const int ANCHOR_TOP_LEFT     = 0; /**< Top-left anchor point. */
static const int ANCHOR_TOP          = 1; /**< Top anchor point. */
static const int ANCHOR_TOP_RIGHT    = 2; /**< Top-right anchor point. */
static const int ANCHOR_LEFT         = 3; /**< Left anchor point. */
static const int ANCHOR_MIDDLE       = 4; /**< Middle anchor point. */
static const int ANCHOR_RIGHT        = 5; /**< Right anchor point. */
static const int ANCHOR_BOTTOM_LEFT  = 6; /**< Bottom-left anchor point. */
static const int ANCHOR_BOTTOM       = 7; /**< Bottom anchor point. */
static const int ANCHOR_BOTTOM_RIGHT = 8; /**< Bottom-right anchor point. */

#ifdef DOXYGEN_SCRIPTING
}
#endif

} // namespace scripting

#endif

/* EOF */
