//  SuperTux
//  Copyright (C) 2017 Tobias Markus <tobbi.bugs@googlemail.com>
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


#ifndef HEADER_SUPERTUX_SCRIPTING_SQRAT_OBJECT_HPP
#define HEADER_SUPERTUX_SCRIPTING_SQRAT_OBJECT_HPP


#include "scripting/scripting.hpp"

/**
 * This class defines scripting classes that have the available method to implement SQRat
 * stuff. This description is just a placeholder anyway.
 */
template<class T>
class SQRatObject
{
public:
    SQRatObject() {}
    typedef Sqrat::Class<T, Sqrat::NoCopy<T>> SQRatClassType;
    /**
     * For the insanely curious: This is the https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
     */
    static void register_exposed_methods(HSQUIRRELVM v, SQRatClassType squirrelClass)
    {
        T::register_exposed_methods(v, squirrelClass);
    };

    virtual ~SQRatObject()
    {
    }

private:
    SQRatObject(const SQRatObject&) = delete;
    SQRatObject& operator=(const SQRatObject&) = delete;
};

#endif
