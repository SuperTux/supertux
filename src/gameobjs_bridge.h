/***************************************************************************
                          gameobjs_bridge.h  -  description
                             -------------------
    begin                : Dec, 19 2004
    copyright            : (C) 2004 by Ricardo Cruz
    email                : rick2@aeiou.pt
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GAMEOBJS_BRIDGE_H
#define GAMEOBJS_BRIDGE_H

#include <string>

//#include "lisp/parser.h"
#include "lisp/lisp.h"
//#include "lisp/writer.h"
//#include "lisp/list_iterator.h"

//class GameObject;
#include "object/gameobjs.h"


/** This is just a set of functions aim to provide a common way
    to create new game objects. Game objects that ain't really
    interactive don't fit in here. */

enum GameObjectsType {
  // Game Objects
  COIN_OBJECT,
  INVISIBLE_BLOCK_OBJECT,
  PLATFORM_OBJECT,
  ROCK_OBJECT,
  // Bad Guys
  JUMPY_BADGUY,
  SNOWBALL_BADGUY,
  BOUNCING_SNOWBALL_BADGUY,
  FLAME_BADGUY,
  FLYING_SNOWBALL_BADGUY,
  MRICEBLOCK_BADGUY,
  MRBOMB_BADGUY,
  DISPENSER_BADGUY,
  SPIKE_BADGUY,
  SPIKY_BADGUY,
  NOLOK_01_BADGUY,
  // Triggers
  DOOR_TRIGGER,

  TOTAL_GAMEOBJECTS,
  NOT_RECOGNIZED_GAMEOBJECT
  };

/* Objects identifiers. */
std::string object_type_to_string(int kind);
GameObjectsType object_name_to_type(const std::string& name);

bool is_object(const std::string& name);

/* Calls to return a pointer to a created GameObjects.
   Don't forget to delete them! */
GameObject* create_object(GameObjectsType kind, const lisp::Lisp& reader);
GameObject* create_object(GameObjectsType kind, const Vector& pos);

#endif /*GAMEOBJS_BRIDGE_H*/
