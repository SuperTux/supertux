#ifndef __SERIALIZABLE_H__
#define __SERIALIZABLE_H__

class LispWriter;

class Serializable
{
public:
  virtual void write(LispWriter& writer) = 0;
};

#endif

