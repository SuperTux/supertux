#ifndef __BOX_H__
#define __BOX_H__

#include "special/moving_object.h"

namespace SuperTux {
  class Sprite;
}
class Player;

using namespace SuperTux;

class Block : public MovingObject
{
public:
  Block(const Vector& pos, Sprite* sprite);
  ~Block();

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

protected:
  virtual void hit(Player& player) = 0;
  void start_bounce();

  Sprite* sprite;
  bool bouncing;
  float bounce_dir;
  float bounce_offset;
  float original_y;
};

class BonusBlock : public Block
{
public:
  BonusBlock(const Vector& pos, int data);

  void try_open();

protected:
  virtual void hit(Player& player);

private:
  int data;
};

class Brick : public Block
{
public:
  Brick(const Vector& pos, int data);

  void try_break(bool playerhit = false);

protected:
  virtual void hit(Player& player);

private:
  bool breakable;
  int coin_counter;
};

#endif

