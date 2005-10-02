#include <config.h>

#include "light.hpp"
#include "sprite/sprite_manager.hpp"
#include "resources.hpp"
#include "video/drawing_context.hpp"
#include "object_factory.hpp"
#include "player.hpp"
#include "sector.hpp"

Light::Light(const lisp::Lisp& )
{
  sprite = sprite_manager->create("lightmap_light");
}

Light::~Light()
{
  delete sprite;
}

void
Light::update(float )
{
}

void
Light::draw(DrawingContext& context)
{
  context.push_target();
  context.set_target(DrawingContext::LIGHTMAP);
  
  sprite->draw(context, Sector::current()->player->get_pos(), 0);
  
  context.pop_target();
}

IMPLEMENT_FACTORY(Light, "light");
