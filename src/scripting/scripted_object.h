#ifndef __SCRIPTED_OBJECT_INTERFACE_H__
#define __SCRIPTED_OBJECT_INTERFACE_H__

namespace Scripting
{

class ScriptedObject
{
public:
#ifndef SCRIPTING_API
  ScriptedObject();
  virtual ~ScriptedObject();
#endif

  virtual void set_animation(const std::string& animation) = 0;
  virtual std::string get_animation() = 0;

  virtual void move(float x, float y) = 0;
  virtual void set_pos(float x, float y) = 0;
  virtual float get_pos_x() = 0;
  virtual float get_pos_y() = 0;
  
  virtual void set_velocity(float x, float y) = 0;
  virtual float get_velocity_x() = 0;
  virtual float get_velocity_y() = 0;
  
  virtual void set_visible(bool visible) = 0;
  virtual bool is_visible() = 0;

  virtual std::string get_name() = 0;
};

}

#endif

