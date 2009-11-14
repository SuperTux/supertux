//  SuperTux - Thunderstorm Game Object
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

#ifndef __THUNDERSTORM_H__
#define __THUNDERSTORM_H__

#include "game_object.hpp"
#include "timer.hpp"
#include "lisp/lisp.hpp"
#include "scripting/thunderstorm.hpp"
#include "script_interface.hpp"

/**
 * Thunderstorm scriptable GameObject; plays thunder, lightning and electrifies water at regular interval
 */
class Thunderstorm : public GameObject, public ScriptInterface
{
public:
    Thunderstorm(const lisp::Lisp& reader);

    void update(float elapsed_time);
    void draw(DrawingContext& context);

    virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
    virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

    /**
     * @name Scriptable Methods
     * @{
     */

    /**
     * Start playing thunder and lightning at configured interval
     */
    void start();

    /**
     * Stop playing thunder and lightning at configured interval
     */
    void stop();

    /**
     * Play thunder
     */
    void thunder();

    /**
     * Play lightning, i.e. call flash() and electrify()
     */
    void lightning();

    /**
     * Display a nice flash
     */
    void flash();

    /**
     * Electrify water throughout the whole sector for a short time
     */
    void electrify();

    /**
     * @}
     */

private:
    bool running; /**< whether we currently automatically trigger lightnings */
    float interval; /**< time between two lightnings */
    int layer; /**< layer, where flash will be painted */

    Timer time_to_thunder; /**< counts down until next thunder */
    Timer time_to_lightning; /**< counts down until next lightning */
    Timer flash_display_timer; /**< counts down while flash is displayed */
};

#endif
