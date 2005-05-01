//  $Id: game_object.h 2293 2005-03-25 20:39:56Z matzebraun $
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
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
#ifndef SUPERTUX_GAMEOBJECT_H
#define SUPERTUX_GAMEOBJECT_H

#include <string>

class DrawingContext;
class ObjectRemoveListener;

/**
 * Base class for all game objects. This contains functions for:
 *  -querying the actual type of the object
 *  -a flag that indicates if the object wants to be removed. Objects with this
 *   flag will be removed at the end of each frame. This is alot safer than
 *   having some uncontrollable "delete this" in the code.
 *  -an action function that is called once per frame and allows the object to
 *   update it's state.
 * 
 * Most GameObjects will also implement the DrawableObject interface so that
 * they can actually be drawn on screen.
 */
class GameObject
{
public:
  GameObject();
  virtual ~GameObject();

  /** This function is called once per frame and allows the object to update
   * it's state. The elapsed_time is the time since the last frame in
   * seconds and should be the base for all timed calculations (don't use
   * SDL_GetTicks directly as this will fail in pause mode)
   */
  virtual void action(float elapsed_time) = 0;

  /** The GameObject should draw itself onto the provided DrawingContext if this
   * function is called.
   */
  virtual void draw(DrawingContext& context) = 0;

  /** returns true if the object is not scheduled to be removed yet */
  bool is_valid() const
    {
      return !wants_to_die;
    }
  /** schedules this object to be removed at the end of the frame */
  void remove_me()
  {
    wants_to_die = true;
  }
  /** registers a remove listener which will be called if the object 
   * gets removed/destroyed
   */
  void add_remove_listener(ObjectRemoveListener* listener)
  {
    RemoveListenerListEntry* entry = new RemoveListenerListEntry();
    entry->next = remove_listeners;
    entry->listener = listener;

    remove_listeners = entry;
  }
  
  // flags
  enum {
    /// the tile so you can stand on it
    FLAG_SOLID       = 0x0001,
    /// can be used to temporatily disable collision detection
    FLAG_NO_COLLDET  = 0x0002
  };                     

  int get_flags() const
  {
    return flags;            
  }

private:
  /** this flag indicates if the object should be removed at the end of the
   * frame
   */
  bool wants_to_die;

  struct RemoveListenerListEntry
  {
    RemoveListenerListEntry* next;
    ObjectRemoveListener* listener;
  };
  RemoveListenerListEntry* remove_listeners;

protected:
  int flags;
};

#endif /*SUPERTUX_GAMEOBJECT_H*/

