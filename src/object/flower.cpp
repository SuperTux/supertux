#include <config.h>

#include <math.h>
#include "flower.h"
#include "resources.h"
#include "camera.h"
#include "sector.h"
#include "player.h"
#include "app/globals.h"
#include "special/sprite_manager.h"

Flower::Flower(const Vector& pos, Type _type)
  : type(_type)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);

  if(_type == FIREFLOWER)
    sprite = sprite_manager->create("fireflower");
  else
    sprite = sprite_manager->create("iceflower"); 
}

Flower::~Flower()
{
  delete sprite;
}

void
Flower::action(float )
{
}

void
Flower::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

HitResponse
Flower::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*>(&other);
  if(!player)
    return ABORT_MOVE;

  if(type == FIREFLOWER)
    player->set_bonus(FIRE_BONUS, true);
  else
    player->set_bonus(ICE_BONUS, true);
  
  SoundManager::get()->play_sound(IDToSound(SND_COFFEE));
  remove_me();
  return ABORT_MOVE;
}

