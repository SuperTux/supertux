#include <config.h>

#include <string>
#include <stdio.h>
#include "scripted_object.h"

#define NOIMPL      printf("%s not implemented.\n", __PRETTY_FUNCTION__)

namespace Scripting
{
  ScriptedObject::ScriptedObject()
  {}

  ScriptedObject::~ScriptedObject()
  {}

  void
  ScriptedObject::set_animation(const std::string& )
  {
    NOIMPL;
  }

  std::string
  ScriptedObject::get_animation()
  {
    NOIMPL;
    return "";
  }

  void
  ScriptedObject::move(float , float )
  {
    NOIMPL;
  }

  void
  ScriptedObject::set_pos(float , float )
  {
    NOIMPL;
  }

  float
  ScriptedObject::get_pos_x()
  {
    NOIMPL;
    return -1;
  }

  float
  ScriptedObject::get_pos_y()
  {
    NOIMPL;
    return -1;
  }

  void
  ScriptedObject::set_velocity(float , float )
  {
    NOIMPL;
  }

  float
  ScriptedObject::get_velocity_x()
  {
    NOIMPL;
    return -1;
  }

  float
  ScriptedObject::get_velocity_y()
  {
    NOIMPL;
    return -1;
  }
}
