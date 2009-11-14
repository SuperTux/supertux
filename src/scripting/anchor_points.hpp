//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __ANCHOR_POINTS_HPP__
#define __ANCHOR_POINTS_HPP__

namespace Scripting {

// TODO get these from the definitions in anchor.h (needs miniswig update)
static const int ANCHOR_TOP         = 0x0010;
static const int ANCHOR_BOTTOM      = 0x0020;
static const int ANCHOR_LEFT        = 0x0001;
static const int ANCHOR_RIGHT       = 0x0002;
static const int ANCHOR_MIDDLE      = 0x0000;
static const int ANCHOR_TOP_LEFT    = 0x0011;
static const int ANCHOR_TOP_RIGHT   = 0x0012;
static const int ANCHOR_BOTTOM_LEFT = 0x0021;
static const int ANCHOR_BOTTOM_RIGHT = 0x0022;

}

#endif
