 
/***************************************************************************
                          gameobjs_bridge.cpp  -  description
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

#include "gameobjs_bridge.h"

#include "object/coin.h"
#include "object/block.h"
#include "object/invisible_block.h"
#include "object/platform.h"
#include "object/bullet.h"
#include "object/rock.h"
#include "badguy/jumpy.h"
#include "badguy/snowball.h"
#include "badguy/bouncing_snowball.h"
#include "badguy/flame.h"
#include "badguy/flyingsnowball.h"
#include "badguy/mriceblock.h"
#include "badguy/mrbomb.h"
#include "badguy/dispenser.h"
#include "badguy/spike.h"
#include "badguy/spiky.h"
#include "badguy/nolok_01.h"
#include "trigger/door.h"
#include "trigger/sequence_trigger.h"
#include "trigger/secretarea_trigger.h"

std::string object_type_to_string(int kind)
{
switch(kind)
  {
  // Game Objects
  case COIN_OBJECT:
    return "coin";
  case INVISIBLE_BLOCK_OBJECT:
    return "invisible-block";
  case PLATFORM_OBJECT:
    return "platform";
  case ROCK_OBJECT:
    return "rock";
  // Bad Guys
  case JUMPY_BADGUY:
    return "jumpy";
  case SNOWBALL_BADGUY:
    return "snowball";
  case BOUNCING_SNOWBALL_BADGUY:
    return "bouncingsnowball";
  case FLAME_BADGUY:
    return "flame";
  case FLYING_SNOWBALL_BADGUY:
    return "flyingsnowball";
  case MRICEBLOCK_BADGUY:
    return "mriceblock";
  case MRBOMB_BADGUY:
    return "mrbomb";
  case DISPENSER_BADGUY:
    return "dispenser";
  case SPIKE_BADGUY:
    return "spike";
  case SPIKY_BADGUY:
    return "spiky";
  case NOLOK_01_BADGUY:
    return "nolok_01";
  // Triggers
  case DOOR_TRIGGER:
    return "door";
  default:
    std::cerr << "Warning: object not recognized.\n";
    return "";  // should not happen
  }
return "";  // avoids warnings
}

GameObjectsType object_name_to_type(const std::string& name)
{
// Game Objects
if(name == "coin")
  return COIN_OBJECT;
else if(name == "invisible-block")
  return INVISIBLE_BLOCK_OBJECT;
else if(name == "platform")
  return PLATFORM_OBJECT;
else if(name == "rock")
  return ROCK_OBJECT;
// Bad Guys
else if(name == "jumpy")
  return JUMPY_BADGUY;
else if(name == "snowball")
  return SNOWBALL_BADGUY;
else if(name == "bouncingsnowball")
  return BOUNCING_SNOWBALL_BADGUY;
else if(name == "flame")
  return FLAME_BADGUY;
else if(name == "flyingsnowball")
  return FLYING_SNOWBALL_BADGUY;
else if(name == "mriceblock")
  return MRICEBLOCK_BADGUY;
else if(name == "mrbomb")
  return MRBOMB_BADGUY;
else if(name == "dispenser")
  return DISPENSER_BADGUY;
else if(name == "spike")
  return SPIKE_BADGUY;
else if(name == "spiky")
  return SPIKY_BADGUY;
else if(name == "nolok_01")
  return NOLOK_01_BADGUY;
// Triggers
else if(name == "door")
  return DOOR_TRIGGER;

// Object not recognized
return NOT_RECOGNIZED_GAMEOBJECT;
}

bool is_object(const std::string& name)
{
if(object_name_to_type(name) == NOT_RECOGNIZED_GAMEOBJECT)
  return false;
return true;
}

GameObject* create_object(GameObjectsType kind, const lisp::Lisp& reader)
{
switch(kind)
  {
  // Game Objects
  case COIN_OBJECT:
  case INVISIBLE_BLOCK_OBJECT:
    std::cerr << "Error: This object doesn't yet supported a Lisp argument.\n";
    return NULL;
//    return new Coin(reader);
//    return new InvisibleBlock(reader);
  case PLATFORM_OBJECT:
    return new Platform(reader);
  case ROCK_OBJECT:
    return new Rock(reader);
  // Bad Guys
  case JUMPY_BADGUY:
    return new Jumpy(reader);
  case SNOWBALL_BADGUY:
    return new SnowBall(reader);
  case BOUNCING_SNOWBALL_BADGUY:
    return new BouncingSnowball(reader);
  case FLAME_BADGUY:
    return new Flame(reader);
  case FLYING_SNOWBALL_BADGUY:
    return new FlyingSnowBall(reader);
  case MRICEBLOCK_BADGUY:
    return new MrIceBlock(reader);
  case MRBOMB_BADGUY:
    return new MrBomb(reader);
  case DISPENSER_BADGUY:
    return new Dispenser(reader);
  case SPIKE_BADGUY:
    return new Spike(reader);
  case SPIKY_BADGUY:
    return new Spiky(reader);
  case NOLOK_01_BADGUY:
    return new Nolok_01(reader);
  // Triggers
  case DOOR_TRIGGER:
    return new Door(reader);
  default:
    std::cerr << "Warning: object not recognized.\n";
    return NULL;  // object not recognized
  }
return NULL;  // avoids warnings
}

GameObject* create_object(GameObjectsType kind, const Vector& pos)
{
switch(kind)
  {
  // Game Objects
  case COIN_OBJECT:
    return new Coin(pos);
  case INVISIBLE_BLOCK_OBJECT:
    return new InvisibleBlock(pos);
  case PLATFORM_OBJECT:
  case ROCK_OBJECT:
  case JUMPY_BADGUY:
  case FLAME_BADGUY:
  case DISPENSER_BADGUY:
  case SPIKY_BADGUY:
    std::cerr << "Error: This object doesn't yet support a position argument.\n";
    return NULL;
//    return new Platform(pos);
//    return new Rock(pos);
  // Bad Guys
//    return new Jumpy(pos.x,pos.y,LEFT);
//    return new Flame(pos.x,pos.y,LEFT);
//    return new Dispenser(pos.x,pos.y,LEFT);
//    return new Spiky(pos.x,pos.y,LEFT);
  case SNOWBALL_BADGUY:
    return new SnowBall(pos.x,pos.y,LEFT);
  case BOUNCING_SNOWBALL_BADGUY:
    return new BouncingSnowball(pos.x,pos.y,LEFT);
  case FLYING_SNOWBALL_BADGUY:
    return new FlyingSnowBall(pos.x,pos.y);
  case MRICEBLOCK_BADGUY:
    return new MrIceBlock(pos.x,pos.y,LEFT);
  case MRBOMB_BADGUY:
    return new MrBomb(pos.x,pos.y,LEFT);
  case SPIKE_BADGUY:
    return new Spike(pos,Spike::WEST);
  case NOLOK_01_BADGUY:
    return new Nolok_01(pos.x,pos.y);
  // Triggers
  case DOOR_TRIGGER:
    return new Door((int)pos.x,(int)pos.y,"","");
  default:
    std::cerr << "Warning: object not recognized.\n";
    return NULL;  // object not recognized
  }
return NULL;  // avoids warnings
}
