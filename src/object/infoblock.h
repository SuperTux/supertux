#ifndef __INFOBLOCK_H__
#define __INFOBLOCK_H__

#include "block.h"

class InfoBlock : public Block
{
public:
  InfoBlock(const lisp::Lisp& lisp);
  virtual ~InfoBlock();
  
protected:
  virtual void hit(Player& player);
  std::string message;
};

#endif

