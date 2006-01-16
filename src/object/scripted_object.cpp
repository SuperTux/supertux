#include <config.h>

#include <stdexcept>

#include "scripted_object.hpp"
#include "video/drawing_context.hpp"
#include "sprite/sprite_manager.hpp"
#include "resources.hpp"
#include "object_factory.hpp"
#include "math/vector.hpp"

ScriptedObject::ScriptedObject(const lisp::Lisp& lisp)
  : solid(true), physic_enabled(true), visible(true), new_vel_set(false),
    layer(LAYER_OBJECTS)
{
  lisp.get("name", name);
  if(name == "")
    throw std::runtime_error("Scripted object must have a name specified");
  
  std::string spritename;
  lisp.get("sprite", spritename);
  if(spritename == "")
    throw std::runtime_error("Scripted object must have a sprite name specified");
  sprite = sprite_manager->create(spritename);

  lisp.get("x", bbox.p1.x);
  lisp.get("y", bbox.p1.y);
  float width = sprite->get_width();
  float height = sprite->get_height();
  lisp.get("width", width);
  lisp.get("height", height);
  bbox.set_size(width, height);

  lisp.get("solid", solid);
  lisp.get("physic-enabled", physic_enabled);
  lisp.get("visible", visible);
  lisp.get("layer", layer);
  if(solid)
    flags |= FLAG_SOLID;
}

ScriptedObject::~ScriptedObject()
{
  delete sprite;
}

void
ScriptedObject::move(float x, float y)
{
  bbox.move(Vector(x, y));
}

void
ScriptedObject::set_pos(float x, float y)
{
  bbox.set_pos(Vector(x, y));
}

float
ScriptedObject::get_pos_x()
{
  return get_pos().x;
}

float
ScriptedObject::get_pos_y()
{
  return get_pos().y;
}

void
ScriptedObject::set_velocity(float x, float y)
{
  new_vel = Vector(x, y);
  new_vel_set = true;
}

float
ScriptedObject::get_velocity_x()
{
  return physic.get_velocity_x();
}

float
ScriptedObject::get_velocity_y()
{
  return physic.get_velocity_y();
}

void
ScriptedObject::set_visible(bool visible)
{
  this->visible = visible;
}

bool
ScriptedObject::is_visible()
{
  return visible;
}

void
ScriptedObject::set_animation(const std::string& animation)
{
  sprite->set_action(animation);
}

std::string
ScriptedObject::get_animation()
{
  return sprite->get_action_name();
}

std::string
ScriptedObject::get_name()
{
  return name;
}

void
ScriptedObject::update(float elapsed_time)
{
  if(!physic_enabled)
    return;

  if(new_vel_set) {
    physic.set_velocity(new_vel.x, new_vel.y);
    new_vel_set = false;
  }
  movement = physic.get_movement(elapsed_time);
}

void
ScriptedObject::draw(DrawingContext& context)
{
  if(!visible)
    return;

  sprite->draw(context, get_pos(), layer);
}

HitResponse
ScriptedObject::collision(GameObject& other, const CollisionHit& )
{
  if(!physic_enabled)
    return FORCE_MOVE;

  if(!(other.get_flags() & FLAG_SOLID))
    return FORCE_MOVE;

  physic.set_velocity(0, 0);
  return CONTINUE;
}

IMPLEMENT_FACTORY(ScriptedObject, "scriptedobject");
