#ifndef __INBUMPABLE_BLOCK_H__
#define __INBUMPABLE_BLOCK_H__

#include "block.h"

class InvisibleTile : public Block
{
public:
  InvisibleTile(const Vector& pos);

  virtual void draw(DrawingContext& context);

protected:
  virtual void hit(Player& player);

private:
  bool visible;
};

#endif

