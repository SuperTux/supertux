#ifndef __INVISIBLE_BLOCK_H__
#define __INVISIBLE_BLOCK_H__

#include "block.h"

class InvisibleBlock : public Block
{
public:
  InvisibleBlock(const Vector& pos);

  virtual void draw(DrawingContext& context);

protected:
  virtual void hit(Player& player);

private:
  bool visible;
};

#endif

