#ifndef __SCRIPTED_OBJECT_H__
#define __SCRIPTED_OBJECT_H__

#include <string>
#include "physic.h"
#include "sprite/sprite.h"
#include "lisp/lisp.h"
#include "moving_object.h"
#include "scripting/scripted_object.h"

class ScriptedObject : public MovingObject, public Scripting::ScriptedObject
{
public:
  ScriptedObject(const lisp::Lisp& lisp);
  virtual ~ScriptedObject();

  void action(float elapsed_time);
  void draw(DrawingContext& context);
  HitResponse collision(GameObject& other, const CollisionHit& hit);

  // --- Scripting Interface stuff ---

  void set_animation(const std::string& animation);
  std::string get_animation();

  void move(float x, float y);
  void set_pos(float x, float y);
  float get_pos_x();
  float get_pos_y();
  void set_velocity(float x, float y);
  float get_velocity_x();
  float get_velocity_y();
  void set_visible(bool visible);
  bool is_visible();

  std::string get_name();

private:
  std::string name;
  bool solid;
  bool physic_enabled;
  bool visible;
  bool new_vel_set;
  Vector new_vel;
  Physic physic;
  Sprite* sprite;
};

#endif

