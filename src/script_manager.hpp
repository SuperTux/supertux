//  $Id$
//
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#ifndef __SCRIPT_MANAGER_H__
#define __SCRIPT_MANAGER_H__

#include <vector>
#include <list>
#include <squirrel.h>
#include <iostream>
#include "timer.hpp"

class GameObject;

/**
 * This class is responsible for managing all running squirrel threads
 * (they are cooperative threads or coroutines)
 * It keeps a list of suspended scripts and receives wakeup events for them
 */
class ScriptManager
{
public:
  ScriptManager();
  ScriptManager(ScriptManager* parent);
  ~ScriptManager();

  void update();

  /**
   * Creates a new thread and registers it with the script manager
   * (so it can suspend and register for wakeup events)
   */
  HSQUIRRELVM create_thread(bool leave_thread_on_stack = false);

  HSQUIRRELVM get_vm() const
  {
    return vm;
  }

  enum WakeupEvent {
    NO_EVENT,
    TIME,
    SCREEN_SWITCHED,
    WAKEUP_EVENT_COUNT
  };

  struct WakeupData {
    explicit WakeupData() : type(NO_EVENT) {}
    explicit WakeupData(WakeupEvent type_) : type(type_) {}

    WakeupEvent type;
    
    union {
      // GAMEOBJECT_DONE
      GameObject* game_object;
    };
  };

  void set_wakeup_event(HSQUIRRELVM vm, WakeupEvent event, float timeout = -1);
  void set_wakeup_event(HSQUIRRELVM vm, WakeupData  event, float timeout = -1);
  void fire_wakeup_event(WakeupEvent event);
  void fire_wakeup_event(WakeupData  event);

  // global (root) instance of the ScriptManager
  static ScriptManager* instance;
  
private:
  class SquirrelVM
  {
  public:
    SquirrelVM(HSQUIRRELVM arg_vm, HSQOBJECT arg_obj);

    HSQUIRRELVM vm;
    HSQOBJECT   vm_obj;
    float       wakeup_time;
    WakeupData  waiting_for_events;
  };
  
  typedef std::list<SquirrelVM> SquirrelVMs;
  SquirrelVMs squirrel_vms;

  HSQUIRRELVM vm;
  ScriptManager* parent;
  std::vector<ScriptManager*> childs;
};

#endif

