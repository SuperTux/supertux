#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "special/moving_object.h"
#include "lisp/lisp.h"

namespace SuperTux {
  class Sprite;
}
class Player;

using namespace SuperTux;

class Block : public MovingObject
{
public:
  Block(Sprite* sprite = 0);
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
  BonusBlock(const lisp::Lisp& lisp);

  void try_open();

protected:
  virtual void hit(Player& player);

private:
  enum Contents {
    CONTENT_COIN,
    CONTENT_FIREGROW,
    CONTENT_ICEGROW,
    CONTENT_STAR,
    CONTENT_1UP
  };

  Contents contents;
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

