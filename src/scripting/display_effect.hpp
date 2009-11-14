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

#ifndef __SCRIPTING_DISPLAY_EFFECT_H__
#define __SCRIPTING_DISPLAY_EFFECT_H__

namespace Scripting
{

class DisplayEffect
{
public:
#ifndef SCRIPTING_API
    virtual ~DisplayEffect()
    {}
#endif

    /// fade display to black
    virtual void fade_out(float fadetime) = 0;
    /// fade display from black to normal
    virtual void fade_in(float fadetime) = 0;
    /// set display black (or back to normal)
    virtual void set_black(bool enabled) = 0;
    /// check if display is set to black
    virtual bool is_black() = 0;
    /// set black borders for cutscenes
    virtual void sixteen_to_nine(float fadetime) = 0;
    /// deactivate borders
    virtual void four_to_three(float fadetime) = 0;

    // fade display until just a small visible circle is left
    // (like what happens in some cartoons at the end)
    // void shrink_fade(Vector goal, float radius, float fadetime);
};

}

#endif
